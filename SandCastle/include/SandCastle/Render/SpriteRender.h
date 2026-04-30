#pragma once

#include <vector>
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Render/Rect.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Render/Material.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Render/Layer.h"
#include "SandCastle/Render/Color.h"

namespace SandCastle
{
	/// @brief ECS component that draws a Sprite at the entity's Transform.
	/// Pair with a Transform on the same entity. The SpriteRenderSystem submits
	/// it to the renderer each frame using the material/layer/color set here.
	class SpriteRender
	{
	public:
		SpriteRender();
		SpriteRender(Sprite* sprite, MaterialID material = 0);

		/// @brief Replace the displayed sprite.
		void SetSprite(Sprite* sprite);
		/// @brief Override the material used to render the sprite.
		void SetMaterial(MaterialID Material);
		/// @brief Override the render layer this sprite is drawn on.
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
		Color color;
		LayerID m_layer;
		static LayerID defaultLayer;
	
	protected:
		MaterialID m_material = 0;
		Sprite* m_sprite;

	};
}
