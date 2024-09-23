project(Nova LANGUAGES C)

file(GLOB_RECURSE NOVA_SOURCES "src/*.c")
list(FILTER NOVA_SOURCES EXCLUDE REGEX "src/platform/*")

if(WIN32)
    file(GLOB_RECURSE NOVA_WIN32_SOURCES "src/platform/windows/*.c")
    list(APPEND NOVA_SOURCES ${NOVA_WIN32_SOURCES})
elseif(UNIX)
    file(GLOB_RECURSE NOVA_UNIX_SOURCES "src/platform/unix/*.c")
    list(APPEND NOVA_SOURCES ${NOVA_UNIX_SOURCES})
endif()

add_library(Nova SHARED ${NOVA_SOURCES})
target_include_directories(Nova PUBLIC "include")
target_compile_definitions(Nova PRIVATE "NV_BUILD")
target_link_libraries(Nova PUBLIC stb)

if(WIN32)
    target_compile_definitions(Nova PUBLIC "WIN32_LEAN_AND_MEAN" "NV_WINDOWS")
endif()
