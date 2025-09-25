#pragma once

#include "SandCastle/UI/UiEnum.h"
#include "SandCastle/Core/Vec.h"

namespace SandCastle
{
	struct CanvasContext
	{
		Vec2f spacing = 0.f;
		Vec2f padding = 0.f;
		LayoutAlign layoutAlignH = LayoutAlign::Begin;
		LayoutAlign layoutAlignV = LayoutAlign::Begin;
		LayoutDir layoutDir = LayoutDir::TopDown;
		UiFrame::Template* frame = nullptr;
	};
}