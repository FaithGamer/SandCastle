#pragma once

#include "SandCastle/Core/Vec.h"

namespace SandCastle
{
	/// @brief Axis-aligned rectangle with Y+ up convention.
	/// `top` is the rectangle's larger Y, `Bottom()` is the smaller Y.
	/// Used for sprite atlas regions, hitboxes, etc.
	class Rect
	{
	public:
		Rect() : left(0), top(0), width(0), height(0)
		{}
		Rect(float Left, float Top, float Width, float Height)
			: left(Left), top(Top), width(Width), height(Height)
		{}
		/// @brief True when the point is within the rectangle bounds.
		bool PointInside(Vec2f point) const;
		bool PointInside(float x, float y) const;
		/// @brief Right edge X coordinate.
		inline float Right() const { return left + width; }
		/// @brief Bottom edge Y coordinate (smaller Y, since the engine uses Y+ up).
		inline float Bottom() const { return top - height; } // Y+ up => bottom is smaller Y

		/// @brief True if `inner` is fully contained inside `outer` (Y+ up convention).
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
