#pragma once
#include <Nova/graphics/opengl/TextureUploadInfo.hpp>
#include <Nova/graphics/opengl/ITexture.hpp>
#include <glm/vec3.hpp>
#include <utility>
#include <span>

namespace Nova
{
	struct TextureWrapping
	{
		TextureWrapMode R = TextureWrapMode::ClampToEdge;
		TextureWrapMode S = TextureWrapMode::ClampToEdge;
		TextureWrapMode T = TextureWrapMode::ClampToEdge;
	};

	struct TextureSpec
	{
		glm::ivec3 Size;
		TextureWrapping Wrapping = {
			.R = TextureWrapMode::ClampToEdge,
			.S = TextureWrapMode::ClampToEdge,
			.T = TextureWrapMode::ClampToEdge,
		};
		InternalFormat Format;
		GLsizei Mipmaps = 1;
		TextureMinFilter MinFilter = TextureMinFilter::Nearest;
		TextureMagFilter MagFilter = TextureMagFilter::Nearest;
	};

	class Texture : public ITexture
	{
	public:
		Texture() = default;

		Texture(TextureTarget target, const TextureSpec &spec);

		constexpr Texture(Texture &&other) noexcept
			: id_(std::exchange(other.id_, 0)),
			  spec_(other.spec_),
			  target_(other.target_) {}

		Texture(const Texture &) = delete;

		~Texture() noexcept override;

		void Upload(const TextureUploadInfo &info, const void *data, bool generateMipmap) const noexcept;

		void UploadFromPBO() const noexcept;

		void Bind(GLuint unit) const noexcept;

		constexpr const TextureSpec &GetSpecification() const noexcept { return spec_; }

		constexpr GLuint GetID() const noexcept override { return id_; }

		constexpr GLsizei GetWidth() const noexcept override { return spec_.Size.x; }

		constexpr GLsizei GetHeight() const noexcept override { return spec_.Size.y; }

		constexpr GLsizei GetDepth() const noexcept { return spec_.Size.z; }

		constexpr glm::ivec3 GetSize() const noexcept { return spec_.Size; }

		constexpr GLsizei GetMipmaps() const noexcept override { return spec_.Mipmaps; }

		constexpr InternalFormat GetFormat() const noexcept override { return spec_.Format; }

		constexpr TextureTarget GetTarget() const noexcept override { return target_; }

		void SetDebugName(const std::string_view name);

		constexpr Texture &operator=(Texture &&other) noexcept
		{
			id_ = std::exchange(other.id_, 0);
			spec_ = other.spec_;
			target_ = other.target_;

			return *this;
		}

	private:
		TextureSpec spec_;
		GLuint id_ = 0;
		TextureTarget target_;
	};
}
