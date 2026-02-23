#pragma once
#include <Nova/graphics/opengl/GL.hpp>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <span>
#include <initializer_list>

namespace Nova
{
	enum class AttachmentFlags
	{
		Default = 0,
		Resizable = 1,
		DrawDest = 2,
		UseRenderbuffer = 8,
	};

	constexpr AttachmentFlags operator&(AttachmentFlags a, AttachmentFlags b) noexcept
	{
		return (AttachmentFlags)((std::underlying_type_t<AttachmentFlags>)a & (std::underlying_type_t<AttachmentFlags>)b);
	}

	constexpr AttachmentFlags operator|(AttachmentFlags a, AttachmentFlags b) noexcept
	{
		return (AttachmentFlags)((std::underlying_type_t<AttachmentFlags>)a | (std::underlying_type_t<AttachmentFlags>)b);
	}

	constexpr AttachmentFlags operator^(AttachmentFlags a, AttachmentFlags b) noexcept
	{
		return (AttachmentFlags)((std::underlying_type_t<AttachmentFlags>)a ^ (std::underlying_type_t<AttachmentFlags>)b);
	}

	constexpr bool IsFlagSet(AttachmentFlags flags, AttachmentFlags x) noexcept
	{
		return (flags & x) == x;
	}

	struct FramebufferAttachmentSpec
	{
		GLsizei Width;
		GLsizei Height;
		InternalFormat Format;
		AttachmentFlags Flags = AttachmentFlags::Default;
		TextureMinFilter MinFilter = TextureMinFilter::Nearest;
		TextureMagFilter MagFilter = TextureMagFilter::Nearest;

		constexpr bool IsResizable() const noexcept
		{
			return IsFlagSet(Flags, AttachmentFlags::Resizable);
		}

		constexpr bool IsDrawDest() const noexcept
		{
			return IsFlagSet(Flags, AttachmentFlags::DrawDest);
		}

		constexpr bool UseRenderbuffer() const noexcept
		{
			return IsFlagSet(Flags, AttachmentFlags::UseRenderbuffer);
		}
		
		constexpr glm::ivec2 GetSize() const noexcept
		{
			return { Width, Height };
		}
	};

	struct FramebufferAttachment
	{
		FramebufferAttachmentSpec Spec;
		GLuint AttachmentID;
	};

	class Framebuffer
	{
	public:
		Framebuffer() = default;
		
		Framebuffer(const Framebuffer&) = delete;
		
		constexpr Framebuffer(Framebuffer&& other) noexcept
			: id_(std::exchange(other.id_, 0)), attachments_(std::move(other.attachments_)) { }

		Framebuffer(std::span<const FramebufferAttachmentSpec> attachmentSpecs);
		
		Framebuffer(std::initializer_list< FramebufferAttachmentSpec> attachmentSpecs)
			: Framebuffer(std::span(attachmentSpecs)) { }

		// TODO Add destructor for framebuffer once correct destruction order is figured out
		// ~Framebuffer() noexcept;
		void Delete() noexcept;
		
		void Bind() const noexcept;
		
		void Unbind() const noexcept;

		void Resize(GLsizei width, GLsizei height) noexcept;

		void Resize(const glm::ivec2& size) noexcept;

		void Blit(Attachment attachment) const noexcept;

		void ClearAttachment(GLint drawBuffer, const glm::vec4& color);

		void ClearAttachment(GLint drawBuffer, float r, float g, float b, float a);

		void ClearAttachment(GLfloat depth);

		void ClearAttachment(GLint stencil);

		void ClearAttachment(GLfloat depth, GLint stencil);

		constexpr const FramebufferAttachment& GetAttachment(size_t index) const { return attachments_[index]; }

		Framebuffer& operator=(const Framebuffer&) = delete;
		
		constexpr Framebuffer& operator=(Framebuffer&& other) noexcept
		{
			id_ = std::exchange(other.id_, 0);
			attachments_ = std::move(other.attachments_);

			return *this;
		}

		constexpr GLuint GetID() const noexcept { return id_; }

	private:
		std::vector<FramebufferAttachment> attachments_;
		GLuint id_;
	};
}