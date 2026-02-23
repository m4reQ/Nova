project "NovaEditor"
    language "C++"
    cppdialect "C++20"
    kind "ConsoleApp"
    architecture "x86_64"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    debugdir "%{TargetDir}"

    files {
        "src/**.cpp",
        "src/**.hpp",
        "premake5.lua",
    }

    includedirs {
        Include.stb,
        Include.dotnet,
        Include.Nova,
        Include.imgui,
        Include.spdlog,
        Include.glm,
        Include.glad,
        Include.json,
        Include.glfw,
        Include.uuid_v4,
    }

    links {
        "Nova",
        "imgui",
    }

    defines {
        "_CRT_SECURE_NO_WARNINGS",
        "FMT_UNICODE=0",
        "NV_STATICLIB",
        "GLFW_INCLUDE_NONE",
        "GLM_ENABLE_EXPERIMENTAL",
    }

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
