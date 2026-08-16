#pragma once
#include <Nova/graphics/Renderer.hpp>
#include <Nova/graphics/opengl/GL.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/debug/Profile.hpp>

constexpr auto maxInstanceCount = 1024zu;
constexpr auto maxMaterialsCount = 32zu;
constexpr auto maxPointLights = 32zu;
constexpr auto maxDirLights = 2zu;
constexpr auto maxTextureHandleAge = 5ul; // in frames
constexpr auto albedoAttachmentIndex = 0ul;
constexpr auto positionAttachmentIndex = 1ul;
constexpr auto normalAttachmentIndex = 2ul;
constexpr auto colorAttachmentIndex = 3ul;
constexpr auto finalAttachmentIndex = 4ul;
constexpr auto shininessAttachmentIndex = 5ul;
constexpr auto depthAttachmentIndex = 6ul;
constexpr auto modelBufferBinding = 0ul;

static glm::mat3 BuildNormalTransformMatrix(const glm::mat4 &transform) noexcept
{
    // Why isn't that constexpr ???
    return glm::transpose(glm::inverse(glm::mat3(transform)));
}

static constexpr glm::vec3 ExtractCameraPosition(const glm::mat4 &view) noexcept
{
    return glm::vec3(glm::inverse(view) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

static constexpr std::pair<float, float> ExtractZPlanes(const glm::mat4 &projection) noexcept
{
    const auto a = projection[2][2];
    const auto b = projection[3][2];

    return {b / (a - 1.0f), b / (a + 1.0f)};
}

static constexpr const std::string_view DebugTypeToString(GLenum type) noexcept
{
    switch (type)
    {
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        return "DEPRECATED BEHAVIOR";
    case GL_DEBUG_TYPE_ERROR:
        return "ERROR";
    case GL_DEBUG_TYPE_MARKER:
        return "MARKER";
    case GL_DEBUG_TYPE_OTHER:
        return "OTHER";
    case GL_DEBUG_TYPE_PERFORMANCE:
        return "PERFORMANCE";
    case GL_DEBUG_TYPE_POP_GROUP:
        return "POP GROUP";
    case GL_DEBUG_TYPE_PORTABILITY:
        return "PORTABILITY";
    case GL_DEBUG_TYPE_PUSH_GROUP:
        return "PUSH GROUP";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        return "UNDEFINED BEHAVIOR";
    default:
        std::unreachable();
    }
}

static constexpr const std::string_view DebugSourceToString(GLenum source) noexcept
{
    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        return "API";
    case GL_DEBUG_SOURCE_APPLICATION:
        return "APPLICATION";
    case GL_DEBUG_SOURCE_OTHER:
        return "OTHER";
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        return "SHADER COMPILER";
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        return "THIRD PARTY";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        return "WINDOW SYSTEM";
    default:
        std::unreachable();
    }
}

static void DebugMessageCallback(
    [[maybe_unused]] GLenum source,
    [[maybe_unused]] GLenum type,
    GLuint,
    [[maybe_unused]] GLenum severity,
    GLsizei,
    [[maybe_unused]] const GLchar *message,
    const void *)
{
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        NV_LOG_ERROR("OpenGL({}): {} - {}", DebugSourceToString(source), DebugTypeToString(type), message);
#ifdef _DEBUG
        __debugbreak();
#endif
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        NV_LOG_WARNING("OpenGL({}): {} - {}", DebugSourceToString(source), DebugTypeToString(type), message);
        break;
    case GL_DEBUG_SEVERITY_LOW:
        NV_LOG_INFO("OpenGL({}): {} - {}", DebugSourceToString(source), DebugTypeToString(type), message);
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        NV_LOG_TRACE("OpenGL({}): {} - {}", DebugSourceToString(source), DebugTypeToString(type), message);
    }
}

static void SortTransparentObjects(std::span<Nova::InstanceData> instanceData, const glm::vec3 &cameraPosition) noexcept
{
    NV_PROFILE_FUNC;

    std::sort(
        instanceData.begin(),
        instanceData.end(),
        [&](const auto &a, const auto &b)
        {
            const auto aDistance = glm::distance(glm::vec3(a.Transform[3]), cameraPosition);
            const auto bDistance = glm::distance(glm::vec3(b.Transform[3]), cameraPosition);
            return aDistance > bDistance;
        });
}

static Nova::GLContext CreateContextAndLoadGL(const Nova::Window &window)
{
    NV_PROFILE_FUNC;

    auto context = Nova::GLContext(window.GetData(), 4, 5);
    context.MakeCurrent();
    context.LoadGL();

    return context;
}

static Nova::RendererInfo RetrieveRendererInfo() noexcept
{
    NV_PROFILE_FUNC;

    return {
        .VendorName = std::string(Nova::GL::GetString(Nova::StringName::Vendor)),
        .RendererName = std::string(Nova::GL::GetString(Nova::StringName::Renderer)),
        .Version = std::string(Nova::GL::GetString(Nova::StringName::Version)),
        .GLSLVersion = std::string(Nova::GL::GetString(Nova::StringName::ShadingLanguageVersion)),
    };
}

static Nova::Texture CreateWhiteTexture()
{
    NV_PROFILE_FUNC;

    constexpr std::array<unsigned char, 4> whiteTextureData{255, 255, 255, 255};
    auto texture = Nova::Texture(
        Nova::TextureTarget::Texture2D,
        Nova::TextureSpec{
            .Size = {1, 1, 0},
            .Format = Nova::InternalFormat::RGBA8});
    texture.Upload(
        Nova::TextureUploadInfo{
            .Size = {1, 1, 0},
            .PixelFormat = Nova::PixelFormat::RGBA,
            .PixelType = Nova::PixelType::UnsignedByte},
        whiteTextureData.data(),
        true);

    return texture;
}

static Nova::Framebuffer CreateFramebuffer(int width, int height)
{
    NV_PROFILE_FUNC;

    return Nova::Framebuffer({
        Nova::FramebufferAttachmentSpec{
            .Width = width,
            .Height = height,
            .Format = Nova::InternalFormat::RGBA8,
            .Flags = Nova::AttachmentFlags::DrawDest | Nova::AttachmentFlags::Resizable,
        }, // [0] color + specular attachment
        Nova::FramebufferAttachmentSpec{
            .Width = width,
            .Height = height,
            .Format = Nova::InternalFormat::RGB16F,
            .Flags = Nova::AttachmentFlags::DrawDest | Nova::AttachmentFlags::Resizable,
        }, // [1] position attachment
        Nova::FramebufferAttachmentSpec{
            .Width = width,
            .Height = height,
            .Format = Nova::InternalFormat::RGB16F,
            .Flags = Nova::AttachmentFlags::DrawDest | Nova::AttachmentFlags::Resizable,
        }, // [2] normal attachment
        Nova::FramebufferAttachmentSpec{
            .Width = width,
            .Height = height,
            .Format = Nova::InternalFormat::RGB8,
            .Flags = Nova::AttachmentFlags::DrawDest | Nova::AttachmentFlags::Resizable,
        }, // [3] output color attachment
        Nova::FramebufferAttachmentSpec{
            .Width = width,
            .Height = height,
            .Format = Nova::InternalFormat::RGB8,
            .Flags = Nova::AttachmentFlags::DrawDest | Nova::AttachmentFlags::Resizable,
        }, // [4] final output
        Nova::FramebufferAttachmentSpec{
            .Width = width,
            .Height = height,
            .Format = Nova::InternalFormat::RGB8,
            .Flags = Nova::AttachmentFlags::DrawDest | Nova::AttachmentFlags::Resizable,
        }, // [5] shininess
        Nova::FramebufferAttachmentSpec{
            .Width = width,
            .Height = height,
            .Format = Nova::InternalFormat::Depth24Stencil8,
            .Flags = Nova::AttachmentFlags::Resizable,
        }, // [6] depth attachment
    });
}

static Nova::ShaderProgram CreateDeferredGeometryProgram(Nova::ShaderCache &cache)
{
    NV_PROFILE_FUNC;

    auto program = cache.LoadCachedProgram(
        "DeferredGeometry",
        []()
        {
            return Nova::ShaderProgram(
                {
                    Nova::SourceFileShaderStage{
                        .Type = Nova::ShaderType::Vertex,
                        .Filepath = "./assets/shaders/deferredGeometry.vert",
                    },
                    Nova::SourceFileShaderStage{
                        .Type = Nova::ShaderType::Fragment,
                        .Filepath = "./assets/shaders/deferredGeometry.frag",
                    },
                },
                {
                    {"outColor", albedoAttachmentIndex},
                    {"outPosition", positionAttachmentIndex},
                    {"outNormal", normalAttachmentIndex},
                    {"outShininess", shininessAttachmentIndex},
                });
        });
    program.SetDebugName("GeometryProgram");

    return program;
}

static Nova::ShaderProgram CreateDeferredLightProgram(Nova::ShaderCache &cache, const Nova::Framebuffer &framebuffer, Nova::BindlessTextureBinder &textureBinder)
{
    NV_PROFILE_FUNC;

    auto program = cache.LoadCachedProgram(
        "DeferredLight",
        []()
        {
            return Nova::ShaderProgram(
                {
                    Nova::SourceFileShaderStage{
                        .Type = Nova::ShaderType::Vertex,
                        .Filepath = "./assets/shaders/deferredLighting.vert",
                    },
                    Nova::SourceFileShaderStage{
                        .Type = Nova::ShaderType::Fragment,
                        .Filepath = "./assets/shaders/deferredLighting.frag",
                    },
                },
                {
                    {"outColor", colorAttachmentIndex},
                });
        });
    program.SetUniform(
        "uGBufferAlbedoSpecular",
        textureBinder.Bind(framebuffer.GetAttachment(albedoAttachmentIndex), true));
    program.SetUniform(
        "uGBufferPosition",
        textureBinder.Bind(framebuffer.GetAttachment(positionAttachmentIndex), true));
    program.SetUniform(
        "uGBufferNormal",
        textureBinder.Bind(framebuffer.GetAttachment(normalAttachmentIndex), true));
    program.SetUniform(
        "uGBufferShininess",
        textureBinder.Bind(framebuffer.GetAttachment(shininessAttachmentIndex), true));
    program.SetDebugName("LightProgram");

    return program;
}

static Nova::ShaderProgram CreateDeferredTransparentProgram(Nova::ShaderCache &cache)
{
    NV_PROFILE_FUNC;

    auto program = cache.LoadCachedProgram(
        "DeferredTransparent",
        []()
        {
            return Nova::ShaderProgram(
                {
                    Nova::SourceFileShaderStage{
                        .Type = Nova::ShaderType::Vertex,
                        .Filepath = "./assets/shaders/deferredTransparent.vert",
                    },
                    Nova::SourceFileShaderStage{
                        .Type = Nova::ShaderType::Fragment,
                        .Filepath = "./assets/shaders/deferredTransparent.frag",
                    },
                },
                {
                    {"outColor", colorAttachmentIndex},
                });
        });
    program.SetDebugName("TransparentProgram");

    return program;
}

static Nova::ShaderProgram CreateDeferredFogProgram(Nova::ShaderCache &cache, const Nova::Framebuffer &framebuffer, Nova::BindlessTextureBinder &textureBinder)
{
    NV_PROFILE_FUNC;

    auto program = cache.LoadCachedProgram(
        "DeferredFog",
        []()
        {
            return Nova::ShaderProgram(
                {
                    Nova::SourceFileShaderStage{
                        .Type = Nova::ShaderType::Vertex,
                        .Filepath = "./assets/shaders/deferredFog.vert",
                    },
                    Nova::SourceFileShaderStage{
                        .Type = Nova::ShaderType::Fragment,
                        .Filepath = "./assets/shaders/deferredFog.frag",
                    },
                },
                {
                    {"outColor", finalAttachmentIndex},
                });
        });
    program.SetUniform(
        "uSceneColor",
        textureBinder.Bind(framebuffer.GetAttachment(colorAttachmentIndex), true));
    program.SetUniform(
        "uDepth",
        textureBinder.Bind(framebuffer.GetAttachment(depthAttachmentIndex), true));
    program.SetDebugName("FogProgram");

    return program;
}

static Nova::ShaderProgram CreateSkyboxProgram(Nova::ShaderCache &cache)
{
    NV_PROFILE_FUNC;

    auto program = cache.LoadCachedProgram(
        "DeferredSkybox",
        []()
        {
            return Nova::ShaderProgram(
                {
                    Nova::SourceFileShaderStage{
                        .Type = Nova::ShaderType::Vertex,
                        .Filepath = "./assets/shaders/skybox.vert",
                    },
                    Nova::SourceFileShaderStage{
                        .Type = Nova::ShaderType::Fragment,
                        .Filepath = "./assets/shaders/skybox.frag",
                    },
                },
                {
                    {"outColor", colorAttachmentIndex},
                });
        });
    program.SetDebugName("SkyboxProgram");

    return program;
}

static Nova::Buffer CreateInstanceBuffer()
{
    NV_PROFILE_FUNC;

    auto buffer = Nova::Buffer(
        sizeof(Nova::InstanceData) * maxInstanceCount,
        Nova::BufferAccessFlags::Writable);
    buffer.SetDebugName("InstanceBuffer");

    return buffer;
}

static Nova::Buffer CreateCameraBuffer()
{
    NV_PROFILE_FUNC;

    auto buffer = Nova::Buffer(sizeof(Nova::CameraData), Nova::BufferAccessFlags::Writable);
    buffer.SetDebugName("CameraBuffer");

    return buffer;
}

static Nova::Buffer CreateMaterialsBuffer()
{
    NV_PROFILE_FUNC;

    auto buffer = Nova::Buffer(
        sizeof(Nova::MaterialData) * maxMaterialsCount,
        Nova::BufferAccessFlags::Writable);
    buffer.SetDebugName("MaterialsBuffer");

    return buffer;
}

static Nova::Buffer CreateLightsBuffer()
{
    NV_PROFILE_FUNC;

    auto buffer = Nova::Buffer(
        sizeof(Nova::PointLightData) * maxPointLights + sizeof(Nova::DirLightData) * maxDirLights,
        Nova::BufferAccessFlags::Writable);
    buffer.SetDebugName("LightsBuffer");

    return buffer;
}

static Nova::VertexArray CreateVertexArray(const Nova::ShaderProgram &geometryProgram, const Nova::Buffer &instanceBuffer)
{
    NV_PROFILE_FUNC;

    return Nova::VertexArray({
        Nova::VertexInput{
            .Stride = sizeof(Nova::ModelVertexData),
            .Descriptors = {
                Nova::VertexDescriptor{
                    .AttributeIndex = geometryProgram.GetResourceLocation("inPosition"),
                    .AttributeType = Nova::AttributeType::Float,
                    .Count = 3,
                },
                Nova::VertexDescriptor{
                    .AttributeIndex = geometryProgram.GetResourceLocation("inNormal"),
                    .AttributeType = Nova::AttributeType::Float,
                    .Count = 3,
                },
                Nova::VertexDescriptor{
                    .AttributeIndex = geometryProgram.GetResourceLocation("inTexCoord"),
                    .AttributeType = Nova::AttributeType::Float,
                    .Count = 2,
                },
            },
        },
        Nova::VertexInput{
            .Stride = sizeof(Nova::InstanceData),
            .Descriptors = {
                Nova::VertexDescriptor{
                    .AttributeIndex = geometryProgram.GetResourceLocation("inMaterialIndex"),
                    .AttributeType = Nova::AttributeType::UnsignedInt,
                    .Count = 1,
                },
                Nova::VertexDescriptor{
                    .AttributeIndex = geometryProgram.GetResourceLocation("inTransform"),
                    .AttributeType = Nova::AttributeType::Float,
                    .Count = 4,
                    .Rows = 4,
                },
                Nova::VertexDescriptor{
                    .AttributeIndex = geometryProgram.GetResourceLocation("inNormalTransform"),
                    .AttributeType = Nova::AttributeType::Float,
                    .Count = 3,
                    .Rows = 3,
                },
            },
            .BufferID = instanceBuffer.GetID(),
            .InstanceDivisor = 1,
        },
    });
}

Nova::Renderer::Renderer(const Window &window)
    : context_(CreateContextAndLoadGL(window)),
      info_(RetrieveRendererInfo()),
      // TODO Check if context supports bindless textures
      textureBinder_(maxTextureHandleAge * static_cast<unsigned int>(context_.GetRefreshRate())),
      whiteTexture_(CreateWhiteTexture()),
      whiteTextureHandle_(textureBinder_.Bind(whiteTexture_, true)),
      framebuffer_(CreateFramebuffer(window.GetWidth(), window.GetHeight())),
      shaderCache_("./shadercache"),
      deferredGeometryProgram_(CreateDeferredGeometryProgram(shaderCache_)),
      deferredLightProgram_(CreateDeferredLightProgram(shaderCache_, framebuffer_, textureBinder_)),
      deferredTransparentProgram_(CreateDeferredTransparentProgram(shaderCache_)),
      deferredFogProgram_(CreateDeferredFogProgram(shaderCache_, framebuffer_, textureBinder_)),
      skyboxProgram_(CreateSkyboxProgram(shaderCache_)),
      instanceBuffer_(CreateInstanceBuffer()),
      instanceBufferData_(instanceBuffer_.GetRegion<InstanceData>(0, sizeof(InstanceData) * maxInstanceCount)),
      cameraBuffer_(CreateCameraBuffer()),
      materialsBuffer_(CreateMaterialsBuffer()),
      materialsBufferData_(materialsBuffer_.GetRegion<MaterialData>(0, sizeof(MaterialData) * maxMaterialsCount)),
      lightsBuffer_(CreateLightsBuffer()),
      pointLightsData_(lightsBuffer_.GetRegion<PointLightData>(0, maxPointLights * sizeof(PointLightData))),
      dirLightsData_(lightsBuffer_.GetRegion<DirLightData>(pointLightsData_.GetCapacityBytes(), maxDirLights * sizeof(DirLightData))),
      vertexArray_(CreateVertexArray(deferredGeometryProgram_, instanceBuffer_))
{
    const auto width = window.GetWidth();
    const auto height = window.GetHeight();

    frameInfo_.Width = width;
    frameInfo_.Height = height;

    GL::Disable(EnableCap::Multisample);
    GL::Enable(EnableCap::ScissorTest);
    GL::Enable(EnableCap::CullFace);
    GL::Enable(EnableCap::DebugOutput);
    GL::Enable(EnableCap::DebugOutputSynchronous);
    GL::FrontFace(WindingOrder::CounterClockwise);
    GL::CullFace(Face::Back);
    GL::Viewport(0, 0, width, height);
    GL::Scissor(0, 0, width, height);
    // TODO move glDebugMessageCallback to GL.hpp
    glDebugMessageCallback(DebugMessageCallback, nullptr);

    ShaderProgram::ReleaseShaderCompiler();
}

Nova::Renderer::~Renderer() noexcept
{
    textureBinder_.UnbindAll(true);
}

void Nova::Renderer::Clear(float r, float g, float b, float a) const noexcept
{
    NV_PROFILE_FUNC;

    GL::ClearColor(r, g, b, a);
    GL::Clear(ClearMask::ColorBufferBit | ClearMask::DepthBufferBit);
}

void Nova::Renderer::SetCamera(const glm::mat4 &view, const glm::mat4 &projection) noexcept
{
    NV_PROFILE_FUNC;

    const auto cameraPosition = ExtractCameraPosition(view);
    cameraPosition_ = cameraPosition;

    auto cameraRegion = cameraBuffer_.GetRegion<CameraData>();
    auto cameraData = cameraRegion.GetBasePtr();
    cameraData->ViewMatrix = view;
    cameraData->ProjectionMatrix = projection;
    cameraData->Position = cameraPosition;

    const auto [zNear, zFar] = ExtractZPlanes(projection);
    cameraData->ZNear = zNear;
    cameraData->ZFar = zFar;

    cameraRegion.Commit();
}

void Nova::Renderer::SetCamera(
    const glm::mat4 &view,
    const glm::mat4 &projection,
    const glm::vec3 &position,
    float zNear,
    float zFar) noexcept
{
    NV_PROFILE_FUNC;

    cameraPosition_ = position;

    auto cameraRegion = cameraBuffer_.GetRegion<CameraData>();
    auto cameraData = cameraRegion.GetBasePtr();
    cameraData->ViewMatrix = view;
    cameraData->ProjectionMatrix = projection;
    cameraData->Position = position;
    cameraData->ZNear = zNear;
    cameraData->ZFar = zFar;

    cameraRegion.Commit();
}

void Nova::Renderer::SetViewport(const Rect<unsigned int> &viewport) noexcept
{
    NV_PROFILE_FUNC;

    frameInfo_.Viewport = viewport;

    GL::Viewport(
        viewport.X,
        viewport.Y,
        viewport.Width,
        viewport.Height);
}

void Nova::Renderer::SetViewport(
    const Rect<unsigned int> &viewport,
    const Rect<unsigned int> &scissor) noexcept
{
    NV_PROFILE_FUNC;

    frameInfo_.Viewport = viewport;
    frameInfo_.Scissor = scissor;

    GL::Viewport(
        viewport.X,
        viewport.Y,
        viewport.Width,
        viewport.Height);
    GL::Scissor(
        scissor.X,
        scissor.Y,
        scissor.Width,
        scissor.Height);
}

void Nova::Renderer::AddPointLight(
    const glm::vec4 &color,
    const glm::vec3 &position,
    float radius) noexcept
{
    if (!pointLightsData_.IsFull())
    {
        auto lightData = pointLightsData_.GetCurrentPtr();
        lightData->Color = color;
        lightData->Position = position;
        lightData->Radius = radius;

        pointLightsData_.Increment();
    }
}

void Nova::Renderer::AddDirectionalLight(
    const glm::vec4 &color,
    const glm::vec3 &direction)
{
    if (!dirLightsData_.IsFull())
    {
        auto lightData = dirLightsData_.GetCurrentPtr();
        lightData->Color = color;
        lightData->Direction = direction;

        dirLightsData_.Increment();
    }
}

void Nova::Renderer::Render(
    std::shared_ptr<Buffer> modelBuffer,
    const Material &material,
    const glm::mat4 &transform)
{
    Render(
        modelBuffer,
        material,
        transform,
        // TODO This should not be a responsibility of the renderer !!!
        BuildNormalTransformMatrix(transform));
}

void Nova::Renderer::Render(
    std::shared_ptr<Buffer> modelBuffer,
    const Material &material,
    const glm::mat4 &transform,
    const glm::mat3 &transformNormal)
{
    auto &instanceData = GetInstanceDataForModel(
        modelBuffer,
        material.Color.a == 0.0f);
    instanceData.emplace_back(
        InstanceData{
            .MaterialIndex = GetMaterialIndex(material),
            .Transform = transform,
            .NormalTransform = transformNormal,
        });
}

void Nova::Renderer::Draw(
    unsigned int frameWidth,
    unsigned int frameHeight,
    std::shared_ptr<Texture> skyboxTexture)
{
    if (frameWidth != frameInfo_.Width || frameHeight != frameInfo_.Height)
    {
        for (const auto &attachment : framebuffer_.GetAttachments())
            textureBinder_.Unbind(attachment, true);

        framebuffer_.Resize(frameWidth, frameHeight);

        deferredLightProgram_.SetUniform(
            "uGBufferAlbedoSpecular",
            textureBinder_.Bind(framebuffer_.GetAttachment(albedoAttachmentIndex), true));
        deferredLightProgram_.SetUniform(
            "uGBufferPosition",
            textureBinder_.Bind(framebuffer_.GetAttachment(positionAttachmentIndex), true));
        deferredLightProgram_.SetUniform(
            "uGBufferNormal",
            textureBinder_.Bind(framebuffer_.GetAttachment(normalAttachmentIndex), true));
        deferredLightProgram_.SetUniform(
            "uGBufferShininess",
            textureBinder_.Bind(framebuffer_.GetAttachment(shininessAttachmentIndex), true));
        deferredFogProgram_.SetUniform(
            "uSceneColor",
            textureBinder_.Bind(framebuffer_.GetAttachment(colorAttachmentIndex), true));
        deferredFogProgram_.SetUniform(
            "uDepth",
            textureBinder_.Bind(framebuffer_.GetAttachment(depthAttachmentIndex), true));

        frameInfo_.Width = frameWidth;
        frameInfo_.Height = frameHeight;
    }

    textureBinder_.Update();
    framebuffer_.Bind();

    GL::DepthMask(true);

    framebuffer_.ClearAttachment(albedoAttachmentIndex, frameInfo_.ClearColor);
    framebuffer_.ClearAttachment(positionAttachmentIndex, glm::vec4(0.0f));
    framebuffer_.ClearAttachment(normalAttachmentIndex, glm::vec4(0.0f));
    framebuffer_.ClearAttachment(colorAttachmentIndex, glm::vec4(0.0f));
    framebuffer_.ClearAttachment(finalAttachmentIndex, glm::vec4(0.0f));
    framebuffer_.ClearAttachment(shininessAttachmentIndex, glm::vec4(0.0f));
    framebuffer_.ClearAttachment(1.0f, 0);

    vertexArray_.Use();

    frameSync_.WaitClient(SyncTimeoutInfinite);

    materialsBufferData_.Commit();
    const auto pointLightsCount = static_cast<GLuint>(pointLightsData_.Commit());
    const auto dirLightsCount = static_cast<GLuint>(dirLightsData_.Commit());

    // geometry pass
    materialsBuffer_.Bind(
        BufferBaseTarget::ShaderStorageBuffer,
        deferredGeometryProgram_.GetResourceLocation("sMaterialData"));
    cameraBuffer_.Bind(
        BufferBaseTarget::UniformBuffer,
        deferredGeometryProgram_.GetResourceLocation("uCameraData"));

    deferredGeometryProgram_.Use();

    GL::Disable(EnableCap::Blend);
    GL::Enable(EnableCap::DepthTest);
    GL::DepthFunc(DepthFunction::LessEqual);
    GL::DepthMask(true);

    for (auto &[model, drawData] : drawData_)
    {
        DrawBatch(model, drawData.OpaqueInstanceData);
        drawData.OpaqueInstanceData.clear();
    }

    // lighting pass
    // TODO Get ambient intensity from frame config
    deferredLightProgram_.SetUniform("uAmbient", 0.5f);
    deferredLightProgram_.SetUniform("uPointLightsCount", pointLightsCount);
    deferredLightProgram_.SetUniform("uDirLightsCount", dirLightsCount);
    deferredLightProgram_.Use();

    lightsBuffer_.Bind(
        BufferBaseTarget::ShaderStorageBuffer,
        deferredLightProgram_.GetResourceLocation("sPointLightsBuffer"),
        0,
        sizeof(PointLightData) * maxPointLights);
    lightsBuffer_.Bind(
        BufferBaseTarget::ShaderStorageBuffer,
        deferredLightProgram_.GetResourceLocation("sDirLightsBuffer"),
        sizeof(PointLightData) * maxPointLights,
        sizeof(DirLightData) * maxDirLights);

    cameraBuffer_.Bind(
        BufferBaseTarget::UniformBuffer,
        deferredLightProgram_.GetResourceLocation("uCameraData"));

    GL::Disable(EnableCap::DepthTest);
    GL::DepthMask(false);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    framebuffer_.Invalidate({
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
    });

    // skybox pass
    if (skyboxTexture != nullptr)
    {
        skyboxProgram_.Use();
        skyboxProgram_.SetUniform("uSkybox", textureBinder_.Bind(*skyboxTexture));

        GL::Enable(EnableCap::DepthTest);
        GL::DepthFunc(DepthFunction::LessEqual);
        GL::DepthMask(false);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // transparent pass
    const auto hasAnyTransparentObjects = std::any_of(
        drawData_.begin(),
        drawData_.end(),
        [](const auto &x)
        {
            return x.second.TransparentInstanceData.size() > 0;
        });
    if (hasAnyTransparentObjects)
    {
        deferredTransparentProgram_.SetUniform("uAmbient", 0.5f);
        deferredTransparentProgram_.SetUniform("uPointLightsCount", pointLightsCount);
        deferredTransparentProgram_.SetUniform("uDirLightsCount", dirLightsCount);
        deferredTransparentProgram_.Use();

        materialsBuffer_.Bind(
            BufferBaseTarget::ShaderStorageBuffer,
            deferredTransparentProgram_.GetResourceLocation("sMaterialData"));

        cameraBuffer_.Bind(
            BufferBaseTarget::UniformBuffer,
            deferredTransparentProgram_.GetResourceLocation("uCameraData"));

        lightsBuffer_.Bind(
            BufferBaseTarget::ShaderStorageBuffer,
            deferredTransparentProgram_.GetResourceLocation("sPointLightsBuffer"),
            0,
            sizeof(PointLightData) * maxPointLights);

        lightsBuffer_.Bind(
            BufferBaseTarget::ShaderStorageBuffer,
            deferredTransparentProgram_.GetResourceLocation("sDirLightsBuffer"),
            sizeof(PointLightData) * maxPointLights,
            sizeof(DirLightData) * maxDirLights);

        GL::Enable(EnableCap::DepthTest);
        GL::DepthFunc(DepthFunction::LessEqual);
        GL::DepthMask(false);

        GL::Enable(EnableCap::Blend);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (auto &[model, drawData] : drawData_)
        {
            SortTransparentObjects(drawData.TransparentInstanceData, cameraPosition_);
            DrawBatch(model, drawData.TransparentInstanceData);
            drawData.TransparentInstanceData.clear();
        }
    }

    // fog pass
    deferredFogProgram_.Use();
    // TODO Get fog color from frame settings
    deferredFogProgram_.SetUniform("uFogColor", glm::vec4(1.0f, 1.0f, 1.0f, 0.01f));

    GL::Disable(EnableCap::DepthTest);
    GL::DepthMask(false);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    frameSync_.Set();
    framebuffer_.Unbind();
    materialsCache_.clear();
}

void Nova::Renderer::DisplayFramebuffer(const Window &window) const noexcept
{
    const auto &finalAttachment = framebuffer_.GetAttachment(finalAttachmentIndex);
    glNamedFramebufferReadBuffer(
        framebuffer_.GetID(),
        GL_COLOR_ATTACHMENT0 + finalAttachmentIndex);
    glBlitNamedFramebuffer(
        framebuffer_.GetID(),
        0,
        0,
        0,
        finalAttachment.GetWidth(),
        finalAttachment.GetHeight(),
        0,
        0,
        window.GetWidth(),
        window.GetHeight(),
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST);
}

std::vector<Nova::InstanceData> &Nova::Renderer::GetInstanceDataForModel(std::shared_ptr<Buffer> modelBuffer, bool useAlpha) noexcept
{
    const auto &it = drawData_.find(modelBuffer);
    if (it != drawData_.end())
        return useAlpha
                   ? it->second.TransparentInstanceData
                   : it->second.OpaqueInstanceData;

    const auto &[data, _] = drawData_.emplace(modelBuffer, DrawData{.Age = 0});
    return useAlpha
               ? data->second.TransparentInstanceData
               : data->second.OpaqueInstanceData;
}

GLuint Nova::Renderer::GetMaterialIndex(const Nova::Material &material)
{
    const auto albedoHandle = material.AlbedoTexture == nullptr
                                  ? whiteTextureHandle_
                                  : textureBinder_.Bind(*material.AlbedoTexture);

    const MaterialData materialData{
        .Color = material.Color,
        .SpecularIntensity = material.SpecularIntensity,
        .Shininess = material.Shininess,
        // TODO Check if we can just use 64-bit int, possibly swapping high and low order words
        .AlbedoHandleHigh = static_cast<GLuint>(albedoHandle & 0xffffffffzu),
        .AlbedoHandleLow = static_cast<GLuint>(albedoHandle >> 32zu),
    };

    const auto it = materialsCache_.find(materialData);
    if (it != materialsCache_.end())
        return it->second;

    const auto materialIndex = materialsBufferData_.GetSize();

    auto dataPtr = materialsBufferData_.GetCurrentPtr();
    *dataPtr = materialData;

    materialsBufferData_.Increment();

    return materialIndex;
}

void Nova::Renderer::DrawBatch(const std::shared_ptr<Buffer> &modelBuffer, std::span<const InstanceData> instanceData) noexcept
{
    vertexArray_.BindVertexBuffer(
        *modelBuffer,
        modelBufferBinding,
        sizeof(ModelVertexData));

    // TODO If instance data transfer doesn't overlap the currently rendered region, dont synchronize, just get next region and use gldraw*baseinstance. synchronize all regions from previous frame before draw
    instanceDataSync_.WaitClient(SyncTimeoutInfinite);

    auto instanceDataRegion = instanceBuffer_.GetRegion<InstanceData>(0, sizeof(InstanceData) * maxInstanceCount);
    instanceDataRegion.Copy(instanceData);
    instanceDataRegion.Commit();

    assert_fits_in<GLsizei>(modelBuffer->GetSize());
    assert_fits_in<GLsizei>(instanceData.size());

    glDrawArraysInstanced(
        GL_TRIANGLES,
        0,
        static_cast<GLsizei>(modelBuffer->GetSize()) / sizeof(ModelVertexData),
        static_cast<GLsizei>(instanceData.size()));

    instanceDataSync_.Set();
}