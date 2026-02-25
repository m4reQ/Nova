#include <Nova/graphics/Renderer.hpp>
#include <Nova/graphics/opengl/GLObject.hpp>
#include <Nova/graphics/opengl/Buffer.hpp>
#include <Nova/graphics/opengl/PersistentMappedBuffer.hpp>
#include <Nova/graphics/opengl/ShaderProgram.hpp>
#include <Nova/graphics/opengl/VertexArray.hpp>
#include <Nova/graphics/opengl/Texture.hpp>
#include <Nova/graphics/opengl/Framebuffer.hpp>
#include <Nova/graphics/opengl/Sync.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/debug/Log.hpp>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <format>

using namespace Nova;

constexpr GLuint c_MaxPointLights = 32;
constexpr GLuint c_MaxDirLights = 2;

struct PointLightData
{
	glm::vec4 Color;
	glm::vec3 Position;
	float Radius;
};

struct DirLightData
{
	glm::vec4 Color;
	glm::vec3 Direction;
};

struct LightingData
{
	PointLightData PointLightsData[c_MaxPointLights];
	DirLightData DirLightsData[c_MaxDirLights];
};

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
constexpr GLsizei c_ShadowMapWidth = 1024;
constexpr GLsizei c_ShadowMapHeight = 1024;
constexpr GLsizei c_MaxShadowCasters = 8;

static std::unordered_map<const Model*, DrawData> s_DrawData;
static std::unordered_map<MaterialID, GLuint> s_Materials;

static PersistentMappedBuffer s_InstanceBuffer;
static PersistentMappedBuffer s_CameraDataBuffer;
static PersistentMappedBuffer s_LightsBuffer;
static Buffer s_MaterialsBuffer;
static ShaderProgram s_ShaderProgram;
static ShaderProgram s_DeferredGeometryProgram;
static ShaderProgram s_DeferredLightProgram;
static VertexArray s_VertexArray;
static Texture s_WhiteTexture;
static Framebuffer s_Framebuffer;
static Sync s_FrameSync;
static Sync s_InstanceDataSync;
static RendererInfo s_RendererInfo;
static LightingData* s_LightingData;
static GLuint s_PointLightsCount;
static GLuint s_DirLightsCount;

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

static ShaderProgram CreateBasicShaderProgram()
{
	NV_PROFILE_FUNC;

	return ShaderProgram({
		ShaderStage::FromGLSL(
			ShaderType::Vertex,
			std::filesystem::path("./assets/shaders/basic.vert")),
		ShaderStage::FromGLSL(
			ShaderType::Fragment,
			std::filesystem::path("./assets/shaders/basic.frag")),
	});
}

static ShaderProgram CreateDeferredGeometryShaderProgram()
{
	NV_PROFILE_FUNC;

	return ShaderProgram({
		ShaderStage::FromGLSL(
			ShaderType::Vertex,
			std::filesystem::path("./assets/shaders/deferredGeometry.vert")),
		ShaderStage::FromGLSL(
			ShaderType::Fragment,
			std::filesystem::path("./assets/shaders/deferredGeometry.frag")),
	});
}

static ShaderProgram CreateDeferredLightingShaderProgram()
{
	NV_PROFILE_FUNC;

	return ShaderProgram({
		ShaderStage::FromGLSL(
			ShaderType::Vertex,
			std::filesystem::path("./assets/shaders/deferredLighting.vert")),
		ShaderStage::FromGLSL(
			ShaderType::Fragment,
			std::filesystem::path("./assets/shaders/deferredLighting.frag")),
	});
}

static void RetrieveRendererInfo() noexcept
{
	s_RendererInfo.VendorName = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
	s_RendererInfo.RendererName = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
	s_RendererInfo.Version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	s_RendererInfo.GLSLVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
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
    NV_PROFILE_FUNC;
	
	s_FrameSync.WaitClient(SyncTimeoutInfinite);

	s_CameraDataBuffer.Write(glm::value_ptr(view), sizeof(glm::mat4));
	s_CameraDataBuffer.Write(glm::value_ptr(projection), sizeof(glm::mat4));
	s_CameraDataBuffer.Write(glm::value_ptr(position), sizeof(glm::vec3));
	s_CameraDataBuffer.Commit();
}

const RendererInfo& Renderer::GetInfo() noexcept
{
	return s_RendererInfo;
}

void Renderer::SetViewport(const Rect<int>& viewport) noexcept
{
    NV_PROFILE_FUNC;
	
	glViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
}

void Renderer::SetViewport(
	const Rect<int>& viewport,
	const Rect<int>& scissor) noexcept
{
    NV_PROFILE_FUNC;
	
	glViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
	glScissor(scissor.X, scissor.Y, scissor.Width, scissor.Height);
}

void Renderer::AddPointLight(const glm::vec4& color, const glm::vec3& position, float radius)
{
	NV_PROFILE_FUNC;

	if (s_PointLightsCount >= c_MaxPointLights)
		return;

	auto& data = s_LightingData->PointLightsData[s_PointLightsCount];
	data.Color = color;
	data.Position = position;
	data.Radius = radius;

	s_PointLightsCount++;
}

void Renderer::AddDirectionalLight(const glm::vec4& color, const glm::vec3& direction)
{
	NV_PROFILE_FUNC;

	if (s_DirLightsCount >= c_MaxDirLights)
		return;

	auto& data = s_LightingData->DirLightsData[s_DirLightsCount];
	data.Color = color;
	data.Direction = direction;

	s_DirLightsCount++;
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

static void DrawBatch(const Model* model, DrawData& data) noexcept
{
	NV_PROFILE_FUNC;

	if (data.InstanceData.empty())
	{
		data.Age++;
		return;
	}

	s_VertexArray.BindVertexBuffer(
		model->GetModelDataBuffer(),
		c_ModelDataBufferBinding,
		sizeof(ModelVertex));

	if (model->UsesIndexBuffer())
		s_VertexArray.BindElementBuffer(model->GetIndexBuffer().value());

	s_InstanceDataSync.WaitClient(SyncTimeoutInfinite);
	s_InstanceBuffer.Write(std::span(data.InstanceData));
	s_InstanceBuffer.Commit();

	if (model->UsesIndexBuffer())
		glDrawElementsInstanced(
			model->GetPrimitiveMode(),
			model->GetIndexDataSize() / sizeof(GLuint),
			GL_UNSIGNED_INT,
			nullptr,
			data.InstanceData.size());
	else
		glDrawArraysInstanced(
			model->GetPrimitiveMode(),
			0,
			model->GetModelDataSize() / sizeof(ModelVertex),
			data.InstanceData.size());
	
	s_InstanceDataSync.Set();

	data.Age = 0;
	data.InstanceData.clear();
}

void Renderer::Draw(const glm::vec4& clearColor)
{
	NV_PROFILE_FUNC;
	
	GL::Disable(EnableCap::Blend);
	GL::Enable(EnableCap::DepthTest);
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
		BufferBaseTarget::UniformBuffer,
		s_DeferredGeometryProgram.GetResourceLocation("uCameraData"));

	s_VertexArray.Use();

	s_DeferredGeometryProgram.Use();

	for (auto& [model, drawData] : s_DrawData)
		DrawBatch(model, drawData);

	s_DeferredLightProgram.SetUniform("uAmbient", 0.3f);
	s_DeferredLightProgram.SetUniform("uPointLightsCount", s_PointLightsCount);
	s_DeferredLightProgram.SetUniform("uDirLightsCount", s_DirLightsCount);
	s_DeferredLightProgram.Use();

	s_FrameSync.WaitClient();

	s_LightsBuffer.Bind(
		BufferBaseTarget::ShaderStorageBuffer,
		s_DeferredLightProgram.GetResourceLocation("sPointLightsBuffer"),
		offsetof(LightingData, PointLightsData),
		sizeof(PointLightData) * c_MaxPointLights);

	s_LightsBuffer.Commit(
		offsetof(LightingData, PointLightsData),
		sizeof(PointLightData) * s_PointLightsCount);
	s_LightsBuffer.Commit(
		offsetof(LightingData, DirLightsData),
		sizeof(DirLightData) * s_DirLightsCount);

	s_CameraDataBuffer.Bind(
		BufferBaseTarget::UniformBuffer,
		s_DeferredLightProgram.GetResourceLocation("uCameraData"));
	
	const std::array<GLuint, 3> gBufferTextureIDs {
		s_Framebuffer.GetAttachment(0).AttachmentID,
		s_Framebuffer.GetAttachment(1).AttachmentID,
		s_Framebuffer.GetAttachment(2).AttachmentID,
	};

	Texture::BindTextures(gBufferTextureIDs);
	
	GL::Disable(EnableCap::DepthTest);
	glDepthMask(GL_FALSE);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	s_FrameSync.Set();
	s_Framebuffer.Unbind();

	s_PointLightsCount = 0;
	s_DirLightsCount = 0;
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

	RetrieveRendererInfo();

	GL::Disable(EnableCap::Multisample);
	GL::Enable(EnableCap::ScissorTest);
	GL::Enable(EnableCap::CullFace);
	GL::Enable(EnableCap::DebugOutput);
	GL::Enable(EnableCap::DebugOutputSynchronous);

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	glDebugMessageCallback(
		[](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, const void*)
		{
#ifdef _DEBUG
			if (severity == GL_DEBUG_SEVERITY_HIGH)
				assert(false && "Opengl error");
#endif

			NV_LOG_INFO("OpenGL: {}.\n", message);
		},
		nullptr);

	const Rect viewportRect { 0, 0, frameWidth, frameHeight };
	SetViewport(viewportRect, viewportRect);

	s_ShaderProgram = CreateBasicShaderProgram();
	s_DeferredGeometryProgram = CreateDeferredGeometryShaderProgram();
	s_DeferredLightProgram = CreateDeferredLightingShaderProgram();

	s_InstanceBuffer = PersistentMappedBuffer(
		sizeof(InstanceData) * 512,
		BufferAccessFlags::Writable);
	s_InstanceBuffer.SetDebugName("InstanceBuffer");
	
	s_CameraDataBuffer = PersistentMappedBuffer(
		sizeof(CameraData),
		BufferAccessFlags::Writable);
	s_CameraDataBuffer.SetDebugName("CameraBuffer");
	
	s_MaterialsBuffer = Buffer(sizeof(glm::vec4) * 32, true);
	s_MaterialsBuffer.SetDebugName("MaterialsBuffer");

	s_LightsBuffer = PersistentMappedBuffer(sizeof(LightingData), BufferAccessFlags::Writable);
	s_LightsBuffer.SetDebugName("LightsBuffer");
	s_LightingData = static_cast<LightingData*>(s_LightsBuffer.GetBasePtr());

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
			.BufferID = (BufferID)s_InstanceBuffer.GetID(),
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