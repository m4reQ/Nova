#include "MainLayer.hpp"
#include <Nova/graphics/Window.hpp>
#include <Nova/graphics/Renderer.hpp>
#include <Nova/input/Input.hpp>
#include <Nova/core/Application.hpp>
#include "components/NameComponent.hpp"
#include "components/TransformComponent.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <format>
#include <fstream>
#include <ranges>
#include <random>
#include <limits>
#include <iostream>

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
        const auto name = std::format("{}###{}", componentName, (uint32_t)entity);
        ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
    }
}

void MainLayer::OnResizeEvent(const Nova::WindowResizeEvent& event) noexcept
{
    Nova::Renderer::SetViewport(event.GetRect(), event.GetRect());
    camera_.SetAspectRatio(event.GetWidth(), event.GetHeight());
}

void MainLayer::OnKeyEvent(const Nova::KeyEvent& event) noexcept
{
    if (!event.IsPressed())
        return;

    if (event.GetKey() == Nova::Key::_1)
        cursorCaptured_ = !cursorCaptured_;
}

void MainLayer::OnMouseMoveEvent(const Nova::MouseMoveEvent& event) noexcept
{
    constexpr float mouseSensitivity = 0.08f;

    yaw_ += (float)-event.GetDeltaX() * mouseSensitivity;
    pitch_ += (float)event.GetDeltaY() * mouseSensitivity;

    camera_.SetRotationDegrees(yaw_, pitch_);
}

void MainLayer::OnMouseScrollEvent(const Nova::MouseScrollEvent& event) noexcept
{
    camera_.SetFOVDegrees(
        std::min(
            std::max(
                camera_.GetFOVDegrees() - (float)event.GetVertical(),
                1.0f),
            89.0f));
}

bool MainLayer::OnEvent(const Nova::Event& event)
{
    switch (event.GetEventType())
    {
    case Nova::EventType::WindowResize:
        OnResizeEvent((const Nova::WindowResizeEvent&)event);
        break;
    case Nova::EventType::Key:
        OnKeyEvent((const Nova::KeyEvent&)event);
        break;
    case Nova::EventType::MouseMove:
        OnMouseMoveEvent((const Nova::MouseMoveEvent&)event);
        break;
    case Nova::EventType::MouseScroll:
        OnMouseScrollEvent((const Nova::MouseScrollEvent&)event);
        break;
    }

    return false;
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

    auto first = entities_.create();
    entities_.emplace<NameComponent>(first, "First");
    entities_.emplace<TransformComponent>(first);
    entities_.emplace<NameComponent>(entities_.create(), "Second");
    entities_.create();
    entities_.create();
}

void MainLayer::OnUpdate(double frametime)
{
    float cameraSpeed = 0.5f * frametime;

    glm::vec3 cameraDelta = glm::zero<glm::vec3>();
    if (Nova::Input::IsKeyDown(Nova::Key::W))
        cameraDelta += camera_.GetFront();
    if (Nova::Input::IsKeyDown(Nova::Key::S))
        cameraDelta -= camera_.GetFront();
    if (Nova::Input::IsKeyDown(Nova::Key::D))
        cameraDelta += glm::normalize(
            glm::cross(
                camera_.GetFront(),
                camera_.GetUp()));
    if (Nova::Input::IsKeyDown(Nova::Key::A))
        cameraDelta -= glm::normalize(
            glm::cross(
                camera_.GetFront(),
                camera_.GetUp()));
    if (Nova::Input::IsKeyDown(Nova::Key::Space))
        cameraDelta += camera_.GetUp();
    if (Nova::Input::IsKeyDown(Nova::Key::LeftShift))
        cameraDelta -= camera_.GetUp();
    
    camera_.Move(cameraDelta * cameraSpeed);
}

void MainLayer::OnRender()
{
    // Render main scene
    Nova::Renderer::SetCamera(
        camera_.GetViewMatrix(),
        camera_.GetProjectionMatrix(),
        camera_.GetPosition());
    Nova::Renderer::SetPolygonMode(Nova::PolygonMode::Fill);

    Nova::Renderer::AddLightSource(
        Nova::LightSource{
            .Color = {1.0f, 0.09f, 0.985f, 0.2f},
            .Position = {0.5f, 0.0f, -1.3f},
        });
    Nova::Renderer::AddLightSource(
        Nova::LightSource{
            .Color = {1.0f, 1.0f, 1.0f, 0.3f},
            .Position = {0.0f, 1.5f, 0.0f},
        });
    Nova::Renderer::AddLightSource(
        Nova::LightSource{
            .Color = {1.0f, 0.4f, 0.1f, 0.03f},
            .Position = camera_.GetPosition(),
        });

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
            ImGui::TreePop();
        }
    }
    ImGui::End();

    ImGui::Begin("Frame info");
    ImGui::Text("Window size: %dx%d", Nova::Window::GetFramebufferWidth(), Nova::Window::GetFramebufferHeight());
    ImGui::Text("Viewport size: %dx%d", 0, 0);
    ImGui::Text("Frametime: %.2lf ms", Nova::Application::GetFrametime() * 1000.0);
    ImGui::Text("FPS: %.2lf", 1.0 / Nova::Application::GetFrametime());
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
    ImGui::PushStyleVar(ImGuiStyleVar_ImageBorderSize, 0.0f);
    ImGui::Begin("Viewport");
    ImGui::Image(
        Nova::Renderer::GetRenderTextureID(Nova::RenderTexture::Output),
        ImGui::GetContentRegionAvail(),
        {0.0f, 1.0f},
        {1.0f, 0.0f});
    ImGui::End();
    ImGui::PopStyleVar(2);

    ImGui::Render();

    Nova::Renderer::Clear(0.0f, 0.0f, 0.0f, 0.0f);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}