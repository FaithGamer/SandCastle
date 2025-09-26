#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Core/Signal.h"

namespace SandCastle
{
	class Ui;
	class UiCheckbox : public UiElem
	{
	public:
		UiElem::Type GetType() const override;
		inline bool IsChecked() const
		{
			return checked;
		}
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
		void SetChecked(bool checked);
		
	protected:
		friend Ui;
		std::vector<Entity> sprites;
		bool checked = false;
		bool* value = nullptr;
	};
}