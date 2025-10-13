#include "pch.h"
#include "SandCastle/UI/UiTxt.h"
#include "SandCastle/Core/LangSignal.h"
#include "SandCastle/Core/Assets.h"
namespace SandCastle
{

	UiTxt::~UiTxt()
	{
		if (keyLoc != "")
			Assets::Instance()->langSignal.StopListen(this);
	}

	UiElem::Type UiTxt::GetType() const
	{
		return UiElem::Type::Text;
	}
	
	void UiTxt::OnLang(LangSignal* signal)
	{
		langSignal.Send(this);
	}

	String UiTxt::GetUtf8() const
	{
		return utf8;
	}

}