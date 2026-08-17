#pragma once
#include <Nova/graphics/opengl/Buffer.hpp>
#include <Nova/graphics/opengl/ShaderProgram.hpp>
#include <Nova/graphics/opengl/BindlessTextureBinder.hpp>
#include <Nova/graphics/opengl/VertexArray.hpp>
#include <Nova/graphics/opengl/Sync.hpp>
#include <Nova/graphics/opengl/Framebuffer.hpp>
#include <Nova/graphics/opengl/Context.hpp>
#include <Nova/graphics/opengl/Texture.hpp>
#include <Nova/graphics/opengl/ShaderCache.hpp>
#include <Nova/graphics/Window.hpp>
#include <Nova/graphics/Rect.hpp>
#include <Nova/graphics/Material.hpp>
#include <Nova/core/Utility.hpp>
#include <glm/glm.hpp>
#include <string>
#include <filesystem>
#include <memory>

namespace Nova
{
    struct ModelVertexData
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TextureCoords;
    };

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

    struct InstanceData
    {
        GLuint MaterialIndex;
        glm::mat4 Transform;
        glm::mat3 NormalTransform;
    };

    struct DrawData
    {
        std::vector<InstanceData> OpaqueInstanceData;
        std::vector<InstanceData> TransparentInstanceData;
        size_t Age;
    };

    struct CameraData
    {
        glm::mat4 ViewMatrix;
        glm::mat4 ProjectionMatrix;
        glm::vec3 Position;
        float ZNear;
        float ZFar;
    };

    struct MaterialData
    {
        glm::vec4 Color;
        float SpecularIntensity;
        float Shininess;
        GLuint AlbedoHandleHigh;
        GLuint AlbedoHandleLow;
    };

    struct RendererInfo
    {
        std::string VendorName;
        std::string RendererName;
        std::string Version;
        std::string GLSLVersion;
    };

    struct FrameInfo
    {
        Rect<unsigned int> Viewport;
        Rect<unsigned int> Scissor;
        unsigned int Width;
        unsigned int Height;
        size_t DrawCalls;
        glm::vec4 ClearColor;
    };

    enum class FramebufferAttachmentType : size_t
    {
        Albedo = 0,
        Position = 1,
        Normal = 2,
        Color = 3,
        Final = 4,
        Shininess = 5,
        Depth = 6,
        EnumMax_,
    };

    class Renderer
    {
    public:
        Renderer() = delete;

        Renderer(const Window &window);

        Renderer(const Renderer &) = delete;

        Renderer(Renderer &&) noexcept = default;

        // NOTE Just to destroy texture binder before white texture
        ~Renderer() noexcept;

        constexpr const RendererInfo &GetInfo() const noexcept { return info_; }

        constexpr const FrameInfo &GetFrameInfo() const noexcept { return frameInfo_; }

        void Clear(float r, float g, float b, float a) const noexcept;

        constexpr void SetClearColor(const glm::vec4 &color) noexcept { frameInfo_.ClearColor = color; }

        constexpr void SetClearColor(float r, float g, float b, float a) noexcept { frameInfo_.ClearColor = glm::vec4(r, g, b, a); }

        void SetCamera(
            const glm::mat4 &view,
            const glm::mat4 &projection) noexcept;

        void SetCamera(
            const glm::mat4 &view,
            const glm::mat4 &projection,
            const glm::vec3 &position,
            float zNear,
            float zFar) noexcept;

        void SetViewport(const Rect<unsigned int> &viewport) noexcept;

        void SetViewport(
            const Rect<unsigned int> &viewport,
            const Rect<unsigned int> &scissor) noexcept;

        void AddPointLight(
            const glm::vec4 &color,
            const glm::vec3 &position,
            float radius) noexcept;

        void AddDirectionalLight(
            const glm::vec4 &color,
            const glm::vec3 &direction);

        void Render(
            std::shared_ptr<Buffer> modelBuffer,
            const Material &material,
            const glm::mat4 &transform);

        void Render(
            std::shared_ptr<Buffer> modelBuffer,
            const Material &material,
            const glm::mat4 &transform,
            const glm::mat3 &transformNormal);

        void Draw(
            unsigned int frameWidth,
            unsigned int frameHeight,
            std::shared_ptr<Texture> skyboxTexture);

        const FramebufferAttachment &GetFramebufferAttachment(FramebufferAttachmentType type) const noexcept;

        const Texture &GetWhiteTexture() const noexcept;

        void DisplayFramebuffer(const Window &window) const noexcept;

        Renderer &operator=(const Renderer &) = delete;

        Renderer &operator=(Renderer &&) noexcept = default;

    private:
        GLContext context_;
        RendererInfo info_;
        Framebuffer framebuffer_;
        BindlessTextureBinder textureBinder_;
        Texture whiteTexture_;
        GLuint64 whiteTextureHandle_;
        ShaderCache shaderCache_;
        ShaderProgram deferredGeometryProgram_;
        ShaderProgram deferredLightProgram_;
        ShaderProgram deferredTransparentProgram_;
        ShaderProgram deferredFogProgram_;
        ShaderProgram skyboxProgram_;
        FrameInfo frameInfo_;
        Buffer cameraBuffer_;
        Buffer instanceBuffer_;
        Buffer materialsBuffer_;
        Buffer lightsBuffer_;
        BufferRegion<InstanceData> instanceBufferData_;
        BufferRegion<MaterialData> materialsBufferData_;
        BufferRegion<PointLightData> pointLightsData_;
        BufferRegion<DirLightData> dirLightsData_;
        VertexArray vertexArray_;
        Sync frameSync_;
        Sync instanceDataSync_;
        std::unordered_map<MaterialData, GLuint, XXHasher<MaterialData>> materialsCache_;
        std::unordered_map<std::shared_ptr<Buffer>, DrawData> drawData_;
        glm::vec3 cameraPosition_;

        std::vector<InstanceData> &GetInstanceDataForModel(std::shared_ptr<Buffer> modelBuffer, bool useAlpha) noexcept;
        GLuint GetMaterialIndex(const Material &material);
        void DrawBatch(const std::shared_ptr<Buffer> &modelBuffer, std::span<const InstanceData> instanceData) noexcept;
    };

    constexpr bool operator==(const MaterialData &a, const MaterialData &b) noexcept
    {
        return a.Color == b.Color &&
               a.SpecularIntensity == b.SpecularIntensity &&
               a.Shininess == b.Shininess &&
               a.AlbedoHandleHigh == b.AlbedoHandleHigh &&
               a.AlbedoHandleLow == b.AlbedoHandleLow;
    }
}