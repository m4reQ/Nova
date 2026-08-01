cmake_minimum_required(VERSION 3.22)

# xxHash
add_library(
    xxHash
    STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/xxHash/xxhash.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/xxHash/xxh_x86dispatch.c")
target_include_directories(
    xxHash
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/xxHash")
set_target_properties(
    xxHash
    PROPERTIES
    C_STANDARD 11)

# imgui
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
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/cpp/imgui_stdlib.cpp")
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
target_link_libraries(imgui PUBLIC glfw)

# imguizmo
add_library(
    imguizmo
    STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/src/ImGuizmo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/src/ImGradient.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/src/ImCurveEdit.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/src/ImSequencer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/src/GraphEditor.cpp")
target_include_directories(
    imguizmo
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo")
target_link_libraries(imguizmo PUBLIC imgui)