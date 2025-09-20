#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Core/Signal.h"
#include "SandCastle/Render/Text.h"
#include "SandCastle/Render/Color.h"
#include "SandCastle/UI/UiFrame.h"

namespace SandCastle
{
	class Ui;
	class UiBtn : public UiElem
	{
	public:
		
		UiElem::Type GetType() const override;
		void SetColor(const Color& color);
		void UpdateFrames();
	protected:
		void OnHover() override;
		void OnUnHover() override;
		void OnClickPressed() override;
		void OnClickReleased() override;
		void ShowHideFrame();
	protected:
		friend Ui;
		Sentence label;
		UiFrame frameIdle;
		UiFrame frameHover;
		UiFrame framePressed;
		Vec2f labelOffset = 0.f;
	};
}
