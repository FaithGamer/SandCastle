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
		inline float Right() const { return left + width; }
		inline float Bottom() const { return top - height; } // Y+ up => bottom is smaller Y

		static inline bool Inside(const Rect& outer, const Rect& inner)
		{
			// X: same as usual
			if (inner.left < outer.left) return false;
			if (inner.Right() > outer.Right()) return false;

			// Y+: top is max Y, bottom is min Y
			if (inner.top > outer.top) return false;                 // can't go above outer top
			if (inner.Bottom() < outer.Bottom()) return false;         // can't go below outer bottom
			return true;
		}

	public:

		float left;
		float top;
		float width;
		float height;
	};
}
