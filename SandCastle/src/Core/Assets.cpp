#include "pch.h"
#include <stb/stb_image.h>
#include "SandCastle/Core/Assets.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Core/Serialization.h"
#include "SandCastle/Core/Files.h"
#include "SandCastle/Render/Shader.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Core/Math.h"
#include "SandCastle/Render/AnimationSystem.h"
#include "SandCastle/Audio/Audio.h"
#include "SandCastle/Render/Renderer2D.h"

namespace SandCastle
{
	void Assets::CreateAnimations()
	{
		for (int i = 0; i < m_animSerialized.size(); i++)
		{
			m_animations.insert(std::make_pair(m_animSerialized[i].first, Animation(m_animSerialized[i].second)));
		}
		m_animSerialized.clear();
	}

	void Assets::ChangeLocaTexture(sptr<OpaqueAsset>& next, const String& key)
	{
		auto Next = static_pointer_cast<Asset<Texture>>(next);
		m_textures[key].Copy(Next->m_data);
	}

	void Assets::ChangeLocaTextual(sptr<OpaqueAsset>& next, const String& key)
	{
		auto Next = static_pointer_cast<Asset<Textual>>(next);
		m_textuals[key] = Next->m_data;
	}

	void Assets::GenerateSprites(String filename, Serialized& spritesheet, const Texture* texture)
	{
		int w = (int)spritesheet.GetInt("Width");
		int h = (int)spritesheet.GetInt("Height");
		int width = Math::Max(1, w);
		int height = Math::Max(1, h);
		Vec2f origin;
		origin.x = spritesheet.GetArray<float>("Origin")[0];
		origin.y = spritesheet.GetArray<float>("Origin")[1];
		Vec2f padding = 0;
		padding.x = spritesheet.GetArray<float>("Padding")[0];
		padding.y = spritesheet.GetArray<float>("Padding")[1];

		int offsetx = 0;
		int offsety = 0;

		auto texSize = texture->GetSize();

		int totalWidth = (width + (int)padding.x * 2);
		int totalHeight = (height + (int)padding.y * 2);
		int columns = texSize.x / totalWidth + texSize.x % totalWidth;
		int rows = texSize.y / totalHeight + texSize.y % totalHeight;
		if (texSize.x % totalWidth > 0)
		{
			LOG_WARN("Sprite crop+padding doesn't fit perfectly texture width: " + filename);
		}
		if (texSize.y % totalHeight > 0)
		{
			LOG_WARN("Sprite crop+padding doesn't fit perfectly texture height: " + filename);
		}

		for (int y = 0; y < rows; y++)
		{
			for (int x = 0; x < columns; x++)
			{
				Rect texRect;
				texRect.left = x * width + padding.x + padding.x * 2 * x;
				texRect.top = y * height + padding.y + padding.y * 2 * y;
				texRect.width = (float)width;
				texRect.height = (float)height;

				String spriteName = filename + "_" + std::to_string((rows - 1) - y) + "_" + std::to_string(x);
				m_sprites.insert(
					std::make_pair(
						spriteName, Sprite(
							texture,
							texRect,
							(numeric::float16_t)origin.x,
							(numeric::float16_t)origin.y)));
			}
		}
	}
	//Serialized DefaultSpriteSheet()
	Serialized Assets::CreateDefaultTextureImportSettings()
	{
		//Create the default texture settings 
		Serialized importSettings = TextureImportSettings::defaultSettings.Serialize();
		return importSettings;
	}
	Serialized Assets::CreateDefaultSpritesheet(const Texture* texture)
	{
		Serialized spritesheet;

		spritesheet["Width"] = texture->GetSize().x;
		spritesheet["Height"] = texture->GetSize().y;
		spritesheet["Origin"] = { 0.f, 0.f };
		spritesheet["Padding"] = { 0.f, 0.f };

		return spritesheet;
	}
	void Assets::AddAnimation(const String& filename, const String& path, bool localized, const String& lang)
	{
		ASSERT_LOG_ERROR(!localized, "Animation cannot be localized.");
		m_animSerialized.emplace_back(std::make_pair(filename, Serialized(path)));
	}
	void Assets::AddTexture(const String& filename, const String& path, bool localized, const String& lang)
	{
		String settingsPath = path.substr(0, path.find_last_of("/") + 1);
		String nameNoExtension = filename.substr(0, filename.find_last_of("."));
		settingsPath += nameNoExtension + ".texture";

		Serialized settings;

		//Check if we have a texture setting file
		if (!std::filesystem::exists(settingsPath))
		{
			LOG_INFO("Creating default texture settings for " + filename);
			//Create it if not
			settings["ImportSettings"] = CreateDefaultTextureImportSettings();
			settings.WriteOnDisk(settingsPath);
		}
		else
		{
			settings.LoadFromDisk(settingsPath);
		}

		//Load the texture import settings from settings
		Serialized importSettingsCfg = settings.GetObj("ImportSettings");
		TextureImportSettings importSettings(importSettingsCfg);

		//Check if the texture setting is valid (no deserializationError)
		ASSERT_LOG_ERROR(!importSettings.DeserializationError(), "Ill formed texture import settings: " + path);

		if (m_reloading)
		{
			//Reload the rexture
			m_textures[filename].Reload(path, importSettings);
			//Do not generate sprites
			return;
		}

		//Create the texture with the import settings

		//Load sprite sheet from settings

		auto MakeSprites = [&](Texture& tex)
			{
				auto spritesheet = settings.GetObj("Spritesheet");
				if (spritesheet.HadGetError())
				{
					//Texture settings was non existant or ill formed
					spritesheet = CreateDefaultSpritesheet(&tex);
					settings["Spritesheet"] = spritesheet;
					settings.WriteOnDisk(settingsPath);
				}
				GenerateSprites(filename, spritesheet, &tex);
				ASSERT_LOG_ERROR(!spritesheet.HadGetError(), "Ill formed spritesheet: " + path);

			};
		if (localized)
		{
			auto locaTexture = MakeAsset<Texture>(path, importSettings);
			InsertLocalizedAsset(filename, lang, locaTexture);
			if (lang == GetLang())
			{
				ASSERT_LOG_ERROR((m_textures.find(filename) == m_textures.end()), "Double asset texture {0}.");
				auto it_insert = m_textures.insert(std::make_pair(filename, Texture()));
				auto& texture = it_insert.first->second;
				texture.Copy(locaTexture->m_data);
				MakeSprites(texture);
			}
		}
		else
		{
			ASSERT_LOG_ERROR((m_textures.find(filename) == m_textures.end()), "Double asset texture {0}.");
			auto it_insert = m_textures.insert(std::make_pair(filename, Texture(path, importSettings)));
			auto& texture = it_insert.first->second;
			MakeSprites(texture);
		}
	}
	void Assets::AddFragmentShader(const String& filename, const String& path, bool localized, const String& lang)
	{
		ASSERT_LOG_ERROR(!localized, "Shader cannot be localized.");
		auto length = filename.size() - (filename.size() - filename.find_last_of("."));
		String shadername = filename.substr(0, length);
		m_shadersPath[shadername].fragment = path;
	}
	void Assets::AddVertexShader(const String& filename, const String& path, bool localized, const String& lang)
	{
		ASSERT_LOG_ERROR(!localized, "Shader cannot be localized.");
		auto length = filename.size() - (filename.size() - filename.find_last_of("."));
		String shadername = filename.substr(0, length);
		m_shadersPath[shadername].vertex = path;
	}
	void Assets::AddGeometryShader(const String& filename, const String& path, bool localized, const String& lang)
	{
		ASSERT_LOG_ERROR(!localized, "Shader cannot be localized.");
		auto length = filename.size() - (filename.size() - filename.find_last_of("."));
		String shadername = filename.substr(0, length);
		m_shadersPath[shadername].geometry = path;
	}

	void Assets::AddAudio(const String& filename, const String& path, bool localized, const String& lang)
	{
		ASSERT_LOG_ERROR(!localized, "Audio cannot be localized. (Yeah haha!)");
		Audio::Instance()->LoadSound(path);
	}

	void Assets::AddTextual(const String& filename, const String& path, bool localized, const String& lang)
	{
		Serialized json(path);
		Json j = (Json)json;
		ASSERT_LOG_ERROR(!json.HadLoadError(), "Cannot load textual {0}", path);
		ASSERT_LOG_ERROR(!json.HadParseError(), "Cannot parse textual {0}", path);
		try
		{
			for (auto it = j.begin(); it != j.end(); it++)
			{
				auto& key = it.key();
				auto value = it.value().get<std::string>();
				if (localized)
				{
					m_localized[lang].assets.insert(std::make_pair(key, MakeAsset<Textual>(value)));
					if (lang == m_lang)
					{
						auto insert_it = m_textuals.insert(std::make_pair(key, value));
						m_localizedAssets.insert(key);
						ASSERT_LOG_ERROR(insert_it.second, "Textual key doubled.\nkey: {0}\nfile: {1}", key, path);
					}
				}
				else
				{
					auto insert_it = m_textuals.insert(std::make_pair(key, value));
					ASSERT_LOG_ERROR(insert_it.second, "Textual key doubled.\nkey: {0}\nfile: {1}", key, path);
				}
			}
		}
		catch (Json::exception& exception)
		{
			ASSERT_LOG_ERROR(false, "Textual exception {0}, when reading {1}", exception.what(), path);
		}
	}

	Assets::Assets()
	{

	}

	void Assets::Init(const String& folder, const String& defaultLang)
	{
		m_folder = folder;
		m_lang = defaultLang;
		m_langFallback = defaultLang;
		//Can't be done in constructor because of recursion
		stbi_set_flip_vertically_on_load(true);
		InitFunctions();
		LoadAssets();
		CompileShaders();
		CreateAnimations();
		InitLang();
	}
	void Assets::InitLang()
	{
		//Check that the base loca is part of the available loca
		if (m_lang == "" && m_availableLangs.empty())
			return; //No localization

		auto f_it = std::find(m_availableLangs.begin(), m_availableLangs.end(), m_lang);
		ASSERT_LOG_ERROR((f_it != m_availableLangs.end()), "The default selected lang is not part of the available langs");

		//Check every localized asset, they MUST exist in the fallback lang
		if (m_langFallback == "")
			return;
		auto& fallback = m_localized.at(m_langFallback);
		for (const auto& asset : m_localizedAssets)
		{
			bool present = fallback.assets.find(asset) != fallback.assets.end();
			ASSERT_LOG_ERROR(present,
				"Localized asset {0}, isn't present in the fallback lang {1}.", asset, m_langFallback);
		}
		//Use the fallback asset in the missing langs
	}
	void Assets::HotReload()
	{
		Renderer2D::ClearBatches();
		m_reloading = true;
		LoadAssets();
		m_reloading = false;
	}
	String Assets::GetFolder()
	{
		return Instance()->m_folder;
	}
	void Assets::SetLang(const String& lang)
	{
		auto i = Instance();
		auto it = i->m_localized.find(lang);
		if (it == i->m_localized.end())
		{
			LOG_ERROR("The following localization key doesn't exist: {0}", lang);
			return;
		}
		i->m_lang = lang;
		for (auto loca : it->second.assets)
		{
			auto& newAsset = loca.second;
			auto& assetKey = loca.first;
			auto it_fun = i->m_changeLocaFunctions.find(newAsset->GetType());
			it_fun->second.CallArg(newAsset, assetKey);
		}
		auto signal = LangSignal(lang);
		i->langSignal.Send(&signal);
	}
	String Assets::GetLang()
	{
		return Instance()->m_lang;
	}
	std::vector<String> Assets::GetAvailableLangs()
	{
		return Instance()->m_availableLangs;
	}
	void Assets::InitFunctions()
	{
		//Add assets
		m_addAssetFunctions.insert(std::make_pair(".anim", Delegate(&Assets::AddAnimation, this)));
		m_addAssetFunctions.insert(std::make_pair(".png", Delegate(&Assets::AddTexture, this)));
		m_addAssetFunctions.insert(std::make_pair(".vert", Delegate(&Assets::AddVertexShader, this)));
		m_addAssetFunctions.insert(std::make_pair(".frag", Delegate(&Assets::AddFragmentShader, this)));
		m_addAssetFunctions.insert(std::make_pair(".geom", Delegate(&Assets::AddGeometryShader, this)));
		m_addAssetFunctions.insert(std::make_pair(".mp3", Delegate(&Assets::AddAudio, this)));
		m_addAssetFunctions.insert(std::make_pair(".wav", Delegate(&Assets::AddAudio, this)));
		m_addAssetFunctions.insert(std::make_pair(".textual", Delegate(&Assets::AddTextual, this)));

		//Change loca 
		m_changeLocaFunctions.insert(std::make_pair(TypeId::GetId<Texture>(), Delegate(&Assets::ChangeLocaTexture, this)));
		m_changeLocaFunctions.insert(std::make_pair(TypeId::GetId<Textual>(), Delegate(&Assets::ChangeLocaTextual, this)));
	}

	void Assets::LoadAssets()
	{
		//Iterate every subfolders in assets and load every file into the asset map
		String root = m_folder;
		String localized = root + "localized/";

		struct Folder
		{
			bool localized = false;
			String lang = "";
			std::filesystem::path path;
		};
		std::list<Folder> folders;
		ASSERT_LOG_ERROR(std::filesystem::exists(root), "The following asset folder doesn't exists: {0}", root);
		folders.push_front(Folder(false, "", root));

		while (!folders.empty())
		{
			auto& folder = folders.front();
			std::filesystem::directory_iterator folder_it(folder.path);
			while (folder_it != std::filesystem::directory_iterator())
			{
				if (std::filesystem::is_regular_file(*folder_it))
				{
					String path = folder_it->path().generic_string();
					if (path.size() < 1)
					{
						folder_it++;
						continue;
					}
					AddAsset(path, folder.localized, folder.lang);
				}
				else
				{
					String pstr = folder_it->path().generic_string();
					String sub = pstr.substr(0, localized.size());
					bool loca = pstr.size() > localized.size() ? sub == localized : false;
					String lang = "";
					if (loca)
					{
						lang = pstr.substr(localized.size(), pstr.size() - localized.size());
						lang = lang.substr(0, lang.find_first_of('/'));
						m_availableLangs.emplace_back(lang);
						//Trick to make sure one default lang is always selected
						//If the user failed to put it in the engine settings.
						if (m_lang == "")
							m_lang = lang;
					}

					folders.push_back(Folder(loca, lang, folder_it->path()));
				}
				folder_it++;
			}
			folders.pop_front();
		}
	}

	void Assets::CompileShaders()
	{
		LOG_INFO("Compiling shaders...");
		for (auto& shader : m_shadersPath)
		{
			String& vertPath = shader.second.vertex;
			String& fragPath = shader.second.fragment;
			String& geomPath = shader.second.geometry;

			if (vertPath == "" || fragPath == "")
			{
				LOG_ERROR("Incomplete shader program, can't compile: [" + vertPath + "], [" + geomPath + "], [" + fragPath + "]");
				continue;
			}

			String vertSrc = Shader::LoadShaderSourceFromFile(vertPath);
			String fragSrc = Shader::LoadShaderSourceFromFile(fragPath);
			String geomSrc = geomPath == "" ? "" : Shader::LoadShaderSourceFromFile(geomPath);

			String path = vertPath;
			size_t i = path.find_last_of("/");
			size_t j = path.find_last_of(".");
			size_t s = path.size();
			String assetName = path.substr(i + 1, (s - i) - (s - j)) + "shader";

			if (geomSrc == "")
			{
				m_shaders.insert(std::make_pair(assetName, Shader(vertSrc, fragSrc, assetName)));
			}
			else
			{
				m_shaders.insert(std::make_pair(assetName, Shader(vertSrc, geomSrc, fragSrc, assetName)));
			}

		}
	}

	void Assets::AddAsset(const String& path, bool localized, const String& lang)
	{
		size_t i = path.find_last_of(".");
		if (i >= path.size())
			return;
		String extension = path.substr(i, path.size() - i);

		//Atm reloading affect only textures
		if (m_reloading && extension != ".png")
			return;

		if (extension == ".texture")
			return;

		auto find_it = m_addAssetFunctions.find(extension);
		if (find_it == m_addAssetFunctions.end())
		{
			//LOG_WARN("Asset file extension not supported, " + path);
			return;
		}

		i = path.find_last_of("/") + 1;
		String filename = path.substr(i, path.size() - i);
		if (localized)
		{
			//Check that the file lang matches the folder lang
			auto flang = filename.substr(0, filename.find_first_of('_'));
			auto f_it = std::find(m_availableLangs.begin(), m_availableLangs.end(), lang);
			ASSERT_LOG_ERROR((f_it != m_availableLangs.end()), "Localized asset with a non-available lang: {0}", path);
			ASSERT_LOG_ERROR((flang == lang), "Localized asset in the wrong folder: {0}", path);
			//Remove the lang from the filename
			filename = filename.substr(flang.size() + 1, filename.size() - flang.size());
			if(extension != ".textual")
				m_localizedAssets.insert(filename);
		}
		find_it->second.CallArg(filename, path, localized, lang);
		//LOG_INFO("Asset loaded " + path);
	}
}