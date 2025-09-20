#include "pch.h"
#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Render/Transform.h"

namespace SandCastle
{
	UiElem::UiElem()
	{
		root = Entity::Create();
		root.AddComponent<Transform>();
	}

	UiElem::UiElem(Entity Root) : root(Root)
	{
	}

	UiElem::~UiElem()
	{
		root.Destroy();
		//Would be nice not to rely on Ui.h
		/*if (parent != nullptr)
		{
			parent->
		}*/
	}

	void UiElem::SetPosition(Vec2f pos)
	{
		position = pos;

		//All elem anchor are top left (except canvas)
		Vec2f wPos = {
		std::round(pos.x + margin.x),
		std::round(pos.y - margin.y)
		};
		root.gtr()->SetPosition(wPos.x, wPos.y, z);
	}
	Vec2f UiElem::GetSize() const
	{
		return size;
	}
	Vec2f UiElem::GetPosition() const
	{
		return position;
	}
	UiElem::State UiElem::GetState() const
	{
		return state;
	}
	void UiElem::ComputeHitbox()
	{
		auto tr = root.GetComponent<Transform>();
		auto pos = tr->GetPosition();
		hitbox = Rect(pos.x, pos.y, size.x, size.y);
	}
	void UiElem::Move(Vec2f offset)
	{
		SetPosition(position + offset);
	}
	bool UiElem::IsInside(Vec2f uiPos)
	{
		return hitbox.PointInside(uiPos);
	}
	void UiElem::Hover()
	{
		OnHover();
		hoverSignal.Send(this);
	}
	void UiElem::UnHover()
	{
		OnUnHover();
		pressed = false;
		unhoverSignal.Send(this);
	}
	void UiElem::ClickPressed()
	{
		OnClickPressed();
		pressed = true;
		clickPressSignal.Send(this);
	}
	void UiElem::ClickReleased()
	{
		if (pressed)
		{
			OnClickReleased();
			clickReleasedSignal.Send(this);
		}
		pressed = false;
	}
}