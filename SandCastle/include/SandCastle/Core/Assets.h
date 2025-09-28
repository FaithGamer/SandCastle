#pragma once
#include "std_macros.h"
#include "SandCastle/Internal/Singleton.h"
#include "TypeId.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Core/Delegate.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Core/Signal.h"
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
		{ }
		Asset(T&& ptr) : m_ptr(ptr)
		{

		}
		~Asset()
		{
	
		}

		Asset(const Asset<T>& asset) = delete;
		Asset(const Asset<T>&& asset) = delete;
		inline T* Ptr()
		{
			return &m_ptr;
		}
		inline int32_t GetType()
		{
			return TypeId::GetId<T>();
		}
	private:

		friend Assets;
		T m_ptr;

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
		static T* Get(String name)
		{
			return Instance()->GetPrivate<T>(name);
		}
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
		void ChangeLocaTexture(sptr<OpaqueAsset>& prev, sptr<OpaqueAsset>& next);
		void ChangeLocaText();

		void GenerateSprites(String filename, Serialized& spritesheet, const Texture* texture);
		void LoadAssets();
		void InitFunctions();
		void AddAsset(const String& path, bool localized, const String& lang);
		void CompileShaders();
		template<class T>
		void InsertAsset(const String& filename, sptr<Asset<T>> asset)
		{
			if (m_assets.find(filename) == m_assets.end())
			{
				m_assets.insert(MakePair(filename, asset));
			}
			else
			{
				LOG_ERROR("More than one asset with the same filename: " + filename + ", only one could be loaded.");
			}
		}
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
		void AddConfig(const String& filename, const String& path, bool localized, const String& lang);
		void AddFragmentShader(const String& filename, const String& path, bool localized, const String& lang);
		void AddVertexShader(const String& filename, const String& path, bool localized, const String& lang);
		void AddGeometryShader(const String& filename, const String& path, bool localized, const String& lang);
		void AddAudio(const String& filename, const String& path, bool localized, const String& lang);

	
		template <class T>
		T* GetPrivate(String name)
		{
			auto find_it = m_assets.find(name);

			ASSERT_LOG_ERROR((bool)(find_it != m_assets.end()), "Cannot find asset, " + name);
			ASSERT_LOG_ERROR((bool)(find_it->second->GetType() == TypeId::GetId<T>()), "Getting wrong asset type, " + name);

			return static_pointer_cast<Asset<T>>(find_it->second)->Ptr();
		} 

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
		std::unordered_map<String, sptr<OpaqueAsset>> m_assets;
		std::unordered_map<String, Delegate<void, Assets, const String&, const String&, bool, const String&>> m_addAssetFunctions;
		std::unordered_map<int32_t, Delegate<void, Assets, sptr<OpaqueAsset>&, sptr<OpaqueAsset>&>> m_changeLocaFunctions;
		std::unordered_map<String, ShaderSources> m_shadersPath;
		std::vector<std::pair<String, Serialized>> m_animations;
	};
}
