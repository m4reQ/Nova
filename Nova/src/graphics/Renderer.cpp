#include <Nova/graphics/Renderer.hpp>
#include <Nova/graphics/opengl/GLObject.hpp>
#include <Nova/graphics/opengl/Buffer.hpp>
#include <Nova/graphics/opengl/PersistentMappedBuffer.hpp>
#include <Nova/graphics/opengl/ShaderProgram.hpp>
#include <Nova/graphics/opengl/VertexArray.hpp>
#include <Nova/graphics/opengl/Texture.hpp>
#include <Nova/graphics/opengl/Framebuffer.hpp>
#include <Nova/graphics/opengl/Sync.hpp>
#include <Nova/graphics/opengl/AlignedType.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/core/Utility.hpp>
#include <xxhash.h>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <format>

#ifdef _DEBUG
#define BREAK_ON_HIGH_SEVERITY(severity) assert((severity != GL_DEBUG_SEVERITY_HIGH) && "OpenGL error")
#else
#define BREAK_ON_HIGH_SEVERITY(severity)
#endif

using namespace Nova;

constexpr auto MaterialHashSeed = 41925337ui64;

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
	float _Padding[1];
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
	float _Padding[1];
};

struct DrawData
{
	std::vector<InstanceData> OpaqueInstanceData;
	std::vector<InstanceData> TransparentInstanceData;
	size_t Age;
};

typedef size_t ModelID;
typedef size_t MaterialID;

constexpr GLuint c_ModelDataBufferBinding = 0;
constexpr GLsizei c_ShadowMapWidth = 1024;
constexpr GLsizei c_ShadowMapHeight = 1024;
constexpr GLsizei c_MaxShadowCasters = 8;

static std::unordered_map<const Model*, DrawData> s_DrawData;

static RendererInfo s_RendererInfo;

// materials
static PersistentMappedBuffer s_MaterialsBuffer;
static std::unordered_map<Material, GLuint, XXHasher<Material>> s_Materials;

// lights
static PersistentMappedBuffer s_LightsBuffer;

static PointLightData* s_PointLights;
static GLuint s_PointLightsCount;
static GLuint s_MaxPointLightsCount;

static DirLightData* s_DirLights;
static GLuint s_DirLightsCount;
static GLuint s_MaxDirLightsCount;

static PersistentMappedBuffer s_CameraDataBuffer;

static PersistentMappedBuffer s_InstanceBuffer;
static ShaderProgram s_DeferredGeometryProgram;
static ShaderProgram s_DeferredLightProgram;
static ShaderProgram s_DeferredTransparentProgram;
static VertexArray s_VertexArray;
static Texture s_WhiteTexture;
static Framebuffer s_Framebuffer;
static Sync s_FrameSync; // guards materials buffer, lights buffer and camera data buffer
static Sync s_InstanceDataSync; // guards instance data buffer between each batch draw
static GLsizei s_CurrentDisplayWidth;
static GLsizei s_CurrentDisplayHeight;

static glm::vec3 s_CameraPosition;

static void ExecuteShadowMapPass() noexcept
{
	NV_PROFILE_FUNC;

	glViewport(0, 0, c_ShadowMapWidth, c_ShadowMapHeight);
}

static GLuint GetMaterialIndex(const Material& material)
{
	NV_PROFILE_FUNC;

	const auto it = s_Materials.find(material);
	if (it != s_Materials.end())
	{
		return it->second;
	}
	else
	{
		const auto dataPtr = s_MaterialsBuffer.GetDataPtr<Material>();
		const auto basePtr = s_MaterialsBuffer.GetBasePtr<Material>();
		const auto materialIndex = dataPtr - basePtr;

		s_MaterialsBuffer.Write(&material, sizeof(Material));
		s_Materials.emplace(material, (GLuint)materialIndex);

		return materialIndex;
	}
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

static ShaderProgram CreateDeferredTransparentShaderProgram()
{
	return ShaderProgram({
		ShaderStage::FromGLSL(
			ShaderType::Vertex,
			std::filesystem::path("./assets/shaders/deferredTransparent.vert")),
		ShaderStage::FromGLSL(
			ShaderType::Fragment,
			std::filesystem::path("./assets/shaders/deferredTransparent.frag")),
	});
}

static void RetrieveRendererInfo() noexcept
{
	s_RendererInfo.VendorName = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
	s_RendererInfo.RendererName = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
	s_RendererInfo.Version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	s_RendererInfo.GLSLVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
}

static glm::mat3 BuildNormalTransformMatrix(const glm::mat4& transform) noexcept
{
	NV_PROFILE_FUNC;
	return glm::transpose(glm::inverse(glm::mat3(transform)));
}

static std::vector<InstanceData>& GetModelInstanceDataStore(const Model* model, bool useTransparency)
{
	NV_PROFILE_FUNC;
	
	const auto& it = s_DrawData.find(model);
	if (it != s_DrawData.end())
		return useTransparency
			? it->second.TransparentInstanceData
			: it->second.OpaqueInstanceData;
	
	const auto& [data, _] = s_DrawData.emplace(model, DrawData { .Age = 0 });
	return useTransparency
		? data->second.TransparentInstanceData
		: data->second.OpaqueInstanceData;
}

void Renderer::Render(
	const Model* model,
	const Material& material,
	const glm::mat4& transform)
{
	NV_PROFILE_FUNC;

	auto& instanceDataStore = GetModelInstanceDataStore(model, !glm::epsilonEqual(material.Color.a, 1.0f, glm::epsilon<float>()));
	instanceDataStore.emplace_back(
		InstanceData {
			.MaterialIndex = GetMaterialIndex(material),
			.Transform = transform,
			.NormalTransform = BuildNormalTransformMatrix(transform),
		});
}

void Renderer::SetCamera(
	const glm::mat4& view,
	const glm::mat4& projection,
	const glm::vec3& position)
{
    NV_PROFILE_FUNC;

	auto cameraData = s_CameraDataBuffer.GetDataPtr<CameraData>();
	cameraData->ViewMatrix = view;
	cameraData->ProjectionMatrix = projection;
	cameraData->Position = position;

	s_CameraPosition = position;
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

	if (s_PointLightsCount >= s_MaxPointLightsCount)
		return;

	auto& data = s_PointLights[s_PointLightsCount++];
	data.Color = color;
	data.Position = position;
	data.Radius = radius;
}

void Renderer::AddDirectionalLight(const glm::vec4& color, const glm::vec3& direction)
{
	NV_PROFILE_FUNC;

	if (s_DirLightsCount >= s_MaxDirLightsCount)
		return;

	auto& data = s_DirLights[s_DirLightsCount++];
	data.Color = color;
	data.Direction = direction;
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

static void DrawBatch(const Model* model, const std::span<InstanceData> instanceData) noexcept
{
	NV_PROFILE_FUNC;

	s_VertexArray.BindVertexBuffer(
		model->GetModelDataBuffer(),
		c_ModelDataBufferBinding,
		sizeof(ModelVertex));

	if (model->UsesIndexBuffer())
		s_VertexArray.BindElementBuffer(model->GetIndexBuffer().value());

	s_InstanceDataSync.WaitClient(SyncTimeoutInfinite);
	
	s_InstanceBuffer.Write(instanceData);
	s_InstanceBuffer.Commit();

	if (model->UsesIndexBuffer())
		glDrawElementsInstanced(
			model->GetPrimitiveMode(),
			model->GetIndexDataSize() / sizeof(GLuint),
			GL_UNSIGNED_INT,
			nullptr,
			instanceData.size());
	else
		glDrawArraysInstanced(
			model->GetPrimitiveMode(),
			0,
			model->GetModelDataSize() / sizeof(ModelVertex),
			instanceData.size());
	
	s_InstanceDataSync.Set();
}

static void SortTransparentObjects(std::span<InstanceData> instanceData) noexcept
{
	NV_PROFILE_FUNC;

	std::sort(
		instanceData.begin(),
		instanceData.end(),
		[](const InstanceData& a, const InstanceData& b)
		{
			const auto aDistance = glm::distance(glm::vec3(a.Transform[3]), s_CameraPosition);
			const auto bDistance = glm::distance(glm::vec3(b.Transform[3]), s_CameraPosition);
			return aDistance > bDistance;
		});
}

static void ExecuteGeometryPass() noexcept
{
	NV_PROFILE_FUNC;

	s_MaterialsBuffer.Bind(
		BufferBaseTarget::ShaderStorageBuffer,
		s_DeferredGeometryProgram.GetResourceLocation("sMaterialData"));
	
	s_CameraDataBuffer.Bind(
		BufferBaseTarget::UniformBuffer,
		s_DeferredGeometryProgram.GetResourceLocation("uCameraData"));

	s_VertexArray.Use();

	s_DeferredGeometryProgram.Use();

	GL::Disable(EnableCap::Blend);
	GL::Enable(EnableCap::DepthTest);
	GL::DepthFunc(DepthFunction::Less);

	for (auto& [model, drawData] : s_DrawData)
	{
		DrawBatch(model, drawData.OpaqueInstanceData);
		drawData.OpaqueInstanceData.clear();
	}
}

static void ExecuteLightingPass() noexcept
{
	NV_PROFILE_FUNC;

	s_DeferredLightProgram.SetUniform("uAmbient", 0.3f);
	s_DeferredLightProgram.SetUniform("uShininess", 86.0f);
	s_DeferredLightProgram.SetUniform("uPointLightsCount", s_PointLightsCount);
	s_DeferredLightProgram.SetUniform("uDirLightsCount", s_DirLightsCount);
	s_DeferredLightProgram.Use();
	
	s_LightsBuffer.Bind(
		BufferBaseTarget::ShaderStorageBuffer,
		s_DeferredLightProgram.GetResourceLocation("sPointLightsBuffer"),
		0,
		sizeof(PointLightData) * s_MaxPointLightsCount);
	s_LightsBuffer.Bind(
		BufferBaseTarget::ShaderStorageBuffer,
		s_DeferredLightProgram.GetResourceLocation("sDirLightsBuffer"),
		sizeof(PointLightData) * s_MaxPointLightsCount,
		sizeof(DirLightData) * s_MaxDirLightsCount);
	
	s_CameraDataBuffer.Bind(
		BufferBaseTarget::UniformBuffer,
		s_DeferredLightProgram.GetResourceLocation("uCameraData"));

	const std::array<GLuint, 3> gBufferTextureIDs {
		s_Framebuffer.GetAttachment(0).AttachmentID,
		s_Framebuffer.GetAttachment(1).AttachmentID,
		s_Framebuffer.GetAttachment(2).AttachmentID,
	};

	glBindTextures(0, gBufferTextureIDs.size(), gBufferTextureIDs.data());

	GL::Disable(EnableCap::DepthTest);
	GL::DepthMask(false);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void ExecuteTransparentPass() noexcept
{
	NV_PROFILE_FUNC;

	s_DeferredTransparentProgram.SetUniform("uAmbient", 0.3f);
	s_DeferredTransparentProgram.SetUniform("uShininess", 86.0f);
	s_DeferredTransparentProgram.SetUniform("uPointLightsCount", s_PointLightsCount);
	s_DeferredTransparentProgram.SetUniform("uDirLightsCount", s_DirLightsCount);
	s_DeferredTransparentProgram.Use();

	s_VertexArray.Use();

	s_CameraDataBuffer.Bind(
		BufferBaseTarget::UniformBuffer,
		s_DeferredTransparentProgram.GetResourceLocation("uCameraData"));
	
	s_LightsBuffer.Bind(
		BufferBaseTarget::ShaderStorageBuffer,
		s_DeferredTransparentProgram.GetResourceLocation("sPointLightsBuffer"),
		0,
		sizeof(PointLightData) * s_MaxPointLightsCount);
	s_LightsBuffer.Bind(
		BufferBaseTarget::ShaderStorageBuffer,
		s_DeferredTransparentProgram.GetResourceLocation("sDirLightsBuffer"),
		sizeof(PointLightData) * s_MaxPointLightsCount,
		sizeof(DirLightData) * s_MaxDirLightsCount);

	GL::Enable(EnableCap::DepthTest);
	GL::DepthFunc(DepthFunction::LessEqual);
	GL::DepthMask(false);

	GL::Enable(EnableCap::Blend);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (auto& [model, drawData] : s_DrawData)
	{
		SortTransparentObjects(drawData.TransparentInstanceData);
		DrawBatch(model, drawData.TransparentInstanceData);

		drawData.TransparentInstanceData.clear();
	}
}

void Renderer::Draw(const glm::vec4& clearColor)
{
	NV_PROFILE_FUNC;

	GL::DepthMask(true);
	
	s_Framebuffer.Resize(s_CurrentDisplayWidth, s_CurrentDisplayHeight);

	s_Framebuffer.Bind();
	s_Framebuffer.ClearAttachment(0, clearColor);
	s_Framebuffer.ClearAttachment(1, glm::zero<glm::vec4>());
	s_Framebuffer.ClearAttachment(2, glm::zero<glm::vec4>());
	s_Framebuffer.ClearAttachment(3, glm::zero<glm::vec4>());
	s_Framebuffer.ClearAttachment(1.0f, 0);

	s_FrameSync.WaitClient(SyncTimeoutInfinite);

	s_CameraDataBuffer.Commit();
	s_MaterialsBuffer.Commit();
	s_LightsBuffer.Commit(
		0,
		sizeof(PointLightData) * s_PointLightsCount);
	s_LightsBuffer.Commit(
		sizeof(PointLightData) * s_MaxPointLightsCount,
		sizeof(DirLightData) * s_DirLightsCount);

	glViewport(0, 0, s_CurrentDisplayWidth, s_CurrentDisplayHeight);
	glScissor(0, 0, s_CurrentDisplayWidth, s_CurrentDisplayHeight);

	ExecuteGeometryPass();
	ExecuteLightingPass();
	ExecuteTransparentPass();

	s_FrameSync.Set();

	s_Framebuffer.Unbind();

	s_PointLightsCount = 0;
	s_DirLightsCount = 0;
	s_Materials.clear();
}

GLuint Renderer::GetRenderTextureID(RenderTexture texture) noexcept
{
	return s_Framebuffer.GetAttachment((size_t)texture).AttachmentID;
}

void Renderer::DisplayFramebuffer() noexcept
{
	s_Framebuffer.Blit(Attachment::Color0);
}

void Renderer::SetDisplaySize(int width, int height) noexcept
{
	s_CurrentDisplayWidth = width;
	s_CurrentDisplayHeight = height;
}

void Renderer::_Initialize(
	int frameWidth,
	int frameHeight,
	GLADloadfunc getProcAddressFunc,
	const RendererSettings& settings)
{
	NV_PROFILE_FUNC;

	s_CurrentDisplayWidth = frameWidth;
	s_CurrentDisplayHeight = frameHeight;

	s_MaxPointLightsCount = settings.MaxPointLights;
	s_MaxDirLightsCount = settings.MaxDirectionalLights;

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
			BREAK_ON_HIGH_SEVERITY(severity);
			NV_LOG_INFO("OpenGL: {}.\n", message);
		},
		nullptr);

	const Rect viewportRect { 0, 0, frameWidth, frameHeight };
	SetViewport(viewportRect, viewportRect);

	s_DeferredGeometryProgram = CreateDeferredGeometryShaderProgram();
	s_DeferredLightProgram = CreateDeferredLightingShaderProgram();
	s_DeferredTransparentProgram = CreateDeferredTransparentShaderProgram();

	s_InstanceBuffer = PersistentMappedBuffer(
		sizeof(InstanceData) * 512,
		BufferAccessFlags::Writable);
	s_InstanceBuffer.SetDebugName("InstanceBuffer");
	
	s_CameraDataBuffer = PersistentMappedBuffer(
		sizeof(CameraData),
		BufferAccessFlags::Writable);
	s_CameraDataBuffer.SetDebugName("CameraBuffer");
	
	s_MaterialsBuffer = PersistentMappedBuffer(
		sizeof(Material) * settings.MaxMaterials,
		BufferAccessFlags::Writable);
	s_MaterialsBuffer.SetDebugName("MaterialsBuffer");

	s_LightsBuffer = PersistentMappedBuffer(
		sizeof(PointLightData) * settings.MaxPointLights + sizeof(DirLightData) * settings.MaxDirectionalLights,
		BufferAccessFlags::Writable);
	s_LightsBuffer.SetDebugName("LightsBuffer");

	s_PointLights = s_LightsBuffer.GetBasePtr<PointLightData>();
	s_DirLights = s_LightsBuffer.GetBasePtr<DirLightData>(sizeof(PointLightData) * settings.MaxPointLights);

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
			.Flags = AttachmentFlags::DrawDest | AttachmentFlags::Resizable,
		}, // color + specular attachment
		FramebufferAttachmentSpec {
			.Width = frameWidth,
			.Height = frameHeight,
			.Format = InternalFormat::RGB16F,
			.Flags = AttachmentFlags::DrawDest | AttachmentFlags::Resizable,
		}, // position attachment
		FramebufferAttachmentSpec {
			.Width = frameWidth,
			.Height = frameHeight,
			.Format = InternalFormat::RGB16F,
			.Flags = AttachmentFlags::DrawDest | AttachmentFlags::Resizable,
		}, // normal attachment
		FramebufferAttachmentSpec {
			.Width = frameWidth,
			.Height = frameHeight,
			.Format = InternalFormat::Depth24Stencil8,
			.Flags = AttachmentFlags::UseRenderbuffer | AttachmentFlags::Resizable,
		}, // depth attachment
		FramebufferAttachmentSpec {
			.Width = frameWidth,
			.Height = frameHeight,
			.Format = InternalFormat::RGB8,
			.Flags = AttachmentFlags::DrawDest | AttachmentFlags::Resizable,
		}, // final output
	});
}

void Renderer::_Shutdown()
{
	_GLObjectBase::DeleteAll();
}