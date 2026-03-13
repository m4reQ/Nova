#pragma once
#include <Nova/core/Utility.hpp>

namespace Nova
{
    enum class DDSFourCC : uint32_t
    {
        DXT1 = 0x44585431,
        DXT2 = 0x44585432,
        DXT3 = 0x44585433,
        DXT4 = 0x44585434,
        DXT5 = 0x44585435,
        DX10 = 0x44583130,
    };

    enum class DDSPixelFormatFlags : uint32_t
    {
        AlphaPixels = 0x1,
        Alpha = 0x2,
        FourCC = 0x4,
        RGB = 0x40,
        YUV = 0x200,
        Luminance = 0x20000,
    };

    enum class DDSFlags : uint32_t
    {
        Caps = 0x1,
        Height = 0x2,
        Width = 0x4,
        Pitch = 0x8,
        PixelFormat = 0x1000,
        MipmapCount = 0x20000,
        LinearSize = 0x80000,
        Depth = 0x800000,
    };

    // https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
    enum class DXGIFormat : uint32_t
    {
        Unknown = 0,
        R32G32B32A32Typeless = 1,
        R32G32B32A32Float = 2,
        R32G32B32A32UInt = 3,
        R32G32B32A32SInt = 4,
        R32G32B32Typeless = 5,
        R32G32B32Float = 6,
        R32G32B32UInt = 7,
        R32G32B32SInt = 8,
        R16G16B16A16Typeless = 9,
        R16G16B16A16Float = 10,
        R16G16B16A16UNorm = 11,
        R16G16B16A16UInt = 12,
        R16G16B16A16SNorm = 13,
        R16G16B16A16SInt = 14,
        R32G32Typeless = 15,
        R32G32Float = 16,
        R32G32UInt = 17,
        R32G32SInt = 18,
        R32G8X24Typeless = 19,
        D32FloatS8X24UInt = 20,
        R32FloatX8X24Typeless = 21,
        X32TypelessG8X24UInt = 22,
        R10G10B10A2Typeless = 23,
        R10G10B10A2UNorm = 24,
        R10G10B10A2UInt = 25,
        R11G11B10Float = 26,
        R8G8B8A8Typeless = 27,
        R8G8B8A8UNorm = 28,
        R8G8B8A8UNormSRGB = 29,
        R8G8B8A8UInt = 30,
        R8G8B8A8SNorm = 31,
        R8G8B8A8SInt = 32,
        R16G16Typeless = 33,
        R16G16Float = 34,
        R16G16UNorm = 35,
        R16G16UInt = 36,
        R16G16SNorm = 37,
        R16G16SInt = 38,
        R32Typeless = 39,
        D32Float = 40,
        R32Float = 41,
        R32UInt = 42,
        R32SInt = 43,
        R24G8Typeless = 44,
        D24UNormS8UInt = 45,
        R24UNormX8Typeless = 46,
        X24TypelessG8UInt = 47,
        R8G8Typeless = 48,
        R8G8UNorm = 49,
        R8G8UInt = 50,
        R8G8SNorm = 51,
        R8G8SInt = 52,
        R16Typeless = 53,
        R16Float = 54,
        D16UNorm = 55,
        R16UNorm = 56,
        R16UInt = 57,
        R16SNorm = 58,
        R16SInt = 59,
        R8Typeless = 60,
        R8UNorm = 61,
        R8UInt = 62,
        R8SNorm = 63,
        R8SInt = 64,
        A8UNorm = 65,
        R1UNorm = 66,
        R9G9B9E5SharedExp = 67,
        R8G8B8G8UNorm = 68,
        G8R8G8B8UNorm = 69,
        BC1Typeless = 70,
        BC1UNorm = 71,
        BC1UNormSRGB = 72,
        BC2Typeless = 73,
        BC2UNorm = 74,
        BC2UNormSRGB = 75,
        BC3Typeless = 76,
        BC3UNorm = 77,
        BC3UNormSRGB = 78,
        BC4Typeless = 79,
        BC4UNorm = 80,
        BC4SNorm = 81,
        BC5Typeless = 82,
        BC5UNorm = 83,
        BC5SNorm = 84,
        B5G6R5UNorm = 85,
        B5G5R5A1UNorm = 86,
        B8G8R8A8UNorm = 87,
        B8G8R8X8UNorm = 88,
        R10G10B10XRBiasA2UNorm = 89,
        B8G8R8A8Typeless = 90,
        B8G8R8A8UNormSRGB = 91,
        B8G8R8X8Typeless = 92,
        B8G8R8X8UNormSRGB = 93,
        BC6HTypeless = 94,
        BC6HUF16 = 95,
        BC6HSF16 = 96,
        BC7Typeless = 97,
        BC7UNorm = 98,
        BC7UNormSRGB = 99,
        AYUV = 100,
        Y410 = 101,
        Y416 = 102,
        NV12 = 103,
        P010 = 104,
        P016 = 105,
        _420OPAQUE = 106,
        YUY2 = 107,
        Y210 = 108,
        Y216 = 109,
        NV11 = 110,
        AI44 = 111,
        IA44 = 112,
        P8 = 113,
        A8P8 = 114,
        B4G4R4A4UNorm = 115,
        P208 = 130,
        V208 = 131,
        V408 = 132,
        SamplerFeedbackMinMipOpaque = 189,
        SamplerFeedbackMinRegionUsedOpaque = 190,
        _ForceUInt = 0xffffffff,
    };

    // https://learn.microsoft.com/en-us/windows/win32/api/d3d10/ne-d3d10-d3d10_resource_dimension
    enum class D3D10ResourceDimension
    {
        Unknown = 0,
        Buffer = 1,
        Texture1D = 2,
        Texture2D = 3,
        Texture3D = 4
    };

    enum class DDSMiscFlags : unsigned int
    {
        TextureCube = 0x4,
    };

    enum class DDSMiscFlags2 : unsigned int
    {
        AlphaModeUnknown = 0x0,
        AlphaModeStraight = 0x1,
        AlphaModePremultiplied = 0x2,
        AlphaModeOpaque = 0x3,
        AlphaModeCustom = 0x4,
    };

    // https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-pixelformat
    struct DDSPixelFormat
    {
        uint32_t Size;
        uint32_t Flags;
        uint32_t FourCC;
        uint32_t RGBBitCount;
        uint32_t RBitMask;
        uint32_t GBitMask;
        uint32_t BBitMask;
        uint32_t ABitMask;
    };

    // https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-header-dxt10
    struct DDSHeaderDXT10
    {
        DXGIFormat Format;
        D3D10ResourceDimension resourceDimension;
        DDSMiscFlags MiscFlag;
        unsigned int ArraySize;
        DDSMiscFlags2 MiscFlags2;
    };

    // https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-header
    struct DDSHeader
    {
        uint32_t Size;
        DDSFlags Flags;
        uint32_t Height;
        uint32_t Width;
        uint32_t PitchOrLinearSize;
        uint32_t Depth;
        uint32_t MipMapCount;
        uint32_t _Reserved1[11];
        DDSPixelFormat PixelFormat;
        uint32_t Caps;
        uint32_t Caps2;
        uint32_t Caps3;
        uint32_t Caps4;
        uint32_t _Reserved2;
    };

    NV_DEFINE_BITWISE_OPERATORS(DDSFlags);

    NV_DEFINE_BITWISE_OPERATORS(DDSPixelFormatFlags);

    NV_DEFINE_BITWISE_OPERATORS(DDSMiscFlags);

    NV_DEFINE_BITWISE_OPERATORS(DDSMiscFlags2);
}