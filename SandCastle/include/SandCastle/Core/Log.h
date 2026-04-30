#pragma once
#include <string>
#include <spdlog/spdlog.h>

#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Core/std_macros.h"


namespace SandCastle
{
	/// @brief Append the last SDL error string to a message and log it through spdlog.
	std::string LogSDLError(std::string str);

	/// @brief Engine logger singleton, thin wrapper around an spdlog logger.
	/// Most code should use the LOG_ERROR / LOG_WARN / LOG_INFO / LOG_TRACE
	/// macros below rather than calling GetLogger() directly.
	class Log : public Singleton<Log>
	{

	public:
		/// @brief Configure the underlying spdlog sinks. Called by Engine::Init().
		void Init();
		/// @brief Raw access to the underlying spdlog logger (advanced usage).
		sptr<spdlog::logger> GetLogger();
	private:
		friend sptr<Log> Singleton<Log>::Instance();
		friend void Singleton<Log>::Kill();
		Log() = default;

		sptr<spdlog::logger> m_logger = nullptr;
	};
}
#ifndef SandCastle_NO_LOGGING
/// @brief Log an error and assert if condition is false. fmt-style variadic args.
#define ASSERT_LOG_ERROR(condition, ...) if(!condition){SandCastle::Log::Instance()->GetLogger()->error(__VA_ARGS__); SandCastle::Log::Instance()->GetLogger()->flush(); assert(condition);}

/// @brief Log an error message (flushes immediately). fmt-style.
#define LOG_ERROR(...) SandCastle::Log::Instance()->GetLogger()->error(__VA_ARGS__); SandCastle::Log::Instance()->GetLogger()->flush();
/// @brief Log a warning message (flushes immediately). fmt-style.
#define LOG_WARN(...) SandCastle::Log::Instance()->GetLogger()->warn(__VA_ARGS__); SandCastle::Log::Instance()->GetLogger()->flush();
/// @brief Log an info message. fmt-style.
#define LOG_INFO(...) SandCastle::Log::Instance()->GetLogger()->info(__VA_ARGS__)
/// @brief Log a trace message (verbose, opt-in level). fmt-style.
#define LOG_TRACE(...) SandCastle::Log::Instance()->GetLogger()->trace(__VA_ARGS__)
#else
#define ASSERT_LOG_ERROR(...)
#define LOG_ERROR(...) 
#define LOG_WARN(...)
#define LOG_INFO(...) 
#define LOG_TRACE(...) 
#endif

