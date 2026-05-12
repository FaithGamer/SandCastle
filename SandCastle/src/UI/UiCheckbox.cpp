#include "pch.h"
#include "SandCastle/UI/UiCheckbox.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/Transform.h"

namespace SandCastle
{
	UiElem::Type UiCheckbox::GetType() const
	{
		return UiElem::Type::Checkbox;
	}
	void UiCheckbox::SetChecked(bool ch)
	{
		checked = ch;
		UpdateVisual();
	}
	void UiCheckbox::OnHover()
	{
		UpdateVisual();
	}
	void UiCheckbox::OnUnHover()
	{
		UpdateVisual();
	}
	void UiCheckbox::OnClickPressed()
	{

	}
	void UiCheckbox::OnClickReleased()
	{
		checked = !checked;
		if (value != nullptr)
			*value = checked;
		checkSignal.Send(this);
		if (checked)
		{
			Show(sprites[Checked], true);
			Show(sprites[Unchecked], false);
			Show(sprites[Hover], false);
		}
		else
		{
			Show(sprites[Checked], false);
			Show(sprites[Unchecked], true);
			Show(sprites[Hover], false);
		}
	}
	void UiCheckbox::UpdateVisual()
	{
		switch (state)
		{
		case State::Idle:
			if (checked)
			{
				Show(sprites[Checked], true);
				Show(sprites[Unchecked], false);
				Show(sprites[Hover], false);
			}
			else
			{
				Show(sprites[Checked], false);
				Show(sprites[Unchecked], true);
				Show(sprites[Hover], false);
			}
			break;
		case State::Hovered:
			Show(sprites[Checked], false);
			Show(sprites[Unchecked], false);
			Show(sprites[Hover], true);
			break;
		case State::Pressed:
			Show(sprites[Checked], false);
			Show(sprites[Unchecked], false);
			Show(sprites[Hover], true);
			break;
		default:
			break;
		}
	}
	void UiCheckbox::Show(Shown& sprite, bool show)
	{
		if (show == sprite.shown)
			return;
		auto rd = sprite.entt.Get<SpriteRender>();
		rd->color.a = show ? 255 : 0;
		sprite.shown = show;
		/*if (show)
		{
			sprite.entt.gtr()->Move(999999, 999999, 0);
		}
		else
		{
			sprite.entt.gtr()->Move(-999999, -999999, 0);

		}*/
	}
}