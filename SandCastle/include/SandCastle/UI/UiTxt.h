#pragma once

#include "SandCastle/UI/UiElem.h"


namespace SandCastle
{
	class Ui::Txt : public Ui::Elem
	{
	public:
		Ui::Txt();
		Ui::Elem::Type GetType() const override;

	public:
		Sentence sentence;

	};
}
