#include <Nova/graphics/Renderer.hpp>
#include <Nova/graphics/opengl/GLObject.hpp>
#include <Nova/graphics/opengl/Buffer.hpp>
#include <Nova/graphics/opengl/ShaderProgram.hpp>
#include <Nova/graphics/opengl/VertexArray.hpp>
#include <Nova/graphics/opengl/Texture.hpp>
#include <Nova/graphics/opengl/Framebuffer.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/debug/Log.hpp>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <format>

using namespace Nova;

struct DrawCommand
{
	GLuint Count;
	GLuint InstanceCount;
	GLuint BaseIndex;
	GLint BaseVertex;
	GLuint BaseInstance;
};

struct DataOffsets
{
	GLuint VertexOffset;
	GLuint IndexOffset;
};

struct InstanceData
{
	GLuint MaterialIndex;
	glm::mat4 Transform;
	glm::mat3 NormalTransform;
};

struct CameraData
{
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;
	glm::vec3 Position;
};

struct DrawData
{
	std::vector<InstanceData> InstanceData;
	size_t Age;
};

typedef size_t ModelID;
typedef size_t MaterialID;

constexpr GLuint c_ModelDataBufferBinding = 0;
constexpr GLuint c_MaxLightSources = 32;
constexpr GLsizei c_ShadowMapWidth = 1024;
constexpr GLsizei c_ShadowMapHeight = 1024;
constexpr GLsizei c_MaxShadowCasters = 8;

static std::unordered_map<const Model*, DrawData> s_DrawData;
static std::unordered_map<MaterialID, GLuint> s_Materials;

static GLuint s_LastLightSource = 0;
static std::array<LightSource, c_MaxLightSources> s_Lights;

static Buffer s_InstanceBuffer;
static Buffer s_CameraDataBuffer;
static Buffer s_MaterialsBuffer;
static Buffer s_LightsBuffer;
static ShaderProgram s_ShaderProgram;
static ShaderProgram s_DeferredGeometryProgram;
static ShaderProgram s_DeferredLightProgram;
static VertexArray s_VertexArray;
static Texture s_WhiteTexture;
static Framebuffer s_Framebuffer;

static void CacheMaterial(const Material& material)
{
	NV_PROFILE_FUNC;

	s_Materials.insert({ material.ID, s_Materials.size() });
	s_MaterialsBuffer.Store(material.Color);
}

static void ExecuteShadowMapPass() noexcept
{
	NV_PROFILE_FUNC;

	glViewport(0, 0, c_ShadowMapWidth, c_ShadowMapHeight);
}

static GLuint GetMaterialIndex(const Material& material)
{
	NV_PROFILE_FUNC;

	const auto& materialCacheEntry = s_Materials.find(material.ID);
	if (materialCacheEntry != s_Materials.cend())
	{
		return materialCacheEntry->second;
	}

	CacheMaterial(material);
	return (GLuint)s_Materials.size() - 1;
}

void Renderer::Render(
	const Model* model,
	const Material& material,
	const glm::mat4& transform)
{
	NV_PROFILE_FUNC;

	InstanceData instanceData{
		.MaterialIndex = GetMaterialIndex(material),
		.Transform = transform,
		.NormalTransform = glm::transpose(glm::inverse(glm::mat3(transform))),
	};

	const auto& modelCacheEntry = s_DrawData.find(model);
	[[likely]] if (modelCacheEntry != s_DrawData.cend())
	{
		auto& drawData = modelCacheEntry->second;
		drawData.InstanceData.push_back(instanceData);
	}
	else
	{
		DrawData drawData{
			.InstanceData = {std::move(instanceData)},
		};

		s_DrawData.insert({ model, std::move(drawData) });
	}
}

void Renderer::SetCamera(
	const glm::mat4& view,
	const glm::mat4& projection,
	const glm::vec3& position)
{
	s_CameraDataBuffer.Store(view);
	s_CameraDataBuffer.Store(projection);
	s_CameraDataBuffer.Store(position);
	s_CameraDataBuffer.Commit();
}

void Renderer::SetViewport(const Rect<int>& viewport) noexcept
{
	glViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
}

void Renderer::SetViewport(
	const Rect<int>& viewport,
	const Rect<int>& scissor) noexcept
{
	glViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
	glScissor(scissor.X, scissor.Y, scissor.Width, scissor.Height);
}

void Renderer::AddLightSource(const LightSource &lightSource)
{
	if (s_LastLightSource >= c_MaxLightSources)
		return;
	
	s_Lights[s_LastLightSource] = lightSource;
	s_LastLightSource++;
}

void Renderer::SetClearColor(float r, float g, float b, float a) noexcept
{
	GL::ClearColor(r, g, b, a);
}

void Renderer::SetClearColor(const glm::vec4& color) noexcept
{
	GL::ClearColor(color);
}

void Renderer::Clear(ClearMask mask) noexcept
{
	NV_PROFILE_FUNC;
	GL::Clear(mask);
}

void Renderer::SetPolygonMode(PolygonMode mode) noexcept
{
	glPolygonMode(GL_FRONT_AND_BACK, (GLenum)mode);
}

void Renderer::Draw(const glm::vec4& clearColor)
{
	NV_PROFILE_FUNC;
	
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	
	s_Framebuffer.Bind();
	s_Framebuffer.ClearAttachment(0, clearColor);
	s_Framebuffer.ClearAttachment(1, glm::zero<glm::vec4>());
	s_Framebuffer.ClearAttachment(2, glm::zero<glm::vec4>());
	s_Framebuffer.ClearAttachment(3, glm::zero<glm::vec4>());
	s_Framebuffer.ClearAttachment(1.0f, 0);

	s_MaterialsBuffer.Bind(
		BindingTarget::ShaderStorageBuffer,
		s_DeferredGeometryProgram.GetResourceLocation("sMaterialData"));

	s_CameraDataBuffer.Bind(
		BindingTarget::UniformBuffer,
		s_DeferredGeometryProgram.GetResourceLocation("uCameraData"));

	s_VertexArray.Use();

	s_DeferredGeometryProgram.Use();

	for (auto& [model, drawData] : s_DrawData)
	{
		NV_PROFILE_SCOPE("::DrawBatch");

		if (drawData.InstanceData.size() == 0)
		{
			drawData.Age++;
			continue;
		}

		s_VertexArray.BindVertexBuffer(
			model->GetModelDataBuffer(),
			c_ModelDataBufferBinding,
			sizeof(ModelVertex));
		if (model->UsesIndexBuffer())
			s_VertexArray.BindElementBuffer(model->GetIndexBuffer().value());

		s_InstanceBuffer.Reset();
		s_InstanceBuffer.Store(std::span(drawData.InstanceData));

		if (model->UsesIndexBuffer())
			glDrawElementsInstanced(
				model->GetPrimitiveMode(),
				model->GetIndexDataSize() / sizeof(GLuint),
				GL_UNSIGNED_INT,
				nullptr,
				drawData.InstanceData.size());
		else
			glDrawArraysInstanced(
				model->GetPrimitiveMode(),
				0,
				model->GetModelDataSize() / sizeof(ModelVertex),
				drawData.InstanceData.size());

		drawData.Age = 0;
		drawData.InstanceData.clear();
	}

	s_DeferredLightProgram.SetUniform("uAmbient", 0.3f);
	s_DeferredLightProgram.SetUniform("uLightsCount", s_LastLightSource);
	s_DeferredLightProgram.Use();

	s_LightsBuffer.Bind(
		BindingTarget::ShaderStorageBuffer,
		s_DeferredLightProgram.GetResourceLocation("sLightData"));
	s_LightsBuffer.Reset();
	s_LightsBuffer.Store(s_Lights.data(), sizeof(LightSource) * s_LastLightSource);

	s_CameraDataBuffer.Bind(
		BindingTarget::UniformBuffer,
		s_DeferredLightProgram.GetResourceLocation("uCameraData"));
	
	s_LastLightSource = 0;

	const std::array<GLuint, 3> gBufferTextureIDs {
		s_Framebuffer.GetAttachment(0).AttachmentID,
		s_Framebuffer.GetAttachment(1).AttachmentID,
		s_Framebuffer.GetAttachment(2).AttachmentID,
	};

	Texture::BindTextures(gBufferTextureIDs);
	
	glDrawArrays(GL_TRIANGLES, 0, 6);

	s_Framebuffer.Unbind();
}

GLuint Renderer::GetRenderTextureID(RenderTexture texture) noexcept
{
	return s_Framebuffer.GetAttachment((size_t)texture).AttachmentID;
}

void Renderer::_Initialize(
	int frameWidth,
	int frameHeight,
	const std::filesystem::path& shaderCacheDirectory,
	GLADloadfunc getProcAddressFunc)
{
	NV_PROFILE_FUNC;

	if (!gladLoadGL(getProcAddressFunc))
		throw std::runtime_error("Failed to load OpenGL bindings.");

	glFrontFace(GL_CCW);
	glDisable(GL_MULTISAMPLE);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDebugMessageCallback(
		[](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, const void*)
		{
			NV_LOG_INFO("OpenGL: {}.\n", message);
		},
		nullptr);

	const Rect viewportRect { 0, 0, frameWidth, frameHeight };
	SetViewport(viewportRect, viewportRect);

	s_ShaderProgram = ShaderProgram({
		ShaderStage::FromGLSL(
			ShaderType::Vertex,
			std::filesystem::path("./assets/shaders/basic.vert")),
		ShaderStage::FromGLSL(
			ShaderType::Fragment,
			std::filesystem::path("./assets/shaders/basic.frag")),
	});

	s_DeferredGeometryProgram = ShaderProgram({
		ShaderStage::FromGLSL(
			ShaderType::Vertex,
			std::filesystem::path("./assets/shaders/deferredGeometry.vert")),
		ShaderStage::FromGLSL(
			ShaderType::Fragment,
			std::filesystem::path("./assets/shaders/deferredGeometry.frag")),
	});

	s_DeferredLightProgram = ShaderProgram({
		ShaderStage::FromGLSL(
			ShaderType::Vertex,
			std::filesystem::path("./assets/shaders/deferredLighting.vert")),
		ShaderStage::FromGLSL(
			ShaderType::Fragment,
			std::filesystem::path("./assets/shaders/deferredLighting.frag")),
	});

	s_InstanceBuffer = Buffer(sizeof(InstanceData) * 2048, true);
	s_InstanceBuffer.SetDebugName("InstanceBuffer");
	
	s_CameraDataBuffer = Buffer(sizeof(glm::mat4) * 2, true);
	s_CameraDataBuffer.Bind(BindingTarget::UniformBuffer, s_DeferredGeometryProgram.GetResourceLocation("uCameraData"));
	s_CameraDataBuffer.SetDebugName("CameraBuffer");
	
	s_MaterialsBuffer = Buffer(sizeof(glm::vec4) * 32, true);
	s_MaterialsBuffer.SetDebugName("MaterialsBuffer");

	s_LightsBuffer = Buffer(sizeof(LightSource) * c_MaxLightSources, true);
	s_LightsBuffer.SetDebugName("LightsBuffer");

	s_VertexArray = VertexArray({
		VertexInput {
			.Stride = sizeof(ModelVertex),
			.Descriptors = {
				VertexDescriptor {
					.AttributeIndex = s_DeferredGeometryProgram.GetResourceLocation("inPosition"),
					.AttributeType = AttributeType::Float,
					.Count = 3,
				},
				VertexDescriptor {
					.AttributeIndex = s_DeferredGeometryProgram.GetResourceLocation("inNormal"),
					.AttributeType = AttributeType::Float,
					.Count = 3,
				},
				// VertexDescriptor {
				// 	.AttributeIndex = s_DeferredGeometryProgram.GetResourceLocation("inTexCoord"),
				// 	.AttributeType = AttributeType::Float,
				// 	.Count = 2,
				// },
			},
		},
		VertexInput {
			.Stride = sizeof(InstanceData),
			.Descriptors = {
				VertexDescriptor {
					.AttributeIndex = s_DeferredGeometryProgram.GetResourceLocation("inMaterialIndex"),
					.AttributeType = AttributeType::UnsignedInt,
					.Count = 1
				},
				VertexDescriptor {
					.AttributeIndex = s_DeferredGeometryProgram.GetResourceLocation("inTransform"),
					.AttributeType = AttributeType::Float,
					.Count = 4,
					.Rows = 4,
				},
				VertexDescriptor {
					.AttributeIndex = s_DeferredGeometryProgram.GetResourceLocation("inNormalTransform"),
					.AttributeType = AttributeType::Float,
					.Count = 3,
					.Rows = 3,
				},
			},
			.BufferID = s_InstanceBuffer.GetID(),
			.InstanceDivisor = 1,
		},
	});

	s_Framebuffer = Framebuffer({
		FramebufferAttachmentSpec {
			.Width = frameWidth,
			.Height = frameHeight,
			.Format = InternalFormat::RGBA8,
			.Flags = AttachmentFlags::Resizable | AttachmentFlags::DrawDest,
		}, // color + specular attachment
		FramebufferAttachmentSpec {
			.Width = frameWidth,
			.Height = frameHeight,
			.Format = InternalFormat::RGB16F,
			.Flags = AttachmentFlags::Resizable | AttachmentFlags::DrawDest,
		}, // position attachment
		FramebufferAttachmentSpec {
			.Width = frameWidth,
			.Height = frameHeight,
			.Format = InternalFormat::RGB16F,
			.Flags = AttachmentFlags::Resizable | AttachmentFlags::DrawDest,
		}, // normal attachment
		FramebufferAttachmentSpec {
			.Width = frameWidth,
			.Height = frameHeight,
			.Format = InternalFormat::Depth24Stencil8,
			.Flags = AttachmentFlags::Resizable | AttachmentFlags::UseRenderbuffer,
		}, // depth attachment
		FramebufferAttachmentSpec {
			.Width = frameWidth,
			.Height = frameHeight,
			.Format = InternalFormat::RGB8,
			.Flags = AttachmentFlags::Resizable | AttachmentFlags::DrawDest,
		}, // final output
	});
}

void Renderer::DisplayFramebuffer() noexcept
{
	s_Framebuffer.Blit(Attachment::Color0);
}

void Renderer::_Initialize(
	std::pair<int, int> frameSize,
	const std::filesystem::path& shaderCacheDirectory,
	GLADloadfunc getProcAddressFunc)
{
	_Initialize(
		frameSize.first,
		frameSize.second,
		shaderCacheDirectory,
		getProcAddressFunc);
}

void Renderer::_Shutdown()
{
	_GLObjectBase::DeleteAll();
}