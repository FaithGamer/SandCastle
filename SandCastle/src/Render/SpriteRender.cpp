#include "pch.h"
#include "SandCastle/Render/SpriteRender.h"

namespace SandCastle
{
	uint32_t SpriteRender::defaultLayer = 0;

	SpriteRender::SpriteRender()
		: m_sprite(nullptr), m_material(nullptr), m_layer(defaultLayer), color(Vec4f(1, 1, 1, 1)),
		needUpdateRenderBatch(true), renderBatch(0), spriteDimensionsChanged(true)
	{

	}
	SpriteRender::SpriteRender(Sprite* sprite, Material* material = nullptr)
		: m_sprite(sprite), m_material(material), m_layer(defaultLayer), color(Vec4f(1, 1, 1, 1)),
		needUpdateRenderBatch(true), renderBatch(0), spriteDimensionsChanged(true)
	{

	}

	void SpriteRender::SetSprite(Sprite* sprite)
	{
		m_sprite = sprite;
		needUpdateRenderBatch = true;

		if (!m_sprite || m_sprite->GetDimensions() == sprite->GetDimensions())
			return;

		spriteDimensionsChanged = true;

	}

	void SpriteRender::SetMaterial(Material* Material)
	{
		m_material = Material;
		needUpdateRenderBatch = true;
	}

	void SpriteRender::SetLayer(uint32_t Layer)
	{
		m_layer = Layer;
		needUpdateRenderBatch = true;
	}
}