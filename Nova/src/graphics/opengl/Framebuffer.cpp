#include <Nova/graphics/opengl/Framebuffer.hpp>
#include <Nova/graphics/Window.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/core/Build.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h>

using namespace Nova;

static constexpr const std::string_view FramebufferStatusToString(FramebufferStatus status)
{
	switch (status)
	{
	case FramebufferStatus::Complete:
		return "Complete";
	case FramebufferStatus::Undefined:
		return "Undefined";
	case FramebufferStatus::IncompleteAttachment:
		return "IncompleteAttachment";
	case FramebufferStatus::IncompleteMissingAttachment:
		return "IncompleteMissingAttachment";
	case FramebufferStatus::IncompleteDrawBuffer:
		return "IncompleteDrawBuffer";
	case FramebufferStatus::IncompleteReadBuffer:
		return "IncompleteReadBuffer";
	case FramebufferStatus::Unsupported:
		return "Unsupported";
	case FramebufferStatus::IncompleteMultisample:
		return "IncompleteMultisample";
	case FramebufferStatus::IncompleteLayerTargets:
		return "IncompleteLayerTargets";
	}

	NV_UNREACHABLE;
}

static constexpr bool IsColorFormat(InternalFormat format) noexcept
{
	switch (format)
	{
	case InternalFormat::DepthComponent32F:
	case InternalFormat::DepthComponent24:
	case InternalFormat::DepthComponent16:
	case InternalFormat::Depth32FStencil8:
	case InternalFormat::Depth24Stencil8:
	case InternalFormat::StencilIndex8:
		return false;
	default:
		return true;
	}
}

static constexpr Attachment GetAttachmentPoint(InternalFormat format, size_t index) noexcept
{
	switch (format)
	{
	case InternalFormat::DepthComponent32F:
	case InternalFormat::DepthComponent24:
	case InternalFormat::DepthComponent16:
		return Attachment::Depth;
	case InternalFormat::Depth32FStencil8:
	case InternalFormat::Depth24Stencil8:
		return Attachment::DepthStencil;
	case InternalFormat::StencilIndex8:
		return Attachment::Stencil;
	default:
		return Attachment::Color0 + index;
	}
}

static void CheckFramebufferStatus(GLuint framebufferID)
{
	const auto status = GL::CheckNamedFramebufferStatus(framebufferID, FramebufferTarget::Framebuffer);
	if (status != FramebufferStatus::Complete)
	{
		NV_LOG_ERROR("Framebuffer {} is incomplete: {}.", framebufferID, FramebufferStatusToString(status));
		throw std::runtime_error("Framebuffer is incomplete.");
	}
}

static std::vector<FramebufferAttachment> CreateAndAttachAttachments(std::span<const FramebufferAttachmentSpec> attachmentSpecs, GLuint framebufferID)
{
	NV_PROFILE_FUNC;

	std::vector<FramebufferAttachment> attachments;
	attachments.reserve(attachmentSpecs.size());

	std::vector<GLenum> drawBuffers;
	for (auto i = 0zu; i < attachmentSpecs.size(); i++)
	{
		const auto &spec = attachmentSpecs[i];
		const auto attachmentPoint = GetAttachmentPoint(spec.Format, i);

		const auto &attachment = attachments.emplace_back(spec);
		attachment.Attach(framebufferID, attachmentPoint);

		if (spec.IsDrawDest() && IsColorFormat(spec.Format))
			drawBuffers.emplace_back(static_cast<GLenum>(attachmentPoint));
	}

	assert_fits_in<GLsizei>(drawBuffers.size());

	glNamedFramebufferDrawBuffers(
		framebufferID,
		static_cast<GLsizei>(drawBuffers.size()),
		drawBuffers.data());

	return attachments;
}

Framebuffer::Framebuffer(std::span<const FramebufferAttachmentSpec> attachmentSpecs)
{
	NV_PROFILE_FUNC;

	glCreateFramebuffers(1, &id_);

	attachments_ = CreateAndAttachAttachments(attachmentSpecs, id_);

	CheckFramebufferStatus(id_);
}

Nova::Framebuffer::~Framebuffer() noexcept
{
	if (id_)
		glDeleteFramebuffers(1, &id_);
}

void Framebuffer::Bind() const noexcept
{
	NV_PROFILE_FUNC;

	glBindFramebuffer(GL_FRAMEBUFFER, id_);
}

void Framebuffer::Unbind() const noexcept
{
	NV_PROFILE_FUNC;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Resize(GLsizei width, GLsizei height) noexcept
{
	NV_PROFILE_FUNC;

	glDeleteFramebuffers(1, &id_);
	glCreateFramebuffers(1, &id_);

	// FIX Unneeded allocation of vector just to be passed into function (do this in one loop?)
	std::vector<FramebufferAttachmentSpec> specs;
	specs.reserve(attachments_.size());

	for (const auto &attachment : attachments_)
	{
		auto spec = attachment.GetSpec();
		spec.Width = width;
		spec.Height = height;

		specs.emplace_back(spec);
	}

	attachments_ = CreateAndAttachAttachments(specs, id_);
}

void Framebuffer::Resize(const glm::ivec2 &size) noexcept
{
	Resize(size.x, size.y);
}

void Framebuffer::Blit(Attachment attachment, unsigned int dstWidth, unsigned int dstHeight) const noexcept
{
	const auto &srcAttachment = GetAttachment(0);

	glNamedFramebufferReadBuffer(id_, (GLenum)attachment);
	glBlitNamedFramebuffer(
		id_,
		0,
		0, 0, srcAttachment.GetWidth(), srcAttachment.GetHeight(),
		0, 0, static_cast<GLint>(dstWidth), static_cast<GLint>(dstHeight),
		GL_COLOR_BUFFER_BIT,
		GL_NEAREST);
}

void Framebuffer::ClearAttachment(GLint drawBuffer, const glm::vec4 &color)
{
	glClearNamedFramebufferfv(id_, GL_COLOR, drawBuffer, glm::value_ptr(color));
}

void Framebuffer::ClearAttachment(GLint drawBuffer, float r, float g, float b, float a)
{
	ClearAttachment(drawBuffer, glm::vec4(r, g, b, a));
}

void Framebuffer::ClearAttachment(GLfloat depth)
{
	glClearNamedFramebufferfv(id_, GL_DEPTH, 0, &depth);
}

void Framebuffer::ClearAttachment(GLint stencil)
{
	glClearNamedFramebufferiv(id_, GL_STENCIL, 0, &stencil);
}

void Framebuffer::ClearAttachment(GLfloat depth, GLint stencil)
{
	glClearNamedFramebufferfi(id_, GL_DEPTH_STENCIL, 0, depth, stencil);
}

void Framebuffer::Invalidate(std::span<const GLenum> attachments) const noexcept
{
	assert_fits_in<GLsizei>(attachments.size());
	glInvalidateNamedFramebufferData(
		id_,
		static_cast<GLsizei>(attachments.size()),
		attachments.data());
}

void Framebuffer::Invalidate(std::initializer_list<GLenum> attachments) const noexcept
{
	Invalidate(std::span(attachments));
}