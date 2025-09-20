#include "pch.h"
#include "SandCastle/UI/UiBtn.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/UI/Ui.h"

namespace SandCastle
{

	UiElem::Type UiBtn::GetType() const
	{
		return UiElem::Type::Button;
	}
	void UiBtn::SetColor(const Color& color)
	{
		frameIdle.SetColor(color);
		framePressed.SetColor(color);
		frameHover.SetColor(color);
	}
	void UiBtn::UpdateFrames()
	{
		frameIdle.Update(this, 0.f);
		framePressed.Update(this, -1.f);
		frameHover.Update(this, -2.f);
		ShowHideFrame();
	}
	void UiBtn::OnHover()
	{
		ShowHideFrame();
	}
	void UiBtn::OnUnHover()
	{
		ShowHideFrame();
		label.root.gtr()->Move(-labelOffset.x, -labelOffset.y, 0.f);
		labelOffset = 0.f;
	}
	void UiBtn::OnClickPressed()
	{
		ShowHideFrame();
		labelOffset += Vec2f(-1, -1);
		label.root.gtr()->Move(labelOffset.x, labelOffset.y, 0.f);
	}
	void UiBtn::OnClickReleased()
	{
		framePressed.SetAlpha(0);
		if (IsInside(Ui::MousePos()))
		{
			frameHover.SetAlpha(255);
			frameIdle.SetAlpha(0);
		}
		else
		{
			frameHover.SetAlpha(0);
			frameIdle.SetAlpha(255);
		}
		label.root.gtr()->Move(-labelOffset.x, -labelOffset.y, 0.f);
		labelOffset = 0.f;
	}
	void UiBtn::ShowHideFrame()
	{
		switch (state)
		{
		case UiElem::State::Idle:
			frameIdle.SetAlpha(255);
			framePressed.SetAlpha(0);
			frameHover.SetAlpha(0);
			break;
		case UiElem::State::Hovered:
			frameIdle.SetAlpha(0);
			framePressed.SetAlpha(0);
			frameHover.SetAlpha(255);
			break;
		case UiElem::State::Pressed:
			frameIdle.SetAlpha(0);
			framePressed.SetAlpha(255);
			frameHover.SetAlpha(0);
			break;
		default:
			break;
		}
	}
}