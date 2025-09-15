#include "pch.h"
#include "SandCastle/Render/UiCanvas.h"

namespace SandCastle
{
	Ui::Elem::Type Ui::Canvas::GetType() const
	{
		return Ui::Elem::Type::Canvas;
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

		Vec2f cursor = Vec2f(border.x, -border.y);
		float wrapSize = 0.f;
		Vec2f stretch = Vec2f(0.f, 0.f);
		//for down-top/right-left direction, we will need to offset
		//every children in the opposite direction to stay consistent with the frame.
		Vec2f offsetTotal = Vec2f(0.f, 0.f);
		Vec2f offset = Vec2f(0.f, 0.f);

		/*- lambda definitions -*/

		void (*WrapSize)(Vec2f childSize, float& wrapSize) = nullptr;
		bool (*WrapCondition)(Bitmask8 fixedSize, Vec2f childSize, Vec2f cursor, Vec2f size) = nullptr;
		void (*Wrapping)(float wrapSize, Vec2f & cursor, Vec2f & offset, Vec2f border) = nullptr;
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
		auto WrapDown = [](float wrapSize, Vec2f& cursor, Vec2f& offset, Vec2f border) -> void
			{
				cursor.y -= wrapSize;
				cursor.x = border.x;
			};
		auto WrapRight = [](float wrapSize, Vec2f& cursor, Vec2f& offset, Vec2f border) -> void
			{
				cursor.x += wrapSize;
				cursor.y = -border.y;
			};
		/*	auto WrapUp = [](float wrapSize, Vec2f& cursor, Vec2f& offset) -> void
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
				};*/


				//Cursor move
		auto MoveCursorDown = [](Vec2f childSize, Vec2f& cursor, Vec2f& offset) -> void
			{
				cursor.y -= childSize.y;
			};
		auto MoveCursorRight = [](Vec2f childSize, Vec2f& cursor, Vec2f& offset) -> void
			{
				cursor.x += childSize.x;
			};
		/*auto MoveCursorUp = [](Vec2f childSize, Vec2f& cursor, Vec2f& offset) -> void
			{
				cursor.y += childSize.y;
				offset.y = -childSize.y;
			};
		auto MoveCursorLeft = [](Vec2f childSize, Vec2f& cursor, Vec2f& offset) -> void
			{
				cursor.x -= childSize.x;
				offset.x = childSize.x;
			};*/



			//Use the right lambda depending on the layout
		//Anchor elemAnchor;
		switch (layoutDir)
		{
		case LayoutDir::TopDown:
			WrapSize = HorizontalWrap;
			MoveCursor = MoveCursorDown;
			Wrapping = WrapRight;
			//Wrapping = layoutWrap == LayoutWrap::Normal ? WrapRight : WrapLeft;
			//Removed because too complex
			//elemAnchor = layoutWrap == LayoutWrap::Normal ? Anchor::TopLeft : Anchor::TopRight;
			break;
		case LayoutDir::LeftRight:
			WrapSize = VerticalWrap;
			MoveCursor = MoveCursorRight;
			Wrapping = WrapDown;
			//Wrapping = layoutWrap == LayoutWrap::Normal ? WrapDown : WrapUp;
			//Removed because too complex
			//elemAnchor = layoutWrap == LayoutWrap::Normal ? Anchor::TopLeft : Anchor::BotLeft;
			break;

			//Removed because too complex

			/*case LayoutDir::DownTop:
				WrapSize = HorizontalWrap;
				MoveCursor = MoveCursorUp;
				Wrapping = layoutWrap == LayoutWrap::Normal ? WrapRight : WrapLeft;
				elemAnchor = layoutWrap == LayoutWrap::Normal ? Anchor::BotLeft : Anchor::BotRight;
				break;
			case LayoutDir::RightLeft:
				WrapSize = VerticalWrap;
				MoveCursor = MoveCursorLeft;
				Wrapping = layoutWrap == LayoutWrap::Normal ? WrapDown : WrapUp;
				elemAnchor = layoutWrap == LayoutWrap::Normal ? Anchor::TopRight : Anchor::BotRight;
				break;
			default:*/
			LOG_ERROR("Ui::Canvas::MakeLayout, Unknown Layout dir!");
			break;
		}

		void (*LineMove)(Ui::Elem * elem, float lineL, Vec2f canvas) = nullptr;
		void (*AllMove)(Ui::Elem * elem, Vec2f stretch, Vec2f canvas) = nullptr;

		switch (layoutDir)
		{
		case LayoutDir::LeftRight:

			switch (layoutAlignH)
			{
			case LayoutAlignH::Left:
				break;
			case LayoutAlignH::Center:
				LineMove = [](Ui::Elem* elem, float lineL, Vec2f canvas)
					{
						auto offset = Vec2f((canvas.x - lineL) * 0.5f, 0.f);
						elem->Move(offset);
					};
				break;
			case LayoutAlignH::Right:
				LineMove = [](Ui::Elem* elem, float lineL, Vec2f canvas)
					{
						auto offset = Vec2f((canvas.x - lineL), 0.f);
						elem->Move(offset);
					};
				break;
			}

			switch (layoutAlignV)
			{
			case LayoutAlignV::Top:
				break;
			case LayoutAlignV::Center:
				AllMove = [](Ui::Elem* elem, Vec2f stretch, Vec2f canvas)
					{
						auto offset = Vec2f(0.f, -(canvas.y - stretch.y) * 0.5f);
						elem->Move(offset);
					};
				break;
			case LayoutAlignV::Bot:
				AllMove = [](Ui::Elem* elem, Vec2f stretch, Vec2f canvas)
					{
						auto offset = Vec2f(0.f, -(canvas.y - stretch.y));
						elem->Move(offset);
					};
				break;
			}
			break; //LeftRight

		case LayoutDir::TopDown:

			switch (layoutAlignH)
			{
			case LayoutAlignH::Left:
				break;
			case LayoutAlignH::Center:
				AllMove = [](Ui::Elem* elem, Vec2f stretch, Vec2f canvas)
					{
						auto offset = Vec2f((canvas.x - stretch.x) * 0.5f, 0.f);
						elem->Move(offset);
					};
				break;
			case LayoutAlignH::Right:
				AllMove = [](Ui::Elem* elem, Vec2f stretch, Vec2f canvas)
					{
						auto offset = Vec2f((canvas.x - stretch.x), 0.f);
						elem->Move(offset);
					};
				break;
			}

			switch (layoutAlignV)
			{
			case LayoutAlignV::Top:
				break;
			case LayoutAlignV::Center:
				LineMove = [](Ui::Elem* elem, float lineL, Vec2f canvas)
					{
						auto offset = Vec2f(0.f, -(canvas.y - lineL) * 0.5f);
						elem->Move(offset);
					};
				break;
			case LayoutAlignV::Bot:
				LineMove = [](Ui::Elem* elem, float lineL, Vec2f canvas)
					{
						auto offset = Vec2f(0.f, -(canvas.y - lineL));
						elem->Move(offset);
					};
				break;
			}

			break; //Top Down
		}

		struct Line
		{
			std::vector<Ui::Elem*> elems;
			float length = 0.f;
		};
		std::vector<Line> wrapLines;
		wrapLines.emplace_back(Line());

		//Position children
		for (int i = 0; i < children.size(); i++)
		{
			auto& child = children[i];
			auto margedSize = child->size + child->margin*2;
			float width = margedSize.x + spacing.x + border.x;
			float height = margedSize.y + spacing.y + border.y;
			if ((fixedSize.Contains(Canvas::Vertical) && (std::abs(cursor.y) + height) > size.y)
				|| fixedSize.Contains(Canvas::Horizontal) && (std::abs(cursor.x) + width) > size.x)
			{
				//It's time to wrap!
				float space = Wrapping == WrapDown ? spacing.y : spacing.x;
				Wrapping(wrapSize + space, cursor, offsetTotal, border);
				wrapSize = 0;
				wrapLines.emplace_back(Line());
			}
			else
			{
				offsetTotal += offset;
			}

			offset = Vec2f(0, 0);
			child->SetPosition(cursor);
			WrapSize(margedSize, wrapSize);
			wrapLines.back().elems.emplace_back(child);
			wrapLines.back().length += layoutDir == LayoutDir::LeftRight ? margedSize.x : margedSize.y;

			//Cursor moves
			MoveCursor(margedSize + spacing, cursor, offset);

			Vec2f cAbs = Vec2f(0, 0);

			if (/*Wrapping == WrapUp ||*/ Wrapping == WrapDown)
				cAbs = Vec2f(std::abs(cursor.x), std::abs(cursor.y) + wrapSize + border.y);
			else
				cAbs = Vec2f(std::abs(cursor.x) + wrapSize + border.x, std::abs(cursor.y));

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

		//Apply alignement
		if (LineMove != nullptr)
		{
			for (int i = 0; i < wrapLines.size(); i++)
			{
				auto& elems = wrapLines[i].elems;
				for (int j = 0; j < elems.size(); j++)
				{
					LineMove(elems[j], wrapLines[i].length, size);
				}
			}
		}

		if (AllMove != nullptr)
		{
			for (int i = 0; i < children.size(); i++)
			{
				AllMove(children[i], stretch, size);
			}
		}
	}
	void Ui::Canvas::SetAnchor(Ui::Anchor Anchor)
	{
		anchor = Anchor;
		SetPosition(position);
	}
	void Ui::Canvas::SetSpacing(Vec2f Spacing)
	{
		spacing = Spacing;
	}
	void Ui::Canvas::SetBorder(Vec2f Border)
	{
		border = Border;
	}
	void Ui::Canvas::SetMargin(Vec2f Margin)
	{
		margin = Margin;
	}
}