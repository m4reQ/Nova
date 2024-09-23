project(stb)

add_library(stb STATIC "_impl.c")
target_include_directories(stb PUBLIC "include")
