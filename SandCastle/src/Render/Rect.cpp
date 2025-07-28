#include "pch.h"
#include "SandCastle/Render/Rect.h"

namespace SandCastle
{
	bool Rect::PointInside(Vec2f point) const
	{
		if (point.x < left
			|| point.x > left + width
			|| point.y > top
			|| point.y < top - height)
			return false;
		return true;
	}

	bool Rect::PointInside(float x, float y) const
	{
		return PointInside({ x, y });
	}
}