project "Nova"
    language "C++"
    cppdialect "C++latest"
    kind "StaticLib"
    architecture "x86_64"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    flags { "MultiProcessorCompile" }

    files {
        "assets/**",
        "include/**.hpp",
        "src/**.cpp",
        "pch.cpp",
        "pch.hpp",
        "premake5.lua",
    }

    pchheader "pch.hpp"
    pchsource "pch.cpp"

    forceincludes {
        "pch.hpp"
    }

    includedirs {
        "include",
        ".", -- for precompiled header to work
        Include.stb,
        Include.dotnet,
        Include.glad,
        Include.uuid_v4,
        Include.imgui,
        Include.glm,
        Include.glfw,
        Include.spdlog,
        Include.json,
        Include.xxHash,
    }

    links {
        "dotnet",
        "stb",
        "glm",
        "Shlwapi",
        "spdlog",
        "glfw",
        "glad",
        "xxHash",
        "uuid_v4",
    }

    defines {
        "NV_BUILD",
        "NV_STATICLIB",
        "_CRT_SECURE_NO_WARNINGS",
        "NOMINMAX",
        "FMT_UNICODE=0",
        "GLFW_INCLUDE_NONE",
        "GLM_ENABLE_EXPERIMENTAL",
    }

    postbuildcommands { "{COPYDIR} assets %[%{TargetDir}/assets]" }

    filter { "toolset:msc" }
        buildoptions { "/utf-8" }
    filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
        optimize "Off"
        defines { "NV_DEBUG", "NV_ENABLE_LOG", "NV_ENABLE_PROFILE" }
        flags { "MultiProcessorCompile" }
    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "Full"
	linktimeoptimization "On"
        defines { "NV_RELEASE" }
        flags { "MultiProcessorCompile" }
    filter { "configurations:Profile" }
        runtime "Release"
        symbols "On"
        optimize "On"
        defines { "NV_PROFILE" }
        flags { "MultiProcessorCompile" }
    filter { "system:windows" }
        defines {
            "WIN32_LEAN_AND_MEAN",
            "NV_WINDOWS",
            "VK_USE_PLATFORM_WIN32_KHR",
        }
    filter { "system:linux" }
        defines { "NV_LINUX" }
    filter { "configurations:Release", "toolset:msc" }
        buildoptions {
            "/Wall",
            "/WX",
        }
    filter { "configurations:Profile", "toolset:msc" }
        linkoptions {
            "/PROFILE"
        }
    filter {}
