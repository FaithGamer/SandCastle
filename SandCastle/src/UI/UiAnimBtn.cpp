#include "pch.h"
#include "SandCastle/UI/UiAnimBtn.h"
#include "SandCastle/Render/Animator.h"
#include "SandCastle/UI/Ui.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/UI/UiCanvas.h"

namespace SandCastle
{
	UiAnimBtn::~UiAnimBtn()
	{
		if (keyLoc != "")
			Assets::Instance()->langSignal.StopListen(this);
	}
	UiElem::Type UiAnimBtn::GetType() const
	{
		return UiElem::Type::Button;
	}

	void UiAnimBtn::SetPosition(Vec2f pos)
	{
		position = pos;
		//Offset sprite to make top left anchor no matter the sprite origin
		auto tr = root.Get<Transform>();
		auto rd = root.Get<SpriteRender>();
		auto spr = rd->GetSprite();
		auto dim = spr->GetDimensions();
		Vec2f offset = {
			((float)spr->orgX + 0.5f) * dim.x,
			((float)spr->orgY - 0.5f) * dim.y
		};

		pos += offset;

		if (parent != nullptr)
		{
			pos += parent->GetPosition();
		}

		pos = {
		std::round(pos.x + margin.x),
		std::round(pos.y - margin.y)
		};
		root.gtr()->SetPosition(pos.x, pos.y, z+zOffset);
	}

	void UiAnimBtn::ComputeHitbox()
	{
		auto tr = root.Get<Transform>();
		auto rd = root.Get<SpriteRender>();
		auto spr = rd->GetSprite();
		auto dim = spr->GetDimensions();
		Vec2f offset = {
			((float)spr->orgX + 0.5f) * dim.x,
			((float)spr->orgY - 0.5f) * dim.y
		};
		auto pos = tr->GetPosition();
		hitbox = Rect(
			pos.x - offset.x,
			pos.y - offset.y,
			size.x,
			size.y);
	}

	void UiAnimBtn::OnHover()
	{
		UpdateAnim();
	}

	void UiAnimBtn::OnUnHover()
	{
		UpdateAnim();
		ResetLabelOffset();
	}

	void UiAnimBtn::OnClickPressed()
	{
		labelOffset += Vec2f(-1, -1);
		label.root.gtr()->Move(labelOffset.x, labelOffset.y, 0.f);
		if (context.pressSound != nullptr)
		{
			context.pressSound->Play();
		}
		UpdateAnim();
	}

	void UiAnimBtn::OnClickReleased()
	{
		if (context.releaseSound != nullptr)
		{
			context.releaseSound->Play();
		}
		ResetLabelOffset();
		UpdateAnim();
	}

	void UiAnimBtn::OnDisable()
	{
		SetLabelColor(context.textColorDisabled);
		UpdateAnim();
		ResetLabelOffset();
	}

	void UiAnimBtn::OnEnable()
	{
		SetLabelColor(context.textColor);
		UpdateAnim();
	}

	void UiAnimBtn::UpdateAnim()
	{
		if (disabled)
		{
			root.Get<Animator>()->SetAnimation("disabled");
			return;
		}
		if (pressed)
		{
			root.Get<Animator>()->SetAnimation("pressed");
			return;
		}
		if (IsInside(Ui::MousePos()))
		{
			root.Get<Animator>()->SetAnimation("hover");
		}
		else
		{
			root.Get<Animator>()->SetAnimation("idle");
		}
	}
}