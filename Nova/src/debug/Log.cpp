#include <Nova/debug/Log.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

static std::shared_ptr<spdlog::logger> s_Logger;

std::shared_ptr<spdlog::logger> Nova::Log::_GetLogger() noexcept
{
	return s_Logger;
}

void Nova::Log::_Initialize(const std::optional<std::filesystem::path> logFilepath) noexcept
{
	std::vector<spdlog::sink_ptr> sinks{
		std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
	};

	if (logFilepath.has_value())
		sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilepath.value().string(), true));
	
	s_Logger = std::make_shared<spdlog::logger>("NOVA", sinks.begin(), sinks.end());
	s_Logger->set_pattern("%^[%l][%H:%M:%S.%e] %v%$");
	s_Logger->set_level(spdlog::level::trace);
	s_Logger->flush_on(spdlog::level::trace);

	spdlog::register_logger(s_Logger);
}
