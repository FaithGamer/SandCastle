#include "pch.h"
#include "SandCastle/UI/UiBtn.h"
#include "SandCastle/Render/SpriteRender.h"

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
	Ui::Elem::Type Ui::Btn::GetType() const
	{
		return Ui::Elem::Type::Button;
	}
	void Ui::Btn::SetColor(const Color& color)
	{
		Coloring(frameIdle, color);
		Coloring(framePressed, color);
		Coloring(frameHover, color);
	}
	void Ui::Btn::OnHover()
	{
		Alpha(frameIdle, 0);
		Alpha(framePressed, 0);
		Alpha(frameHover, 255);
	}
	void Ui::Btn::OnUnHover()
	{
		Alpha(frameIdle, 255);
		Alpha(framePressed, 0);
		Alpha(frameHover, 0);
		label.root.gtr()->Move(-labelOffset.x, -labelOffset.y, 0.f);
		labelOffset = 0.f;
	}
	void Ui::Btn::OnClickPressed()
	{
		Alpha(frameIdle, 0);
		Alpha(framePressed, 255);
		Alpha(frameHover, 0);
		labelOffset += Vec2f(-1, -1);
		label.root.gtr()->Move(labelOffset.x, labelOffset.y, 0.f);
	}
	void Ui::Btn::OnClickReleased()
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