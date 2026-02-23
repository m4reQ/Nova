VulkanSdkPath = os.getenv("VULKAN_SDK")
VulkanCompilerPath = "%{VulkanSdkPath}/Bin/glslangValidator.exe"

Include = {}
Include["dotnet"] = "%{wks.location}/vendor/dotnet"
Include["stb"] = "%{wks.location}/vendor/stb/include"
Include["volk"] = "%{wks.location}/vendor/volk"
Include["vulkan"] = "%{VulkanSdkPath}/Include"
Include["uuid_v4"] = "%{wks.location}/vendor/uuid_v4"
Include["imgui"] = "%{wks.location}/vendor/imgui"
Include["Nova"] = "%{wks.location}/Nova/include"
Include["Nova_VulkanRenderer"] = "%{wks.location}/Nova.Renderer.Vulkan/include"
Include["glad"] = "%{wks.location}/vendor/glad/include"
Include["glm"] = "%{wks.location}/vendor/glm"
Include["glfw"] = "%{wks.location}/vendor/glfw/include"
Include["spdlog"] = "%{wks.location}/vendor/spdlog/include"
Include["json"] = "%{wks.location}/vendor/json/include"
Include["xxHash"] = "%{wks.location}/vendor/xxHash"

TargetDir = "%{wks.location}/build/bin/%{cfg.buildcfg}"
ObjDir = "%{wks.location}/build/obj/%{cfg.buildcfg}"

Architecture = "x86_64"

workspace "Nova"
    configurations { "Debug", "Release", "Profile" }

    debuggertype "NativeWithManagedCore"

    startproject "NovaEditor"

    defines {
        "_UNICODE",
        "UNICODE"
    }

group "Nova"
    include "Nova"
    include "NovaHost"
    include "NovaScript"
group "Editor"
    include "NovaEditor"
    -- include "EditorScript"
group "vendor"
    include "vendor"
group ""
