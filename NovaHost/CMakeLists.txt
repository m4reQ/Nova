cmake_minimum_required(VERSION 3.22)

project(NovaHost LANGUAGES CSharp)

file(
    GLOB_RECURSE
    NOVAHOST_SOURCES
    CONFIGURE_DEPENDS
    "src/*.cs")
add_library(
    NovaHost
    SHARED
    ${NOVAHOST_SOURCES})
target_compile_options(
    NovaHost
    PRIVATE
    "/langversion:latest"
    "/unsafe"
    "-debug:portable")
set_target_properties(
    NovaHost
    PROPERTIES
    VS_GLOBAL_Nullable "enable"
    DOTNET_SDK "Microsoft.NET.Sdk"
    DOTNET_TARGET_FRAMEWORK "net9.0")