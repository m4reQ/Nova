#include <Nova/core/Application.hpp>
#include <Nova/dotnet/Assembly.hpp>
#include "MainLayer.hpp"
#include "OverlayLayer.hpp"
#include <iostream>

int main()
{
    try
    {
        Nova::Application::Initialize(
            Nova::ApplicationSettings{
                .WindowSettings{
                    .Width = 1920,
                    .Height = 1080,
                    .Title = "Nova Editor",
                    .Flags = Nova::WindowFlags::StartMaximized | Nova::WindowFlags::Vsync},
                .DotnetSettings{
                    .Version = "10.0",
                    .RuntimeConfigPath = "./assets/Nova.runtimeconfig.json",
                },
                .ShaderCacheDirectory = "./shadercache",
                .TextInputBufferSize = 512});
        Nova::Application::PushLayer<MainLayer>();
        Nova::Application::Run();
    }
    catch (const std::exception& exc)
    {
        std::cerr << exc.what() << std::endl;
        return 1;
    }
    

    return 0;
}
