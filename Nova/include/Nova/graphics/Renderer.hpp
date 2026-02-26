#pragma once
#include <Nova/core/Build.hpp>
#include <Nova/graphics/opengl/Buffer.hpp>
#include <Nova/graphics/opengl/ShaderProgram.hpp>
#include <Nova/graphics/opengl/Texture.hpp>
#include <Nova/graphics/opengl/GL.hpp>
#include <Nova/graphics/Rect.hpp>
#include <Nova/graphics/Material.hpp>
#include <Nova/assets/Model.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <utility>

namespace Nova
{
	struct RendererInfo
	{
		std::string_view VendorName;
		std::string_view RendererName;
		std::string_view Version;
		std::string_view GLSLVersion;
	};

	enum class PolygonMode
	{
		Fill = GL_FILL,
		Wireframe = GL_LINE,
		Points = GL_POINT,
	};

	enum class RenderTexture
	{
		Color = 0,
		Position = 1,
		Normal = 2,
		Depth = 3,
		Output = 4,
	};

	namespace Renderer
	{
		NV_API void Render(
			const Model* model,
			const Material& material,
			const glm::mat4& transform);

		NV_API void SetCamera(
			const glm::mat4& view,
			const glm::mat4& projection,
			const glm::vec3& viewPosition);

		NV_API void SetViewport(const Rect<int>& viewport) noexcept;

		NV_API void SetViewport(
			const Rect<int>& viewport,
			const Rect<int>& scissor) noexcept;

		NV_API void SetPolygonMode(PolygonMode mode) noexcept;

		NV_API GLuint GetRenderTextureID(RenderTexture texture) noexcept;

		NV_API const RendererInfo& GetInfo() noexcept;

		NV_API void SetDisplaySize(int width, int height) noexcept;

		/*NV_API void BeginFrame(int displayWidth, int displayHeight);

		NV_API void BeginFrame(glm::ivec2 displaySize);

		NV_API void BeginFrame(std::pair<int, int> displaySize);

		NV_API void SetViewport(const Rect<int> &viewport) noexcept;

		NV_API void SetViewport(const Rect<int> &viewport, const Rect<int> &scissor) noexcept;

		NV_API void SetCameraTransform(const glm::mat4 &view, const glm::mat4 &projection);

		NV_API void SetModelBuffers(const Buffer &vertexBuffer) noexcept;

		NV_API void SetModelBuffers(const Buffer &vertexBuffer, const Buffer &elementBuffer, GLsizei elementsCount, GLenum elementType) noexcept;

		NV_API void SetShader(const ShaderProgram &program);

		NV_API void RenderInstance(const glm::mat4 &transform, const glm::vec4 &color);

		NV_API void RenderInstance(const glm::mat4 &transform, const glm::vec4 &color, const Texture &texture);

		NV_API void RenderInstance(const glm::mat4 &transform, const glm::vec4 &color, const Texture &texture, uint32_t entityID);*/

		NV_API void AddPointLight(const glm::vec4& color, const glm::vec3& position, float radius);

		NV_API void AddDirectionalLight(const glm::vec4& color, const glm::vec3& direction);

		NV_API void Clear(ClearMask mask = ClearMask::ColorBufferBit) noexcept;

		NV_API void SetClearColor(float r, float g, float b, float a) noexcept;

		NV_API void SetClearColor(const glm::vec4 &color) noexcept;

		NV_API void Draw(const glm::vec4& clearColor);

		NV_API void DisplayFramebuffer() noexcept;

		void _Initialize(
			int frameWidth,
			int frameHeight,
			const std::filesystem::path &shaderCacheDirectory,
			GLADloadfunc getProcAddressFunc);

		void _Initialize(
			std::pair<int, int> frameSize,
			const std::filesystem::path &shaderCacheDirectory,
			GLADloadfunc getProcAddressFunc);

		void _Shutdown();
	}
}
