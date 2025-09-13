#include "pch.h"
#include "SandCastle/Render/UiCanvas.h"

namespace SandCastle
{
	Ui::Elem::Type Ui::Canvas::GetType() const
	{
		return Ui::Elem::Type::Canvas;
	}
	void Ui::Canvas::SetPosition(Vec2f pos)
	{
	}
	void Ui::Canvas::MakeLayout()
	{
		//Set the position of every children
		//And stretch the canvas when needed
		Vec2f cursor = Vec2f(0, 0);
		float wrapSize = 0.f;
		Vec2f stretch = 0.f;
		for (int i = 0; i < children.size(); i++)
		{
			//First I'm gonna pretend it's only TopDown, Wrap normal, Left Align

			auto& child = children[i];
			if (fixedSize.Contains(Canvas::Vertical) && (std::abs(cursor.y) + child->size.y) > size.y)
			{
				//It's time to wrap!
				cursor.y = 0;
				cursor.x += wrapSize;
				wrapSize = 0;
			}
	
			child->SetPosition(cursor);
			wrapSize = child->size.x > wrapSize ? child->size.x : wrapSize;
			cursor.y -= child->size.y;

			Vec2f cAbs = Vec2f(std::abs(cursor.x) + wrapSize, std::abs(cursor.y));
			//Update stretching
			stretch.x = cAbs.x > stretch.x ? cAbs.x : stretch.x;
			stretch.y = cAbs.y > stretch.y ? cAbs.y : stretch.y;
		}

		//Apply stretching
		if (!fixedSize.Contains(Canvas::Vertical) && size.y < stretch.y)
		{
			size.y = stretch.y;
		}
		if (!fixedSize.Contains(Canvas::Horizontal) && size.x < stretch.x)
		{
			size.x = stretch.x;
		}

	}
}