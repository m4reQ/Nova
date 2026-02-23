cmake_minimum_required(VERSION 3.22)

project(Nova LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# entt
add_subdirectory(vendor/entt)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(vendor/glfw)

# GLAD
add_subdirectory(vendor/glad)

# GLM
set(GLM_BUILD_LIBRARY OFF)
set(GLM_BUILD_TESTS OFF)
set(GLM_BUILD_INSTALL OFF)
set(GLM_ENABLE_CXX_20 ON)
add_subdirectory(vendor/glm)

# JSON
set(NLOHMANN_JSON_BUILD_MODULES OFF)
add_subdirectory(vendor/json)

# spdlog
set(SPDLOG_ENABLE_PCH ON)
set(SPDLOG_BUILD_EXAMPLE OFF)
set(SPDLOG_BUILD_TESTS OFF)
set(SPDLOG_BUILD_BENCH OFF)
set(SPDLOG_USE_STD_FORMAT ON)
set(SPDLOG_INSTALL OFF)
add_subdirectory(vendor/spdlog)

# stb
add_subdirectory(vendor/stb)

# uuid_v4
add_subdirectory(vendor/uuid_v4)

# dotnet
add_subdirectory(vendor/dotnet)

# xxHash
add_subdirectory(vendor)

# Nova
add_subdirectory(Nova)

# NovaHost
add_subdirectory(NovaHost)

# EditorScript
add_subdirectory(EditorScript)

# Nova Editor
add_subdirectory(NovaEditor)

set(CMAKE_VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:NovaEditor>)