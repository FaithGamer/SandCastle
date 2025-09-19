#include "pch.h"
#include "SandCastle/UI/UiElem.h"
#include "SandCastle/UI/UiEnum.h"
#include "SandCastle/Render/Transform.h"

namespace SandCastle
{
	Ui::Elem::Elem()
	{
		root = Entity::Create();
		root.AddComponent<Transform>();
	}

	Ui::Elem::Elem(Entity Root) : root(Root)
	{
	}

	Ui::Elem::~Elem()
	{
		root.Destroy();
	}

	void Ui::Elem::SetPosition(Vec2f pos)
	{
		position = pos;

		//All elem anchor are top left (except canvas)
		Vec2f wPos = {
		std::round(pos.x + margin.x),
		std::round(pos.y - margin.y)
		};
		root.gtr()->SetPosition(wPos.x, wPos.y, z);
	}
	Vec2f Ui::Elem::GetSize() const
	{
		return size;
	}
	Vec2f Ui::Elem::GetPosition() const
	{
		return position;
	}
	void Ui::Elem::ComputeHitbox()
	{
		auto tr = root.GetComponent<Transform>();
		auto pos = tr->GetPosition();
		hitbox = Rect(pos.x, pos.y, size.x, size.y);
	}
	void Ui::Elem::Move(Vec2f offset)
	{
		SetPosition(position + offset);
	}
	bool Ui::Elem::IsInside(Vec2f uiPos)
	{
		return hitbox.PointInside(uiPos);
	}
	void Ui::Elem::Hover()
	{
		OnHover();
		hoverSignal.Send(this);
	}
	void Ui::Elem::UnHover()
	{
		OnUnHover();
		pressed = false;
		unhoverSignal.Send(this);
	}
	void Ui::Elem::ClickPressed()
	{
		OnClickPressed();
		pressed = true;
		clickPressSignal.Send(this);
	}
	void Ui::Elem::ClickReleased()
	{
		if (pressed)
		{
			OnClickReleased();
			clickReleasedSignal.Send(this);
		}
		pressed = false;
	}
}