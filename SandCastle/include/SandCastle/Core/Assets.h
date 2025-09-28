#pragma once
#include "std_macros.h"
#include "SandCastle/Internal/Singleton.h"
#include "TypeId.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Core/Delegate.h"
#include "SandCastle/Core/Signal.h"
#include "SandCastle/Core/Textual.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Render/Animation.h"
#include "SandCastle/Render/Shader.h"
#include <unordered_map>

namespace SandCastle
{
	class Assets;
	class Engine;

	class OpaqueAsset
	{
	public:
		virtual ~OpaqueAsset() {};
		virtual int32_t GetType() = 0;
	protected:
	};

	struct LangSignal
	{
		String lang;
	};

	template<class T>
	class Asset : public OpaqueAsset
	{
	public:
		Asset()
		{
		}
		Asset(T&& ptr) : m_data(ptr)
		{

		}
		~Asset()
		{

		}

		Asset(const Asset<T>& asset) = delete;
		Asset(const Asset<T>&& asset) = delete;
		inline T* Ptr()
		{
			return &m_data;
		}
		inline int32_t GetType()
		{
			return TypeId::GetId<T>();
		}
	private:

		friend Assets;
		T m_data;

	};

	class Assets : public Singleton<Assets>
	{
	public:

		Assets();
		void Init(const String& defaultLang);

		void HotReload();
		static void SetLang(const String& lang);
		static String GetLang();
		template <class T>
		inline static T* Get(const String& name)
		{
			ASSERT_LOG_ERROR(false, "Asset type non implemented.");
		}
		template <>
		inline static Sprite* Get(const String& name);
		template <>
		inline static Texture* Get(const String& name);
		template <>
		inline static Shader* Get(const String& name);
		template <>
		inline static Animation* Get(const String& name);
		template <>
		inline static Textual* Get(const String& name);

		Signal<LangSignal> langSignal;
	private:
		friend sptr<Assets> Singleton<Assets>::Instance();
		friend void Singleton<Assets>::Kill();

		template<class T, class... Args>
		static sptr<Asset<T>> MakeAsset(Args&&... args)
		{
			return makesptr<Asset<T>>(T(args...));
			//asset->m_ptr = new T(args...);
			//return asset;
		}

		void InitLang();
		void ChangeLocaTexture(sptr<OpaqueAsset>& prev, const String& key);
		void ChangeLocaTextual(sptr<OpaqueAsset>& prev, const String& key);

		void GenerateSprites(String filename, Serialized& spritesheet, const Texture* texture);
		void LoadAssets();
		void InitFunctions();
		void AddAsset(const String& path, bool localized, const String& lang);
		void CompileShaders();

		template<class T>
		void InsertLocalizedAsset(const String& filename, const String& lang, sptr<Asset<T>> asset)
		{
			m_localized[lang].assets.insert(MakePair(filename, asset));
		}
		void CreateAnimations();
		Serialized CreateDefaultTextureImportSettings();
		Serialized CreateDefaultSpritesheet(const Texture* texture);
		void AddAnimation(const String& filename, const String& path, bool localized, const String& lang);
		void AddTexture(const String& filename, const String& path, bool localized, const String& lang);
		void AddFragmentShader(const String& filename, const String& path, bool localized, const String& lang);
		void AddVertexShader(const String& filename, const String& path, bool localized, const String& lang);
		void AddGeometryShader(const String& filename, const String& path, bool localized, const String& lang);
		void AddAudio(const String& filename, const String& path, bool localized, const String& lang);
		void AddTextual(const String& filename, const String& path, bool localized, const String& lang);

		struct ShaderSources
		{
			String vertex;
			String geometry;
			String fragment;
		};

	private:
		friend Engine;
		String m_lang = "";
		String m_langFallback = "";
		bool m_reloading = false;
		struct Localized
		{
			std::unordered_map<String, sptr<OpaqueAsset>> assets;
		};
		std::unordered_map<String, Localized> m_localized;
		std::vector<String> m_availableLangs;
		std::set<String> m_localizedAssets;

		std::unordered_map<String, Sprite> m_sprites;
		std::unordered_map<String, Texture> m_textures;
		std::unordered_map<String, Shader> m_shaders;
		std::unordered_map<String, Animation> m_animations;
		std::unordered_map<String, Textual> m_textuals;

		std::unordered_map<String, Delegate<void, Assets, const String&, const String&, bool, const String&>> m_addAssetFunctions;
		std::unordered_map<int32_t, Delegate<void, Assets, sptr<OpaqueAsset>&, const String&>> m_changeLocaFunctions;
		std::unordered_map<String, ShaderSources> m_shadersPath;
		std::vector<std::pair<String, Serialized>> m_animSerialized;
	};
}

#include "Assets.tpp"
