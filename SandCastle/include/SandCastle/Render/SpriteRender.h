#pragma once

#include <vector>
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Render/Rect.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Render/Material.h"
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Render/Sprite.h"

namespace SandCastle
{
	/// @brief Component to render sprite
	class SpriteRender
	{
	public:
		SpriteRender();
		SpriteRender(Sprite* sprite, Material* material);

		void SetSprite(Sprite* sprite);
		void SetMaterial(Material* Material);
		void SetLayer(uint32_t Layer);

		inline Sprite* GetSprite() const
		{
			return m_sprite;
		}

		inline const Texture* GetTexture() const
		{
			return m_sprite->GetTexture();
		}

		inline Material* GetMaterial() const
		{
			return m_material;
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
		Material* m_material;
		Sprite* m_sprite;
		uint32_t m_layer;

	};
}
