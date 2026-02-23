#pragma once
#include <Nova/core/Build.hpp>
#include <Nova/core/Event.hpp>
#include <string>
#include <memory>

namespace Nova
{
	class NV_API Layer
	{
	public:
		Layer() = default;
		constexpr Layer(const std::string_view name) noexcept
			: m_Name(name) {}

		virtual ~Layer() noexcept = default;

		virtual void OnUpdate(double frametime) {}
		virtual void OnRender() {}
		virtual bool OnEvent(const Event& event) { return false; }

		template <std::derived_from<Layer> TLayer, typename... Args>
		void TransitionTo(Args &&...args)
		{
			TransitionToImpl(std::make_unique<TLayer>(std::forward<Args>(args)...));
		}

		constexpr std::string_view GetName() const noexcept { return m_Name; }

	private:
		std::string m_Name;

		void TransitionToImpl(std::unique_ptr<Layer>&& layer);
	};
}
