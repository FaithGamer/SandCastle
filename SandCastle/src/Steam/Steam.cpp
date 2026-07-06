#include "pch.h"
#include <algorithm>
#pragma warning(push)
#pragma warning(disable : 4996) //strncpy in Valve's matchmakingtypes.h
#include <steam/steam_api.h>
#pragma warning(pop)

#include "SandCastle/Steam/Steam.h"
#include "SandCastle/Engine.h"
#include "SandCastle/ECS/System.h"
#include "SandCastle/ECS/Systems.h"
#include "SandCastle/Core/Time.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	namespace
	{
		bool steamEnabled = false;
		SteamSettings steamSettings;
	}

	/// Internal system pushed by Steam::Init. Pumps the Steam callbacks on
	/// FixedUpdate, uploads the stats periodically, shuts the API down when
	/// the engine destroys its systems.
	class SteamSystem : public System
	{
	public:
		~SteamSystem()
		{
			SteamAPI_Shutdown();
			steamEnabled = false;
		}

		void FixedUpdate() override
		{
			SteamAPI_RunCallbacks();

			if (steamSettings.storeStatsInterval <= 0.f)
				return;
			m_storeStatsTimer += Time::FixedDelta();
			if (m_storeStatsTimer >= steamSettings.storeStatsInterval)
			{
				m_storeStatsTimer = 0.f;
				SteamUserStats()->StoreStats();
			}
		}

		int GetUsedMethod() override { return FixedUpdt; }
		std::string DebugName() override { return "SteamSystem"; }

	private:
		STEAM_CALLBACK(SteamSystem, OnUserStatsReceived, UserStatsReceived_t);

		float m_storeStatsTimer = 0.f;
	};

	void SteamSystem::OnUserStatsReceived(UserStatsReceived_t* data)
	{
		LOG_INFO("Steam user stats received.");
	}

	bool Steam::Init(SteamSettings settings)
	{
		steamSettings = settings;
		ASSERT_LOG_ERROR(settings.appId != 0, "SteamSettings.appId is not set.");

		if (settings.restartAppIfNecessary && SteamAPI_RestartAppIfNecessary(settings.appId))
		{
			LOG_INFO("Steam is relaunching the game, exiting.");
			Engine::Stop();
			return false;
		}

		if (!SteamAPI_Init())
		{
			LOG_ERROR("Cannot initialize the Steam API.");
			if (settings.stopOnInitFail)
			{
				//Basic DRM: refuse to run without Steam.
				Engine::Stop();
				return false;
			}
			return true;
		}

		steamEnabled = true;
		Systems::Push<SteamSystem>();

		if (!SteamUserStats()->RequestCurrentStats())
		{
			LOG_ERROR("Couldn't retrieve the Steam user current stats.");
		}
		return true;
	}

	bool Steam::Enabled()
	{
		return steamEnabled;
	}

	uint32_t Steam::GetAppId()
	{
		return steamSettings.appId;
	}

	void Steam::UnlockAchievement(const String& name)
	{
		if (!steamEnabled)
			return;
		if (!SteamUserStats()->SetAchievement(name.c_str()))
		{
			LOG_ERROR("Failed to unlock achievement {0}", name);
		}
		else
		{
			SteamUserStats()->StoreStats();
		}
	}

	bool Steam::IsAchievementUnlocked(const String& name)
	{
		if (!steamEnabled)
			return false;
		bool unlocked = false;
		if (!SteamUserStats()->GetAchievement(name.c_str(), &unlocked))
		{
			LOG_ERROR("Couldn't get the state of achievement {0}", name);
		}
		return unlocked;
	}

	void Steam::ClearAchievement(const String& name)
	{
		if (!steamEnabled)
			return;
		SteamUserStats()->ClearAchievement(name.c_str());
		SteamUserStats()->StoreStats();
	}

	void Steam::ResetAllAchievements()
	{
		if (!steamEnabled)
			return;
		auto count = SteamUserStats()->GetNumAchievements();
		for (uint32 i = 0; i < count; i++)
		{
			SteamUserStats()->ClearAchievement(SteamUserStats()->GetAchievementName(i));
		}
		SteamUserStats()->StoreStats();
	}

	void Steam::ShowAchievementProgress(const String& name, uint32_t current, uint32_t max)
	{
		if (!steamEnabled)
			return;
		if (!SteamUserStats()->IndicateAchievementProgress(name.c_str(), current, max))
		{
			LOG_INFO("Can't show achievement progress: {0}, either the progression is maxed or the achievement doesn't exist.", name);
		}
	}

	void Steam::SetStat(const String& name, int value)
	{
		if (!steamEnabled)
			return;
		if (!SteamUserStats()->SetStat(name.c_str(), value))
		{
			LOG_ERROR("Can't change steam stat: {0}", name);
		}
	}

	int Steam::GetStat(const String& name)
	{
		if (!steamEnabled)
			return 0;
		int value = 0;
		if (!SteamUserStats()->GetStat(name.c_str(), &value))
		{
			LOG_ERROR("Can't read steam stat: {0}", name);
		}
		return value;
	}

	void Steam::StoreStats()
	{
		if (!steamEnabled)
			return;
		SteamUserStats()->StoreStats();
	}

	uint64_t Steam::GetPlayerId()
	{
		if (!steamEnabled)
			return 0;
		return SteamUser()->GetSteamID().ConvertToUint64();
	}

	String Steam::GetPlayerName()
	{
		if (!steamEnabled)
			return "";
		return SteamFriends()->GetPersonaName();
	}

	String Steam::GetLanguage()
	{
		if (!steamEnabled)
			return "";
		static const std::unordered_map<String, String> langMap = {
			{"arabic", "ar"},
			{"bulgarian", "bg"},
			{"schinese", "zh-CN"},
			{"tchinese", "zh-CN"},
			{"czech", "cs"},
			{"danish", "da"},
			{"dutch", "nl"},
			{"english", "en"},
			{"finnish", "fi"},
			{"french", "fr"},
			{"german", "de"},
			{"greek", "el"},
			{"hungarian", "hu"},
			{"indonesian", "id"},
			{"italian", "it"},
			{"japanese", "ja"},
			{"koreana", "ko"},
			{"norwegian", "no"},
			{"polish", "pl"},
			{"portuguese", "pt-BR"},
			{"brazilian", "pt-BR"},
			{"romanian", "ro"},
			{"russian", "ru"},
			{"spanish", "es"},
			{"latam", "es-419"},
			{"swedish", "sv"},
			{"thai", "th"},
			{"turkish", "tr"},
			{"ukrainian", "uk"},
			{"vietnamese", "vi"}
		};

		String lang = SteamUtils()->GetSteamUILanguage();
		auto it = langMap.find(lang);
		if (it != langMap.end())
			return it->second;
		LOG_WARN("Steam language \"{0}\" can't be mapped to a short language code.", lang);
		return "";
	}

	String Steam::GetSteamLanguage()
	{
		if (!steamEnabled)
			return "";
		return SteamUtils()->GetSteamUILanguage();
	}

	bool Steam::IsDlcInstalled(uint32_t appId)
	{
		if (!steamEnabled)
			return false;
		return SteamApps()->BIsDlcInstalled(appId);
	}

	bool Steam::IsSubscribed(uint32_t appId)
	{
		if (!steamEnabled)
			return false;
		return SteamApps()->BIsSubscribedApp(appId);
	}

	bool Steam::IsAppInstalled(uint32_t appId)
	{
		if (!steamEnabled)
			return false;
		return SteamApps()->BIsAppInstalled(appId);
	}

	bool Steam::GetAppInstallDir(uint32_t appId, String& folder)
	{
		if (!steamEnabled || !SteamApps()->BIsAppInstalled(appId))
			return false;

		char dir[4096];
		auto size = SteamApps()->GetAppInstallDir(appId, dir, sizeof(dir));
		folder = dir;
		folder.resize(size);
		std::replace(folder.begin(), folder.end(), '\\', '/');
		folder += '/';
		return true;
	}

	bool Steam::IsOnSteamDeck()
	{
		if (!steamEnabled)
			return false;
		return SteamUtils()->IsSteamRunningOnSteamDeck();
	}

	void Steam::OpenUrlOverlay(const String& url)
	{
		if (!steamEnabled)
			return;
		SteamFriends()->ActivateGameOverlayToWebPage(url.c_str());
	}
}
