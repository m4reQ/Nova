#pragma once
#include <spdlog/spdlog.h>
#include <filesystem>
#include <optional>

#ifdef NV_DEBUG
#define NV_LOG_INITIALIZE(logFilepath) Nova::Log::_Initialize(logFilepath)
#define NV_LOG_TRACE(...) Nova::Log::_GetLogger()->trace(__VA_ARGS__)
#define NV_LOG_INFO(...) Nova::Log::_GetLogger()->info(__VA_ARGS__)
#define NV_LOG_WARNING(...) Nova::Log::_GetLogger()->warn(__VA_ARGS__)
#define NV_LOG_ERROR(...) Nova::Log::_GetLogger()->error(__VA_ARGS__)
#define NV_LOG_CRITICAL(...) Nova::Log::_GetLogger()->critical(__VA_ARGS__)
#else
#define NV_LOG_INITIALIZE(logFilepath)
#define NV_LOG_TRACE(...)
#define NV_LOG_INFO(...)
#define NV_LOG_WARNING(...)
#define NV_LOG_ERROR(...)
#define NV_LOG_CRITICAL(...)
#endif

namespace Nova::Log
{
	std::shared_ptr<spdlog::logger> _GetLogger() noexcept;
	void _Initialize(const std::optional<std::filesystem::path> logFilepath = std::nullopt) noexcept;
}
