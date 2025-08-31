#include "pch.h"
#include "SandCastle/Render/SpriteRender.h"

namespace SandCastle
{
	LayerID SpriteRender::defaultLayer = 0;

	SpriteRender::SpriteRender()
		: m_sprite(nullptr), m_material(0), m_layer(defaultLayer)
	{

	}
	SpriteRender::SpriteRender(Sprite* sprite, MaterialID material)
		: m_sprite(sprite), m_material(material), m_layer(defaultLayer)
	{

	}

	void SpriteRender::SetSprite(Sprite* sprite)
	{
		m_sprite = sprite;

		if (!m_sprite || m_sprite->GetDimensions() == sprite->GetDimensions())
			return;

	}

	void SpriteRender::SetMaterial(MaterialID Material)
	{
		m_material = Material;
	}

	void SpriteRender::SetLayer(LayerID Layer)
	{
		m_layer = Layer;
	}
}