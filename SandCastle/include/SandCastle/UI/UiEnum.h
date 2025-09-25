#pragma once

namespace SandCastle
{
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
	enum class LayoutDir : int
	{
		TopDown,
		LeftRight
	};

	enum class LayoutAlign : int
	{
		Begin,
		Middle,
		End
	};
}