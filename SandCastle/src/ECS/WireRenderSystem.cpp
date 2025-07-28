#include "pch.h"
#include "SandCastle/ECS/WireRenderSystem.h"
#include "SandCastle/Render/WireRender.h"
#include "SandCastle/Render/Renderer2D.h"

namespace SandCastle
{
	WireRenderSystem::WireRenderSystem()
	{
		SetPriority(-9999);
	}

	void WireRenderSystem::OnRender()
	{
		sptr<Renderer2D> renderer = Renderer2D::Instance();

		ForeachComponents<WireRender, Transform>([renderer](WireRender& wire, Transform& transform)
			{
				renderer->DrawWire(wire, transform, wire.GetLayer());
			});
	}

	int WireRenderSystem::GetUsedMethod()
	{
		return System::Method::Render;
	}
}