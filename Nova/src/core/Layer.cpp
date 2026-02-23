#include <Nova/core/Layer.hpp>
#include <Nova/core/Application.hpp>

using namespace Nova;

void Layer::TransitionToImpl(std::unique_ptr<Layer>&& layer)
{
	Application::_QueueLayerTransition(
		this,
		std::move(layer));
}