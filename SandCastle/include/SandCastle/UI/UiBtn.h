#pragma once

#include "SandCastle/UI/UiElem.h"


namespace SandCastle
{
	class Ui::Btn : public Ui::Elem
	{
	public:

	public:
		Ui::Elem::Type GetType() const override;

	public:
		Sentence sentence;

	};
}
