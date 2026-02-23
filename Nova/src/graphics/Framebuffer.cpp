#include <Nova/graphics/opengl/Framebuffer.hpp>
#include <Nova/graphics/Window.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/core/Build.hpp>
#include <glm/gtc/type_ptr.hpp>

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

static void SetupTextureFromSpec(GLuint texture, const FramebufferAttachmentSpec& spec) noexcept
{
	NV_PROFILE_FUNC;

	GL::TextureStorage2D(texture, 1, spec.Format, spec.Width, spec.Height);
	GL::TextureParameter(texture, TextureParameterName::TextureWrapR, (GLint)TextureWrapMode::ClampToEdge);
	GL::TextureParameter(texture, TextureParameterName::TextureWrapS, (GLint)TextureWrapMode::ClampToEdge);
	GL::TextureParameter(texture, TextureParameterName::TextureWrapT, (GLint)TextureWrapMode::ClampToEdge);
	GL::TextureParameter(texture, spec.MinFilter);
	GL::TextureParameter(texture, spec.MagFilter);
}

static void SetupRenderbufferFromSpec(GLuint renderbuffer, const FramebufferAttachmentSpec& spec) noexcept
{
	NV_PROFILE_FUNC;

	glNamedRenderbufferStorage(renderbuffer, (GLenum)spec.Format, spec.Width, spec.Height);
}

Framebuffer::Framebuffer(std::span<const FramebufferAttachmentSpec> attachmentSpecs)
{
	NV_PROFILE_FUNC;

	glCreateFramebuffers(1, &id_);

	const auto attachmentsCount = (GLsizei)attachmentSpecs.size();

	attachments_.reserve(attachmentsCount);

	std::vector<GLenum> drawBuffers;
	for (GLsizei i = 0; i < attachmentsCount; i++)
	{
		const auto& spec = attachmentSpecs[i];
		const auto attachmentPoint = GetAttachmentPoint(spec.Format, i);

		GLuint attachmentID{};
		if (spec.UseRenderbuffer())
		{
			glCreateRenderbuffers(1, &attachmentID);
			SetupRenderbufferFromSpec(attachmentID, spec);
			glNamedFramebufferRenderbuffer(
				id_,
				(GLenum)attachmentPoint,
				GL_RENDERBUFFER,
				attachmentID);
		}
		else
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &attachmentID);
			SetupTextureFromSpec(attachmentID, spec);
			GL::NamedFramebufferTexture(
				id_,
				attachmentPoint,
				attachmentID,
				0);
		}
		
		if (spec.IsDrawDest() && IsColorFormat(spec.Format))
			drawBuffers.emplace_back((GLenum)attachmentPoint);

		attachments_.emplace_back(
			FramebufferAttachment{
				.Spec = spec,
				.AttachmentID = attachmentID });
	}

	glNamedFramebufferDrawBuffers(id_, (GLsizei)drawBuffers.size(), drawBuffers.data());

	const auto status = GL::CheckNamedFramebufferStatus(id_, FramebufferTarget::Framebuffer);
	if (status != FramebufferStatus::Complete)
	{
		NV_LOG_ERROR("Framebuffer {} is incomplete: {}.", id_, FramebufferStatusToString(status));
		throw std::runtime_error("Framebuffer is incomplete.");
	}
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

void Framebuffer::Delete() noexcept
{
	NV_PROFILE_FUNC;

	for (const auto& attachment : attachments_)
	{
		if (attachment.Spec.UseRenderbuffer())
			glDeleteRenderbuffers(1, &attachment.AttachmentID);
		else
			glDeleteTextures(1, &attachment.AttachmentID);
	}

	attachments_.clear();

	glDeleteFramebuffers(1, &id_);
	id_ = 0;
}


void Framebuffer::Resize(GLsizei width, GLsizei height) noexcept
{
	NV_PROFILE_FUNC;

	Delete();

	for (size_t i = 0; i < attachments_.size(); i++)
	{
		auto& attachment = attachments_[i];

		if (attachment.Spec.IsResizable() &&
			attachment.Spec.Width != width &&
			attachment.Spec.Height != height)
		{
			attachment.Spec.Width = width;
			attachment.Spec.Height = height;
			
			const auto attachmentPoint = GetAttachmentPoint(attachment.Spec.Format, i);

			if (attachment.Spec.UseRenderbuffer())
			{
				glCreateRenderbuffers(1, &attachment.AttachmentID);
				SetupRenderbufferFromSpec(attachment.AttachmentID, attachment.Spec);
				glNamedFramebufferRenderbuffer(
					id_,
					(GLenum)attachmentPoint,
					GL_RENDERBUFFER,
					attachment.AttachmentID);
			}
			else
			{
				glCreateTextures(GL_TEXTURE_2D, 1, &attachment.AttachmentID);
				SetupTextureFromSpec(attachment.AttachmentID, attachment.Spec);
				GL::NamedFramebufferTexture(
					id_,
					attachmentPoint,
					attachment.AttachmentID,
					1);
			}
		}
	}
}

void Framebuffer::Resize(const glm::ivec2& size) noexcept
{
	Resize(size.x, size.y);
}

void Framebuffer::Blit(Attachment attachment) const noexcept
{
	const auto& srcAttachment = GetAttachment(0);
	const auto [dstWidth, dstHeight] = Window::GetFramebufferSize();

	glNamedFramebufferReadBuffer(id_, (GLenum)attachment);
	glBlitNamedFramebuffer(
		id_,
		0,
		0, 0, srcAttachment.Spec.Width, srcAttachment.Spec.Height,
		0, 0, dstWidth, dstHeight,
		GL_COLOR_BUFFER_BIT,
		GL_NEAREST);
}

void Framebuffer::ClearAttachment(GLint drawBuffer, const glm::vec4& color)
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