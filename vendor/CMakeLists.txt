cmake_minimum_required(VERSION 3.22)

add_library(
    xxHash
    STATIC
    "xxHash/xxhash.c"
    "xxHash/xxh_x86dispatch.c")
target_include_directories(
    xxHash
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/xxHash")
set_target_properties(
    xxHash
    PROPERTIES
    C_STANDARD 11)

add_library(
    imgui
    STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_win32.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp")
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
target_link_libraries(imgui PUBLIC glfw)