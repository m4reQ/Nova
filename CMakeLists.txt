cmake_minimum_required(VERSION 3.22)

project(Main)

if(WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()
add_subdirectory(vendor/volk)

add_subdirectory(vendor/stb)
add_subdirectory(Nova)
add_subdirectory(NovaEditor)
