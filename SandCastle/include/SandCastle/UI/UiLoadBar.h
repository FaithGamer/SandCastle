#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/UI/UiFrame.h"
#include "SandCastle/UI/UiContext.h"
#include "SandCastle/Render/Text.h"

namespace SandCastle
{
	class Ui;
	class UiLoadBar : public UiElem
	{
	public:
		~UiLoadBar();
		UiElem::Type GetType() const override;
		void SetProgress(float current, float goal);
		float GetProgress() const;
		float GetCurrent() const;
		float GetGoal() const;

	private:
		friend Ui;
		void UpdateFilling();
		void UpdateLabel();

		LoadBarContext context;
		UiFrame frameContour;
		UiFrame frameFilling;
		Sentence label;
		float current = 0.f;
		float goal = 1.f;
	};
}
