#pragma once

#include "SandCastle/UI/UiElem.h"


namespace SandCastle
{
	class Ui::Txt : public Ui::Elem
	{
	public:
		Ui::Txt();
		Ui::Elem::Type GetType() const override;
		void Update(std::string_view utf8);
	protected:
		friend Ui;
		Sentence sentence;
		Color color;

	};
}
