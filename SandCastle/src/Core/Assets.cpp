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
		for (int i = 0; i < m_animations.size(); i++)
		{
			InsertAsset(m_animations[i].first, MakeAsset<Animation>(m_animations[i].second));
		}
		m_animations.clear();
	}

	void Assets::ChangeLocaTexture(sptr<OpaqueAsset>& prev, sptr<OpaqueAsset>& next)
	{
		auto Prev = static_pointer_cast<Asset<Texture>>(prev);
		auto Next = static_pointer_cast<Asset<Texture>>(next);
		Prev->m_ptr.Copy(Next->m_ptr);
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
				InsertAsset(spriteName, MakeAsset<Sprite>(
					texture,
					texRect,
					(numeric::float16_t)origin.x,
					(numeric::float16_t)origin.y));
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
		m_animations.emplace_back(std::make_pair(filename, Serialized(path)));
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
			static_pointer_cast<Asset<Texture>>(m_assets[filename])->m_ptr.Reload(path, importSettings);
			//Do not generate sprites
			return;
		}
		
		//Create the texture with the import settings
		auto texture = MakeAsset<Texture>(path, importSettings);
		//Load sprite sheet from settings
		auto spritesheet = settings.GetObj("Spritesheet");
		if (spritesheet.HadGetError())
		{
			//Texture settings was non existant or ill formed
			spritesheet = CreateDefaultSpritesheet(texture->Ptr());
			settings["Spritesheet"] = spritesheet;
			settings.WriteOnDisk(settingsPath);
		}
		if (localized)
		{
			InsertLocalizedAsset(filename, lang, texture);
			if (lang == GetLang())
			{
				InsertAsset(filename, texture);
				auto textureInPlace = static_pointer_cast<Asset<Texture>>(m_assets[filename]);
				GenerateSprites(filename, spritesheet, &textureInPlace->m_ptr);
			}
		}
		else
		{
			GenerateSprites(filename, spritesheet, texture->Ptr());
			InsertAsset(filename, texture);
		}

		//Check if the sprites generated are correct
		ASSERT_LOG_ERROR(!spritesheet.HadGetError(), "Ill formed spritesheet: " + path);

	}
	void Assets::AddConfig(const String& filename, const String& path, bool localized, const String& lang)
	{
		ASSERT_LOG_ERROR(!localized, "Config cannot be localized.");
		InsertAsset(filename, MakeAsset<Serialized>(path));
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

	Assets::Assets()
	{

	}

	void Assets::Init()
	{
		//Can't be done in constructor because of recursion
		stbi_set_flip_vertically_on_load(true);
		InitFunctions();
		LoadAssets();
		CompileShaders();
		CreateAnimations();
		SetLang(m_lang);
		InitLoca();
	}
	void Assets::InitLoca()
	{

	
	}
	void Assets::HotReload()
	{
		Renderer2D::ClearBatches();
		m_reloading = true;
		LoadAssets();
		m_reloading = false;
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
		for (auto& loca : it->second.assets)
		{
			auto& newAsset = loca.second;
			auto& assetKey = loca.first;
			auto& currentAsset = i->m_assets[assetKey]; //> swap does not occur between new and current ? add & to i->m_assets?
			auto it_fun = i->m_changeLocaFunctions.find(newAsset->GetType());
			it_fun->second.Call(currentAsset, newAsset);
		}
		i->langSignal.Send(LangSignal(lang));
	}
	String Assets::GetLang()
	{
		return Instance()->m_lang;
	}
	void Assets::InitFunctions()
	{
		//Add assets
		m_addAssetFunctions.insert(std::make_pair(".anim", Delegate(&Assets::AddAnimation, this)));
		m_addAssetFunctions.insert(std::make_pair(".png", Delegate(&Assets::AddTexture, this)));
		m_addAssetFunctions.insert(std::make_pair(".config", Delegate(&Assets::AddConfig, this)));
		m_addAssetFunctions.insert(std::make_pair(".vert", Delegate(&Assets::AddVertexShader, this)));
		m_addAssetFunctions.insert(std::make_pair(".frag", Delegate(&Assets::AddFragmentShader, this)));
		m_addAssetFunctions.insert(std::make_pair(".geom", Delegate(&Assets::AddGeometryShader, this)));
		m_addAssetFunctions.insert(std::make_pair(".mp3", Delegate(&Assets::AddAudio, this)));
		m_addAssetFunctions.insert(std::make_pair(".wav", Delegate(&Assets::AddAudio, this)));

		//Change loca 
		m_changeLocaFunctions.insert(MakePair(TypeId::GetId<Texture>(), Delegate(&Assets::ChangeLocaTexture, this)));
	}

	void Assets::LoadAssets()
	{
		//Iterate every subfolders in assets and load every file into the asset map
		String root = "assets/";
		String localized = root + "localized/";

		struct Folder
		{
			bool localized = false;
			String lang = "";
			std::filesystem::path path;
		};
		std::list<Folder> folders;
		if (!std::filesystem::exists(root))
		{
			LOG_ERROR("No assets folder found.");
		}
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
					bool loca = pstr.size() > localized.size() ?  sub == localized : false;
					String lang = "";
					if (loca)
					{
						lang = pstr.substr(localized.size(), pstr.size() - localized.size());
						lang = lang.substr(0, lang.find_first_of('/'));
						m_availableLangs.emplace_back(lang);
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

			auto shaderAsset = geomSrc == "" ? MakeAsset<Shader>(vertSrc, fragSrc) : MakeAsset<Shader>(vertSrc, geomSrc, fragSrc);
			String path = vertPath;
			size_t i = path.find_last_of("/");
			size_t j = path.find_last_of(".");
			size_t s = path.size();
			String assetName = path.substr(i + 1, (s - i) - (s - j)) + "shader";
			shaderAsset->Ptr()->m_name = assetName;
			InsertAsset(assetName, shaderAsset);
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
		}
		find_it->second.Call(filename, path, localized, lang);
		//LOG_INFO("Asset loaded " + path);
	}
}