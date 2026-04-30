#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/UI/UiFrame.h"
#include "SandCastle/UI/UiContext.h"
#include "SandCastle/Render/Text.h"

namespace SandCastle
{
	class Ui;
	/// @brief Progress / loading bar widget made of a contour frame and a
	/// filling frame. The filling width represents `current / goal`. Optional
	/// label modes (None / Percent / ValueGoal) display the progress as text.
	class UiLoadBar : public UiElem
	{
	public:
		~UiLoadBar();
		UiElem::Type GetType() const override;
		/// @brief Update the displayed progress; resizes the filling frame and refreshes the label.
		void SetProgress(double current, double goal);
		/// @brief Tint applied to the filling frame.
		void SetFillingColor(Color color);
		/// @brief Use Math::FormatCompact (e.g. "1.5k") for the value label.
		void SetUseCompact(bool useCompact);
		/// @brief Suffix appended to the value label (e.g. " HP").
		void SetSuffix(String Suffix);
		/// @brief current / goal in [0, 1].
		double GetProgress() const;
		/// @brief Current absolute value.
		double GetCurrent() const;
		/// @brief Target absolute value.
		double GetGoal() const;

	private:
		friend Ui;
		void UpdateFilling();
		void UpdateLabel();

		LoadBarContext context;
		UiFrame frameContour;
		UiFrame frameFilling;
		Sentence label;
		String suffix="";
		double current = 0.0;
		double goal = 1.0;
		bool useCompactFormat = false;
	};
}
