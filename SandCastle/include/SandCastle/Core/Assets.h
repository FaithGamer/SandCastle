#pragma once
#include "std_macros.h"
#include "SandCastle/Internal/Singleton.h"
#include "TypeId.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Core/Delegate.h"
#include "SandCastle/Render/Texture.h"
#include <unordered_map>

namespace SandCastle
{
	class Assets;

	class OpaqueAsset
	{
	public:
		virtual ~OpaqueAsset() {};
		virtual int32_t GetType() = 0;
	protected:
	};

	template<class T>
	class Asset : public OpaqueAsset
	{
	public:
		Asset(T* ptr) : m_ptr(ptr)
		{

		}
		~Asset()
		{
			delete m_ptr;
		}

		Asset(const Asset<T>& asset) = delete;
		Asset(const Asset<T>&& asset) = delete;
		inline T* Ptr()
		{
			return m_ptr;
		}
		inline int32_t GetType()
		{
			return TypeId::GetId<T>();
		}
	private:

		friend Assets;
		T* m_ptr;

	};

	class Assets : public Singleton<Assets>
	{
	public:

		Assets();
		void Init();

		void HotReload();
	

		template <class T>
		static T* Get(String name)
		{
			return Instance()->GetPrivate<T>(name);
		}

	private:
		friend sptr<Assets> Singleton<Assets>::Instance();
		friend void Singleton<Assets>::Kill();

		template<class T, class... Args>
		static sptr<Asset<T>> MakeAsset(Args&&... args)
		{
			return makesptr<Asset<T>>(new T(args...));
			//asset->m_ptr = new T(args...);
			//return asset;
		}

		void GenerateSprites(String filename, Serialized& spritesheet, const Texture* texture);
		void LoadAssets();
		void InitAddAssetFunctions();
		void AddAsset(String path);
		void CompileShaders();
		template<class T>
		void InsertAsset(String filename, sptr<Asset<T>> asset)
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
		void CreateAnimations();
		Serialized CreateDefaultTextureImportSettings();
		Serialized CreateDefaultSpritesheet(const Texture* texture);
		void AddAnimation(String filename, String path);
		void AddTexture(String filename, String path);
		void AddConfig(String filename, String path);
		void AddFragmentShader(String filename, String path);
		void AddVertexShader(String filename, String path);
		void AddGeometryShader(String filename, String path);
		void AddAudio(String filename, String path);

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
		bool m_reloading = false;
		std::unordered_map<String, sptr<OpaqueAsset>> m_assets;
		std::unordered_map<String, Delegate<void, String, String>> m_addAssetFunctions;
		std::unordered_map<String, ShaderSources> m_shadersPath;
		TextureImportSettings m_defaultImportSettings;
		std::vector<std::pair<String, Serialized>> m_animations;
	};
}
