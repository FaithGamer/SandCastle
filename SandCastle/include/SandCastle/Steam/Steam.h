#pragma once
#include <cstdint>
#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{
	/// @brief Parameters consumed by Steam::Init().
	struct SteamSettings
	{
		/// @brief Your game's Steam AppId. Mandatory.
		uint32_t appId = 0;
		/// @brief If the game was launched outside of Steam, relaunch it through
		/// Steam (SteamAPI_RestartAppIfNecessary). Steam::Init returns false when
		/// the relaunch is triggered: exit immediately without calling Engine::Launch.
		bool restartAppIfNecessary = true;
		/// @brief Basic DRM: when SteamAPI_Init fails (Steam not running, no license),
		/// Steam::Init returns false so you can refuse to run the game.
		/// Set to false to let the game run without Steam features instead.
		bool stopOnInitFail = true;
		/// @brief Interval in seconds between two automatic StoreStats uploads.
		/// 0 disables the periodic upload (achievements still store immediately).
		float storeStatsInterval = 60.f;
	};

	/// @brief Optional Steamworks integration. Entirely opt-in: a game that never
	/// calls Steam::Init (or any other method of this class) has no Steam
	/// dependency and needs no Steam DLL. When Init was skipped or failed with
	/// stopOnInitFail = false, every method below is a safe no-op returning a
	/// neutral default, so game code can call them unconditionally.
	/// steam_api64.lib is merged into SandCastle.lib; games using Steam only ship
	/// vendor/steam_api64.dll next to the executable.
	class Steam
	{
	public:
		/// @brief Initialize the Steam API. Call before Engine::Launch (before or
		/// after Engine::Init both work). Pushes an internal system that pumps the
		/// Steam callbacks on FixedUpdate and shuts the API down with the engine.
		/// @return false when the game must exit now: either Steam is relaunching
		/// the game (restartAppIfNecessary) or the API failed with stopOnInitFail.
		/// true otherwise; check Enabled() to know if Steam is actually live.
		static bool Init(SteamSettings settings);
		/// @brief True when the Steam API is initialized and every call is live.
		static bool Enabled();
		/// @brief AppId passed to Init, 0 if not initialized.
		static uint32_t GetAppId();

		//Achievements

		/// @brief Unlock an achievement by its API name and store it immediately.
		static void UnlockAchievement(const String& name);
		/// @brief True if the achievement is already unlocked for the current user.
		static bool IsAchievementUnlocked(const String& name);
		/// @brief Relock an achievement (dev/testing).
		static void ClearAchievement(const String& name);
		/// @brief Relock every achievement of the app (dev/testing).
		static void ResetAllAchievements();
		/// @brief Pop the Steam overlay progress toast for an achievement.
		static void ShowAchievementProgress(const String& name, uint32_t current, uint32_t max);

		//Stats

		/// @brief Set an integer stat by its API name (stored on the periodic upload).
		static void SetStat(const String& name, int value);
		/// @brief Read an integer stat, 0 when unavailable.
		static int GetStat(const String& name);
		/// @brief Force-upload the pending stats/achievements to Steam now.
		static void StoreStats();

		//Player

		/// @brief SteamID64 of the current user, 0 when Steam is disabled.
		static uint64_t GetPlayerId();
		/// @brief Persona (display) name of the current user, "" when disabled.
		static String GetPlayerName();

		//Language

		/// @brief Steam client UI language mapped to a short language code
		/// ("english" gives "en", "brazilian" gives "pt-BR"...), matching the
		/// folder names used by the engine localization. "" when unmapped or disabled.
		static String GetLanguage();
		/// @brief Raw Steam language name ("english", "koreana"...), "" when disabled.
		static String GetSteamLanguage();

		//Apps / DLC

		/// @brief True when the given DLC is owned and installed.
		static bool IsDlcInstalled(uint32_t appId);
		/// @brief True when the current user owns the given app (BIsSubscribedApp).
		static bool IsSubscribed(uint32_t appId);
		/// @brief True when the given app is installed on this machine.
		static bool IsAppInstalled(uint32_t appId);
		/// @brief Fetch the install folder of an installed app, with forward
		/// slashes and a trailing '/'. Returns false when the app isn't installed.
		static bool GetAppInstallDir(uint32_t appId, String& folder);

		//Misc

		/// @brief True when running on a Steam Deck through the Steam API.
		static bool IsOnSteamDeck();
		/// @brief Open a web page in the Steam overlay (falls back to the user's
		/// browser when the overlay is disabled).
		static void OpenUrlOverlay(const String& url);
	};
}
