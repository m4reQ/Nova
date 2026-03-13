#pragma once
#include <Nova/graphics/opengl/GL.hpp>
#include <Nova/graphics/opengl/FramebufferAttachment.hpp>
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
		bool AllowBindless = false;
	};

	struct TextureUploadInfo
	{
		glm::ivec3 Size;
		glm::ivec3 Offset;
		GLint Mipmap;
		PixelFormat PixelFormat;
		PixelType PixelType;
	};

	class Texture : public IFramebufferAttachment
	{
	public:
		Texture() = default;

		Texture(TextureTarget target, const TextureSpec &spec);

		constexpr Texture(Texture &&other) noexcept
			: id_(std::exchange(other.id_, 0)),
			  bindlessHandle_(std::exchange(other.bindlessHandle_, 0)),
			  spec_(other.spec_),
			  target_(other.target_) {}

		Texture(const Texture &) = delete;

		~Texture() noexcept;

		void Upload(const TextureUploadInfo &info, const void *data) const noexcept;

		void UploadFromPBO() const noexcept;

		void Bind(GLuint unit) const noexcept;

		void MakeResident() const noexcept;

		void MakeNonResident() const noexcept;

		constexpr const TextureSpec &GetSpecification() const noexcept { return spec_; }

		constexpr GLuint GetID() const noexcept override { return id_; }

		constexpr GLuint64 GetBindlessHandle() const noexcept { return bindlessHandle_; }

		constexpr bool SupportsBindless() const noexcept { return bindlessHandle_ != 0; }

		constexpr GLsizei GetWidth() const noexcept override { return spec_.Size.x; }

		constexpr GLsizei GetHeight() const noexcept override { return spec_.Size.y; }

		constexpr glm::ivec3 GetSize() const noexcept { return spec_.Size; }

		constexpr GLsizei GetDepth() const noexcept { return spec_.Size.z; }

		constexpr TextureTarget GetTarget() const noexcept { return target_; }

		constexpr InternalFormat GetFormat() const noexcept override { return spec_.Format; }

		constexpr operator GLuint() const noexcept { return id_; }

		constexpr Texture &operator=(Texture &&other) noexcept
		{
			id_ = std::exchange(other.id_, 0);
			bindlessHandle_ = std::exchange(other.bindlessHandle_, 0);
			spec_ = other.spec_;
			target_ = other.target_;

			return *this;
		}

	private:
		TextureSpec spec_;
		GLuint64 bindlessHandle_ = 0;
		GLuint id_ = 0;
		TextureTarget target_;
	};
}
