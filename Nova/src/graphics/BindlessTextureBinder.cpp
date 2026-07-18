#include <Nova/graphics/opengl/BindlessTextureBinder.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/debug/Log.hpp>
#include <stdexcept>

using namespace Nova;

BindlessTextureBinder::BindlessTextureBinder(GLuint maxBindingAge) noexcept
    : maxBindingAge_(maxBindingAge) {}

BindlessTextureBinder::BindlessTextureBinder(BindlessTextureBinder &&other) noexcept
    : maxBindingAge_(other.maxBindingAge_),
      bindings_(std::move(other.bindings_)),
      bindlessHandles_(std::move(other.bindlessHandles_)) {}

BindlessTextureBinder::~BindlessTextureBinder() noexcept
{
    for (const auto &binding : bindings_)
        glMakeTextureHandleNonResidentARB(binding.Handle);
}

bool BindlessTextureBinder::IsBound(const ITexture &texture) const noexcept
{
    NV_PROFILE_FUNC;

    for (const auto &binding : bindings_)
        if (binding.TextureID == texture.GetID())
            return true;

    return false;
}

GLuint64 BindlessTextureBinder::Bind(const ITexture &texture, bool isPersistent)
{
    NV_PROFILE_FUNC;

    auto binding = GetBindingInternal(texture);
    if (binding != bindings_.end())
    {
        binding->Age = 0;
        return binding->Handle;
    }

    GLuint64 handle = 0;

    const auto handleIter = bindlessHandles_.find(texture.GetID());
    if (handleIter != bindlessHandles_.end())
    {
        handle = handleIter->second;
        NV_LOG_TRACE("Texture with ID {} has a bindless handle ({:#x}) but it is not resident.", texture.GetID(), handle);
    }
    else
    {
        NV_LOG_TRACE("Texture with ID {} has no bindless handle associated with it.", texture.GetID());
        handle = glGetTextureHandleARB(texture.GetID());

        if (handle == 0)
        {
            NV_LOG_ERROR("Failed to create bindless handle for a texture with ID: {}.", texture.GetID());
            throw std::runtime_error("Failed to create bindless texture handle.");
        }

        NV_LOG_TRACE("Created bindless hande ({:#x}) for a texture with ID {}.", handle, texture.GetID());
        bindlessHandles_.emplace(texture.GetID(), handle);
    }

    // handle exists, but not resident
    while (true)
    {
        glMakeTextureHandleResidentARB(handle);

        if (glIsTextureHandleResidentARB(handle))
        {
            NV_LOG_TRACE(
                "Bindless handle ({:#x}) for a texture with ID {} was made resident. Handle lifetime: {}.",
                handle,
                texture.GetID(),
                isPersistent
                    ? std::string("persistent")
                    : std::format("{} frames", maxBindingAge_));
            break;
        }
        else
        {
            NV_LOG_TRACE("Failed to make bindless handle resident. Evicting textures...");

            // try to evict oldest texture
            std::list<BindlessTextureBinding>::iterator oldestBinding = bindings_.begin();
            for (auto binding = bindings_.begin(); binding != bindings_.end(); binding++)
            {
                if (!binding->IsPersistent && binding->Age >= oldestBinding->Age)
                    oldestBinding = binding;
            }

            if (oldestBinding->IsPersistent)
            {
                NV_LOG_ERROR("Couldn't make bindless handle resident after evicting all non-persistent textures.");
                throw std::runtime_error("Failed to make texture handle resident.");
            }
            else
            {
                NV_LOG_TRACE("Evicted texture {} with age of {} frames.", oldestBinding->TextureID, oldestBinding->Age);

                glMakeTextureHandleNonResidentARB(oldestBinding->Handle);
                bindings_.erase(oldestBinding);
            }
        }
    }

    bindings_.emplace_back(
        BindlessTextureBinding{
            .Handle = handle,
            .TextureID = texture.GetID(),
            .Age = 0,
            .IsPersistent = isPersistent,
        });

    NV_LOG_TRACE("Successfully made texture with ID {} persistent.", texture.GetID());

    // handle is valid and resident
    return handle;
}

GLuint64 BindlessTextureBinder::GetBinding(const ITexture &texture) const noexcept
{
    const auto binding = GetBindingInternal(texture);
    return binding != bindings_.end()
               ? binding->Handle
               : 0;
}

void BindlessTextureBinder::Unbind(const ITexture &texture, bool deleteHandle) noexcept
{
    NV_PROFILE_FUNC;

    auto binding = bindings_.begin();
    while (binding != bindings_.end())
    {
        if (binding->TextureID == texture.GetID())
        {
            NV_LOG_TRACE("Unbound texture with ID {}.", binding->TextureID);

            glMakeTextureHandleNonResidentARB(binding->Handle);

            if (deleteHandle)
            {
                NV_LOG_TRACE("Deleted bindless texture handle {:#x} from binder cache.", binding->Handle);
                bindlessHandles_.erase(binding->TextureID);
            }

            bindings_.erase(binding);
            break;
        }
        else
        {
            binding++;
        }
    }
}

void BindlessTextureBinder::Unbind(GLuint64 handle, bool deleteHandle) noexcept
{
    NV_PROFILE_FUNC;

    auto binding = bindings_.begin();
    while (binding != bindings_.end())
    {
        if (binding->Handle == handle)
        {
            glMakeTextureHandleNonResidentARB(binding->Handle);

            if (deleteHandle)
                bindlessHandles_.erase(binding->TextureID);

            bindings_.erase(binding);
            break;
        }
        else
        {
            binding++;
        }
    }
}

void BindlessTextureBinder::UnbindAll(bool unbindPersistent) noexcept
{
    NV_PROFILE_FUNC;

    auto binding = bindings_.begin();
    while (binding != bindings_.end())
    {
        if (!binding->IsPersistent || (binding->IsPersistent && unbindPersistent))
        {
            glMakeTextureHandleNonResidentARB(binding->Handle);
            binding = bindings_.erase(binding);
        }
        else
        {
            binding++;
        }
    }
}

void BindlessTextureBinder::Update() noexcept
{
    NV_PROFILE_FUNC;

    auto binding = bindings_.begin();
    while (binding != bindings_.end())
    {
        if (binding->Age++ >= maxBindingAge_ && !binding->IsPersistent)
        {
            NV_LOG_TRACE("Evicted texture with ID {} due to inactivity.", binding->TextureID);

            glMakeTextureHandleNonResidentARB(binding->Handle);
            binding = bindings_.erase(binding);
        }
        else
            binding++;
    }
}

BindlessTextureBinder &BindlessTextureBinder::operator=(BindlessTextureBinder &&other) noexcept
{
    bindings_ = std::move(other.bindings_);
    bindlessHandles_ = std::move(other.bindlessHandles_);
    maxBindingAge_ = other.maxBindingAge_;

    return *this;
}

std::list<BindlessTextureBinding>::iterator BindlessTextureBinder::GetBindingInternal(const ITexture &texture) noexcept
{
    NV_PROFILE_FUNC;

    const auto textureID = texture.GetID();
    return std::find_if(
        bindings_.begin(),
        bindings_.end(),
        [=](const BindlessTextureBinding &binding)
        {
            return binding.TextureID == textureID;
        });
}

std::list<BindlessTextureBinding>::const_iterator BindlessTextureBinder::GetBindingInternal(const ITexture &texture) const noexcept
{
    NV_PROFILE_FUNC;

    const auto textureID = texture.GetID();
    return std::find_if(
        bindings_.begin(),
        bindings_.end(),
        [=](const BindlessTextureBinding &binding)
        {
            return binding.TextureID == textureID;
        });
}