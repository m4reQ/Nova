#pragma once
#include <Nova/core/Utility.hpp>
#include <glad/gl.h>
#include <glm/vec4.hpp>
#include <string>
#include <span>
#include <concepts>

namespace Nova
{
    // TODO Add implicit casts to GLenum for strong typed enumerations

    enum class DepthFunction : GLenum
    {
        /// @brief Never passes.
        Never = GL_NEVER,

        /// @brief Passes if the incoming depth value is less than the stored depth value.
        Less = GL_LESS,

        /// @brief Passes if the incoming depth value is equal to the stored depth value.
        Equal = GL_EQUAL,

        /// @brief Passes if the incoming depth value is less than or equal to the stored depth value.
        LessEqual = GL_LEQUAL,

        /// @brief Passes if the incoming depth value is greater than the stored depth value.
        Greater = GL_GREATER,

        /// @brief Passes if the incoming depth value is not equal to the stored depth value.
        NotEqual = GL_NOTEQUAL,

        /// @brief Passes if the incoming depth value is greater than or equal to the stored depth value.
        GreaterEqual = GL_GEQUAL,

        /// @brief Always passes.
        Always = GL_ALWAYS,
    };

    enum class BufferMapFlags : GLbitfield
    {
        /// @brief Indicates that the returned pointer may be used to read buffer object data.
        /// No GL error is generated if the pointer is used to query a mapping which excludes this flag, but the result is undefined and system errors (possibly including program termination) may occur.
        MapReadBit = GL_MAP_READ_BIT,

        /// @brief Indicates that the returned pointer may be used to modify buffer object data.
        /// No GL error is generated if the pointer is used to modify a mapping which excludes this flag, but the result is undefined and system errors (possibly including program termination) may occur.
        MapWriteBit = GL_MAP_WRITE_BIT,

        /// @brief Indicates that the mapping is to be made in a persistent fashion and that the client intends to hold and use the returned pointer during subsequent GL operation.
        /// It is not an error to call drawing commands (render) while buffers are mapped using this flag.
        /// It is an error to specify this flag if the buffer's data store was not allocated through a call to the glBufferStorage command in which the GL_MAP_PERSISTENT_BIT was also set.
        MapPersistentBit = GL_MAP_PERSISTENT_BIT,

        /// @brief Indicates that a persistent mapping is also to be coherent.
        /// Coherent maps guarantee that the effect of writes to a buffer's data store by either the client or server will eventually become visible to the other without further intervention from the application.
        /// In the absence of this bit, persistent mappings are not coherent and modified ranges of the buffer store must be explicitly communicated to the GL, either by unmapping the buffer, or through a call to glFlushMappedBufferRange or glMemoryBarrier.
        MapCoherentBit = GL_MAP_COHERENT_BIT,

        /// @brief Indicates that the previous contents of the specified range may be discarded.
        /// Data within this range are undefined with the exception of subsequently written data.
        /// No GL error is generated if subsequent GL operations access unwritten data, but the result is undefined and system errors (possibly including program termination) may occur.
        /// This flag may not be used in combination with GL_MAP_READ_BIT.
        MapInvalidateRangeBit = GL_MAP_INVALIDATE_RANGE_BIT,

        /// @brief Indicates that the previous contents of the entire buffer may be discarded.
        /// Data within the entire buffer are undefined with the exception of subsequently written data.
        /// No GL error is generated if subsequent GL operations access unwritten data, but the result is undefined and system errors (possibly including program termination) may occur.
        /// This flag may not be used in combination with GL_MAP_READ_BIT.
        MapInvalidateBufferBit = GL_MAP_INVALIDATE_BUFFER_BIT,

        /// @brief Indicates that one or more discrete subranges of the mapping may be modified.
        /// When this flag is set, modifications to each subrange must be explicitly flushed by calling glFlushMappedBufferRange.
        /// No GL error is set if a subrange of the mapping is modified and not flushed, but data within the corresponding subrange of the buffer are undefined.
        /// This flag may only be used in conjunction with GL_MAP_WRITE_BIT.
        /// When this option is selected, flushing is strictly limited to regions that are explicitly indicated with calls to glFlushMappedBufferRange prior to unmap; if this option is not selected glUnmapBuffer will automatically flush the entire mapped range when called.
        MapFlushExplicitBit = GL_MAP_FLUSH_EXPLICIT_BIT,

        /// @brief Indicates that the GL should not attempt to synchronize pending operations on the buffer prior to returning from glMapBufferRange or glMapNamedBufferRange.
        /// No GL error is generated if pending operations which source or modify the buffer overlap the mapped region, but the result of such previous and any subsequent operations is undefined.
        MapUnsynchronizedBit = GL_MAP_UNSYNCHRONIZED_BIT,
    };

    enum class BufferStorageFlags : GLbitfield
    {
        /// @brief The contents of the data store may be updated after creation through calls to glBufferSubData.
        /// If this bit is not set, the buffer content may not be directly updated by the client.
        /// The data argument may be used to specify the initial content of the buffer's data store regardless of the presence of the GL_DYNAMIC_STORAGE_BIT.
        /// Regardless of the presence of this bit, buffers may always be updated with server-side calls such as glCopyBufferSubData and glClearBufferSubData.
        DynamicStorageBit = GL_DYNAMIC_STORAGE_BIT,

        /// @brief The data store may be mapped by the client for read access and a pointer in the client's address space obtained that may be read from.
        MapReadBit = GL_MAP_READ_BIT,

        /// @brief The data store may be mapped by the client for write access and a pointer in the client's address space obtained that may be written through.
        MapWriteBit = GL_MAP_WRITE_BIT,

        /// @brief The client may request that the server read from or write to the buffer while it is mapped.
        /// The client's pointer to the data store remains valid so long as the data store is mapped, even during execution of drawing or dispatch commands.
        MapPersistentBit = GL_MAP_PERSISTENT_BIT,

        /// @brief Shared access to buffers that are simultaneously mapped for client access and are used by the server will be coherent, so long as that mapping is performed using glMapBufferRange.
        /// That is, data written to the store by either the client or server will be immediately visible to the other with no further action taken by the application.
        /// In particular,
        /// 
        /// If GL_MAP_COHERENT_BIT is not set and the client performs a write followed by a call to the glMemoryBarrier command with the GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT set, then in subsequent commands the server will see the writes.
        /// 
        /// If GL_MAP_COHERENT_BIT is set and the client performs a write, then in subsequent commands the server will see the writes.
        ///
        /// If GL_MAP_COHERENT_BIT is not set and the server performs a write, the application must call glMemoryBarrier with the GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT set and then call glFenceSync with GL_SYNC_GPU_COMMANDS_COMPLETE (or glFinish). Then the CPU will see the writes after the sync is complete.
        ///
        /// If GL_MAP_COHERENT_BIT is set and the server does a write, the app must call glFenceSync with GL_SYNC_GPU_COMMANDS_COMPLETE (or glFinish). Then the CPU will see the writes after the sync is complete.
        MapCoherentBit = GL_MAP_COHERENT_BIT,

        /// @brief When all other criteria for the buffer storage allocation are met, this bit may be used by an implementation to determine whether to use storage that is local to the server or to the client to serve as the backing store for the buffer.
        ClientStorageBit = GL_CLIENT_STORAGE_BIT,
    };

    enum class TextureWrapMode : GLenum
    {
        ClampToEdge = GL_CLAMP_TO_EDGE,
        ClampToBorder = GL_CLAMP_TO_BORDER,
        MirroredRepeat = GL_MIRRORED_REPEAT,
        Repeat = GL_REPEAT,
        MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE,
    };

    enum class TextureMinFilter : GLenum
    {
        Nearest = GL_NEAREST,
        Linear = GL_LINEAR,
        NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
        LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
        NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
        LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR,
    };

    enum class TextureMagFilter : GLenum
    {
        Nearest = GL_NEAREST,
        Linear = GL_LINEAR,
    };

    enum class TextureParameterName : GLenum
    {
        DepthStencilTextureMode = GL_DEPTH_STENCIL_TEXTURE_MODE,
        TextureBaseLevel = GL_TEXTURE_BASE_LEVEL,
        TextureCompareFunc = GL_TEXTURE_COMPARE_FUNC,
        TextureCompareMode = GL_TEXTURE_COMPARE_MODE,
        TextureLODBias = GL_TEXTURE_LOD_BIAS,
        TextureMinFilter = GL_TEXTURE_MIN_FILTER,
        TextureMagFilter = GL_TEXTURE_MAG_FILTER,
        TextureMinLOD = GL_TEXTURE_MIN_LOD,
        TextureMaxLOD = GL_TEXTURE_MAX_LOD,
        TextureMaxLevel = GL_TEXTURE_MAX_LEVEL,
        TextureSwizzleR = GL_TEXTURE_SWIZZLE_R,
        TextureSwizzleG = GL_TEXTURE_SWIZZLE_G,
        TextureSwizzleB = GL_TEXTURE_SWIZZLE_B,
        TextureSwizzleA = GL_TEXTURE_SWIZZLE_A,
        TextureWrapS = GL_TEXTURE_WRAP_S,
        TextureWrapT = GL_TEXTURE_WRAP_T,
        TextureWrapR = GL_TEXTURE_WRAP_R,
        TextureBorderColor = GL_TEXTURE_BORDER_COLOR,
        TextureSwizzleRGBA = GL_TEXTURE_SWIZZLE_RGBA,
    };

    enum class FramebufferStatus : GLenum
    {
        Complete = GL_FRAMEBUFFER_COMPLETE,
        Undefined = GL_FRAMEBUFFER_UNDEFINED,
        IncompleteAttachment = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
        IncompleteMissingAttachment = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
        IncompleteDrawBuffer = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
        IncompleteReadBuffer = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
        Unsupported = GL_FRAMEBUFFER_UNSUPPORTED,
        IncompleteMultisample = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
        IncompleteLayerTargets = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS,
    };

    enum class FramebufferTarget : GLenum
    {
        DrawFramebuffer = GL_DRAW_FRAMEBUFFER,
        ReadFramebuffer = GL_READ_FRAMEBUFFER,
        Framebuffer = GL_FRAMEBUFFER,
    };

    enum class InternalFormat : GLenum
    {
        DepthComponent32F = GL_DEPTH_COMPONENT32F,
        DepthComponent24 = GL_DEPTH_COMPONENT24,
        DepthComponent16 = GL_DEPTH_COMPONENT16,
        Depth32FStencil8 = GL_DEPTH32F_STENCIL8,
        Depth24Stencil8 = GL_DEPTH24_STENCIL8,
        StencilIndex8 = GL_STENCIL_INDEX8,
        R8 = GL_R8,
        R8Snorm = GL_R8_SNORM,
        R16 = GL_R16,
        R16Snorm = GL_R16_SNORM,
        RG8 = GL_RG8,
        RG8Snorm = GL_RG8_SNORM,
        RG16 = GL_RG16,
        RG16Snorm = GL_RG16_SNORM,
        R3G3B2 = GL_R3_G3_B2,
        RGB4 = GL_RGB4,
        RGB5 = GL_RGB5,
        RGB8 = GL_RGB8,
        RGB8Snorm = GL_RGB8_SNORM,
        RGB10 = GL_RGB10,
        RGB12 = GL_RGB12,
        RGB16Snorm = GL_RGB16_SNORM,
        RGBA2 = GL_RGBA2,
        RGBA4 = GL_RGBA4,
        RGB5A1 = GL_RGB5_A1,
        RGBA8 = GL_RGBA8,
        RGBA8Snorm = GL_RGBA8_SNORM,
        RGB10A2 = GL_RGB10_A2,
        RGB10A2UI = GL_RGB10_A2UI,
        RGBA12 = GL_RGBA12,
        RGBA16 = GL_RGBA16,
        SRGB8 = GL_SRGB8,
        SRGB8Alpha8 = GL_SRGB8_ALPHA8,
        R16F = GL_R16F,
        RG16F = GL_RG16F,
        RGB16F = GL_RGB16F,
        RGBA16F = GL_RGBA16F,
        R32F = GL_R32F,
        RG32F = GL_RG32F,
        RGB32F = GL_RGB32F,
        RGBA32F = GL_RGBA32F,
        R11FG11FB10F = GL_R11F_G11F_B10F,
        RGB9E5 = GL_RGB9_E5,
        R8I = GL_R8I,
        R8UI = GL_R8UI,
        R16I = GL_R16I,
        R16UI = GL_R16UI,
        R32I = GL_R32I,
        R32UI = GL_R32UI,
        RG8I = GL_RG8I,
        RG8UI = GL_RG8UI,
        RG16I = GL_RG16I,
        RG16UI = GL_RG16UI,
        RG32I = GL_RG32I,
        RG32UI = GL_RG32UI,
        RGB8I = GL_RGB8I,
        RGB8UI = GL_RGB8UI,
        RGB16I = GL_RGB16I,
        RGB16UI = GL_RGB16UI,
        RGB32I = GL_RGB32I,
        RGB32UI = GL_RGB32UI,
        RGBA8I = GL_RGBA8I,
        RGBA8UI = GL_RGBA8UI,
        RGBA16I = GL_RGBA16I,
        RGBA16UI = GL_RGBA16UI,
        RGBA32I = GL_RGBA32I,
        RGBA32UI = GL_RGBA32UI,
    };

    enum class Attachment : GLenum
    {
        Depth = GL_DEPTH_ATTACHMENT,
        Stencil = GL_STENCIL_ATTACHMENT,
        DepthStencil = GL_DEPTH_STENCIL_ATTACHMENT,
        Color0 = GL_COLOR_ATTACHMENT0,
        Color1 = GL_COLOR_ATTACHMENT1,
        Color2 = GL_COLOR_ATTACHMENT2,
        Color3 = GL_COLOR_ATTACHMENT3,
        Color4 = GL_COLOR_ATTACHMENT4,
        Color5 = GL_COLOR_ATTACHMENT5,
        Color6 = GL_COLOR_ATTACHMENT6,
        Color7 = GL_COLOR_ATTACHMENT7,
        Color8 = GL_COLOR_ATTACHMENT8,
        Color9 = GL_COLOR_ATTACHMENT9,
        Color10 = GL_COLOR_ATTACHMENT10,
        Color11 = GL_COLOR_ATTACHMENT11,
        Color12 = GL_COLOR_ATTACHMENT12,
        Color13 = GL_COLOR_ATTACHMENT13,
        Color14 = GL_COLOR_ATTACHMENT14,
        Color15 = GL_COLOR_ATTACHMENT15,
        Color16 = GL_COLOR_ATTACHMENT16,
        Color17 = GL_COLOR_ATTACHMENT17,
        Color18 = GL_COLOR_ATTACHMENT18,
        Color19 = GL_COLOR_ATTACHMENT19,
        Color20 = GL_COLOR_ATTACHMENT20,
        Color21 = GL_COLOR_ATTACHMENT21,
        Color22 = GL_COLOR_ATTACHMENT22,
        Color23 = GL_COLOR_ATTACHMENT23,
        Color24 = GL_COLOR_ATTACHMENT24,
        Color25 = GL_COLOR_ATTACHMENT25,
        Color26 = GL_COLOR_ATTACHMENT26,
        Color27 = GL_COLOR_ATTACHMENT27,
        Color28 = GL_COLOR_ATTACHMENT28,
        Color29 = GL_COLOR_ATTACHMENT29,
        Color30 = GL_COLOR_ATTACHMENT30,
        Color31 = GL_COLOR_ATTACHMENT31,
    };

    enum class ClearMask : GLenum
    {
        ColorBufferBit = GL_COLOR_BUFFER_BIT,
        DepthBufferBit = GL_DEPTH_BUFFER_BIT,
        StencilBufferBit = GL_STENCIL_BUFFER_BIT,
    };

    enum class TextureTarget : GLenum
    {
        Texture1D = GL_TEXTURE_1D,
        Texture1DArray = GL_TEXTURE_1D_ARRAY,
        Texture2D = GL_TEXTURE_2D,
        Texture2DArray = GL_TEXTURE_2D_ARRAY,
        Texture2DMultisample = GL_TEXTURE_2D_MULTISAMPLE,
        Texture2DMultisampleArray = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
        Texture3D = GL_TEXTURE_3D,
        TextureCubeMap = GL_TEXTURE_CUBE_MAP,
        TextureCubeMapArray = GL_TEXTURE_CUBE_MAP_ARRAY,
        TextureBuffer = GL_TEXTURE_BUFFER,
    };

    enum class ProgramInterface1 : GLenum
    {
        Uniform = GL_UNIFORM,
        UniformBlock = GL_UNIFORM_BLOCK,
        ProgramInput = GL_PROGRAM_INPUT,
        ProgramOutput = GL_PROGRAM_OUTPUT,
        VertexSubroutine = GL_VERTEX_SUBROUTINE,
        TessControlSubroutine = GL_TESS_CONTROL_SUBROUTINE,
        TessEvaluationSubroutine = GL_TESS_EVALUATION_SUBROUTINE,
        GeometrySubroutine = GL_GEOMETRY_SUBROUTINE,
        FragmentSubroutine = GL_FRAGMENT_SUBROUTINE,
        ComputeSubroutine = GL_COMPUTE_SUBROUTINE,
        VertexSubroutineUniform = GL_VERTEX_SUBROUTINE_UNIFORM,
        TessControlSubroutineUniform = GL_TESS_CONTROL_SUBROUTINE_UNIFORM,
        TessEvaluationSubroutineUniform = GL_TESS_EVALUATION_SUBROUTINE_UNIFORM,
        GeometrySubroutineUniform = GL_GEOMETRY_SUBROUTINE_UNIFORM,
        FragmentSubroutineUniform = GL_FRAGMENT_SUBROUTINE_UNIFORM,
        ComputeSubroutineUniform = GL_COMPUTE_SUBROUTINE_UNIFORM,
        TransformFeedbackVarying = GL_TRANSFORM_FEEDBACK_VARYING,
        BufferVariable = GL_BUFFER_VARIABLE,
        ShaderStorageBlock = GL_SHADER_STORAGE_BLOCK,
    };

	enum class ProgramInterface2 : GLenum
	{
        Uniform = GL_UNIFORM,
        UniformBlock = GL_UNIFORM_BLOCK,
        AtomicCounterBuffer = GL_ATOMIC_COUNTER_BUFFER,
        ProgramInput = GL_PROGRAM_INPUT,
        ProgramOutput = GL_PROGRAM_OUTPUT,
        VertexSubroutine = GL_VERTEX_SUBROUTINE,
        TessControlSubroutine = GL_TESS_CONTROL_SUBROUTINE,
        TessEvaluationSubroutine = GL_TESS_EVALUATION_SUBROUTINE,
        GeometrySubroutine = GL_GEOMETRY_SUBROUTINE,
        FragmentSubroutine = GL_FRAGMENT_SUBROUTINE,
        ComputeSubroutine = GL_COMPUTE_SUBROUTINE,
        VertexSubroutineUniform = GL_VERTEX_SUBROUTINE_UNIFORM,
        TessControlSubroutineUniform = GL_TESS_CONTROL_SUBROUTINE_UNIFORM,
        TessEvaluationSubroutineUniform = GL_TESS_EVALUATION_SUBROUTINE_UNIFORM,
        GeometrySubroutineUniform = GL_GEOMETRY_SUBROUTINE_UNIFORM,
        FragmentSubroutineUniform = GL_FRAGMENT_SUBROUTINE_UNIFORM,
        ComputeSubroutineUniform = GL_COMPUTE_SUBROUTINE_UNIFORM,
        TransformFeedbackVarying = GL_TRANSFORM_FEEDBACK_VARYING,
        BufferVariable = GL_BUFFER_VARIABLE,
        ShaderStorageBlock = GL_SHADER_STORAGE_BLOCK,
        TransformFeedbackBuffer = GL_TRANSFORM_FEEDBACK_BUFFER,
	};

    enum class ProgramInterfacePName : GLenum
    {
        ActiveResources = GL_ACTIVE_RESOURCES,
        MaxNameLength = GL_MAX_NAME_LENGTH,
        MaxNumActiveVariables = GL_MAX_NUM_ACTIVE_VARIABLES,
        MaxNumCompatibleSubroutines = GL_MAX_NUM_COMPATIBLE_SUBROUTINES,
    };

    enum class ProgramResourceProps : GLenum
    {
        NameLength = GL_NAME_LENGTH,
        Type = GL_TYPE,
        ArraySize = GL_ARRAY_SIZE,
        Offest = GL_OFFSET,
        BlockIndex = GL_BLOCK_INDEX,
        ArrayStride = GL_ARRAY_STRIDE,
        MatrixStride = GL_MATRIX_STRIDE,
        IsRowMajor = GL_IS_ROW_MAJOR,
        AtomicCounterBufferIndex = GL_ATOMIC_COUNTER_BUFFER_INDEX,
        TextureBuffer = GL_TEXTURE_BUFFER,
        BufferBinding = GL_BUFFER_BINDING,
        BufferDataSize = GL_BUFFER_DATA_SIZE,
        NumActiveVariables = GL_NUM_ACTIVE_VARIABLES,
        ActiveVariables = GL_ACTIVE_VARIABLES,
        ReferencedByVertexShader = GL_REFERENCED_BY_VERTEX_SHADER,
        ReferencedByTessControlShader = GL_REFERENCED_BY_TESS_CONTROL_SHADER,
        ReferencedByTessEvaluationShader = GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
        ReferencedByGeometryShader = GL_REFERENCED_BY_GEOMETRY_SHADER,
        ReferencedByFragmentShader = GL_REFERENCED_BY_FRAGMENT_SHADER,
        ReferencedByComputeShader = GL_REFERENCED_BY_COMPUTE_SHADER,
        NumCompatibleSubroutines = GL_NUM_COMPATIBLE_SUBROUTINES,
        CompatibleSubroutines = GL_COMPATIBLE_SUBROUTINES,
        TopLevelArraySize = GL_TOP_LEVEL_ARRAY_SIZE,
        TopLevelArrayStride = GL_TOP_LEVEL_ARRAY_STRIDE,
        Location = GL_LOCATION,
        LocationIndex = GL_LOCATION_INDEX,
        IsPerPatch = GL_IS_PER_PATCH,
        LocationComponent = GL_LOCATION_COMPONENT,
        TransformFeedbackBufferIndex = GL_TRANSFORM_FEEDBACK_BUFFER_INDEX,
        TransformFeedbackBufferStride = GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE,
    };

    enum class BufferBaseTarget : GLenum
    {
        AtomicCounterBuffer = GL_ATOMIC_COUNTER_BUFFER,
        TransformFeedbackBuffer = GL_TRANSFORM_FEEDBACK_BUFFER,
        UniformBuffer = GL_UNIFORM_BUFFER,
        ShaderStorageBuffer =  GL_SHADER_STORAGE_BUFFER,
    };

    enum class BufferBindTarget : GLenum
    {
        ArrayBuffer = GL_ARRAY_BUFFER,
        AtomicCounterBuffer = GL_ATOMIC_COUNTER_BUFFER,
        CopyReadBuffer = GL_COPY_READ_BUFFER,
        CopyWriteBuffer = GL_COPY_WRITE_BUFFER,
        DispatchIndirectBuffer = GL_DISPATCH_INDIRECT_BUFFER,
        DrawIndirectBuffer = GL_DRAW_INDIRECT_BUFFER,
        ElementArrayBuffer = GL_ELEMENT_ARRAY_BUFFER,
        PixelPackBuffer = GL_PIXEL_PACK_BUFFER,
        PixelUnpackBUffer = GL_PIXEL_UNPACK_BUFFER,
        QueryBuffer = GL_QUERY_BUFFER,
        ShaderStorageBuffer = GL_SHADER_STORAGE_BUFFER,
        TextureBuffer = GL_TEXTURE_BUFFER,
        TransformFeedbackBuffer = GL_TRANSFORM_FEEDBACK_BUFFER,
        UniformBuffer = GL_UNIFORM_BUFFER,
    };

    enum class EnableCap : GLenum
    {
        /// @brief If enabled, blend the computed fragment color values with the values in the color buffers.
        /// See glBlendFunc.
        Blend = GL_BLEND,

        /// @brief If enabled, clip geometry against user-defined half space i.
        ClipDistance0 = GL_CLIP_DISTANCE0,

        /// @brief If enabled, clip geometry against user-defined half space i.
        ClipDistance1 = GL_CLIP_DISTANCE1,

        /// @brief If enabled, clip geometry against user-defined half space i.
        ClipDistance2 = GL_CLIP_DISTANCE2,

        /// @brief If enabled, clip geometry against user-defined half space i.
        ClipDistance3 = GL_CLIP_DISTANCE3,

        /// @brief If enabled, clip geometry against user-defined half space i.
        ClipDistance4 = GL_CLIP_DISTANCE4,

        /// @brief If enabled, clip geometry against user-defined half space i.
        ClipDistance5 = GL_CLIP_DISTANCE5,

        /// @brief If enabled, clip geometry against user-defined half space i.
        ClipDistance6 = GL_CLIP_DISTANCE6,

        /// @brief If enabled, clip geometry against user-defined half space i.
        ClipDistance7 = GL_CLIP_DISTANCE7,

        /// @brief If enabled, apply the currently selected logical operation to the computed fragment color and color buffer values.
        /// See glLogicOp.
        ColorLogicOp = GL_COLOR_LOGIC_OP,

        /// @brief If enabled, cull polygons based on their winding in window coordinates.
        /// See glCullFace.
        CullFace = GL_CULL_FACE,

        /// @brief If enabled, debug messages are produced by a debug context.
        /// When disabled, the debug message log is silenced.
        /// Note that in a non-debug context, very few, if any messages might be produced, even when GL_DEBUG_OUTPUT is enabled.
        DebugOutput = GL_DEBUG_OUTPUT,

        /// @brief If enabled, debug messages are produced synchronously by a debug context.
        // If disabled, debug messages may be produced asynchronously.
        // In particular, they may be delayed relative to the execution of GL commands, and the debug callback function may be called from a thread other than that in which the commands are executed.
        // See glDebugMessageCallback.
        DebugOutputSynchronous = GL_DEBUG_OUTPUT_SYNCHRONOUS,

        /// @brief If enabled, the -wc<=zc<=wc plane equation is ignored by view volume clipping (effectively, there is no near or far plane clipping).
        // See glDepthRange.
        DepthClamp = GL_DEPTH_CLAMP,

        /// @brief If enabled, do depth comparisons and update the depth buffer.
        /// Note that even if the depth buffer exists and the depth mask is non-zero, the depth buffer is not updated if the depth test is disabled.
        /// See glDepthFunc and glDepthRange.
        DepthTest = GL_DEPTH_TEST,

        /// @brief If enabled, dither color components or indices before they are written to the color buffer.
        Dither = GL_DITHER,

        /// @brief If enabled and the value of GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING for the framebuffer attachment corresponding to the destination buffer is GL_SRGB, the R, G, and B destination color values (after conversion from fixed-point to floating-point) are considered to be encoded for the sRGB color space and hence are linearized prior to their use in blending.
        FramebufferSRGB = GL_FRAMEBUFFER_SRGB,

        /// @brief If enabled, draw lines with correct filtering. Otherwise, draw aliased lines.
        /// See glLineWidth.
        LineSmooth = GL_LINE_SMOOTH,

        /// @brief If enabled, use multiple fragment samples in computing the final color of a pixel.
        /// See glSampleCoverage.
        Multisample = GL_MULTISAMPLE,

        /// @brief If enabled, and if the polygon is rendered in GL_FILL mode, an offset is added to depth values of a polygon's fragments before the depth comparison is performed.
        /// See glPolygonOffset.
        PolygonOffsetFill = GL_POLYGON_OFFSET_FILL,

        /// @brief If enabled, and if the polygon is rendered in GL_LINE mode, an offset is added to depth values of a polygon's fragments before the depth comparison is performed.
        /// See glPolygonOffset.
        PolygonOffsetLine = GL_POLYGON_OFFSET_LINE,
        
        /// @brief If enabled, an offset is added to depth values of a polygon's fragments before the depth comparison is performed, if the polygon is rendered in GL_POINT mode.
        /// See glPolygonOffset.
        PolygonOffsetPoint = GL_POLYGON_OFFSET_POINT,

        /// @brief If enabled, draw polygons with proper filtering.
        /// Otherwise, draw aliased polygons.
        /// For correct antialiased polygons, an alpha buffer is needed and the polygons must be sorted front to back.
        PolygonSmooth = GL_POLYGON_SMOOTH,

        /// @brief Enables primitive restarting.
        /// If enabled, any one of the draw commands which transfers a set of generic attribute array elements to the GL will restart the primitive when the index of the vertex is equal to the primitive restart index.
        /// See glPrimitiveRestartIndex.
        PrimitiveRestart = GL_PRIMITIVE_RESTART,

        /// @brief Enables primitive restarting with a fixed index.
        /// If enabled, any one of the draw commands which transfers a set of generic attribute array elements to the GL will restart the primitive when the index of the vertex is equal to the fixed primitive index for the specified index type.
        /// The fixed index is equal to 2n-1 where n is equal to 8 for GL_UNSIGNED_BYTE, 16 for GL_UNSIGNED_SHORT and 32 for GL_UNSIGNED_INT.
        PrimitiveRestartFixedIndex = GL_PRIMITIVE_RESTART_FIXED_INDEX,

        /// @brief If enabled, primitives are discarded after the optional transform feedback stage, but before rasterization.
        // Furthermore, when enabled, glClear, glClearBufferData, glClearBufferSubData, glClearTexImage, and glClearTexSubImage are ignored.
        RasterizerDiscard = GL_RASTERIZER_DISCARD,

        /// @brief If enabled, compute a temporary coverage value where each bit is determined by the alpha value at the corresponding sample location.
        /// The temporary coverage value is then ANDed with the fragment coverage value.
        SampleAlphaToCoverage = GL_SAMPLE_ALPHA_TO_COVERAGE,

        /// @brief If enabled, each sample alpha value is replaced by the maximum representable alpha value.
        SampleAlphaToOne = GL_SAMPLE_ALPHA_TO_ONE,

        /// @brief If enabled, the fragment's coverage is ANDed with the temporary coverage value.
        /// If GL_SAMPLE_COVERAGE_INVERT is set to GL_TRUE, invert the coverage value.
        /// See glSampleCoverage.
        SampleCoverage = GL_SAMPLE_COVERAGE,

        /// @brief If enabled, the active fragment shader is run once for each covered sample, or at fraction of this rate as determined by the current value of GL_MIN_SAMPLE_SHADING_VALUE.
        /// See glMinSampleShading.
        SampleShading = GL_SAMPLE_SHADING,

        /// @brief If enabled, the sample coverage mask generated for a fragment during rasterization will be ANDed with the value of GL_SAMPLE_MASK_VALUE before shading occurs.
        /// See glSampleMaski.
        SampleMask = GL_SAMPLE_MASK,

        /// @brief If enabled, discard fragments that are outside the scissor rectangle.
        /// See glScissor.
        ScissorTest = GL_SCISSOR_TEST,

        /// @brief If enabled, do stencil testing and update the stencil buffer.
        /// See glStencilFunc and glStencilOp.
        StencilMask = GL_STENCIL_TEST,

        /// @brief If enabled, cubemap textures are sampled such that when linearly sampling from the border between two adjacent faces, texels from both faces are used to generate the final sample value.
        /// When disabled, texels from only a single face are used to construct the final sample value.
        TextureCubeMapSeamless = GL_TEXTURE_CUBE_MAP_SEAMLESS,

        /// @brief If enabled and a vertex or geometry shader is active, then the derived point size is taken from the (potentially clipped) shader builtin gl_PointSize and clamped to the implementation-dependent point size range.
        ProgramPointSize = GL_PROGRAM_POINT_SIZE,
    };

	namespace GL
	{
        /// <summary>
        /// Query a property of an interface in a program
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetProgramInterface.xhtml
        /// </summary>
        /// <param name="program">The name of a program object whose interface to query.</param>
        /// <param name="programInterface">A token identifying the interface within program to query.</param>
        /// <param name="pName">The name of the parameter within programInterface to query.</param>
        /// <returns>The value of pname for the program interface.</returns>
        inline GLint GetProgramInterface(
            GLuint program,
            ProgramInterface2 programInterface,
            ProgramInterfacePName pName) noexcept
        {
            GLint result{};
            glGetProgramInterfaceiv(
                program,
                (GLenum)programInterface,
                (GLenum)pName,
                &result);

            return result;
        }

        /// @brief Copy all or part of the data store of a buffer object to the data store of another buffer object.
        ///
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCopyBufferSubData.xhtml
        /// @param readBuffer Specifies the name of the source buffer object.
        /// @param writeBuffer Specifies the name of the destination buffer object.
        /// @param readOffset Specifies the offset, in basic machine units, within the data store of the source buffer object at which data will be read.
        /// @param writeOffset Specifies the offset, in basic machine units, within the data store of the destination buffer object at which data will be written.
        /// @param size Specifies the size, in basic machine units, of the data to be copied from the source buffer object to the destination buffer object.
        inline void CopyNamedBufferSubData(
            GLuint readBuffer,
            GLuint writeBuffer,
            GLintptr readOffset,
            GLintptr writeOffset,
            GLsizeiptr size) noexcept
        {
            glCopyNamedBufferSubData(readBuffer, writeBuffer, readOffset, writeOffset, size);
        }

        /// @brief Map all or part of a buffer object's data store into the client's address space.
        ///
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMapBufferRange.xhtml
        /// @param buffer Specifies the name of the buffer object.
        /// @param offset Specifies the starting offset within the buffer of the range to be mapped.
        /// @param length Specifies the length of the range to be mapped.
        /// @param access Specifies a combination of access flags indicating the desired access to the mapped range.
        inline void* MapNamedBufferRange(
            GLuint buffer,
            GLintptr offset,
            GLsizeiptr length,
            BufferMapFlags access) noexcept
        {
            return glMapNamedBufferRange(buffer, offset, length, (GLbitfield)access);
        }

        /// <summary>
        /// Query the name of an indexed resource within a program
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetProgramResourceName.xhtml
        /// </summary>
        /// <param name="program">The name of a program object whose resources to query.</param>
        /// <param name="programInterface">A token identifying the interface within program containing the indexed resource.</param>
        /// <param name="index">The index of the resource within programInterface of program.</param>
        /// <param name="bufSize">The initial size of the string buffer.</param>
        /// <returns>The name of the resource.</returns>
        inline std::string GetProgramResourceName(
            GLuint program,
            ProgramInterface1 programInterface,
            GLuint index,
            GLsizei bufSize) noexcept
        {
            std::string result(bufSize, '\0');
            GLsizei length = 0;

            glGetProgramResourceName(
                program,
                (GLenum)programInterface,
                index,
                (GLsizei)result.capacity(),
                &length,
                result.data());

            result.resize(length);
            return result;
        }

        /// <summary>
        /// Retrieve values for multiple properties of a single active resource within a program object
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetProgramResource.xhtml
        /// </summary>
        /// <param name="program">The name of a program object whose resources to query.</param>
        /// <param name="programInterface">A token identifying the interface within program containing the resource named name.</param>
        /// <param name="index">The index of the resource within programInterface of program.</param>
        /// <param name="props">Properties of the resource to be queried.</param>
        /// <param name="params">Buffer to which queried values will be written.</param>
        inline void GetProgramResource(
            GLuint program,
            ProgramInterface2 programInterface,
            GLuint index,
            const std::span<ProgramResourceProps> props,
            std::span<GLint> params) noexcept
        {
            glGetProgramResourceiv(
                program,
                (GLenum)programInterface,
                index,
                (GLsizei)props.size(),
                (const GLenum*)props.data(),
                (GLsizei)params.size_bytes(),
                nullptr,
                params.data());
        }

        /// <summary>
        /// Create vertex array object.
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCreateVertexArrays.xhtml
        /// </summary>
        /// <returns>Previously unused vertex array object name, representing a new vertex array object initialized to the default state.</returns>
        inline GLuint CreateVertexArray() noexcept
        {
            GLuint result = 0;
            glCreateVertexArrays(1, &result);

            return result;
        }

        /// @brief Create buffer object.
        ///
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCreateBuffers.xhtml
        /// @return Previously unused buffer name, representing a new buffer object initialized as if it had been bound to an unspecified target.
        inline GLuint CreateBuffer() noexcept
        {
            GLuint result = 0;
            glCreateBuffers(1, &result);

            return result;
        }

        /// @brief Creates and initializes a buffer object's immutable data store.
        /// @param buffer Specifies the name of the buffer object.
        /// @param size Specifies the size in bytes of the buffer object's new data store.
        /// @param data Specifies a pointer to data that will be copied into the data store for initialization, or NULL if no data is to be copied.
        /// @param flags Specifies the intended usage of the buffer's data store.
        inline void NamedBufferStorage(GLuint buffer, GLsizeiptr size, const void *data, BufferStorageFlags flags) noexcept
        {
            glNamedBufferStorage(buffer, size, data, (GLenum)flags);
        }

        /// <summary>
        /// Bind a buffer to a vertex buffer bind point.
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindVertexBuffer.xhtml
        /// </summary>
        /// <param name="vao">Specifies the name of the vertex array object.</param>
        /// <param name="bindingIndex">The index of the vertex buffer binding point to which to bind the buffer.</param>
        /// <param name="buffer">The name of a buffer to bind to the vertex buffer binding point.</param>
        /// <param name="offset">The offset of the first element of the buffer.</param>
        /// <param name="stride">The distance between elements within the buffer.</param>
        inline void VertexArrayVertexBuffer(
            GLuint vao,
            GLuint bindingIndex,
            GLuint buffer,
            GLintptr offset,
            GLsizei stride) noexcept
        {
            glVertexArrayVertexBuffer(vao, bindingIndex, buffer, offset, stride);
        }

        /// <summary>
        /// Modify the rate at which generic vertex attributes advance.
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glVertexBindingDivisor.xhtml
        /// </summary>
        /// <param name="vao">Specifies the name of the vertex array object.</param>
        /// <param name="bindingIndex">The index of the binding whose divisor to modify.</param>
        /// <param name="divisor">The new value for the instance step rate to apply.</param>
        inline void VertexArrayBindingDivisor(
            GLuint vao,
            GLuint bindingIndex,
            GLuint divisor) noexcept
        {
            glVertexArrayBindingDivisor(vao, bindingIndex, divisor);
        }

        /// <summary>
        /// Associate a vertex attribute and a vertex buffer binding for a vertex array object.
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glVertexAttribBinding.xhtml
        /// </summary>
        /// <param name="vao">Specifies the name of the vertex array object.</param>
        /// <param name="attribIndex">The index of the attribute to associate with a vertex buffer binding.</param>
        /// <param name="bindingIndex">The index of the vertex buffer binding with which to associate the generic vertex attribute.</param>
        inline void VertexArrayAttribBinding(
            GLuint vao,
            GLuint attribIndex,
            GLuint bindingIndex) noexcept
        {
            glVertexArrayAttribBinding(vao, attribIndex, bindingIndex);
        }

        /// <summary>
        /// Configures element array buffer binding of a vertex array object.
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glVertexArrayElementBuffer.xhtml
        /// </summary>
        /// <param name="vao">Specifies the name of the vertex array object.</param>
        /// <param name="buffer">Specifies the name of the buffer object to use for the element array buffer binding.</param>
        inline void VertexArrayElementBuffer(GLuint vao, GLuint buffer) noexcept
        {
            glVertexArrayElementBuffer(vao, buffer);
        }

        /// <summary>
        /// Enable or disable a generic vertex attribute array.
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glEnableVertexAttribArray.xhtml
        /// </summary>
        /// <param name="vao">Specifies the name of the vertex array object.</param>
        /// <param name="index">Specifies the index of the generic vertex attribute to be enabled or disabled.</param>
        inline void EnableVertexArrayAttrib(GLuint vao, GLuint index) noexcept
        {
            glEnableVertexArrayAttrib(vao, index);
        }

        /// <summary>
        /// Bind a vertex array object.
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindVertexArray.xhtml
        /// </summary>
        /// <param name="vao">Specifies the name of the vertex array to bind.</param>
        inline void BindVertexArray(GLuint vao) noexcept
        {
            glBindVertexArray(vao);
        }

        /// <summary>
        /// Attach a level of a texture object as a logical buffer of a framebuffer object.
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glFramebufferTexture.xhtml
        /// </summary>
        /// <param name="framebuffer">Specifies the name of the framebuffer object.</param>
        /// <param name="attachment">Specifies the attachment point of the framebuffer.</param>
        /// <param name="texture">Specifies the name of an existing texture object to attach.</param>
        /// <param name="level">Specifies the mipmap level of the texture object to attach.</param>
        inline void NamedFramebufferTexture(
            GLuint framebuffer,
            Attachment attachment,
            GLuint texture,
            GLint level) noexcept
        {
            glNamedFramebufferTexture(framebuffer, (GLenum)attachment, texture, level);
        }

        // TODO Add docs for TextureParameter

        inline void TextureParameter(GLuint texture, TextureParameterName parameter, GLfloat value) noexcept
        {
            glTextureParameterf(texture, (GLenum)parameter, value);
        }

        inline void TextureParameter(GLuint texture, TextureParameterName parameter, GLint value) noexcept
        {
            glTextureParameteri(texture, (GLenum)parameter, value);
        }

        // TODO Add missing overloads for other texture parameters

        inline void TextureParameter(GLuint texture, TextureMinFilter minFilter) noexcept
        {
            TextureParameter(texture, TextureParameterName::TextureMinFilter, (GLint)minFilter);
        }

        inline void TextureParameter(GLuint texture, TextureMagFilter magFilter) noexcept
        {
            TextureParameter(texture, TextureParameterName::TextureMagFilter, (GLint)magFilter);
        }

        /// <summary>
        /// check the completeness status of a framebuffer
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCheckFramebufferStatus.xhtml
        /// </summary>
        /// <param name="framebuffer">Specifies the name of the framebuffer object.</param>
        /// <param name="target">Specify the target against which framebuffer completeness is checked.</param>
        /// <returns>The completeness status of a framebuffer object when treated as a read or draw framebuffer, depending on the value of target.</returns>
        inline FramebufferStatus CheckNamedFramebufferStatus(GLuint framebuffer, FramebufferTarget target) noexcept
        {
            return (FramebufferStatus)glCheckNamedFramebufferStatus(framebuffer, (GLenum)target);
        }

        /// @brief Specify clear values for the color buffers.
        /// 
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glClearColor.xhtml
        /// @param red Specify the red value used when the color buffers are cleared. The initial values is 0.
        /// @param green Specify the green value used when the color buffers are cleared. The initial values is 0.
        /// @param blue Specify the blue value used when the color buffers are cleared. The initial values is 0.
        /// @param alpha Specify the alpha value used when the color buffers are cleared. The initial values is 0.
        inline void ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha = 1.0f) noexcept
        {
            glClearColor(red, green, blue, alpha);
        }

        /// @brief Specify clear values for the color buffers.
        /// 
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glClearColor.xhtml
        /// @param color Specify the red, green, blue, and alpha values used when the color buffers are cleared. The initial values are all 0.
        inline void ClearColor(const glm::vec4& color) noexcept
        {
            glClearColor(color.r, color.g, color.b, color.a);
        }

        /// @brief Specify the clear value for the depth buffer.
        ///
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glClearDepth.xhtml
        /// @param depth Specifies the depth value used when the depth buffer is cleared. The initial value is 1.
        inline void ClearDepth(GLdouble depth) noexcept
        {
            glClearDepth(depth);
        }

        /// @brief Specify the clear value for the depth buffer.
        ///
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glClearDepth.xhtml
        /// @param depth Specifies the depth value used when the depth buffer is cleared. The initial value is 1.
        inline void ClearDepth(GLfloat depth) noexcept
        {
            glClearDepthf(depth);
        }

        /// @brief Specify the clear value for the stencil buffer.
        /// 
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glClearStencil.xhtml
        /// @param stencil Specifies the index used when the stencil buffer is cleared. The initial value is 0.
        inline void ClearStencil(GLint stencil) noexcept
        {
            glClearStencil(stencil);
        }

        /// @brief Clear buffers to preset values.
        ///
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glClear.xhtml
        /// @param mask Bitwise OR of masks that indicate the buffers to be cleared.
        inline void Clear(ClearMask mask) noexcept
        {
            glClear((GLenum)mask);
        }

        /// @brief Bind a buffer object to an indexed buffer target.
        ///
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindBufferBase.xhtml
        /// @param target Specify the target of the bind operation.
        /// @param index Specify the index of the binding point within the array specified by target.
        /// @param buffer The name of a buffer object to bind to the specified binding point.
        inline void BindBufferBase(BufferBaseTarget target, GLuint index, GLuint buffer) noexcept
        {
            glBindBufferBase((GLenum)target, index, buffer);
        }

        /// @brief Bind a range within a buffer object to an indexed buffer target.
        /// 
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindBufferRange.xhtml
        /// @param target Specify the target of the bind operation.
        /// @param index Specify the index of the binding point within the array specified by target.
        /// @param buffer The name of a buffer object to bind to the specified binding point.
        /// @param offset The starting offset in basic machine units into the buffer object.
        /// @param size The amount of data in machine units that can be read from the buffer object while used as an indexed target.
        inline void BindBufferRange(BufferBaseTarget target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) noexcept
        {
            glBindBufferRange((GLenum)target, index, buffer, offset, size);
        }

        /// @brief Bind a named buffer object.
        /// 
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindBuffer.xhtml
        /// @param target Specifies the target to which the buffer object is bound.
        /// @param buffer Specifies the name of a buffer object.
        inline void BindBuffer(BufferBindTarget target, GLuint buffer) noexcept
        {
            glBindBuffer((GLenum)target, buffer);
        }

        /// @brief Enable server-side GL capabilities.
        ///
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glEnable.xhtml
        /// @param cap Specifies a symbolic constant indicating a GL capability.
        inline void Enable(EnableCap cap) noexcept
        {
            glEnable((GLenum)cap);
        }

        /// @brief Disable server-side GL capabilities.
        ///
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glEnable.xhtml
        /// @param cap Specifies a symbolic constant indicating a GL capability.
        inline void Disable(EnableCap cap) noexcept
        {
            glDisable((GLenum)cap);
        }

        /// @brief Create texture object.
        ///
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCreateTextures.xhtml
        /// @param target Specifies the effective texture target of created texture.
        /// @return Previously unused texture name, representing a new texture object of the dimensionality and type specified by target and initialized to the default values for that texture type.
        inline GLuint CreateTexture(TextureTarget target)
        {
            GLuint id = 0;
            glCreateTextures((GLenum)target, 1, &id);

            return id;
        }

        /// @brief Simultaneously specify storage for all levels of a two-dimensional or one-dimensional array texture.
        ///
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml
        /// @param texture Specifies the texture object name.
        /// @param levels Specify the number of texture levels.
        /// @param internalformat Specifies the sized internal format to be used to store texture image data.
        /// @param width Specifies the width of the texture, in texels.
        /// @param height Specifies the height of the texture, in texels. 
        inline void TextureStorage2D(GLuint texture, GLsizei levels, InternalFormat internalformat, GLsizei width, GLsizei height)
        {
            glTextureStorage2D(texture, levels, (GLenum)internalformat, width, height);
        }

        /// @brief enable or disable writing into the depth buffer
        ///
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDepthMask.xhtml
        /// @param flag Specifies whether the depth buffer is enabled for writing. If flag is false, depth buffer writing is disabled. Otherwise, it is enabled. Initially, depth buffer writing is enabled.
        inline void DepthMask(bool flag) noexcept
        {
            glDepthMask(flag ? GL_TRUE : GL_FALSE);
        }

        /// @brief Specify the value used for depth buffer comparisons.
        ///
        /// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDepthFunc.xhtml
        /// @param func Specifies the depth comparison function.
        inline void DepthFunc(DepthFunction func) noexcept
        {
            glDepthFunc((GLenum)func);
        }
	}

    template <std::integral T>
    constexpr Attachment operator+(Attachment a, T b) noexcept
    {
        using t = std::underlying_type_t<Attachment>;
        return (Attachment)((t)(a) + (t)(b));
    }

    NV_DEFINE_BITWISE_OPERATORS(ClearMask);
    NV_DEFINE_BITWISE_OPERATORS(BufferStorageFlags);
    NV_DEFINE_BITWISE_OPERATORS(BufferMapFlags);
}