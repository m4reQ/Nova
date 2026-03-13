#include "MainLayer.hpp"
#include <Nova/graphics/Window.hpp>
#include <Nova/graphics/Renderer.hpp>
#include <Nova/input/Input.hpp>
#include <Nova/core/Application.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/assets/Assets.hpp>
#include <Nova/assets/Image.hpp>
#include <Nova/assets/Model.hpp>
#include <Nova/ecs/components/NameComponent.hpp>
#include <Nova/ecs/components/TransformComponent.hpp>
#include <Nova/ecs/components/LightComponent.hpp>
#include <Nova/ecs/components/CameraComponent.hpp>
#include <Nova/ecs/components/RenderComponent.hpp>
#include <Nova/ecs/components/ScriptComponent.hpp>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <format>
#include <fstream>
#include <ranges>
#include <random>
#include <limits>
#include <iostream>
#include "CameraController.hpp"

constexpr entt::entity InvalidEntity = (entt::entity)-1;

struct HeartData
{
    glm::mat4 Transform;
    Nova::Material Material;
};

static std::vector<HeartData> hearts_;

template <typename T>
static T Random(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) noexcept
{
    return (T)(((float)std::rand() / (float)RAND_MAX) * (max - min) + min);
}

static float RandomFloat(float min = 0.0f, float max = 1.0f) noexcept
{
    return Random(min, max);
}

static glm::vec4 RandomColor(bool randomizeAlpha = false) noexcept
{
    return glm::vec4(
        RandomFloat(0.5f),
        RandomFloat(0.5f),
        RandomFloat(0.5f),
        RandomFloat() > 0.5f ? RandomFloat(0.3f, 0.6f) : 1.0f);
}

template <typename T>
static T &SelectRandomElement(std::span<T> container) noexcept
{
    return container[Random(0zu, container.size() - 1)];
}

static glm::mat4 BuildTransformMatrix(const glm::vec3 &position, const glm::quat &rotation, const glm::vec3 scale) noexcept
{
    const auto rotationMatrix = glm::mat3_cast(rotation);

    glm::mat4 result;
    result[0] = glm::vec4(rotationMatrix[0] * scale.x, 0.0f);
    result[1] = glm::vec4(rotationMatrix[1] * scale.x, 0.0f);
    result[2] = glm::vec4(rotationMatrix[2] * scale.x, 0.0f);
    result[3] = glm::vec4(position, 1.0f);

    return result;
}

template <typename TComponent>
static void TryAddEntityComponentTreeNode(const entt::registry &registry, entt::entity entity, const std::string_view componentName, bool isSelected)
{
    if (registry.try_get<TComponent>(entity))
        ImGui::TreeNodeEx(
            std::format("{}##{}", componentName, (uint32_t)entity).c_str(),
            ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | (isSelected ? ImGuiTreeNodeFlags_Selected : 0));
}

static void RenderScene(const entt::registry &scene, entt::entity cameraEntity)
{
    // set main scene camera
    auto view = glm::identity<glm::mat4>();
    auto projection = glm::identity<glm::mat4>();
    auto cameraPosition = glm::vec3(0.0f);
    auto cameraZNear = -1.0f;
    auto cameraZFar = 1.0f;

    const auto camera = scene.try_get<Nova::CameraComponent>(cameraEntity);
    if (camera)
    {
        cameraPosition = camera->Position;

        view = glm::lookAt(
            camera->Position,
            camera->Position + camera->Front,
            camera->Up);

        if (camera->Type == Nova::CameraType::Perspective)
        {
            cameraZNear = camera->Data.Perspective.ZNear;
            cameraZFar = camera->Data.Perspective.ZFar;
            projection = glm::perspective(
                camera->Data.Perspective.FOV,
                camera->Data.Perspective.AspectRatio,
                camera->Data.Perspective.ZNear,
                camera->Data.Perspective.ZFar);
        }
        else
        {
            projection = glm::ortho(
                camera->Data.Ortho.Left,
                camera->Data.Ortho.Right,
                camera->Data.Ortho.Bottom,
                camera->Data.Ortho.Top);
        }
    }

    Nova::Renderer::SetCamera(
        view,
        projection,
        cameraPosition,
        cameraZNear,
        cameraZFar);

    // set directional lights
    scene.view<Nova::DirectionalLightComponent>().each(
        [](auto entity, const auto &lightComponent)
        {
            Nova::Renderer::AddDirectionalLight(
                glm::vec4(lightComponent.Color, lightComponent.Intensity),
                lightComponent.Direction);
        });

    // set point lights
    scene.view<Nova::TransformComponent, Nova::PointLightComponent>().each(
        [](auto entity, const auto &transform, const auto &light)
        {
            Nova::Renderer::AddPointLight(
                glm::vec4(light.Color, light.Intensity),
                transform.Position,
                light.Radius);
        });

    // render objects
    {
        NV_PROFILE_SCOPE("::RenderObjects");
        scene.view<Nova::TransformComponent, RenderComponent>().each(
            [](auto entity, const auto &transform, const auto &render)
            {
                Nova::Renderer::Render(
                    render.Model,
                    *render.Material,
                    BuildTransformMatrix(
                        transform.Position,
                        transform.Rotation,
                        transform.Scale));
            });
    }

    // draw
    // Nova::Renderer::Draw(glm::vec4(0.529f, 0.529f, 0.529f, 1.0f));
}

static void AddDirectionalLightComponentInspector(entt::registry &scene, entt::entity entity) noexcept
{
    auto component = scene.try_get<Nova::DirectionalLightComponent>(entity);
    if (component == nullptr)
        return;

    ImGui::SeparatorText("Directional light");
    ImGui::ColorEdit3("Color", glm::value_ptr(component->Color));
    ImGui::DragFloat("Intensity", &component->Intensity, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat3("Direction", glm::value_ptr(component->Direction), 0.01f, -1.0f, 1.0f);
}

static void AddCameraComponentInspector(entt::registry &scene, entt::entity entity) noexcept
{
    auto component = scene.try_get<Nova::CameraComponent>(entity);
    if (component == nullptr)
        return;

    ImGui::SeparatorText("Camera");

    const std::array<std::pair<Nova::CameraType, const char *>, 2> cameraTypes{
        std::make_pair(Nova::CameraType::Orthographic, "orthographic"),
        std::make_pair(Nova::CameraType::Perspective, "perspective"),
    };

    const auto currentCameraTypeIdx = component->Type == Nova::CameraType::Orthographic ? 0 : 1;
    const auto &currentCameraTypeData = cameraTypes[currentCameraTypeIdx];
    if (ImGui::BeginCombo(std::format("Camera type##{}", (int)entity).c_str(), currentCameraTypeData.second))
    {
        for (int i = 0; i < cameraTypes.size(); i++)
        {
            const auto &cameraTypeData = cameraTypes[i];
            auto isSelected = component->Type == cameraTypeData.first;
            if (ImGui::Selectable(std::format("{}##CameraType{}", cameraTypeData.second, (int)entity).c_str(), &isSelected))
                component->Type = cameraTypeData.first;
        }

        ImGui::EndCombo();
    }

    if (component->Type == Nova::CameraType::Perspective)
    {
        auto cameraFOV = glm::degrees(component->Data.Perspective.FOV);
        ImGui::DragFloat("FOV", &cameraFOV, 1.0f, 0.1f, 179.9f);

        component->Data.Perspective.FOV = glm::radians(cameraFOV);

        ImGui::DragFloat("Aspect ratio", &component->Data.Perspective.AspectRatio, 0.01f, 0.1f);
        ImGui::DragFloat("Z-Near", &component->Data.Perspective.ZNear, 0.01f, 0.0001f);
        ImGui::DragFloat("Z-Far", &component->Data.Perspective.ZFar, 0.01f, component->Data.Perspective.ZNear);
    }
    else
    {
        ImGui::DragFloat("Left", &component->Data.Ortho.Left, 0.01f);
        ImGui::DragFloat("Right", &component->Data.Ortho.Right, 0.01f);
        ImGui::DragFloat("Bottom", &component->Data.Ortho.Bottom, 0.01f);
        ImGui::DragFloat("Top", &component->Data.Ortho.Top, 0.01f);
    }
}

static void AddPointLightComponentInspector(entt::registry &scene, entt::entity entity) noexcept
{
    auto component = scene.try_get<Nova::PointLightComponent>(entity);
    if (!component)
        return;

    ImGui::SeparatorText("Point light");

    ImGui::ColorEdit3("Color", glm::value_ptr(component->Color));
    ImGui::DragFloat("Intensity", &component->Intensity, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Radius", &component->Radius, 0.01f, 0.0f);
}

static void AddTransformComponentInspector(entt::registry &scene, entt::entity entity) noexcept
{
    auto component = scene.try_get<Nova::TransformComponent>(entity);
    if (!component)
        return;

    ImGui::SeparatorText("Transform");

    ImGui::DragFloat3("Position", glm::value_ptr(component->Position), 0.01f);
    ImGui::DragFloat3("Scale", glm::value_ptr(component->Scale), 0.01f);

    auto rotationDegrees = glm::degrees(component->Rotation);
    ImGui::DragFloat3("Rotation", glm::value_ptr(rotationDegrees), 0.01f, 0.0f, 360.0f);

    component->Rotation = glm::radians(rotationDegrees);
}

MainLayer::MainLayer()
    : Nova::Layer("MainLayer")
{
    if (!ImGui::CreateContext())
        throw std::runtime_error("Failed to initialize ImGui.");

    auto &io = ImGui::GetIO();
    io.DisplaySize = ImVec2{(float)Nova::Window::GetWidth(), (float)Nova::Window::GetHeight()};
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    if (!ImGui_ImplGlfw_InitForOpenGL(Nova::Window::GetNativeHandle(), true))
        throw std::runtime_error("Failed to initialize ImGui GLFW backend.");

    if (!ImGui_ImplOpenGL3_Init())
        throw std::runtime_error("Failed to initialize ImGui OpenGL backend.");

    std::srand(std::time(nullptr));

    std::vector<Nova::Material> randomMaterials_;
    randomMaterials_.reserve(8);

    for (size_t i = 0; i < 8; i++)
        randomMaterials_.emplace_back(
            Nova::Material{
                .Color = RandomColor(),
                .SpecularIntensity = 1.0f,
            });

    hearts_.reserve(5 * 5);

    for (size_t y = 0; y < 5; y++)
    {
        for (size_t x = 0; x < 5; x++)
        {
            hearts_.emplace_back(
                HeartData{
                    .Transform = glm::scale(
                        glm::translate(
                            glm::identity<glm::mat4>(),
                            glm::vec3(0.2f * x, 0.0f, -1.0f - 0.2f * y)),
                        glm::vec3(0.04f, 0.04f, 0.04f)),
                    .Material = SelectRandomElement(
                        std::span(
                            randomMaterials_.data(),
                            randomMaterials_.size())),
                });
        }
    }

    model_ = Nova::Assets::LoadAssetFromFile<Nova::Model>("./assets/heart.obj", "HeartModel");
    image_ = Nova::Assets::LoadAssetFromFile<Nova::Image>("./assets/texture1.png", "TexturePNG");
    modelIconImage_ = Nova::Assets::LoadAssetFromFile<Nova::Image>("./assets/modelIcon.png", std::nullopt, Nova::AssetFlags::Internal);

    // initialize main camera
    mainCameraEntity_ = entities_.create();
    entities_.emplace<NameComponent>(mainCameraEntity_, "MainCamera");
    entities_.emplace<Nova::CameraComponent>(
        mainCameraEntity_,
        Nova::CameraComponent::CreatePerspective(
            glm::radians(45.0f),
            Nova::Window::GetAspectRatio(),
            0.1f,
            3.0f));
    entities_.emplace<Nova::TransformComponent>(
        mainCameraEntity_,
        Nova::TransformComponent{
            .Position = {0.0f, 0.0f, 0.0f}});
    entities_.emplace<Nova::PointLightComponent>(
        mainCameraEntity_,
        Nova::PointLightComponent{
            .Color = {1.0f, 1.0f, 0.6f},
            .Intensity = 0.9f,
            .Radius = 0.7f});
    auto &scriptComponent = entities_.emplace<CPPScriptComponent>(mainCameraEntity_, CPPScriptComponent::Create<CameraController>());
    scriptComponent.ControllerInstance->OnAttach(entities_, mainCameraEntity_);

    // initialize lights
    auto light1 = entities_.create();
    entities_.emplace<NameComponent>(light1, "Point light 1");
    entities_.emplace<Nova::PointLightComponent>(
        light1,
        Nova::PointLightComponent{
            .Color = {1.0f, 0.09f, 0.985f},
            .Intensity = 0.4f,
            .Radius = 0.2f,
        });
    entities_.emplace<Nova::TransformComponent>(
        light1,
        Nova::TransformComponent{
            .Position = {0.5f, 0.0f, -1.3f},
        });

    auto light2 = entities_.create();
    entities_.emplace<NameComponent>(light2, "Point light 2");
    entities_.emplace<Nova::PointLightComponent>(
        light2,
        Nova::PointLightComponent{
            .Color = {1.0f, 0.5f, 0.03f},
            .Intensity = 0.4f,
            .Radius = 0.2f,
        });
    entities_.emplace<Nova::TransformComponent>(
        light2,
        Nova::TransformComponent{
            .Position = {0.5f, 0.0f, -1.6f},
        });

    auto light3 = entities_.create();
    entities_.emplace<NameComponent>(light3, "Point light 3");
    entities_.emplace<Nova::PointLightComponent>(
        light3,
        Nova::PointLightComponent{
            .Color = {1.0f, 0.5f, 0.8f},
            .Intensity = 0.4f,
            .Radius = 0.2f,
        });
    entities_.emplace<Nova::TransformComponent>(
        light3,
        Nova::TransformComponent{
            .Position = {-0.2f, 0.0f, -1.3f},
        });

    auto light4 = entities_.create();
    entities_.emplace<NameComponent>(light4, "Point light 4");
    entities_.emplace<Nova::PointLightComponent>(
        light4,
        Nova::PointLightComponent{
            .Color = {1.0f, 1.0f, 1.0f},
            .Intensity = 0.5f,
            .Radius = 0.6f,
        });
    entities_.emplace<Nova::TransformComponent>(
        light4,
        Nova::TransformComponent{
            .Position = {0.0f, 1.5f, 0.0f},
        });

    auto dirLight = entities_.create();
    entities_.emplace<NameComponent>(dirLight, "Sunlight");
    entities_.emplace<Nova::DirectionalLightComponent>(
        dirLight,
        Nova::DirectionalLightComponent{
            .Color = {1.0f, 1.0f, 1.0f},
            .Intensity = 0.2f,
            .Direction = {0.5f, 0.0f, -1.0f},
        });
}

void MainLayer::OnUpdate(double frametime)
{
    entities_.view<CPPScriptComponent>().each(
        [=](auto entity, auto &script)
        {
            script.ControllerInstance->OnUpdate(frametime);
        });
}

bool MainLayer::OnEvent(const Nova::Event &event)
{
    entities_.view<CPPScriptComponent>().each(
        [&](auto entity, auto &script)
        {
            script.ControllerInstance->OnEvent(event);
        });

    return false;
}

void MainLayer::OnRender()
{
    RenderScene(entities_, mainCameraEntity_);

    for (const auto &heart : hearts_)
        Nova::Renderer::Render(model_.get(), heart.Material, heart.Transform);

    Nova::Renderer::Draw(
        glm::vec4(0.529f, 0.529f, 0.529f, 1.0f),
        fogColor_);

    // Render GUI
    ImGui::NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();

    ImGui::BeginMainMenuBar();
    ImGui::EndMainMenuBar();

    ImGui::DockSpaceOverViewport(0, 0, ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::Begin("Scene");
    for (const auto entity : entities_.view<entt::entity>())
    {
        const auto nameComponent = entities_.try_get<NameComponent>(entity);
        const auto &name = nameComponent
                               ? nameComponent->Name
                               : std::format("Entity {}.", (uint32_t)entity);
        const auto isSelected = selectedEntity_ == entity;
        const auto flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | (isSelected ? ImGuiTreeNodeFlags_Selected : 0);

        const auto treeNodeOpen = ImGui::TreeNodeEx(name.c_str(), flags);

        if (ImGui::IsItemClicked())
            selectedEntity_ = entity;

        if (treeNodeOpen)
        {
            TryAddEntityComponentTreeNode<Nova::TransformComponent>(entities_, entity, "Transform", isSelected);
            TryAddEntityComponentTreeNode<Nova::DirectionalLightComponent>(entities_, entity, "Light (Directional)", isSelected);
            TryAddEntityComponentTreeNode<Nova::PointLightComponent>(entities_, entity, "Light (Point)", isSelected);
            TryAddEntityComponentTreeNode<Nova::CameraComponent>(entities_, entity, "Camera", isSelected);
            TryAddEntityComponentTreeNode<CPPScriptComponent>(entities_, entity, "Script", isSelected);
            ImGui::TreePop();
        }
    }
    ImGui::End();

    ImGui::Begin("Inspector");
    if (selectedEntity_ != (entt::entity)-1)
    {
        AddCameraComponentInspector(entities_, selectedEntity_);
        AddDirectionalLightComponentInspector(entities_, selectedEntity_);
        AddPointLightComponentInspector(entities_, selectedEntity_);
        AddTransformComponentInspector(entities_, selectedEntity_);
    }
    ImGui::End();

    ImGui::Begin("Assets");

    constexpr float thumbnailSize = 128.0f;
    constexpr float padding = 16.0f;
    constexpr float cellSize = thumbnailSize + padding;

    const float panelWidth = ImGui::GetContentRegionAvail().x;
    const int columnCount = std::max((int)(panelWidth / cellSize), 1);

    ImGui::Columns(columnCount, 0, false);

    for (const auto &[id, asset] : Nova::Assets::GetAssets())
    {
        if (asset->IsInternal())
            continue;

        ImGui::BeginChild(id.hash());

        switch (asset->GetType())
        {
        case Nova::AssetType::Image:
        {
            const auto image = std::static_pointer_cast<Nova::Image>(asset);
            const auto size = image->GetTexture().GetSize();
            const auto aspectRatio = size.x / (float)size.y;
            const auto imageThumbnailSize = ImVec2{thumbnailSize, thumbnailSize / aspectRatio};
            const auto imageYOffset = (thumbnailSize - thumbnailSize / aspectRatio) / 2;
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + imageYOffset);
            ImGui::Image(image->GetTexture().GetID(), imageThumbnailSize);
            break;
        }
        case Nova::AssetType::Model:
        {
            ImGui::Image(
                modelIconImage_->GetTexture().GetID(),
                {thumbnailSize, thumbnailSize});
            break;
        }
        }

        if (asset->GetName().has_value())
            ImGui::TextWrapped(asset->GetName().value().data());

        ImGui::EndChild();

        if (ImGui::IsItemHovered())
        {
            ImGui::GetWindowDrawList()->AddRect(
                ImGui::GetItemRectMin(),
                ImGui::GetItemRectMax(),
                0xFFFFFFFF);

            const auto &source = asset->GetSource();
            ImGui::SetTooltip(
                "Source: %s",
                source.Type == Nova::AssetSourceType::File
                    ? source.Filepath.string().c_str()
                    : "MEMORY");
        }

        ImGui::NextColumn();
    }

    ImGui::Columns(1);

    ImGui::End();

    ImGui::Begin("Frame info");
    ImGui::Text("Window size: %dx%d", Nova::Window::GetFramebufferWidth(), Nova::Window::GetFramebufferHeight());
    ImGui::Text("Viewport size: %dx%d", 0, 0);
    ImGui::Text("Frametime: %.2lf ms", Nova::Application::GetFrametime() * 1000.0);
    ImGui::Text("FPS: %.2lf", 1.0 / Nova::Application::GetFrametime());
    ImGui::Separator();

    const auto &rendererInfo = Nova::Renderer::GetInfo();
    ImGui::Text("Renderer: %s", rendererInfo.RendererName.data());
    ImGui::SetItemTooltip(rendererInfo.RendererName.data());

    ImGui::Text("Vendor: %s", rendererInfo.VendorName.data());
    ImGui::SetItemTooltip(rendererInfo.VendorName.data());

    ImGui::Text("Version: %s", rendererInfo.Version.data());
    ImGui::SetItemTooltip(rendererInfo.Version.data());

    ImGui::Text("GLSL Version: %s", rendererInfo.GLSLVersion.data());
    ImGui::SetItemTooltip(rendererInfo.GLSLVersion.data());

    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
    ImGui::PushStyleVar(ImGuiStyleVar_ImageBorderSize, 0.0f);
    ImGui::Begin("Viewport");

    const auto viewportSize = ImGui::GetContentRegionAvail();

    Nova::Renderer::SetDisplaySize(viewportSize.x, viewportSize.y);

    auto camera = entities_.try_get<Nova::CameraComponent>(mainCameraEntity_);
    if (camera && camera->Type == Nova::CameraType::Perspective)
        camera->Data.Perspective.AspectRatio = viewportSize.x / viewportSize.y;

    ImGui::Image(
        Nova::Renderer::GetRenderTextureID(Nova::RenderTexture::Output),
        viewportSize,
        {0.0f, 1.0f},
        {1.0f, 0.0f});
    ImGui::End();
    ImGui::PopStyleVar(2);

    ImGui::Render();

    const Nova::Rect<int> viewport{
        .X = 0,
        .Y = 0,
        .Width = Nova::Window::GetWidth(),
        .Height = Nova::Window::GetHeight(),
    };
    Nova::Renderer::SetViewport(viewport, viewport);
    Nova::Renderer::Clear();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}