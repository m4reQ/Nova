project "NovaHost"
    language "C#"
    csversion "preview"
    dotnetframework "net8.0"
    clr "Unsafe"
    kind "SharedLib"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "src/**.cs",
        "NovaHost.runtimeconfig.json",
    }

    postbuildcommands {
        '{COPYFILE} "%{wks.location}NovaHost/NovaHost.runtimeconfig.json" "%{!cfg.targetdir}/NovaHost.runtimeconfig.json"'
    }

    vsprops {
        Nullable = "enable",
        AppendTargetFrameworkToOutputPath = "false"
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
    filter { "configurations:Profile", "toolset:msc" }
        buildoptions {
            "/PROFILE"
        }
    filter {}