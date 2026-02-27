#pragma once
#include <Nova/core/Build.hpp>
#include <Nova/core/Layer.hpp>
#include <Nova/core/Event.hpp>
#include <Nova/graphics/Window.hpp>
#include <Nova/graphics/RendererSettings.hpp>
#include <Nova/dotnet/DotnetSettings.hpp>
#include <vector>
#include <concepts>
#include <filesystem>

namespace Nova
{
    struct ApplicationSettings
    {
        WindowSettings WindowSettings;
        DotnetSettings DotnetSettings;
        RendererSettings RendererSettings;
        std::filesystem::path ShaderCacheDirectory;
        size_t TextInputBufferSize;
    };

    namespace Application
    {
        /// <summary>
        /// Private API. Don't user directly!
        /// </summary>
        void _QueueLayerTransition(Layer *from, std::unique_ptr<Layer>&& to) noexcept;

        /// <summary>
        /// Private API. Don't use directly!
        /// </summary>
        /// <param name="event"></param>
        void InvokeEvent_(const Event& event);

        /// <summary>
        /// Private API. Don't use directly!
        /// </summary>
        template <typename TEvent, typename... Args>
            requires (std::derived_from<TEvent, Event>)
        void InvokeEvent_(Args&&... args)
        {
            TEvent event(std::forward<Args>(args)...);
            InvokeEvent_(event);
        }

        NV_API void Initialize(const ApplicationSettings &settings);

        NV_API void Run();

        NV_API void Stop();

        NV_API bool IsInitialized() noexcept;

        NV_API bool IsRunning() noexcept;

        NV_API double GetFrametime() noexcept;

        NV_API std::vector<std::unique_ptr<Layer>>& GetLayers() noexcept;

        NV_API bool HasLayer(const std::string_view layerName);

        NV_API Layer *GetTopLayer() noexcept;

        NV_API Layer *GetBottomLayer() noexcept;

        NV_API Layer *GetLayer(const std::string_view layerName) noexcept;

        template <typename TLayer>
            requires(std::derived_from<TLayer, Layer>)
        TLayer *GetLayer(const std::string_view layerName) noexcept
        {
            return static_cast<TLayer*>(GetLayer(layerName));
        }

        NV_API void PushLayer(std::unique_ptr<Layer>&& layer);

        template <typename TLayer, typename... Args>
            requires(std::derived_from<TLayer, Layer>)
        void PushLayer(Args&&... args)
        {
            PushLayer(std::make_unique<TLayer>(std::forward<Args>(args)...));
        }

        NV_API void InsertLayerAfter(const std::string_view layerName, std::unique_ptr<Layer>&& layer);

        template <typename TLayer, typename... Args>
            requires(std::derived_from<TLayer, Layer>)
        void InsertLayerAfter(const std::string_view layerName, Args&&... args)
        {
            InsertLayerAfter(layerName, std::make_unique<TLayer>(std::forward<Args>(args)...));
        }

        NV_API void InsertLayerBefore(const std::string_view layerName, std::unique_ptr<Layer>&& layer);

        template <typename TLayer, typename... Args>
            requires(std::derived_from<TLayer, Layer>)
        void InsertLayerBefore(const std::string_view layerName, Args&&... args)
        {
            InsertLayerBefore(layerName, std::make_unique<TLayer>(std::forward<Args>(args)...));
        }
    }
}
