project(NovaEditor LANGUAGES C)

file(GLOB_RECURSE NOVA_EDITOR_SOURCES "src/*.c")
add_executable(NovaEditor ${NOVA_EDITOR_SOURCES})
target_link_libraries(NovaEditor PUBLIC Nova)

add_custom_command(
    TARGET NovaEditor
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:NovaEditor> $<TARGET_FILE_DIR:NovaEditor>
    COMMAND_EXPAND_LISTS)
