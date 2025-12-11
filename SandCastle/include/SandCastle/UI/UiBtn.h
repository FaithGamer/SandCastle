#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Core/Signal.h"
#include "SandCastle/Render/Text.h"
#include "SandCastle/Render/Color.h"
#include "SandCastle/UI/UiFrame.h"
#include "SandCastle/Core/LangSignal.h"
#include "SandCastle/UI/UiContext.h"

namespace SandCastle
{
	class Ui;
	class UiBtn : public UiElem
	{
	public:
		virtual ~UiBtn();
		UiElem::Type GetType() const override;
		void SetColor(const Color& color);
		void UpdateFrames();
	protected:
		virtual void OnHover() override;
		virtual void OnUnHover() override;
		virtual void OnClickPressed() override;
		virtual void OnClickReleased() override;
		virtual void OnDisable() override;
		virtual void OnEnable() override;
		void OnLang(LangSignal* signal);
		void ShowHideFrame();
		void ResetLabelOffset();
		void SetLabelColor(Color color);
		Signal<UiBtn*> langSignal;

	protected:
		friend Ui;
		String keyLoc = "";
		ButtonContext context;
		Sentence label;
		UiFrame frameIdle;
		UiFrame frameHover;
		UiFrame framePressed;
		UiFrame frameDisabled;
		Vec2f labelOffset = 0.f;
	};
}
