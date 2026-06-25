#include "pch.h"
#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/UI/UiCanvas.h"
#include "SandCastle/UI/Ui.h"
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
	void UiElem::AddNav(NavDir dir, UiElem* target)
	{
		navTargets[(int)dir] = target;
	}
	UiElem* UiElem::GetNav(NavDir dir) const
	{
		return navTargets[(int)dir];
	}
	void UiElem::Navigate()
	{
		Ui::SetNavigated(this);
	}
	void UiElem::NavPressed(NavDir dir)
	{
		navPressSignals[(int)dir].Send(this);
	}
	void UiElem::NavReleased(NavDir dir)
	{
		navReleaseSignals[(int)dir].Send(this);
	}
	void UiElem::ResetPress()
	{
		if (!pressed)
			return;
		// Return to the resting visual without firing the released callback: navigating
		// away from a held button cancels the press, it doesn't trigger the action.
		state = State::Idle;
		pressed = false;
		OnUnHover();
	}
	void UiElem::SelectPressed()
	{
		selectPressSignal.Send(this);
		if (Ui::IsClickIsSelect() && clickable && !disabled)
		{
			// Mirror ClickPressed so the pressed visual frame shows in gamepad mode.
			state = State::Pressed;
			pressed = true;
			OnClickPressed();
			clickPressSignal.Send(this);
		}
	}
	void UiElem::SelectReleased()
	{
		selectReleaseSignal.Send(this);
		if (Ui::IsClickIsSelect() && clickable && !disabled && pressed)
		{
			// A navigated element rests on the idle frame (the selector ring marks
			// navigation), so return to Idle rather than Hovered like the mouse path.
			state = State::Idle;
			pressed = false;
			OnClickReleased();
			clickReleasedSignal.Send(this);
		}
	}
	void UiElem::CancelPressed()
	{
		cancelPressSignal.Send(this);
	}
	void UiElem::CancelReleased()
	{
		cancelReleaseSignal.Send(this);
	}
}