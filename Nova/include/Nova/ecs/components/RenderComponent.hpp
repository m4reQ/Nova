#pragma once
#include <Nova/assets/Model.hpp>
#include <Nova/graphics/Material.hpp>

namespace Nova
{
    struct RenderComponent
    {
        Nova::Model *Model;
        Nova::Material *Material;
    };
}