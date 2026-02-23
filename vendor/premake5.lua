project "xxHash"
    language "C"
    cdialect "C11"
    kind "StaticLib"

    architecture "x86_64"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "xxHash/xxhash.c",
        "xxHash/xxhash.h",
        "xxHash/xxh_x86dispatch.c",
        "xxHash/xxh_x86dispatch.h",
        "xxHash/xxh3.h",
    }

    includedirs {
        Include.xxHash,
    }

    defines {
        "XXH_STATIC_LINKING_ONLY",
    }

    filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
        optimize "Off"
    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "Full"
    filter { "configurations:Profile" }
        runtime "Release"
        symbols "On"
        optimize "On"
    filter {}

project "json"
    language "C++"
    cppdialect "C++20"
    kind "None"

    architecture "x86_64"

    files {
        "json/include/nlohmann/**.hpp",
        "json/single_include/nlohmann/**.hpp",
        "json/src/modules/**.cppm"
    }

    includedirs {
        Include.json,
    }

    filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
        optimize "Off"
    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "Full"
    filter { "configurations:Profile" }
        runtime "Release"
        symbols "On"
        optimize "On"
    filter {}

project "spdlog"
    language "C++"
    cppdialect "C++20"
    kind "None"

    architecture "x86_64"

    files {
        "spdlog/include/**.h",
    }

    includedirs {
        Include.spdlog
    }

    defines {
        "FMT_UNICODE=0"
    }

    filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
        optimize "Off"
    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "Full"
    filter { "configurations:Profile" }
        runtime "Release"
        symbols "On"
        optimize "On"
    filter { "toolset:msc" }
        buildoptions { "/utf-8" }
    filter {}

project "glfw"
    language "C"
    cdialect "C17"
    kind "StaticLib"
    architecture "x86_64"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "glfw/include/glfw3.h",
        "glfw/include/glfw3native.h",
        "glfw/src/internal.h",
        "glfw/src/platform.h",
        "glfw/src/mappings.h",
        "glfw/src/context.c",
        "glfw/src/init.c",
        "glfw/src/input.c",
        "glfw/src/monitor.c",
        "glfw/src/platform.c",
        "glfw/src/vulkan.c",
        "glfw/src/window.c",
        "glfw/src/egl_context.c",
        "glfw/src/osmesa_context.c",
        "glfw/src/null_platform.h",
        "glfw/src/null_joystick.h",
        "glfw/src/null_init.c",
        "glfw/src/null_monitor.c",
        "glfw/src/null_window.c",
        "glfw/src/null_joystick.c",
    }

    filter { "system:Windows" }
        systemversion "latest"
        staticruntime "On"

        files {
            "glfw/src/win32_time.h",
            "glfw/src/win32_thread.h",
            "glfw/src/win32_module.c",
            "glfw/src/win32_time.c",
            "glfw/src/win32_thread.c",
            "glfw/src/win32_platform.h",
            "glfw/src/win32_joystick.h",
            "glfw/src/win32_init.c",
            "glfw/src/win32_joystick.c",
            "glfw/src/win32_monitor.c",
            "glfw/src/win32_window.c",
            "glfw/src/wgl_context.c",
        }

        defines {
            "_GLFW_WIN32"
        }
    filter { "system:linux" }
        systemversion "latest"
        staticruntime "On"
        pic "On"

        files {
            "glfw/src/posix_time.h",
            "glfw/src/posix_thread.h",
            "glfw/src/posix_module.c",
            "glfw/src/posix_time.c",
            "glfw/src/posix_thread.c",
            "glfw/src/posix_poll.h",
            "glfw/src/posix_poll.c",
            "glfw/src/linux_joystick.h",
            "glfw/src/linux_joystick.c",
            "glfw/src/x11_platform.h",
            "glfw/src/xkb_unicode.h",
            "glfw/src/x11_init.c",
            "glfw/src/x11_monitor.c",
            "glfw/src/x11_window.c",
            "glfw/src/xkb_unicode.c",
            "glfw/src/glx_context.c",
        }

        defines {
            "_GLFW_X11"
        }
    filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
        optimize "Off"
        defines { "NV_DEBUG" }
    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "Full"
        defines { "NV_RELEASE" }
    filter { "configurations:Profile" }
        runtime "Release"
        symbols "On"
        optimize "On"
        defines { "NV_PROFILE" }
    filter {}

    includedirs {
        "glfw/include"
    }

project "glm"
    language "C++"
    cppdialect "C++20"
    kind "None"
    architecture "x86_64"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "glm/**.cpp",
        "glm/**.hpp",
        "glm/**.inl",
        "glm/**.h",
    }

    includedirs {
        Include.glm
    }

    filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
        optimize "Off"
        defines { "NV_DEBUG" }
    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "Full"
        defines { "NV_RELEASE" }
    filter { "configurations:Profile" }
        runtime "Release"
        symbols "On"
        optimize "On"
        defines { "NV_PROFILE" }
    filter {}

project "glad"
    language "C"
    cdialect "C17"
    kind "StaticLib"
    architecture "x86_64"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "glad/src/**.c",
    }

    includedirs {
        "glad/include"
    }

    filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
        optimize "Off"
        defines { "NV_DEBUG" }
    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "Full"
        defines { "NV_RELEASE" }
    filter { "configurations:Profile" }
        runtime "Release"
        symbols "On"
        optimize "On"
        defines { "NV_PROFILE" }
    filter {}

project "dotnet"
    language "C"
    cdialect "C17"
    kind "None"
    architecture "x86_64"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "dotnet/**.h",
    }

    filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
        optimize "Off"
        defines { "NV_DEBUG" }
    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "Full"
        defines { "NV_RELEASE" }
    filter { "configurations:Profile" }
        runtime "Release"
        symbols "On"
        optimize "On"
        defines { "NV_PROFILE" }
    filter {}

project "stb"
    language "C"
    cdialect "C17"
    kind "StaticLib"
    architecture "x86_64"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "stb/include/stb/**.h",
        "stb/_impl.c",
    }

    includedirs {
        "stb/include"
    }

    filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
        optimize "Off"
        defines { "NV_DEBUG" }
    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "Full"
        defines { "NV_RELEASE" }
    filter { "configurations:Profile" }
        runtime "Release"
        symbols "On"
        optimize "On"
        defines { "NV_PROFILE" }
    filter {}

project "volk"
    language "C"
    cdialect "C17"
    kind "StaticLib"
    architecture "x86_64"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "volk/volk.h",
        "volk/volk.c",
    }

    includedirs {
        "volk",
        Include.vulkan,
    }

    filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
        optimize "Off"
        defines { "NV_DEBUG" }
    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "Full"
        defines { "NV_RELEASE" }
    filter { "configurations:Profile" }
        runtime "Release"
        symbols "On"
        optimize "On"
        defines { "NV_PROFILE" }
    filter { "system:windows" }
        defines { "VK_USE_PLATFORM_WIN32_KHR" }
    filter {}

project "uuid_v4"
    language "C++"
    cppdialect "C++17"
    kind "None"
    architecture "x86_64"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "uuid_v4/**.h",
    }

    includedirs {
        "uuid_v4",
    }

    filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
        optimize "Off"
        defines { "NV_DEBUG" }
    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "Full"
        defines { "NV_RELEASE" }
    filter { "configurations:Profile" }
        runtime "Release"
        symbols "On"
        optimize "On"
        defines { "NV_PROFILE" }
    filter {}

project "imgui"
	language "C++"
    cppdialect "C++17"
    kind "StaticLib"
    architecture "x86_64"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "imgui/*.h",
        "imgui/*.cpp",
        "imgui/backends/imgui_impl_vulkan.h",
        "imgui/backends/imgui_impl_vulkan.cpp",
        "imgui/backends/imgui_impl_win32.h",
        "imgui/backends/imgui_impl_win32.cpp",
        "imgui/backends/imgui_impl_opengl3.h",
        "imgui/backends/imgui_impl_opengl3_loader.h",
        "imgui/backends/imgui_impl_opengl3.cpp",
        "imgui/backends/imgui_impl_glfw.h",
        "imgui/backends/imgui_impl_glfw.cpp",
    }

    links {
        "glfw"
    }

    includedirs {
        "imgui",
        Include.vulkan,
        Include.glfw,
    }

	filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
        optimize "Off"
        defines { "NV_DEBUG" }
    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "Full"
        defines { "NV_RELEASE" }
    filter { "configurations:Profile" }
        runtime "Release"
        symbols "On"
        optimize "On"
        defines { "NV_PROFILE" }
    filter {}
