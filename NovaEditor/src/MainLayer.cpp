#include "MainLayer.hpp"
#include <Nova/graphics/Window.hpp>
#include <Nova/graphics/Renderer.hpp>
#include <Nova/input/Input.hpp>
#include <Nova/core/Application.hpp>
#include <Nova/ecs/components/NameComponent.hpp>
#include <Nova/ecs/components/TransformComponent.hpp>
#include <Nova/ecs/components/LightComponent.hpp>
#include <Nova/ecs/components/TransformComponent.hpp>
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

static float RandomFloat(float min = 0.0f, float max = 1.0f) noexcept
{
    return ((float)std::rand() / (float)RAND_MAX) * (max - min) + min;
}

template <typename T>
static T Random(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) noexcept
{
    return (T)(((float)std::rand() / (float)RAND_MAX) * (max - min) + min);
}

static glm::vec4 RandomColor(bool randomizeAlpha = false) noexcept
{
    return glm::vec4(
        RandomFloat(0.5f),
        RandomFloat(0.5f),
        RandomFloat(0.5f),
        randomizeAlpha ? RandomFloat() : 1.0f);
}

template <typename T>
static T& SelectRandomElement(std::span<T> container) noexcept
{
    return container[Random(0zu, container.size() - 1)];
}

static glm::mat4 BuildTransformMatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3 scale) noexcept
{
    const auto rotationMatrix = glm::mat3_cast(rotation);

    glm::mat4 result;
    result[0] = glm::vec4(rotationMatrix[0] * scale.x, 0.0f);
    result[1] = glm::vec4(rotationMatrix[1] * scale.x, 0.0f);
    result[2] = glm::vec4(rotationMatrix[2] * scale.x, 0.0f);
    result[3] = glm::vec4(position, 1.0f);

    return result;
}

static Nova::Model LoadModelFromObjFile(const std::filesystem::path& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("Failed to open model file.");

    std::vector<glm::vec3> positionData;
    std::vector<glm::vec2> textureCoordData;
    std::vector<glm::vec3> normalData;
    std::vector<Nova::ModelVertex> modelData;

    std::string line;
    while (std::getline(file, line))
    {
        switch (line[0])
        {
        case 'v':
            if (line[1] == 't')
            {
                glm::vec2 data;
                std::sscanf(
                    line.data(),
                    "vt %f %f\n",
                    &data.x,
                    &data.y);
                textureCoordData.emplace_back(data);
            }
            else if (line[1] == 'n')
            {
                glm::vec3 data;
                std::sscanf(
                    line.data(),
                    "vn %f %f %f\n",
                    &data.x,
                    &data.y,
                    &data.z);
                normalData.emplace_back(data);
            }
            else
            {
                glm::vec3 data;
                std::sscanf(
                    line.data(),
                    "v %f %f %f\n",
                    &data.x,
                    &data.y,
                    &data.z);
                positionData.emplace_back(data);
            }
            
            break;
        case 'f':
            int vertexIndices[3];
            int textureIndices[3];
            int normalIndices[3];
            std::sscanf(
                line.data(),
                "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vertexIndices[0],
                &textureIndices[0],
                &normalIndices[0],
                &vertexIndices[1],
                &textureIndices[1],
                &normalIndices[1],
                &vertexIndices[2],
                &textureIndices[2],
                &normalIndices[2]);
            
            for (int i = 0; i < 3; i++)
            {
                Nova::ModelVertex modelVertex{
                    .Position = positionData[vertexIndices[i] - 1],
                    .Normal = normalData[normalIndices[i] - 1],
                    .TextureCoords = textureCoordData[textureIndices[i] - 1],
                };
                modelData.emplace_back(modelVertex);
            }

            break;
        }
    }

    return Nova::Model(
        1,
        Nova::Buffer(
            modelData.size() * sizeof(Nova::ModelVertex),
            false,
            false,
            modelData.data()));
}

template <typename TComponent>
static void TryAddEntityComponentTreeNode(const entt::registry& registry, entt::entity entity, const std::string_view componentName)
{
    const auto component = registry.try_get<TComponent>(entity);
    if (component)
    {
        const auto name = std::format("{}##{}", componentName, (uint32_t)entity);
        ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
    }
}

static void RenderScene(const entt::registry& scene, entt::entity cameraEntity)
{
    // set main scene camera
    auto view = glm::identity<glm::mat4>();
    auto projection = glm::identity<glm::mat4>();
    auto cameraPosition = glm::vec3(0.0f);

    const auto camera = scene.try_get<CameraComponent>(cameraEntity);
    if (camera)
    {
        view = glm::lookAt(
            camera->Position,
            camera->Position + camera->Front,
            camera->Up);
        projection = camera->Type == CameraType::Perspective
            ? glm::perspective(
                camera->Data.Perspective.FOV,
                camera->Data.Perspective.AspectRatio,
                camera->Data.Perspective.ZNear,
                camera->Data.Perspective.ZFar)
            : glm::ortho(
                camera->Data.Ortho.Left,
                camera->Data.Ortho.Right,
                camera->Data.Ortho.Bottom,
                camera->Data.Ortho.Top);
        cameraPosition = camera->Position;
    }

    Nova::Renderer::SetCamera(view, projection, cameraPosition);

    // set directional lights
    scene.view<DirectionalLightComponent>().each(
        [](auto entity, const auto& lightComponent)
        {
            Nova::Renderer::AddDirectionalLight(
                glm::vec4(lightComponent.Color, lightComponent.Intensity),
                lightComponent.Direction);
        });
    
    // set point lights
    scene.view<TransformComponent, PointLightComponent>().each(
        [](auto entity, const auto& transform, const auto& light)
        {
            Nova::Renderer::AddPointLight(
                glm::vec4(light.Color, light.Intensity),
                transform.Position,
                light.Radius);
        });

    // render objects
    scene.view<TransformComponent, RenderComponent>().each(
        [](auto entity, const auto& transform, const auto& render)
        {
            Nova::Renderer::Render(
                render.Model,
                *render.Material,
                BuildTransformMatrix(
                    transform.Position,
                    transform.Rotation,
                    transform.Scale));
        });

    // draw
    // Nova::Renderer::Draw(glm::vec4(0.529f, 0.529f, 0.529f, 1.0f));
}

MainLayer::MainLayer()
    : Nova::Layer("MainLayer")
{
    if (!ImGui::CreateContext())
        throw std::runtime_error("Failed to initialize ImGui.");

    auto& io = ImGui::GetIO();
    io.DisplaySize = ImVec2{ (float)Nova::Window::GetWidth(), (float)Nova::Window::GetHeight() };
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
                .ID = i,
                .Color = RandomColor(),
            });
    
    hearts_.reserve(5 * 5);

    for (size_t y = 0; y < 5; y++)
    {
        for (size_t x = 0; x < 5; x++)
        {
            hearts_.emplace_back(
                HeartData {
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
            
    model_ = LoadModelFromObjFile("./assets/heart.obj");

    // initialize main camera
    mainCameraEntity_ = entities_.create();
    entities_.emplace<NameComponent>(mainCameraEntity_, "MainCamera");
    entities_.emplace<CameraComponent>(
        mainCameraEntity_,
        CameraComponent::CreatePerspective(
            glm::radians(45.0f),
            Nova::Window::GetAspectRatio(),
            0.001f,
            3.0f));
    auto& scriptComponent = entities_.emplace<CPPScriptComponent>(mainCameraEntity_, CPPScriptComponent::Create<CameraController>());
    scriptComponent.ControllerInstance->OnAttach(entities_, mainCameraEntity_);

    // initialize lights
    auto light1 = entities_.create();
    entities_.emplace<NameComponent>(light1, "Point light 1");
    entities_.emplace<PointLightComponent>(
        light1,
        PointLightComponent {
            .Color = {1.0f, 0.09f, 0.985f},
            .Intensity = 0.4f,
            .Radius = 0.2f,
        });
    entities_.emplace<TransformComponent>(
        light1,
        TransformComponent {
            .Position = {0.5f, 0.0f, -1.3f},
        });

    auto light2 = entities_.create();
    entities_.emplace<NameComponent>(light2, "Point light 2");
    entities_.emplace<PointLightComponent>(
        light2,
        PointLightComponent {
            .Color = {1.0f, 0.5f, 0.03f},
            .Intensity = 0.4f,
            .Radius = 0.2f,
        });
    entities_.emplace<TransformComponent>(
        light2,
        TransformComponent {
            .Position = {0.5f, 0.0f, -1.6f},
        });

    auto light3 = entities_.create();
    entities_.emplace<NameComponent>(light3, "Point light 3");
    entities_.emplace<PointLightComponent>(
        light3,
        PointLightComponent {
            .Color = {1.0f, 0.5f, 0.8f},
            .Intensity = 0.4f,
            .Radius = 0.2f,
        });
    entities_.emplace<TransformComponent>(
        light3,
        TransformComponent {
            .Position = {-0.2f, 0.0f, -1.3f},
        });

    auto light4 = entities_.create();
    entities_.emplace<NameComponent>(light4, "Point light 4");
    entities_.emplace<PointLightComponent>(
        light4,
        PointLightComponent {
            .Color = {1.0f, 1.0f, 1.0f},
            .Intensity = 0.5f,
            .Radius = 0.6f,
        });
    entities_.emplace<TransformComponent>(
        light4,
        TransformComponent {
            .Position = {0.0f, 1.5f, 0.0f},
        });

    auto dirLight = entities_.create();
    entities_.emplace<NameComponent>(dirLight, "Sunlight");
    entities_.emplace<DirectionalLightComponent>(
        dirLight,
        DirectionalLightComponent {
            .Color = {1.0f, 1.0f, 1.0f},
            .Intensity = 0.2f,
            .Direction = {0.5f, 0.0f, -1.0f},
        });
}

void MainLayer::OnUpdate(double frametime)
{
    entities_.view<CPPScriptComponent>().each(
        [=](auto entity, auto& script)
        {
            script.ControllerInstance->OnUpdate(frametime);
        });
}

bool MainLayer::OnEvent(const Nova::Event& event)
{
    entities_.view<CPPScriptComponent>().each(
        [&](auto entity, auto& script)
        {
            script.ControllerInstance->OnEvent(event);
        });
    
    return false;
}

void MainLayer::OnRender()
{
    RenderScene(entities_, mainCameraEntity_);

    for (const auto& heart : hearts_)
        Nova::Renderer::Render(&model_, heart.Material, heart.Transform);

    Nova::Renderer::Draw(glm::vec4(0.529f, 0.529f, 0.529f, 1.0f));

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
        const auto& name = nameComponent
            ? nameComponent->Name
            : std::format("Entity {}.", (uint32_t)entity);
        if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            TryAddEntityComponentTreeNode<TransformComponent>(entities_, entity, "Transform");
            TryAddEntityComponentTreeNode<DirectionalLightComponent>(entities_, entity, "Light (Directional)");
            TryAddEntityComponentTreeNode<PointLightComponent>(entities_, entity, "Light (Point)");
            TryAddEntityComponentTreeNode<CameraComponent>(entities_, entity, "Camera");
            TryAddEntityComponentTreeNode<CPPScriptComponent>(entities_, entity, "Script");
            ImGui::TreePop();
        }
    }
    ImGui::End();

    ImGui::Begin("Frame info");
    ImGui::Text("Window size: %dx%d", Nova::Window::GetFramebufferWidth(), Nova::Window::GetFramebufferHeight());
    ImGui::Text("Viewport size: %dx%d", 0, 0);
    ImGui::Text("Frametime: %.2lf ms", Nova::Application::GetFrametime() * 1000.0);
    ImGui::Text("FPS: %.2lf", 1.0 / Nova::Application::GetFrametime());
    ImGui::Separator();

    const auto& rendererInfo = Nova::Renderer::GetInfo();
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
    
    auto camera = entities_.try_get<CameraComponent>(mainCameraEntity_);
    if (camera && camera->Type == CameraType::Perspective)
        camera->Data.Perspective.AspectRatio = viewportSize.x / viewportSize.y;

    ImGui::Image(
        Nova::Renderer::GetRenderTextureID(Nova::RenderTexture::Output),
        viewportSize,
        {0.0f, 1.0f},
        {1.0f, 0.0f});
    ImGui::End();
    ImGui::PopStyleVar(2);

    ImGui::Render();

    Nova::Renderer::Clear();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}