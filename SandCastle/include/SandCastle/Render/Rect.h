#pragma once

#include "SandCastle/Core/Vec.h"

namespace SandCastle
{
	class Rect
	{
	public:
		Rect() : left(0), top(0), width(0), height(0)
		{}
		Rect(float Left, float Top, float Width, float Height)
			: left(Left), top(Top), width(Width), height(Height)
		{}
		bool PointInside(Vec2f point) const;
		bool PointInside(float x, float y) const;

	public:

		float left;
		float top;
		float width;
		float height;
	};
}
