file(
    GLOB_RECURSE
    EDITOR_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(NovaEditor ${EDITOR_SOURCES})
target_link_libraries(NovaEditor PRIVATE Nova imgui EnTT)
set_target_properties(
    NovaEditor
    PROPERTIES
    CXX_STANDARD 23)
add_custom_command(
    TARGET NovaEditor
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:NovaEditor>/assets)
add_dependencies(
    NovaEditor
    EditorScript)