//#include <Nova/graphics/Renderer.hpp>
//#include <Nova/graphics/opengl/Buffer.hpp>
//#include <Nova/graphics/opengl/Texture.hpp>
//#include <Nova/graphics/opengl/VertexArray.hpp>
//#include <Nova/graphics/ShaderCache.hpp>
//#include <Nova/debug/Profile.hpp>
//#include <Nova/debug/Log.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <glfw/glfw3.h>
//#include <vector>
//#include <map>
//
//using namespace Nova;
//
//struct InstanceData
//{
//	glm::mat4 Transform;
//	glm::vec4 Color;
//	uint32_t TextureIndex;
//	uint32_t EntityID;
//};
//
//struct ModelData
//{
//	glm::vec3 Position;
//	glm::vec3 Normal;
//	glm::vec2 TexureCoords;
//};
//
//struct MaterialData
//{
//	glm::vec4 Color;
//	uint32_t AmbientTextureIndex;
//	uint32_t DiffuseTextureIndex;
//	uint32_t SpecularTextureIndex;
//	float Shininess;
//};
//
//struct CameraData
//{
//	glm::mat4 CameraView;
//	glm::mat4 CameraProjection;
//};
//
//constexpr GLuint c_ModelDataBufferBindingIndex = 0;
//constexpr GLuint c_InstanceDataBufferBindingIndex = 1;
//constexpr GLsizeiptr c_MaxInstanceCount = 2048;
//constexpr GLsizeiptr c_MaxModelVertexCount = 4096;
//constexpr GLsizeiptr c_MaxModelElementCount = 1024;
//constexpr GLsizeiptr c_MaxMaterialCount = 16;
//constexpr uint32_t c_MaxUserTextureCount = 16;
//constexpr GLsizeiptr c_MaxLightSources = 32;
//
//static std::unordered_map<size_t, std::pair<size_t, size_t>> s_ModelCache;
//
//static Buffer s_InstanceDataBuffer;
//static Buffer s_CameraTransformBuffer;
//static Buffer s_MaterialDataBuffer;
//static Buffer s_LightSourceDataBuffer;
//static ShaderProgram s_BasicShaderProgram;
//static VertexArray s_BasicVertexArray;
//static ShaderCache s_ShaderCache;
//
//static std::map<GLuint, uint32_t> s_TextureIndexMap;
//static std::array<GLuint, c_MaxUserTextureCount> s_Textures;
//
//static GLsizei s_ElementsCount;
//static GLenum s_ElementType;
//
//template <typename T>
//constexpr size_t GetPtrByteOffset(T *base, T *current) noexcept
//{
//	return (current - base) * sizeof(T);
//}
//
//static uint32_t GetTextureIndex(const Texture &texture)
//{
//	const auto textureID = (GLuint)texture.GetID();
//
//	const auto textureIndexPos = s_TextureIndexMap.find(textureID);
//	if (textureIndexPos != s_TextureIndexMap.end())
//	{
//		return textureIndexPos->second;
//	}
//
//	if (s_TextureIndexMap.size() == c_MaxUserTextureCount)
//	{
//		return -1;
//	}
//
//	auto textureIndex = s_TextureIndexMap.size();
//	s_TextureIndexMap.insert({textureID, textureIndex});
//	s_Textures[textureIndex] = textureID;
//
//	return textureIndex;
//}
//
//static void FlushTextureIndexCache()
//{
//	s_TextureIndexMap.clear();
//}
//
//static GLsizei GetCurrentInstanceCount() noexcept
//{
//	return s_InstanceDataBuffer.GetDataSize() / sizeof(InstanceData);
//}
//
//void Renderer::AddLightSource(const LightSource &lightSource)
//{
//	// const auto currentLightsCount = s_LightSourceDataPtr - s_LightSourceDataBasePtr;
//	// if (currentLightsCount >= c_MaxLightSources)
//	// {
//	// 	NV_LOG_WARNING("Too many light sources defined for current scene (max. {}). Additional lights will be omitted. You can control amount of lights by changing RendererSettings.MaxLightsCount", c_MaxLightSources);
//	// 	return;
//	// }
//
//	// s_LightSourceDataPtr->Position = lightSource.Position;
//	// s_LightSourceDataPtr->Color = lightSource.Color;
//	// s_LightSourceDataPtr++;
//}
//
//void Renderer::_Initialize(std::pair<int, int> frameSize, const std::filesystem::path &shaderCacheDirectory)
//{
//	_Initialize(frameSize.first, frameSize.second, shaderCacheDirectory);
//}
//
//static const std::string_view DebugSourceToString(GLenum source)
//{
//	switch (source)
//	{
//	case GL_DEBUG_SOURCE_API:
//		return "API";
//	case GL_DEBUG_SOURCE_APPLICATION:
//		return "APPLICATION";
//	case GL_DEBUG_SOURCE_OTHER:
//		return "OTHER";
//	case GL_DEBUG_SOURCE_SHADER_COMPILER:
//		return "SHADER_COMPILER";
//	case GL_DEBUG_SOURCE_THIRD_PARTY:
//		return "THIRD_PARTY";
//	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
//		return "WINDOW_SYSTEM";
//	}
//
//	NV_UNREACHABLE;
//}
//
//static const std::string_view DebugTypeToString(GLenum type)
//{
//	switch (type)
//	{
//	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
//		return "DEPRECATED_BEHAVIOR";
//	case GL_DEBUG_TYPE_ERROR:
//		return "ERROR";
//	case GL_DEBUG_TYPE_MARKER:
//		return "MARKER";
//	case GL_DEBUG_TYPE_OTHER:
//		return "OTHER";
//	case GL_DEBUG_TYPE_PERFORMANCE:
//		return "PERFORMANCE";
//	case GL_DEBUG_TYPE_POP_GROUP:
//		return "POP_GROUP";
//	case GL_DEBUG_TYPE_PORTABILITY:
//		return "PORTABILITY";
//	case GL_DEBUG_TYPE_PUSH_GROUP:
//		return "PUSH_GROUP";
//	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
//		return "UNDEFINED_BEHAVIOR";
//	}
//
//	NV_UNREACHABLE;
//}
//
//static void LoadOpenGL()
//{
//	NV_PROFILE_FUNC;
//
//	if (!gladLoadGL(glfwGetProcAddress))
//	{
//		throw std::runtime_error("Failed to initialize OpenGL bindings.");
//	}
//
//#ifdef NV_DEBUG
//	glEnable(GL_DEBUG_OUTPUT);
//	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
//	glDebugMessageCallback(
//		[](GLenum source, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar *message, const void *)
//		{
//			const auto sourceStr = DebugSourceToString(source);
//			const auto typeStr = DebugTypeToString(type);
//			switch (severity)
//			{
//			case GL_DEBUG_SEVERITY_NOTIFICATION:
//				NV_LOG_TRACE("OpenGL[{}][{}] {}.\n", sourceStr, typeStr, message);
//				break;
//			case GL_DEBUG_SEVERITY_LOW:
//				NV_LOG_INFO("OpenGL[{}][{}] {}.\n", sourceStr, typeStr, message);
//				break;
//			case GL_DEBUG_SEVERITY_MEDIUM:
//				NV_LOG_WARNING("OpenGL[{}][{}] {}.\n", sourceStr, typeStr, message);
//				break;
//			case GL_DEBUG_SEVERITY_HIGH:
//				NV_LOG_ERROR("OpenGL[{}][{}] {}.\n", sourceStr, typeStr, message);
//				NV_DEBUG_FAIL("OpenGL error");
//				break;
//			}
//		},
//		nullptr);
//#endif
//}
//
//static void CreateBuffers()
//{
//	NV_PROFILE_FUNC;
//
//	s_InstanceDataBuffer = Buffer(c_MaxInstanceCount * sizeof(InstanceData), true);
//	s_MaterialDataBuffer = Buffer(c_MaxMaterialCount * sizeof(MaterialData), true);
//	s_LightSourceDataBuffer = Buffer(c_MaxLightSources * sizeof(LightSource), true);
//	s_CameraTransformBuffer = Buffer(sizeof(CameraData), true);
//}
//
//static void CreateShaders(const std::filesystem::path &shaderCacheDirectory)
//{
//	NV_PROFILE_FUNC;
//
//	s_ShaderCache.SetDirectory(shaderCacheDirectory);
//	s_BasicShaderProgram = s_ShaderCache.LoadCachedProgram(
//		"BasicShaderProgram",
//		[]()
//		{
//			NV_PROFILE_SCOPE("::CreateBasicShaderProgram");
//
//			const std::array<ShaderStage, 2> stages{
//				ShaderStage::FromGLSL(
//					ShaderType::Vertex,
//					std::filesystem::path("./assets/shaders/basic.vert")),
//				ShaderStage::FromGLSL(
//					ShaderType::Fragment,
//					std::filesystem::path("./assets/shaders/basic.frag")),
//			};
//
//			return ShaderProgram(stages);
//		});
//}
//
//void Renderer::_Initialize(int frameWidth, int frameHeight, const std::filesystem::path &shaderCacheDirectory)
//{
//	NV_PROFILE_FUNC;
//
//	LoadOpenGL();
//	CreateBuffers();
//	CreateShaders(shaderCacheDirectory);
//
//	s_CameraTransformBuffer.Bind(
//		BindingTarget::UniformBuffer,
//		s_BasicShaderProgram.GetResourceLocation("uCameraData"));
//	s_MaterialDataBuffer.Bind(
//		BindingTarget::UniformBuffer,
//		s_BasicShaderProgram.GetResourceLocation("uMaterialData"));
//	s_LightSourceDataBuffer.Bind(
//		BindingTarget::UniformBuffer,
//		s_BasicShaderProgram.GetResourceLocation("uLightData"));
//}
//
//void Renderer::_Shutdown()
//{
//	NV_PROFILE_FUNC;
//
//	_GLObjectBase::DeleteAll();
//}
//
//void Renderer::BeginFrame(int displayWidth, int displayHeight)
//{
//	NV_ASSERT(displayWidth > 0, "Width must be greater than 0");
//	NV_ASSERT(displayHeight > 0, "Height must be greater than 0");
//	// s_Framebuffer.Resize(displayWidth, displayHeight);
//}
//
//void Renderer::BeginFrame(glm::ivec2 displaySize)
//{
//	BeginFrame(displaySize.x, displaySize.y);
//}
//
//void Renderer::BeginFrame(std::pair<int, int> displaySize)
//{
//	BeginFrame(displaySize.first, displaySize.second);
//}
//
//void Renderer::SetCameraTransform(const glm::mat4 &view, const glm::mat4 &projection)
//{
//	NV_PROFILE_FUNC;
//
//	s_CameraTransformBuffer.Store(glm::value_ptr(view), sizeof(glm::mat4));
//	s_CameraTransformBuffer.Store(glm::value_ptr(projection), sizeof(glm::mat4));
//	s_CameraTransformBuffer.Commit();
//}
//
//void Renderer::SetModelBuffers(const Buffer &vertexBuffer) noexcept
//{
//	NV_PROFILE_FUNC;
//
//	s_BasicVertexArray.BindVertexBuffer(vertexBuffer, c_ModelDataBufferBindingIndex, sizeof(ModelData));
//}
//
//void Renderer::SetModelBuffers(const Buffer &vertexBuffer, const Buffer &elementBuffer, GLsizei elementsCount, GLenum elementType) noexcept
//{
//	NV_PROFILE_FUNC;
//
//	SetModelBuffers(vertexBuffer);
//
//	s_BasicVertexArray.BindElementBuffer(elementBuffer);
//	s_ElementsCount = elementsCount;
//	s_ElementType = elementType;
//}
//
//void Renderer::Clear(float r, float g, float b, float a)
//{
//	NV_PROFILE_FUNC;
//
//	glClearColor(r, g, b, a);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	// s_Framebuffer.Clear(r, g, b, a);
//}
//
//void Renderer::Clear(const glm::vec4 &color)
//{
//	Clear(color.r, color.g, color.b, color.a);
//}
//
//void Renderer::RenderInstance(const glm::mat4 &transform, const glm::vec4 &color)
//{
//	NV_PROFILE_FUNC;
//
//	if (GetCurrentInstanceCount() >= c_MaxInstanceCount)
//	{
//		Renderer::ExecuteDrawCall();
//	}
//
//	InstanceData data{
//		.Transform = transform,
//		.Color = color,
//		.TextureIndex = 0,
//		.EntityID = 0,
//	};
//	s_InstanceDataBuffer.Store(data);
//}
//
//void Renderer::RenderInstance(const glm::mat4 &transform, const glm::vec4 &color, const Texture &texture)
//{
//	NV_PROFILE_FUNC;
//
//	if (GetCurrentInstanceCount() >= c_MaxInstanceCount)
//	{
//		Renderer::ExecuteDrawCall();
//	}
//
//	auto textureIndex = GetTextureIndex(texture);
//	if (textureIndex == -1)
//	{
//		Renderer::ExecuteDrawCall();
//		FlushTextureIndexCache();
//
//		textureIndex = GetTextureIndex(texture);
//	}
//
//	const InstanceData data{
//		.Transform = transform,
//		.Color = color,
//		.TextureIndex = textureIndex,
//		.EntityID = 0,
//	};
//	s_InstanceDataBuffer.Store(data);
//}
//
//void Renderer::RenderInstance(const glm::mat4 &transform, const glm::vec4 &color, const Texture &texture, uint32_t entityID)
//{
//}
//
//void Renderer::SetViewport(const Rect<int> &viewport) noexcept
//{
//	glViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
//}
//
//void Renderer::SetViewport(const Rect<int> &viewport, const Rect<int> &scissor) noexcept
//{
//	SetViewport(viewport);
//	glScissor(scissor.X, scissor.Y, scissor.Width, scissor.Height);
//}
//
//void Renderer::ExecuteDrawCall()
//{
//	NV_PROFILE_FUNC;
//
//	const GLsizei instanceCount = GetCurrentInstanceCount();
//	if (instanceCount == 0)
//	{
//		return;
//	}
//
//	s_InstanceDataBuffer.Commit();
//
//	s_BasicShaderProgram.Use();
//	s_BasicVertexArray.Use();
//
//	Texture::GetWhiteTexture().BindToUnit(0);
//	Texture::BindTextures(s_Textures, 1);
//
//	glDrawElementsInstanced(
//		GL_TRIANGLES,
//		s_ElementsCount,
//		s_ElementType,
//		nullptr,
//		instanceCount);
//}
