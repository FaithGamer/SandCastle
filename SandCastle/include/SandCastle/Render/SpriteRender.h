#pragma once

#include <vector>
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Render/Rect.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Render/Material.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Render/Layer.h"

namespace SandCastle
{
	/// @brief Component to render sprite
	class SpriteRender
	{
	public:
		SpriteRender();
		SpriteRender(Sprite* sprite, MaterialID material = 0);

		void SetSprite(Sprite* sprite);
		void SetMaterial(MaterialID Material);
		void SetLayer(LayerID Layer);

		inline Sprite* GetSprite() const
		{
			return m_sprite;
		}

		inline const Texture* GetTexture() const
		{
			return m_sprite->GetTexture();
		}

		inline MaterialID GetMaterialID() const
		{
			return m_material;
		}

		inline LayerID GetLayer() const
		{
			return m_layer;
		}

	public:
		Vec4f color;
		bool spriteDimensionsChanged;
		static LayerID defaultLayer;
	
	protected:
		MaterialID m_material = 0;
		Sprite* m_sprite;
		LayerID m_layer;

	};
}
