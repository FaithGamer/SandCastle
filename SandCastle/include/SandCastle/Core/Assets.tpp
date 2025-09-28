
#include "SandCastle/Core/Assets.h"

namespace SandCastle
{
	template <>
	inline static Sprite* Assets::Get(const String& name)
	{
		auto it = Instance()->m_sprites.find(name);
		ASSERT_LOG_ERROR((it != Instance()->m_sprites.end()), "Sprite {0}, doesn't exists.");
		return &it->second;
	}
	template <>
	inline static Texture* Assets::Get(const String& name)
	{
		auto it = Instance()->m_textures.find(name);
		ASSERT_LOG_ERROR((it != Instance()->m_textures.end()), "Texture {0}, doesn't exists.");
		return &it->second;
	}
	template <>
	inline static Shader* Assets::Get(const String& name)
	{
		auto it = Instance()->m_shaders.find(name);
		ASSERT_LOG_ERROR((it != Instance()->m_shaders.end()), "Shader {0}, doesn't exists.");
		return &it->second;
	}
	template <>
	inline static Animation* Assets::Get(const String& name)
	{
		auto it = Instance()->m_animations.find(name);
		ASSERT_LOG_ERROR((it != Instance()->m_animations.end()), "Animation {0}, doesn't exists.");
		return &it->second;
	}
	template <>
	inline static Textual* Assets::Get(const String& name)
	{
		auto it = Instance()->m_textuals.find(name);
		ASSERT_LOG_ERROR((it != Instance()->m_textuals.end()), "Textual {0}, doesn't exists.");
		return &it->second;
	}
}