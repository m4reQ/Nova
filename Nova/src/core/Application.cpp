#include <Nova/core/Application.hpp>
#include <Nova/core/Layer.hpp>
#include <Nova/dotnet/Host.hpp>
#include <Nova/graphics/Window.hpp>
#include <Nova/graphics/Renderer.hpp>
#include <Nova/input/Input.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/debug/Profile.hpp>
#include <filesystem>

using namespace Nova;

static bool s_IsRunning;
static bool s_IsInitialized = false;
static double s_Frametime = 1.0;
static std::chrono::high_resolution_clock::time_point s_FrameStart;
static std::vector<std::unique_ptr<Layer>> s_LayerStack;
static std::vector<std::pair<Layer*, std::unique_ptr<Layer>>> s_LayerTransitionQueue;

static auto FindLayerByName(const std::string_view name) noexcept
{
    NV_PROFILE_FUNC;

    return std::find_if(
        s_LayerStack.begin(),
        s_LayerStack.end(),
        [=](const std::unique_ptr<Layer>& layer)
        {
            return layer->GetName() == name;
        });
}

static void UpdateFrametime() noexcept
{
    const auto frameStart = std::chrono::high_resolution_clock::now();
    s_Frametime = std::chrono::duration<double>(frameStart - s_FrameStart).count();
    s_FrameStart = frameStart;
}

static void Update()
{
    NV_PROFILE_FUNC;

    UpdateFrametime();

    Window::Update_();

    if (Window::ShouldClose())
    {
        Application::Stop();
        return;
    }

    for (const std::unique_ptr<Layer>& layer : s_LayerStack)
        layer->OnUpdate(s_Frametime);
}

static void Render()
{
    NV_PROFILE_FUNC;

    if (!Window::IsVisible())
        return;
    
    for (const std::unique_ptr<Layer>& layer : s_LayerStack)
        layer->OnRender();

    Window::SwapBuffers_();
}

static void ProcessLayerTransitions() noexcept
{
    NV_PROFILE_FUNC;

    for (auto& [from, to] : s_LayerTransitionQueue)
        for (auto& targetLayer : s_LayerStack)
            if (targetLayer.get() == from)
                targetLayer = std::move(to);

    s_LayerTransitionQueue.clear();
}

static void Shutdown() noexcept
{
    NV_PROFILE_FUNC;

    Renderer::_Shutdown();
    Window::Shutdown_();
    Dotnet::Shutdown_();
}

void Application::Initialize(const ApplicationSettings &settings)
{
    NV_LOG_INITIALIZE("./NovaLog.txt");
    NV_LOG_INFO("Using working directory \"{}\".", std::filesystem::current_path().string());
    NV_PROFILE_SET_ENABLED(true);
    NV_PROFILE_BEGIN_SESSION("./NovaProfileSession.json");

    Dotnet::Initialize_(settings.DotnetSettings);
    Window::Initialize_(settings.WindowSettings);
    Renderer::_Initialize(
        Window::GetWidth(),
        Window::GetHeight(),
        Window::GetLoaderFunc_(),
        settings.RendererSettings);

    s_IsInitialized = true;
}

void Application::Run()
{
    NV_PROFILE_FUNC;

    s_IsRunning = true;
    while (s_IsRunning)
    {
        NV_PROFILE_SCOPE("ProcessFrame");

        Update();
        Render();
        ProcessLayerTransitions();
    }

    Shutdown();

    NV_PROFILE_END_SESSION();
}

void Application::Stop()
{
    s_IsRunning = false;
}

bool Application::IsInitialized() noexcept
{
    return s_IsInitialized;
}

bool Application::IsRunning() noexcept
{
    return s_IsRunning;
}

double Application::GetFrametime() noexcept
{
    return s_Frametime;
}

Layer *Application::GetTopLayer() noexcept
{
    return s_LayerStack.size() == 0
               ? nullptr
               : (s_LayerStack.end() - 1)->get();
}

Layer *Application::GetBottomLayer() noexcept
{
    return s_LayerStack.size() == 0
               ? nullptr
               : s_LayerStack.begin()->get();
}

Layer *Application::GetLayer(const std::string_view layerName) noexcept
{
    NV_PROFILE_FUNC;

    const auto &layers = GetLayers();
    const auto layer = FindLayerByName(layerName);
    return layer != layers.end()
               ? layer->get()
               : nullptr;
}

bool Application::HasLayer(const std::string_view layerName)
{
    return FindLayerByName(layerName) != s_LayerStack.end();
}

std::vector<std::unique_ptr<Layer>>& Application::GetLayers() noexcept
{
    return s_LayerStack;
}

void Application::PushLayer(std::unique_ptr<Layer>&& layer)
{
    s_LayerStack.emplace_back(std::move(layer));
}

void Application::InsertLayerAfter(const std::string_view layerName, std::unique_ptr<Layer>&& layer)
{
    s_LayerStack.insert(
        FindLayerByName(layerName) + 1,
        std::move(layer));
}

void Application::InsertLayerBefore(const std::string_view layerName, std::unique_ptr<Layer>&& layer)
{
    s_LayerStack.insert(
        FindLayerByName(layerName),
        std::move(layer));
}

void Application::_QueueLayerTransition(Layer* from, std::unique_ptr<Layer>&& to) noexcept
{
    s_LayerTransitionQueue.emplace_back(from, std::move(to));
}

void Application::InvokeEvent_(const Event& event)
{
    NV_PROFILE_FUNC;

    for (auto& layer : s_LayerStack)
    {
        NV_PROFILE_SCOPE("ProcessLayerEvent");

        if (layer->OnEvent(event))
            break;
    }
}
