#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Core/Signal.h"
#include "SandCastle/Render/Text.h"
#include "SandCastle/Render/Color.h"

namespace SandCastle
{
	class Ui;
	class UiBtn : public UiElem
	{
	public:

	public:
		UiElem::Type GetType() const override;
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
