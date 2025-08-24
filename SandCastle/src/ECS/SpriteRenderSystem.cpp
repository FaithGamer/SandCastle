#include "pch.h"
#include "SandCastle/ECS/SpriteRenderSystem.h"

#include "SandCastle/Render.h"

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
		static auto group = Entity::registry.group<SpriteRender, Transform>();
		group.each([&](SpriteRender& sprite, Transform& tr)
			{
				sprite.needUpdateRenderBatch = true;
			});
	}

	void SpriteRenderSystem::LateUpdate()
	{
		sptr<Renderer2D> renderer = Renderer2D::Instance();
		auto group = Entity::registry.group<SpriteRender, Transform>();
		if (!m_zSort)
		{
			group.each([&](SpriteRender& sprite, Transform& tr)
				{
					if (sprite.GetSprite() == nullptr)
						return;
					if (sprite.needUpdateRenderBatch)
					{
						sprite.renderBatch = renderer->GetBatchId(sprite.GetLayer(), sprite.GetMaterial());
						sprite.needUpdateRenderBatch = false;
					}
					renderer->PushQuad(MakeQuadRenderDataFromSpriteRender(&sprite, &tr));
				});
		}
		else
		{
			std::list<OrderedSpriteTransform> ordered;

			group.each([&](SpriteRender& sprite, Transform& tr)
				{
					if (sprite.GetSprite() == nullptr)
						return;
					auto ord = OrderedSpriteTransform(&sprite, &tr, tr.GetPosition().z);
					ordered.emplace_back(ord);
				});

			ordered.sort();

			for (auto& sprite : ordered)
			{
				if (sprite.sprite->needUpdateRenderBatch)
				{
					sprite.sprite->renderBatch = renderer->GetBatchId(sprite.sprite->GetLayer(), sprite.sprite->GetMaterial());
					sprite.sprite->needUpdateRenderBatch = false;
				}
				renderer->PushQuad(MakeQuadRenderDataFromSpriteRender(sprite.sprite, sprite.transform));
			}
		}
	}

	QuadRenderData SpriteRenderSystem::MakeQuadRenderDataFromSpriteRender(const SpriteRender* render, const Transform* transform)
	{
		auto sprite = render->GetSprite();
		auto texture = sprite->GetTexture();
		auto uvs = sprite->GetUVs();

		return QuadRenderData(
			1,
			transform->GetPosition(),
			uvs,
			sprite->GetDimensions() * (Vec2f)transform->GetScale(),
			transform->GetRotation().z,
			texture->GetId(),
			render->GetLayer(),
			render->renderBatch,
			render->color.a
		);
	}

	int SpriteRenderSystem::GetUsedMethod()
	{
		return System::Method::LateUpdt;
	}
}