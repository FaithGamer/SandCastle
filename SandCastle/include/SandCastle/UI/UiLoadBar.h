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
		void SetProgress(double current, double goal);
		void SetFillingColor(Color color);
		double GetProgress() const;
		double GetCurrent() const;
		double GetGoal() const;

	private:
		friend Ui;
		void UpdateFilling();
		void UpdateLabel();

		LoadBarContext context;
		UiFrame frameContour;
		UiFrame frameFilling;
		Sentence label;
		double current = 0.0;
		double goal = 1.0;
	};
}
