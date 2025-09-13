#pragma once

#include "SandCastle/Render/Ui.h"

namespace SandCastle
{
	/// @brief What is the point of origin for the movement of a canvas.
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

	/// @brief In which direction UiElem are going
	/// 
	/// o = element.
	/// | = canvas borders.
	/// 
	/// LayoutDir::TopDown + LayoutAlign::Left:
	/// 
	/// |o  |  element1 
	/// |o	|  element2
	/// |o	|  element3
	/// |	|
	/// 
	/// LayoutDir::RightLeft + LayoutAlign::Left:
	/// 
	/// |oo | element2, element1
	/// |	| 
	/// |	| 
	/// |	|
	/// 
	/// LayoutDir::RightLeft + LayoutAlign::Right:
	/// 
	/// | oo| element2, element1
	/// |	| 
	/// |	| 
	/// |	|
	/// 
	enum class Ui::LayoutDir : int
	{
		TopDown,
		DownTop,
		LeftRight,
		RightLeft
	};

	/// @brief When UiElem position reach the bound of the canvas
	/// where we put it.
	/// 
	/// o = element.
	/// | = canvas borders.
	/// 
	/// LayoutWrap::Normal + LayoutDir::TopDown:
	/// 
	/// |ooo| o<- the element4 has no space and would end up outside the canvas (LayoutDir: LeftRight)
	/// |o	|<- Wrapping put it here 
	/// |	|
	/// |	|
	/// 
	/// LayoutWrap::Inverse + LayoutDir::TopDown:
	/// 
	/// |o  |<- element4
	/// |ooo|
	/// |	|
	/// |	|
	/// 
	/// LayoutWrap::Normal + LayoutDir::DownTop
	/// 
	/// |   |
	/// |	|
	/// |o	|<- element4
	/// |ooo|
	/// 
	/// 
	/// LayoutWrap::Inverse + LayoutDir::DownTop
	/// 
	/// |   |
	/// |	|
	/// |ooo|
	/// |o  |<- element4
	/// 

	enum class Ui::LayoutWrap : int
	{
		Normal,
		Inverse
	};

	enum class Ui::LayoutAlign : int
	{
		Left,
		Center,
		Right
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
