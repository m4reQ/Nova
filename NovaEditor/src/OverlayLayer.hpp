#pragma once
#include <Nova/core/Layer.hpp>
#include <Nova/graphics/Renderer.hpp>

class OverlayLayer final : public Nova::Layer
{
public:
    OverlayLayer();

    bool OnEvent(const Nova::Event& event) override;
    void OnRender() override;

private:
    size_t currentPolygonModeIdx_ = 0;

    void OnResizeEvent(const Nova::WindowResizeEvent& event) noexcept;
    void OnKeyEvent(const Nova::KeyEvent& event) noexcept;
    void OnCloseEvent(const Nova::WindowCloseEvent& event) noexcept;
};