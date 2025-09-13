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

	void Ui::Canvas::OffsetRange(int begin, int end, Vec2f offset)
	{
		for (int i = begin; i < end; i++)
		{
			if (children.size() <= i)
			{
				LOG_ERROR("Canvas::OffsetRange, out of range.");
				return;
			}
			auto& child = children[i];
			child->root.gtr()->Move(offset);
		}
	}

	void Ui::Canvas::MakeLayout()
	{
		//Set the position of every children according to layout
		//And stretch the canvas when needed

		Vec2f cursor = Vec2f(0.f, 0.f);
		float wrapSize = 0.f;
		Vec2f stretch = Vec2f(0.f, 0.f);
		//for down-top/right-left direction, we will need to offset
		//every children in the opposite direction to stay consistent with the frame.
		Vec2f offset = Vec2f(0.f, 0.f);

		/* lambda definitions */

		void (*WrapSize)(Vec2f childSize, float& wrapSize) = nullptr;
		bool (*WrapCondition)(Bitmask8 fixedSize, Vec2f childSize, Vec2f cursor, Vec2f size) = nullptr;
		void (*Wrapping)(float wrapSize, Vec2f & cursor, Vec2f & offset) = nullptr;
		void (*MoveCursor)(Vec2f childSize, Vec2f & cursor, Vec2f & offset) = nullptr;

		//WrapSize
		auto HorizontalWrap = [](Vec2f childSize, float& wrapSize) -> void
			{
				wrapSize = childSize.x > wrapSize ? childSize.x : wrapSize;
			};
		auto VerticalWrap = [](Vec2f childSize, float& wrapSize) -> void
			{
				wrapSize = childSize.y > wrapSize ? childSize.y : wrapSize;
			};

		//Wrapping
		auto WrapDown = [](float wrapSize, Vec2f& cursor, Vec2f& offset) -> void
			{
				cursor.y -= wrapSize;
				cursor.x = 0;
			};
		auto WrapUp = [](float wrapSize, Vec2f& cursor, Vec2f& offset) -> void
			{
				cursor.y += wrapSize;
				offset.y -= wrapSize;
				cursor.x = 0;
			};
		auto WrapLeft = [](float wrapSize, Vec2f& cursor, Vec2f& offset) -> void
			{
				cursor.x -= wrapSize;
				offset.x += wrapSize;
				cursor.y = 0;
			};
		auto WrapRight = [](float wrapSize, Vec2f& cursor, Vec2f& offset) -> void
			{
				cursor.x += wrapSize;
				cursor.y = 0;
			};

		//Cursor move
		auto MoveCursorDown = [](Vec2f childSize, Vec2f& cursor, Vec2f& offset) -> void
			{
				cursor.y -= childSize.y;
			};
		auto MoveCursorUp = [](Vec2f childSize, Vec2f& cursor, Vec2f& offset) -> void
			{
				cursor.y += childSize.y;
				offset.y -= childSize.y;
			};
		auto MoveCursorLeft = [](Vec2f childSize, Vec2f& cursor, Vec2f& offset) -> void
			{
				cursor.x -= childSize.x;
				offset.x += childSize.x;
			};
		auto MoveCursorRight = [](Vec2f childSize, Vec2f& cursor, Vec2f& offset) -> void
			{
				cursor.x += childSize.x;
			};

		//Use the right lambda depending on the layout
		switch (layoutDir)
		{
		case LayoutDir::TopDown:
			WrapSize = HorizontalWrap;
			MoveCursor = MoveCursorDown;
			Wrapping = layoutWrap == LayoutWrap::Normal ? WrapRight : WrapLeft;
			break;
		case LayoutDir::LeftRight:
			WrapSize = VerticalWrap;
			MoveCursor = MoveCursorRight;
			Wrapping = layoutWrap == LayoutWrap::Normal ? WrapDown : WrapUp;
			break;
		case LayoutDir::DownTop:
			WrapSize = HorizontalWrap;
			MoveCursor = MoveCursorUp;
			Wrapping = layoutWrap == LayoutWrap::Normal ? WrapRight : WrapLeft;
			break;
		case LayoutDir::RightLeft:
			WrapSize = VerticalWrap;
			MoveCursor = MoveCursorLeft;
			Wrapping = layoutWrap == LayoutWrap::Normal ? WrapDown : WrapUp;
			break;
		default:
			LOG_ERROR("Ui::Canvas::MakeLayout, Unknown Layout dir!");
			break;
		}

		for (int i = 0; i < children.size(); i++)
		{
			auto& child = children[i];
			if ((fixedSize.Contains(Canvas::Vertical) && (std::abs(cursor.y) + child->size.y) > size.y)
				|| fixedSize.Contains(Canvas::Horizontal) && (std::abs(cursor.x) + child->size.x) > size.x)
			{
				//It's time to wrap!
				Wrapping(wrapSize, cursor, offset);
				wrapSize = 0;
			}

			child->SetPosition(cursor);
			WrapSize(child->size, wrapSize);

			//Cursor moves
			MoveCursor(child->size, cursor, offset);

			Vec2f cAbs = Vec2f(0, 0);

			if (Wrapping == WrapUp || Wrapping == WrapDown)
				cAbs = Vec2f(std::abs(cursor.x), std::abs(cursor.y) + wrapSize);
			else
				cAbs = Vec2f(std::abs(cursor.x) + wrapSize, std::abs(cursor.y));

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

		//Apply offset
		if (offset.Magnitude() > 0.f)
		{
			OffsetRange(0, children.size(), offset);
		}
	}
}