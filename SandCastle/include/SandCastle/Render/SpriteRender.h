#pragma once

#include <vector>
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Render/Rect.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Render/Shader.h"
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Render/Sprite.h"

namespace SandCastle
{
	/// @brief Component to render sprite
	class SpriteRender
	{
	public:
		SpriteRender();
		SpriteRender(Sprite* sprite, Shader* Shader);

		void SetSprite(Sprite* sprite);
		void SetShader(Shader* Shader);
		void SetLayer(uint32_t Layer);

		inline Sprite* GetSprite() const
		{
			return m_sprite;
		}

		inline const Texture* GetTexture() const
		{
			return m_sprite->GetTexture();
		}

		inline Shader* GetShader() const
		{
			return m_shader;
		}

		inline uint32_t GetLayer() const
		{
			return m_layer;
		}

	public:
		Vec4f color;
		bool needUpdateRenderBatch;
		uint32_t renderBatch;
		bool spriteDimensionsChanged;
	
	protected:
		Shader* m_shader;
		Sprite* m_sprite;
		uint32_t m_layer;

	};
}
