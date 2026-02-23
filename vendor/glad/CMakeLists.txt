cmake_minimum_required(VERSION 3.22)

project(glad LANGUAGES C)

add_library(
    gl
    STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gl.c")
target_include_directories(
    gl
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(
    wgl
    STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/wgl.c")
target_include_directories(
    wgl
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include")