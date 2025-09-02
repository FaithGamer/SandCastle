#pragma once

#include "SandCastle/Render/UiElem.h"


namespace SandCastle
{
	class Ui::Txt : public Ui::Elem
	{
	public:

	public:
		virtual Ui::Elem::Type GetType() const;

	public:
		Sentence sentence;

	};
}
