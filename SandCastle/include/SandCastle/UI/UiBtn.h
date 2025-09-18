#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Core/Signal.h"

namespace SandCastle
{
	class Ui;
	class Ui::Btn : public Ui::Elem
	{
	public:

	public:
		Ui::Elem::Type GetType() const override;
		virtual void OnHover();
		virtual void OnUnHover();
		virtual void OnClickPressed();
		virtual void OnClickReleased();
		bool signalOnRelease = false;
		Signal<Ui::Elem*> signal;
	protected:
		friend Ui;
		Sentence label;
		Entity frameIdle;
		Entity frameHover;
		Entity framePressed;
	};
}
