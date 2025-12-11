#include "pch.h"
#include "SandCastle/UI/UiBtn.h"
#include "SandCastle/UI/Ui.h"
#include "SandCastle/Core/Assets.h"

namespace SandCastle
{
	UiBtn::~UiBtn()
	{
		if (keyLoc != "")
			Assets::Instance()->langSignal.StopListen(this);
	}
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
		frameDisabled.Update(this, -3.f);
		ShowHideFrame();
	}
	void UiBtn::OnHover()
	{
		ShowHideFrame();
	}
	void UiBtn::OnUnHover()
	{
		ShowHideFrame();
		ResetLabelOffset();
	}
	void UiBtn::OnClickPressed()
	{
		ShowHideFrame();
		labelOffset += Vec2f(-1, -1);
		label.root.gtr()->Move(labelOffset.x, labelOffset.y, 0.f);
		if (context.pressSound != nullptr)
		{
			context.pressSound->Play();
		}
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
		if (context.releaseSound != nullptr)
		{
			context.releaseSound->Play();
		}
		ResetLabelOffset();
	}
	void UiBtn::OnDisable()
	{
		SetLabelColor(context.textColorDisabled);
		ShowHideFrame();
		ResetLabelOffset();
		
	}
	void UiBtn::OnEnable()
	{
		SetLabelColor(context.textColor);
		ShowHideFrame();
		for (auto& glyph : label.glyphEntities)
		{
			glyph.Get<SpriteRender>()->color = context.textColor;
		}
	}
	void UiBtn::OnLang(LangSignal* signal)
	{
		langSignal.Send(this);
	}
	void UiBtn::ShowHideFrame()
	{
		if (disabled)
		{
			frameIdle.SetAlpha(0);
			framePressed.SetAlpha(0);
			frameHover.SetAlpha(0);
			frameDisabled.SetAlpha(255);
			return;
		}
		switch (state)
		{
		case UiElem::State::Idle:
			frameIdle.SetAlpha(255);
			framePressed.SetAlpha(0);
			frameHover.SetAlpha(0);
			frameDisabled.SetAlpha(0);
			break;
		case UiElem::State::Hovered:
			frameIdle.SetAlpha(0);
			framePressed.SetAlpha(0);
			frameHover.SetAlpha(255);
			frameDisabled.SetAlpha(0);
			break;
		case UiElem::State::Pressed:
			frameIdle.SetAlpha(0);
			framePressed.SetAlpha(255);
			frameHover.SetAlpha(0);
			frameDisabled.SetAlpha(0);
			break;
		default:
			break;
		}
	}
	void UiBtn::ResetLabelOffset()
	{
		label.root.gtr()->Move(-labelOffset.x, -labelOffset.y, 0.f);
		labelOffset = 0.f;
	}
	void UiBtn::SetLabelColor(Color color)
	{
		for (auto& glyph : label.glyphEntities)
		{
			glyph.Get<SpriteRender>()->color = color;
		}
	}
}