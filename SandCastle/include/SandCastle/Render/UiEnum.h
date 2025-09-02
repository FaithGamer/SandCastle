#pragma once

#include "SandCastle/Render/Ui.h"

namespace SandCastle
{

	enum class Ui::Anchor
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

	enum class Ui::Layout : int
	{
		TopDown,
		DownTop,
		LeftRight,
		RightLeft
	};

	enum class Ui::LayoutDir : int
	{
		Normal,
		Inverse
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
