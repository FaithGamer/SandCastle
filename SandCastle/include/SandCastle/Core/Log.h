#pragma once
#include <string>
#include <spdlog/spdlog.h>

#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Core/std_macros.h"


namespace SandCastle
{
	std::string LogSDLError(std::string str);

	class Log : public Singleton<Log>
	{

	public:
		void Init(bool logging);
		sptr<spdlog::logger> GetLogger();
	private:
		friend sptr<Log> Singleton<Log>::Instance();
		friend void Singleton<Log>::Kill();
		Log() = default;

		sptr<spdlog::logger> m_logger = nullptr;
	};
}
#ifndef SandCastle_NO_LOGGING
#define ASSERT_LOG_ERROR(condition, ...) if(!condition){SandCastle::Log::Instance()->GetLogger()->error(__VA_ARGS__); SandCastle::Log::Instance()->GetLogger()->flush(); assert(condition);}

#define LOG_ERROR(...) SandCastle::Log::Instance()->GetLogger()->error(__VA_ARGS__); SandCastle::Log::Instance()->GetLogger()->flush();
#define LOG_WARN(...) SandCastle::Log::Instance()->GetLogger()->warn(__VA_ARGS__); SandCastle::Log::Instance()->GetLogger()->flush();
#define LOG_INFO(...) SandCastle::Log::Instance()->GetLogger()->info(__VA_ARGS__)
#define LOG_TRACE(...) SandCastle::Log::Instance()->GetLogger()->trace(__VA_ARGS__)
#else
#define ASSERT_LOG_ERROR(...)
#define LOG_ERROR(...) 
#define LOG_WARN(...)
#define LOG_INFO(...) 
#define LOG_TRACE(...) 
#endif

