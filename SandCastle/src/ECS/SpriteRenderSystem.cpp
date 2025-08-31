#include "pch.h"
#include "SandCastle/Render/SpriteRenderSystem.h"
#include "SandCastle/Render/Renderer2D.h"

namespace SandCastle
{
	struct OrderedSpriteTransform
	{
		SpriteRender* sprite;
		Transform* transform;
		float z;
		bool operator<(const OrderedSpriteTransform rhs) const
		{
			return z > rhs.z;
		}
	};

	SpriteRenderSystem::SpriteRenderSystem() : m_zSort(false)
	{
		SetPriority(9999);
	}

	void SpriteRenderSystem::SetZSort(bool sort)
	{
		m_zSort = sort;
	}

	bool SpriteRenderSystem::GetZSort()
	{
		return m_zSort;
	}

	void SpriteRenderSystem::OnClearBatches()
	{

	}

	void SpriteRenderSystem::LateUpdate()
	{
		sptr<Renderer2D> renderer = Renderer2D::Instance();
		auto group = Entity::registry.group<SpriteRender, Transform>();

		group.each([&](SpriteRender& sprite, Transform& tr)
			{
				if (sprite.GetSprite() == nullptr)
					return;
				renderer->PushQuad(MakeQuadRenderDataFromSpriteRender(&sprite, &tr));
			});

	}

	QuadRenderData SpriteRenderSystem::MakeQuadRenderDataFromSpriteRender(const SpriteRender* render, const Transform* transform)
	{
		auto sprite = render->GetSprite();
		auto texture = sprite->GetTexture();
		auto uvs = sprite->GetUVs();

		return QuadRenderData(
			1,
			transform->GetPosition(),
			sprite->orgX,
			sprite->orgY,
			uvs,
			sprite->GetDimensions() * (Vec2f)transform->GetScale(),
			transform->GetRotation().z,
			texture->GetId(),
			render->GetLayer(),
			render->GetMaterialID(),
			render->color.a
		);
	}

	int SpriteRenderSystem::GetUsedMethod()
	{
		return System::Method::LateUpdt;
	}
}