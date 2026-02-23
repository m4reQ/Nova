#include <Nova/core/Application.hpp>
#include <Nova/dotnet/Assembly.hpp>
#include "MainLayer.hpp"
#include "OverlayLayer.hpp"

int main()
{
    Nova::Application::Initialize(
        Nova::ApplicationSettings{
            .WindowSettings{
                .Width = 1080,
                .Height = 720,
                .Title = "Nova Editor",
                .Flags = Nova::WindowFlags::Resizable | Nova::WindowFlags::Vsync},
            .DotnetSettings{
                .RuntimeConfigPath = "./assets/Nova.runtimeconfig.json",
            },
            .ShaderCacheDirectory = "./shadercache",
            .TextInputBufferSize = 512});
    Nova::Application::PushLayer<MainLayer>();
    Nova::Application::Run();

    return 0;
}
