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

	SpriteRenderSystem::SpriteRenderSystem() : m_zSort(true)
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
		ForeachComponents<SpriteRender>([&](SpriteRender& sprite)
			{
				sprite.needUpdateRenderBatch = true;
			});
	}
	void SpriteRenderSystem::OnRender()
	{
		sptr<Renderer2D> renderer = Renderer2D::Instance();

		if (!m_zSort)
		{
			ForeachComponents<SpriteRender, Transform>([renderer](SpriteRender& sprite, Transform& transform)
				{
					if (sprite.needUpdateRenderBatch)
					{
						sprite.renderBatch = renderer->GetBatchId(sprite.GetLayer(), sprite.GetShader());
						sprite.needUpdateRenderBatch = false;
					}
					auto data = MakeQuadRenderDataFromSpriteRender(&sprite, &transform);
					renderer->DrawQuad(data);
				});
		}
		else
		{
			std::list<OrderedSpriteTransform> ordered;

			ForeachComponents<SpriteRender, Transform>([&ordered, renderer](SpriteRender& sprite, Transform& transform)
				{
					auto ord = OrderedSpriteTransform(&sprite, &transform, transform.GetPosition().z);
					ordered.emplace_back(ord);
				});

			ordered.sort();

			for (auto& sprite : ordered)
			{
				if (sprite.sprite->needUpdateRenderBatch)
				{
					sprite.sprite->renderBatch = renderer->GetBatchId(sprite.sprite->GetLayer(), sprite.sprite->GetShader());
					sprite.sprite->needUpdateRenderBatch = false;
				}
				auto data = MakeQuadRenderDataFromSpriteRender(sprite.sprite, sprite.transform);
				renderer->DrawQuad(data);
			}
		}
	}

	QuadRenderData SpriteRenderSystem::MakeQuadRenderDataFromSpriteRender(const SpriteRender* render, const Transform* transform)
	{
		auto sprite = render->GetSprite();
		auto texture = sprite->GetTexture();
		auto uvs = render->GetSprite()->GetUVs();

		return QuadRenderData(
			1,
			transform->GetPosition(),
			uvs,
			sprite->GetDimensions() * (Vec2f)transform->GetScale(),
			transform->GetRotation().z,
			texture->GetId(),
			texture->GetPixelPerUnit(),
			render->GetLayer(),
			render->renderBatch,
			render->color.a
		);
	}

	int SpriteRenderSystem::GetUsedMethod()
	{
		return System::Method::Render;
	}
}