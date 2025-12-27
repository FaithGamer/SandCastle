#include "pch.h"
#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/UI/UiCanvas.h"
namespace SandCastle
{

	UiElem::~UiElem()
	{
		destroySignal.Send(this);
		if (root.Valid())
			root.Destroy();
		if (data.Valid())
			data.Destroy();
	}

	UiElem::ID UiElem::GetID() const
	{
		return id;
	}

	UiCanvas* UiElem::GetParent() const
	{
		return parent;
	}

	int UiElem::GetParentCount() const
	{
		if (parent == nullptr)
			return 0;
		int count = 1;
		UiCanvas* p = parent->GetParent();
		while (p != nullptr)
		{
			count++;
			p = p->GetParent();
		}
		return count;
	}

	void UiElem::SetPosition(Vec2f pos)
	{
		position = pos;

		if (parent != nullptr)
		{
			pos += parent->GetPosition();
		}

		//All elem anchor are top left (except canvas)
		Vec2f wPos = {
		std::round(pos.x + margin.x),
		std::round(pos.y - margin.y)
		};
		root.gtr()->SetPosition(wPos.x, wPos.y, z+zOffset);
	}
	Vec2f UiElem::GetSize() const
	{
		return size;
	}
	Vec2f UiElem::GetPosition() const
	{
		if (parent != nullptr)
			return position + parent->GetPosition();
		else
			return position;
	}
	Vec2f UiElem::GetLocalPosition() const
	{
		return position;
	}
	UiElem::State UiElem::GetState() const
	{
		return state;
	}
	void UiElem::ComputeHitbox()
	{
		auto tr = root.Get<Transform>();
		auto pos = tr->GetPosition();
		hitbox = Rect(pos.x, pos.y, size.x, size.y);
	}
	void UiElem::SetAbsolutePos(bool absolute)
	{
		absolutePos = absolute;
	}
	void UiElem::SetData(Entity entt)
	{
		if (data.Valid())
			data.Destroy();
		data = entt;
	}
	void UiElem::SetZOffset(float offset)
	{
		zOffset = offset;
		SetPosition(position);
	}
	void UiElem::SetHoverableWhenDisabled(bool yesorno)
	{
		hoverableWhenDisabled = yesorno;
	}
	Entity UiElem::GetData()
	{
		return data;
	}
	bool UiElem::IsDestroyed() const
	{
		return parent == nullptr ? destroyed : (destroyed || parent->IsDestroyed());
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
		state = State::Hovered;
		OnHover();
		hoverSignal.Send(this);
	}
	void UiElem::Disable()
	{
		if (disabled == true)
			return;
		disabled = true;
		OnDisable();
	}
	void UiElem::Enable()
	{
		if (disabled == false)
			return;
		disabled = false;
		OnEnable();
	}
	void UiElem::UnHover()
	{
		state = State::Idle;
		pressed = false;
		OnUnHover();
		unhoverSignal.Send(this);
	}
	void UiElem::ClickPressed()
	{
		if (disabled == true)
			return;
		state = State::Pressed;
		pressed = true;
		OnClickPressed();
		clickPressSignal.Send(this);
	}
	void UiElem::ClickReleased()
	{
		if (disabled == true)
			return;
		state = State::Hovered;
		if (pressed)
		{
			pressed = false;
			OnClickReleased();
			clickReleasedSignal.Send(this);
		}
	}
}