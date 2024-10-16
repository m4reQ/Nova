cmake_minimum_required(VERSION 3.8.2)

project(NovaHost LANGUAGES CSharp)

set(CMAKE_VS_NUGET_PACKAGE_RESTORE ON)

file(GLOB_RECURSE NOVAHOST_SOURCES CONFIGURE_DEPENDS "src/*.cs")
add_library(NovaHost SHARED ${NOVAHOST_SOURCES})
# FIXME For some reason /nullable flag doesn't work, probably bcs it expects <Nullable> tag inside csproj
target_compile_options(NovaHost PRIVATE "/langversion:latest" "/unsafe" "/nullable")
set_target_properties(
    NovaHost PROPERTIES
    DOTNET_TARGET_FRAMEWORK "net8.0"
    DOTNET_SDK "Microsoft.NET.Sdk"
    VS_DOTNET_REFERENCES "System;System.Runtime.InteropServices")

# TODO Find a way to automatically run `dotnet restore` before build
# TODO Find a way to copy runtimeconfig.json file to output directory
