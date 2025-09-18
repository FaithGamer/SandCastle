#include "pch.h"
#include "SandCastle/UI/UiTxt.h"

namespace SandCastle
{
	Ui::Txt::Txt() : Ui::Elem(sentence.root)
	{
	}
	Ui::Elem::Type Ui::Txt::GetType() const
	{
		return Ui::Elem::Type::Text;
	}
	void Ui::Txt::Update(std::string_view utf8)
	{

	}
}