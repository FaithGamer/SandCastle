#include "pch.h"
#include "SandCastle/UI/UiBtn.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/UI/Ui.h"

namespace SandCastle
{
	void Alpha(Entity& entity, unsigned char alpha)
	{
		auto children = entity.GetComponent<Children>();
		for (auto& child : children->children)
		{
			Entity(child).GetComponent<SpriteRender>()->color.a = alpha;
		}
	};
	void Coloring(Entity& entity, const Color& color)
	{
		auto children = entity.GetComponent<Children>();
		for (auto& child : children->children)
		{
			auto spr = Entity(child).GetComponent<SpriteRender>();
			spr->color.r = color.r;
			spr->color.g = color.g;
			spr->color.b = color.b;
		}
	};
	UiElem::Type UiBtn::GetType() const
	{
		return UiElem::Type::Button;
	}
	void UiBtn::SetColor(const Color& color)
	{
		Coloring(frameIdle, color);
		Coloring(framePressed, color);
		Coloring(frameHover, color);
	}
	void UiBtn::OnHover()
	{
		Alpha(frameIdle, 0);
		Alpha(framePressed, 0);
		Alpha(frameHover, 255);
	}
	void UiBtn::OnUnHover()
	{
		Alpha(frameIdle, 255);
		Alpha(framePressed, 0);
		Alpha(frameHover, 0);
		label.root.gtr()->Move(-labelOffset.x, -labelOffset.y, 0.f);
		labelOffset = 0.f;
	}
	void UiBtn::OnClickPressed()
	{
		Alpha(frameIdle, 0);
		Alpha(framePressed, 255);
		Alpha(frameHover, 0);
		labelOffset += Vec2f(-1, -1);
		label.root.gtr()->Move(labelOffset.x, labelOffset.y, 0.f);
	}
	void UiBtn::OnClickReleased()
	{
		Alpha(framePressed, 0);
		if (IsInside(Ui::MousePos()))
		{
			Alpha(frameHover, 255);
			Alpha(frameIdle, 0);
		}
		else
		{
			Alpha(frameHover, 0);
			Alpha(frameIdle, 255);
		}
		label.root.gtr()->Move(-labelOffset.x, -labelOffset.y, 0.f);
		labelOffset = 0.f;
	}
}