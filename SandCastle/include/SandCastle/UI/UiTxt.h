#pragma once

#include "SandCastle/UI/UiElem.h"


namespace SandCastle
{
	class Ui::Txt : public Ui::Elem
	{
	public:
		Ui::Txt();
		Ui::Elem::Type GetType() const override;
	protected:
		friend Ui;
		Sentence sentence;
		FontID font;
		Color color;
		TextAlign align;

	};
}
