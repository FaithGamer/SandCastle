#include "pch.h"
#include "SandCastle/UI/UiTxt.h"
#include "SandCastle/UI/Ui.h"
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

	void UiTxt::SetColor(const Color& color)
	{
		const Color& c = context.color;
		if (c.r == color.r && c.g == color.g && c.b == color.b && c.a == color.a)
			return;
		context.color = color;
		Ui::UpdateText(this, utf8, false); //re-render glyphs with the new color
	}

}