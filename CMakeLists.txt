cmake_minimum_required(VERSION 3.22)

project(Main)

if(WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()
add_subdirectory(vendor/volk)

add_subdirectory(vendor/dotnet)
add_subdirectory(vendor/stb)
add_subdirectory(Nova)
add_subdirectory(NovaHost)
add_subdirectory(NovaScript)
add_subdirectory(NovaEditor)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT NovaEditor)
