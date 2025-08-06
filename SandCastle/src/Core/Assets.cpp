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

#define TEXTURE_IMPORT_SETTING_IS_ERROR
#define SPRITESHEET_IS_ERROR

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

				String spriteName = filename + "_" + std::to_string(y) + "_" + std::to_string(x);
				InsertAsset(spriteName, MakeAsset<Sprite>(texture, texRect, origin));
			}
		}
	}
	//Serialized DefaultSpriteSheet()
	Serialized Assets::CreateDefaultTextureImportSettings()
	{
		//Create the default texture settings 
		Serialized importSettings = m_defaultImportSettings.Serialize();
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
	void Assets::AddAnimation(String filename, String path)
	{
		m_animations.emplace_back(std::make_pair(filename, Serialized(path)));
	}
	void Assets::AddTexture(String filename, String path)
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
		if (importSettings.DeserializationError())
		{
#ifdef TEXTURE_IMPORT_SETTING_IS_ERROR
			ASSERT_LOG_ERROR(false, "Ill formed texture import settings: " + path);
#else
			LOG_WARN("Ill formed texture import settings, creating default texture settings for " + filename);
			importSettingsCfg = CreateDefaultTextureSettings(settingsPath);
			settings["ImportSettings"] = importSettingsCfg;
			settings.WriteOnDisk(settingsPath);
			importSettings.Deserialize(importSettingsCfg);
#endif

		}

		if (m_reloading)
		{
			//Reload the rexture
			static_pointer_cast<Asset<Texture>>(m_assets[filename])->m_ptr->Reload(path, importSettings);
			//Do not generate sprites
			return;
		}

		//Create the texture with the import settings
		auto texture = MakeAsset<Texture>(path, importSettings);
		InsertAsset(filename, texture);

		//Load sprite sheet from settings
		auto spritesheet = settings.GetObj("Spritesheet");
		if (spritesheet.HadGetError())
		{
			//Texture settings was non existant or ill formed
			spritesheet = CreateDefaultSpritesheet(texture->Ptr());
			settings["Spritesheet"] = spritesheet;
			settings.WriteOnDisk(settingsPath);
		}

		//Create all the sprites asset based on texture and sprite sheet
		GenerateSprites(filename, spritesheet, texture->Ptr());

		//Check if the sprites generated are correct
		if (spritesheet.HadGetError())
		{
#ifdef SPRITESHEET_IS_ERROR
			ASSERT_LOG_ERROR(false, "Ill formed spritesheet: " + path);
#else
			LOG_WARN("Ill formed spritesheet, creating default spritesheet for " + filename);
			spritesheet = CreateDefaultSpritesheet(texture->Ptr());
			settings["Spritesheet"] = spritesheet;
			settings.WriteOnDisk();
#endif

		}
	}
	void Assets::AddConfig(String filename, String path)
	{
		InsertAsset(filename, MakeAsset<Serialized>(path));
	}
	void Assets::AddFragmentShader(String filename, String path)
	{
		auto length = filename.size() - (filename.size() - filename.find_last_of("."));
		String shadername = filename.substr(0, length);
		m_shadersPath[shadername].fragment = path;
	}
	void Assets::AddVertexShader(String filename, String path)
	{
		auto length = filename.size() - (filename.size() - filename.find_last_of("."));
		String shadername = filename.substr(0, length);
		m_shadersPath[shadername].vertex = path;
	}
	void Assets::AddGeometryShader(String filename, String path)
	{
		auto length = filename.size() - (filename.size() - filename.find_last_of("."));
		String shadername = filename.substr(0, length);
		m_shadersPath[shadername].geometry = path;
	}

	void Assets::AddAudio(String filename, String path)
	{
		Audio::Instance()->LoadSound(path);
	}

	Assets::Assets()
	{

	}

	void Assets::Init()
	{
		//Can't be done in constructor because of recursion
		InitAddAssetFunctions();
		LoadAssets();
		CompileShaders();
		CreateAnimations();
	}
	void Assets::HotReload()
	{
		Renderer2D::ClearBatches();
		m_reloading = true;
		LoadAssets();
		m_reloading = false;
	}
	void Assets::InitAddAssetFunctions()
	{
		m_addAssetFunctions.insert(std::make_pair(".anim", Delegate(&Assets::AddAnimation, this)));
		m_addAssetFunctions.insert(std::make_pair(".png", Delegate(&Assets::AddTexture, this)));
		m_addAssetFunctions.insert(std::make_pair(".config", Delegate(&Assets::AddConfig, this)));
		m_addAssetFunctions.insert(std::make_pair(".vert", Delegate(&Assets::AddVertexShader, this)));
		m_addAssetFunctions.insert(std::make_pair(".frag", Delegate(&Assets::AddFragmentShader, this)));
		m_addAssetFunctions.insert(std::make_pair(".geom", Delegate(&Assets::AddGeometryShader, this)));
		m_addAssetFunctions.insert(std::make_pair(".mp3", Delegate(&Assets::AddAudio, this)));
		m_addAssetFunctions.insert(std::make_pair(".wav", Delegate(&Assets::AddAudio, this)));
	}

	void Assets::LoadAssets()
	{
		//Iterate every subfolders in assets and load every file into the asset map
		std::filesystem::path root = "assets/";

		std::list<std::filesystem::path> folders;
		if (!std::filesystem::exists(root))
		{
			LOG_ERROR("No assets folder found.");
		}
		folders.push_front(root);

		while (!folders.empty())
		{
			std::filesystem::directory_iterator folder_it(folders.front());
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
					AddAsset(path);
				}
				else
				{
					folders.push_back(folder_it->path());
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

	void Assets::AddAsset(String path)
	{
		size_t i = path.find_last_of(".");
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

		String pathtmp = path;
		i = path.find_last_of("/") + 1;
		String filename = path.substr(i, path.size() - i);
		find_it->second.Call(filename, pathtmp);
		//LOG_INFO("Asset loaded " + path);
	}
}