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
		void SetChecked(bool checked);
		Signal<bool> checkSignal;
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
	};
}