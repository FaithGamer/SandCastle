#include "pch.h"
#include "SandCastle/Render/SpriteRenderSystem.h"
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Render/Window.h"
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
		if (Window::GetMinimized())
			return;
		group.each([&](Entity e, SpriteRender& sprite, Transform& tr)
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
		//One walk up the parent chain for all three values. Asking the three
		//getters separately would climb it three times.
		auto world = transform->GetWorld();

		return QuadRenderData(
			1,
			world.position,
			sprite->orgX,
			sprite->orgY,
			uvs,
			sprite->GetDimensions() * world.scale,
			world.rotation,
			texture->GetId(),
			render->GetLayer(),
			render->GetMaterialID(),
			render->color
		);
	}

	int SpriteRenderSystem::GetUsedMethod()
	{
		return System::Method::LateUpdt;
	}
}