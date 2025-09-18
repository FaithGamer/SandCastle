#pragma once

#include "SandCastle/UI/Ui.h"

namespace SandCastle
{
	/// @brief What is the point of origin for the movement of a canvas.
	enum class Ui::LayoutDir : int
	{
		TopDown,
		//DownTop,
		LeftRight
		//RightLeft
	};

	enum class Ui::LayoutWrap : int
	{
		Normal
		//Inverse
	};

	enum class Ui::LayoutAlignH : int
	{
		Left,
		Center,
		Right
	};

	enum class Ui::LayoutAlignV : int
	{
		Top,
		Center,
		Bot
	};

	enum class Ui::TexBorder : int
	{
		Top,
		Left,
		Mid,
		Right,
		Bot
	};

	enum class Ui::SpriteCorner : int
	{
		TopLeft,
		TopRight,
		BotLeft,
		BotRight
	};
}
