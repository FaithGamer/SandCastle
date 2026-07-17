#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Core/Signal.h"

namespace SandCastle
{
	class Ui;
	/// @brief Three-sprite checkbox (unchecked / hovered / checked).
	/// A bound bool* is read ONCE at creation (to set the initial visual) and
	/// written on every click; it is NOT polled each frame, so external changes
	/// to the bool do not update the visual — call SetChecked() for those.
	/// checkSignal fires whenever the checked state flips.
	class UiCheckbox : public UiElem
	{
		struct Shown
		{
			Entity entt;
			bool shown = true;
		};
	public:
		UiElem::Type GetType() const override;
		/// @brief True if currently checked.
		inline bool IsChecked() const
		{
			return checked;
		}
		void SetChecked(bool checked);
		/// @brief Broadcast every time the checked state flips.
		Signal<UiCheckbox*> checkSignal;
	protected:
		typedef enum
		{
			Unchecked,
			Hover,
			Checked
		};
		void OnHover() override;
		void OnUnHover() override;
		void OnClickPressed() override;
		void OnClickReleased() override;
		void UpdateVisual();
		void Show(Shown& entt, bool show);
		
	protected:
		friend Ui;
		std::vector<Shown> sprites;
		bool checked = false;
		bool* value = nullptr;
	};
}