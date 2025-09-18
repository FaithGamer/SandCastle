#include "pch.h"
#include "SandCastle/UI/UiTxt.h"

namespace SandCastle
{
	Ui::Txt::Txt() : Ui::Elem(sentence.root)
	{
		LOG_INFO("txt constructor");
	}
	Ui::Elem::Type Ui::Txt::GetType() const
	{
		return Ui::Elem::Type::Text;
	}
}