cmake_minimum_required(VERSION 3.22)

project(EditorScript LANGUAGES CSharp)

file(
    GLOB_RECURSE
    EDITORSCRIPT_SOURCES
    CONFIGURE_DEPENDS
    "src/*.cs")
add_library(
    EditorScript
    SHARED
    ${EDITORSCRIPT_SOURCES})
target_compile_options(
    EditorScript
    PRIVATE
    "/langversion:latest"
    "/unsafe"
    "-debug:portable")
set_target_properties(
    EditorScript
    PROPERTIES
    VS_GLOBAL_Nullable "enable"
    DOTNET_SDK "Microsoft.NET.Sdk"
    DOTNET_TARGET_FRAMEWORK "net9.0")