#include "OverlayLayer.hpp"
#include <Nova/core/Application.hpp>
#include <Nova/graphics/Renderer.hpp>
#include <Nova/graphics/Window.hpp>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include <stdexcept>

OverlayLayer::OverlayLayer()
    : Nova::Layer("OverlayLayer")
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
}

void OverlayLayer::OnResizeEvent(const Nova::WindowResizeEvent& event) noexcept
{
	const auto rect = event.GetRect();
	Nova::Renderer::SetViewport(rect, rect);
}

void OverlayLayer::OnKeyEvent(const Nova::KeyEvent& event) noexcept
{
    constexpr const std::array<Nova::PolygonMode, 3> polygonModes{ Nova::PolygonMode::Fill, Nova::PolygonMode::Wireframe, Nova::PolygonMode::Points };

    if (!event.IsPressed())
        return;

    switch (event.GetKey())
    {
    case Nova::Key::F11:
    {
        const auto fullscreenMode = Nova::Window::IsFullscreen()
            ? Nova::FullscreenMode::Windowed
            : Nova::FullscreenMode::Fullscreen;
        Nova::Window::SetFullscreen(fullscreenMode);
        break;
    }
    }
}
void OverlayLayer::OnCloseEvent(const Nova::WindowCloseEvent& event) noexcept
{
	std::cout << "Goodbye!\n";
}

bool OverlayLayer::OnEvent(const Nova::Event& event)
{
    switch (event.GetEventType())
    {
    case Nova::EventType::WindowClose:
        OnCloseEvent(static_cast<const Nova::WindowCloseEvent&>(event));
        break;
    case Nova::EventType::Key:
        OnKeyEvent(static_cast<const Nova::KeyEvent&>(event));
        break;
    case Nova::EventType::WindowResize:
        OnResizeEvent(static_cast<const Nova::WindowResizeEvent&>(event));
        break;
    }

    return false;
}

void OverlayLayer::OnRender()
{
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::BeginMainMenuBar();
    ImGui::EndMainMenuBar();

    ImGui::Begin("Renderer settings");

    constexpr std::array<std::pair<const char*, Nova::PolygonMode>, 3> polygonModes{
        std::pair{ "Fill", Nova::PolygonMode::Fill },
        std::pair{ "Wireframe", Nova::PolygonMode::Wireframe },
        std::pair{ "Points", Nova::PolygonMode::Points },
    };

    if (ImGui::BeginCombo("Polygon mode", polygonModes[currentPolygonModeIdx_].first))
    {
        for (size_t i = 0; i < polygonModes.size(); i++)
        {
            const auto& polygonMode = polygonModes[i];
            if (ImGui::Selectable(polygonMode.first, i == currentPolygonModeIdx_))
            {
                Nova::Renderer::SetPolygonMode(polygonMode.second);
                currentPolygonModeIdx_ = i;
            }
        }

        ImGui::EndCombo();
    }
    
    ImGui::End();

    ImGui::Begin("Frame info");
    ImGui::Text("Frametime: %.5lf", Nova::Application::GetFrametime());
    ImGui::Text("FPS: %.2lf", 1.0 / Nova::Application::GetFrametime());
    ImGui::Text("Window size: %dx%d", Nova::Window::GetWidth(), Nova::Window::GetHeight());
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
    ImGui::Begin("Viewport");
    ImGui::Image((ImTextureID)Nova::Renderer::GetRenderTextureID(Nova::RenderTexture::Color), ImGui::GetContentRegionAvail());
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::Render();

    Nova::Renderer::Clear(0.0f, 0.0f, 0.0f, 1.0f);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}