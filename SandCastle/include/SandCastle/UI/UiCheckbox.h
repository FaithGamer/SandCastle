#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Core/Signal.h"

namespace SandCastle
{
	class Ui;
	/// @brief Three-sprite checkbox (unchecked / hovered / checked).
	/// If a bool* is bound at creation, the engine reads it back every frame
	/// so external code can drive the visual state. checkSignal fires whenever
	/// the checked state flips.
	class UiCheckbox : public UiElem
	{
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
		void Show(Entity& entt, bool show);
		
	protected:
		friend Ui;
		std::vector<Entity> sprites;
		bool checked = false;
		bool* value = nullptr;
	};
}