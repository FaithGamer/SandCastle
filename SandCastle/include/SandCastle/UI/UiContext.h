#pragma once

#include "SandCastle/UI/UiEnum.h"
#include "SandCastle/UI/UiFrame.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Render/Text.h"
#include "SandCastle/Audio/Sound.h"

namespace SandCastle
{	
	struct ButtonContext
	{
		String fontName;
		Color textColor;
		Vec2f padding = Vec2f(0.f, 0.f);
		UiFrame::Template* frameIdle = nullptr;
		UiFrame::Template* frameHover = nullptr;
		UiFrame::Template* framePressed = nullptr;
		Sound* pressSound = nullptr;
		Sound* releaseSound = nullptr;
	};

	struct CheckboxContext
	{
		String texture = "";
	};

	struct CanvasContext
	{
		Vec2f spacing = 0.f;
		Vec2f padding = 0.f;
		LayoutAlign layoutAlignH = LayoutAlign::Begin;
		LayoutAlign layoutAlignV = LayoutAlign::Begin;
		LayoutDir layoutDir = LayoutDir::TopDown;
		UiFrame::Template* frame = nullptr;
	};

	struct TextContext
	{
		String fontName;
		Color color;
		TextAlign align = TextAlign::Left;
	};

	struct UiContext
	{
		CanvasContext canvas;
		CheckboxContext checkbox;
		ButtonContext button;
		TextContext text;
		Vec2f margin = Vec2f(0.f, 0.f);
		Vec2f rootMargin = Vec2f(0.f, 0.f);
		LayerID layer = 0;
		Material* material = nullptr;
		CanvasAnchor rootAnchor = CanvasAnchor::MiddleCenter;
		float z = 0.f;
		int interactionGroup = 0;
	};
}