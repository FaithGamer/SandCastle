#pragma once

#include "EngineSettings.h"

namespace SandCastle
{
	/// @brief Static lifecycle controller for the engine.
	/// Three calls drive a typical program: Init() to boot all subsystems
	/// (Window, Renderer2D, Audio, Physics, Assets, Inputs, Ui, Systems),
	/// Launch() to enter the main loop, and Stop() to break out of it.
	class Engine
	{
	public:
		/// @brief Boot every engine subsystem from an in-memory settings struct.
		/// Must be called before any other engine API. Defaults to EngineSettings().
		static void Init(EngineSettings settings = EngineSettings());
		/// @brief Boot every engine subsystem from a JSON settings file on disk.
		/// @param settingsPath Path to a JSON file describing an EngineSettings.
		static void Init(const String& settingsPath);
		/// @brief Enter the main loop. Returns when Stop() is called or the window closes.
		/// Call this after Init() and after pushing your initial Systems.
		static void Launch();
		/// @brief Request the main loop to exit at the end of the current frame.
		static void Stop();
		/// @brief Has Init() already completed successfully.
		static bool IsInit();
	private:
		static bool init;
		static bool play;
	};
}
