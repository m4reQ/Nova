#include "ImGuiContextWrapper.hpp"
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>
#include <src/ImGuizmo.h>
#include <stdexcept>

ImGuiContextWrapper::ImGuiContextWrapper(const Nova::Window &window)
    : context_(ImGui::CreateContext())
{
    if (context_ == nullptr)
        throw std::runtime_error("Failed to create ImGui context.");

    auto &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.DisplaySize = ImVec2{
        static_cast<float>(window.GetWidth()),
        static_cast<float>(window.GetHeight()),
    };

    if (!ImGui_ImplWin32_InitForOpenGL(window.GetData().handle))
        throw std::runtime_error("Failed to initialize ImGui GLFW backend.");

    if (!ImGui_ImplOpenGL3_Init())
        throw std::runtime_error("Failed to initialize ImGui OpenGL backend.");

    ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
}

ImGuiContextWrapper::~ImGuiContextWrapper() noexcept
{
    if (context_ != nullptr)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext(context_);
    }
}