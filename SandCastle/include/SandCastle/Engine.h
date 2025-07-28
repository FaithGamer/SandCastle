#pragma once

namespace SandCastle
{
	class Engine
	{
	public:
		/// @brief Call in your main function before doing anything else.
		/// @param logging enable logging
		static void Init(bool logging = true);
		/// @brief Call in your main function after Engine::Init and optionally your initializations.
		static void Launch();
		static void Stop();
	private:
		static bool play;
	};
}
