#pragma once
#include <Nova/graphics/opengl/FramebufferAttachment.hpp>
#include <vector>
#include <span>
#include <initializer_list>

namespace Nova
{
	class Framebuffer
	{
	public:
		Framebuffer() = default;

		Framebuffer(const Framebuffer &) = delete;

		constexpr Framebuffer(Framebuffer &&other) noexcept
			: id_(std::exchange(other.id_, 0)),
			  attachments_(std::move(other.attachments_)) {}

		Framebuffer(std::span<const FramebufferAttachmentSpec> attachmentSpecs);

		Framebuffer(std::initializer_list<FramebufferAttachmentSpec> attachmentSpecs)
			: Framebuffer(std::span(attachmentSpecs)) {}

		// TODO Add destructor for framebuffer once correct destruction order is figured out
		~Framebuffer() noexcept;

		void Bind() const noexcept;

		void Unbind() const noexcept;

		void Resize(GLsizei width, GLsizei height) noexcept;

		void Resize(const glm::ivec2 &size) noexcept;

		void Blit(Attachment attachment, unsigned int width, unsigned int height) const noexcept;

		void ClearAttachment(GLint drawBuffer, const glm::vec4 &color);

		void ClearAttachment(GLint drawBuffer, float r, float g, float b, float a);

		void ClearAttachment(GLfloat depth);

		void ClearAttachment(GLint stencil);

		void ClearAttachment(GLfloat depth, GLint stencil);

		void Invalidate(std::span<const GLenum> attachments) const noexcept;

		void Invalidate(std::initializer_list<GLenum> attachments) const noexcept;

		constexpr const std::vector<FramebufferAttachment> &GetAttachments() const noexcept { return attachments_; }

		constexpr const FramebufferAttachment &GetAttachment(size_t index) const { return attachments_[index]; }

		Framebuffer &operator=(const Framebuffer &) = delete;

		// TODO Make default move constructors zero out IDs
		constexpr Framebuffer &operator=(Framebuffer &&other) noexcept
		{
			id_ = std::exchange(other.id_, 0);
			attachments_ = std::move(other.attachments_);

			return *this;
		}

		constexpr GLuint GetID() const noexcept { return id_; }

	private:
		std::vector<FramebufferAttachment> attachments_;
		GLuint id_ = 0;
	};
}