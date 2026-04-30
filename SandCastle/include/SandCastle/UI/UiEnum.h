#pragma once

namespace SandCastle
{
	/// @brief Where a root UiCanvas is anchored on the screen (3x3 grid).
	enum class CanvasAnchor
	{
		TopLeft,
		TopCenter,
		TopRight,
		MiddleLeft,
		MiddleCenter,
		MiddleRight,
		BotLeft,
		BotCenter,
		BotRight
	};
	/// @brief Direction in which a UiCanvas stacks its children.
	enum class LayoutDir : int
	{
		TopDown,
		LeftRight
	};

	/// @brief Cross-axis alignment of a UiCanvas's children.
	enum class LayoutAlign : int
	{
		Begin,
		Middle,
		End
	};
}