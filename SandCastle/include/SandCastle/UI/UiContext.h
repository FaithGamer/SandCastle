#pragma once

#include "SandCastle/UI/UiEnum.h"
#include "SandCastle/UI/UiFrame.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Render/Text.h"
#include "SandCastle/Audio/Sound.h"

namespace SandCastle
{	
	struct Animation;
	/// @brief Styling for UiBtn: font, label colors, padding, the four frame templates, and optional press/release sounds.
	struct ButtonContext
	{
		String fontName;
		Color textColor;
		Color textColorDisabled;
		Vec2f padding = Vec2f(0.f, 0.f);
		UiFrame::Template* frameIdle = nullptr;
		UiFrame::Template* frameHover = nullptr;
		UiFrame::Template* framePressed = nullptr;
		UiFrame::Template* frameDisabled = nullptr;
		Sound* pressSound = nullptr;
		Sound* releaseSound = nullptr;
	};

	/// @brief Styling for UiAnimBtn: one Animation per state.
	struct AnimButtonContext
	{
		Animation* idle = nullptr;
		Animation* hover = nullptr;
		Animation* pressed = nullptr;
		Animation* disabled = nullptr;
	};

	/// @brief Styling for UiCheckbox: the 3-sprite texture used for unchecked/hovered/checked.
	struct CheckboxContext
	{
		String texture = "";
	};

	/// @brief Styling and layout rules for UiCanvas: spacing, padding, child alignment and direction, optional background frame.
	struct CanvasContext
	{
		Vec2f spacing = 0.f;
		Vec2f padding = 0.f;
		LayoutAlign layoutAlignH = LayoutAlign::Begin;
		LayoutAlign layoutAlignV = LayoutAlign::Begin;
		LayoutDir layoutDir = LayoutDir::TopDown;
		UiFrame::Template* frame = nullptr;
	};

	/// @brief Styling for UiTxt: font, color, and horizontal alignment.
	struct TextContext
	{
		String fontName;
		Color color;
		TextAlign align = TextAlign::Left;
	};

	/// @brief How a UiLoadBar formats its label: hide it, show "NN%", or show "current/goal".
	enum class LoadBarTextMode
	{
		None,
		Percent,
		ValueGoal
	};

	/// @brief Styling for UiLoadBar: contour and filling frames, fill margin, fill color, label mode/font/color.
	struct LoadBarContext
	{
		UiFrame::Template* frameContour = nullptr;
		UiFrame::Template* frameFilling = nullptr;
		Vec2f fillingMargin = Vec2f(0.f, 0.f);
		Color fillingColor = Color(255, 255, 255, 255);
		LoadBarTextMode textMode = LoadBarTextMode::None;
		String fontName;
		Color textColor;
	};

	/// @brief Top-level styling state used by Ui at element-creation time.
	/// Set via Ui::Set* helpers; snapshot/restore via SnapshotContext/Context.
	struct UiContext
	{
		CanvasContext canvas;
		CheckboxContext checkbox;
		ButtonContext button;
		AnimButtonContext animButton;
		TextContext text;
		LoadBarContext loadBar;
		Vec2f margin = Vec2f(0.f, 0.f);
		Vec2f rootMargin = Vec2f(0.f, 0.f);
		LayerID layer = 0;
		Material* material = nullptr;
		CanvasAnchor rootAnchor = CanvasAnchor::MiddleCenter;
		float z = 0.f;
		int interactionGroup = 0;
	};
}