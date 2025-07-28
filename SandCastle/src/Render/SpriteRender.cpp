#include "pch.h"
#include "SandCastle/Render/SpriteRender.h"

namespace SandCastle
{
	SpriteRender::SpriteRender()
		: m_sprite(nullptr), m_shader(nullptr), m_layer(0), color(Vec4f(1, 1, 1, 1)),
		needUpdateRenderBatch(true), renderBatch(0), spriteDimensionsChanged(true)
	{

	}
	SpriteRender::SpriteRender(sptr<Sprite> sprite, sptr<Shader> Shader = nullptr)
		: m_sprite(sprite), m_shader(Shader), m_layer(0), color(Vec4f(1, 1, 1, 1)),
		needUpdateRenderBatch(true), renderBatch(0), spriteDimensionsChanged(true)
	{

	}

	void SpriteRender::SetSprite(sptr<Sprite> sprite)
	{
		m_sprite = sprite;
		needUpdateRenderBatch = true;

		if (!m_sprite || m_sprite->GetDimensions() == sprite->GetDimensions())
			return;

		spriteDimensionsChanged = true;

	}

	void SpriteRender::SetShader(sptr<Shader> Shader)
	{
		m_shader = Shader;
		needUpdateRenderBatch = true;
	}

	void SpriteRender::SetLayer(uint32_t Layer)
	{
		m_layer = Layer;
		needUpdateRenderBatch = true;
	}
}