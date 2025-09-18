#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Core/Signal.h"
#include "SandCastle/Render/Color.h"

namespace SandCastle
{
	class Ui;
	class Ui::Btn : public Ui::Elem
	{
	public:

	public:
		Ui::Elem::Type GetType() const override;
		void SetColor(const Color& color);
	protected:
		void OnHover() override;
		void OnUnHover() override;
		void OnClickPressed() override;
		void OnClickReleased() override;
	protected:
		friend Ui;
		Sentence label;
		Entity frameIdle;
		Entity frameHover;
		Entity framePressed;
		Vec2f labelOffset = 0.f;
	};
}
