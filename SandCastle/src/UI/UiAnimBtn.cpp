#include "pch.h"
#include "SandCastle/UI/UiAnimBtn.h"
#include "SandCastle/Render/Animator.h"
#include "SandCastle/UI/Ui.h"
#include "SandCastle/Core/Assets.h"

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