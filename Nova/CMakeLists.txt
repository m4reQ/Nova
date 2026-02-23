file(
    GLOB_RECURSE
    NOVA_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_library(Nova STATIC ${NOVA_SOURCES})
target_link_libraries(
    Nova
    PUBLIC
    EnTT
    glfw
    gl
    glm
    spdlog
    stb
    dotnet
    xxHash
    nlohmann_json)
target_compile_definitions(
    Nova
    PUBLIC
    "$<IF:$<CONFIG:Debug>,NV_DEBUG,NV_RELEASE>"
    "NV_STATICLIB"
    "GLM_ENABLE_EXPERIMENTAL"
    "GLFW_INCLUDE_NONE"
    "NOMINMAX"
    PRIVATE
    "NV_BUILD")
if(WIN32)
    target_compile_definitions(
        Nova
        PUBLIC
        "NV_WINDOWS")
endif()
set_target_properties(
    Nova
    PROPERTIES
    CXX_STANDARD 23)
target_include_directories(
    Nova
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_dependencies(
    Nova
    NovaHost)
# target_precompile_headers(
#     Nova
#     PRIVATE
#     "${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp")

add_custom_command(
    TARGET Nova
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:NovaEditor>/assets)