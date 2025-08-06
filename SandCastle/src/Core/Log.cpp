#include "pch.h"
#include <SDL3/SDL.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	std::string LogSDLError(std::string str)
	{
		return std::string("SDL error: " + str + " " + SDL_GetError());
	}

	void Log::Init(bool logfile)
	{
#ifndef SandCastle_NO_LOGGING
		spdlog::set_pattern("%^[%T] %n: %v%$");

		std::vector<spdlog::sink_ptr> sinks;

#ifndef SandCastle_NO_CONSOLE
		sinks.push_back(makesptr<spdlog::sinks::stdout_color_sink_mt>());
#endif
#ifndef SandCastle_NO_LOGFILE
		if(logfile)
			sinks.push_back(makesptr<spdlog::sinks::rotating_file_sink_mt>("logfile.txt", 5000000, 0));
#endif
		m_logger = makesptr<spdlog::logger>("SANDCASTLE", begin(sinks), end(sinks));
		m_logger->set_level(spdlog::level::trace);
#endif
	}

	std::shared_ptr<spdlog::logger> Log::GetLogger()
	{
		return m_logger;
	}
}


