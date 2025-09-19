#include "pch.h"
#include "SandCastle/UI/UiTxt.h"

namespace SandCastle
{
	UiTxt::UiTxt() : UiElem(sentence.root)
	{
	}
	UiElem::Type UiTxt::GetType() const
	{
		return UiElem::Type::Text;
	}

}