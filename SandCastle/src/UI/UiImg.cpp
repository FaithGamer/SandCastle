#include "pch.h"
#include "SandCastle/UI/UiImg.h"
#include "SandCastle/Render/Sprite.h"

namespace SandCastle
{
	UiElem::Type UiImg::GetType() const
	{
		return UiElem::Type::Image;
	}
}