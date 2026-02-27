#pragma once
#include <vector>
#include <array>
#include <span>
#include <memory>
#include <utility>
#include <stdexcept>
#include <unordered_map>
#include <string>
#include <string_view>
#include <format>
#include <iostream>
#include <optional>
#include <filesystem>
#include <algorithm>
#include <type_traits>

#ifdef NV_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <glad/gl.h>
#include <xxhash.h>
#include <stb/stb_image.h>