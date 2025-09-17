#include "pch.h"
#include "SandCastle/UI/UiCanvas.h"

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
		//And contentSize the canvas when needed

		/*- lambda definitions -*/

		void (*WrapSize)(Vec2f childSize, float& wrapSize) = nullptr;
		bool (*WrapCondition)(Bitmask8 fixedSize, Vec2f childSize, Vec2f cursor, Vec2f size) = nullptr;
		void (*Wrapping)(float wrapSize, Vec2f & cursor, Vec2f border) = nullptr;
		void (*MoveCursor)(Vec2f childSize, Vec2f & cursor) = nullptr;

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
		auto WrapDown = [](float wrapSize, Vec2f& cursor, Vec2f border) -> void
			{
				cursor.y -= wrapSize;
				cursor.x = border.x;
			};
		auto WrapRight = [](float wrapSize, Vec2f& cursor, Vec2f border) -> void
			{
				cursor.x += wrapSize;
				cursor.y = -border.y;
			};

		//Cursor move
		auto MoveCursorDown = [](Vec2f childSize, Vec2f& cursor) -> void
			{
				cursor.y -= childSize.y;
			};
		auto MoveCursorRight = [](Vec2f childSize, Vec2f& cursor) -> void
			{
				cursor.x += childSize.x;
			};

		//Anchor elemAnchor;
		switch (layoutDir)
		{
		case LayoutDir::TopDown:
			WrapSize = HorizontalWrap;
			MoveCursor = MoveCursorDown;
			Wrapping = WrapRight;
			break;
		case LayoutDir::LeftRight:
			WrapSize = VerticalWrap;
			MoveCursor = MoveCursorRight;
			Wrapping = WrapDown;
			break;
			LOG_ERROR("Ui::Canvas::MakeLayout, Unknown Layout dir!");
			break;
		}
		struct Line
		{
			std::vector<Ui::Elem*> elems;
			//float start = 0.f;
			//float end = 0.f;
			float wrapSize = 0.f;
			float length = 0.f;
			float center = 0.f;
		};
		void (*LineMoveH)(Ui::Elem * elem, const Line & line, Vec2f contentSize, Vec2f canvas) = nullptr;
		void (*LineMoveV)(Ui::Elem * elem, const Line & line, Vec2f contentSize, Vec2f canvas) = nullptr;


		switch (layoutDir)
		{
		case LayoutDir::LeftRight:

			switch (layoutAlignH)
			{
			case LayoutAlignH::Left:
				break;
			case LayoutAlignH::Center:
				LineMoveH = [](Ui::Elem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f((canvas.x - line.length) * 0.5f, 0.f);
						elem->Move(offset);
					};

				break;
			case LayoutAlignH::Right:
				LineMoveH = [](Ui::Elem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f((canvas.x - line.length), 0.f);
						elem->Move(offset);
					};
				break;
			}

			switch (layoutAlignV)
			{
			case LayoutAlignV::Top:
				break;
			case LayoutAlignV::Center:
				LineMoveV = [](Ui::Elem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f(0.f, -(line.wrapSize - elem->size.y) * 0.5f);
						offset += Vec2f(0.f, -(canvas.y - contentSize.y) * 0.5f);
						elem->Move(offset);
					};
				break;
			case LayoutAlignV::Bot:
				LineMoveV = [](Ui::Elem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f(0.f, -(line.wrapSize - elem->size.y));
						offset += Vec2f(0.f, -(canvas.y - contentSize.y));
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
				LineMoveH = [](Ui::Elem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f((line.wrapSize - elem->size.x) * 0.5f, 0.f);
						offset += Vec2f((canvas.x - contentSize.x) * 0.5f, 0.f);
						elem->Move(offset);
					};
				break;
			case LayoutAlignH::Right:
				LineMoveH = [](Ui::Elem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f((line.wrapSize - elem->size.x), 0.f);
						offset += Vec2f((canvas.x - contentSize.x), 0.f);
						elem->Move(offset);
					};
				break;
			}

			switch (layoutAlignV)
			{
			case LayoutAlignV::Top:
				break;
			case LayoutAlignV::Center:
				LineMoveV = [](Ui::Elem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f(0.f, -(canvas.y - line.length) * 0.5f);
						elem->Move(offset);
					};
				break;
			case LayoutAlignV::Bot:
				LineMoveV = [](Ui::Elem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f(0.f, -(canvas.y - line.length));
						elem->Move(offset);
					};
				break;
			}

			break; //Top Down
		}


		std::vector<Line> wrapLines;
		wrapLines.emplace_back(Line());

		/* - Apply layout - */
		Vec2f cursor = Vec2f(border.x, -border.y);
		float wrapSize = 0.f;
		Vec2f contentSize = Vec2f(0.f, 0.f);
		float totalWrap = 0.f;

		for (int i = 0; i < children.size(); i++)
		{
			auto& child = children[i];
			auto margedSize = child->size + child->margin * 2;
			float width = margedSize.x + border.x;
			float height = margedSize.y + border.y;

			if (
				(std::abs(cursor.y) + height > sizeLimit.y && Wrapping == WrapRight)
				||
				(std::abs(cursor.x) + width > sizeLimit.x && Wrapping == WrapDown)
				)
			{
				//Finish current line.
				totalWrap += wrapSize;
				//wrapLines.back().end = totalWrap;
				//Add space between lines
				float space = Wrapping == WrapDown ? spacing.y : spacing.x;
				totalWrap += space;
				//Create new line
				wrapLines.emplace_back(Line());
				//wrapLines.back().start = totalWrap;
				//Move cursor to new line
				Wrapping(wrapSize + space, cursor, border);
				wrapSize = 0.f;
			}

			child->SetPosition(cursor);
			WrapSize(margedSize, wrapSize);

			//Line
			wrapLines.back().wrapSize = wrapSize;
			wrapLines.back().elems.emplace_back(child);
			Vec2f space = wrapLines.back().elems.size() == 1 ? 0.f : spacing; //no spacing for the first line's element
			Vec2f advance = margedSize + space;
			wrapLines.back().length += layoutDir == LayoutDir::LeftRight ? advance.x : advance.y;

			//Cursor moves
			MoveCursor(margedSize + spacing, cursor);

			//Update contentSize, based on cursor position
			Vec2f aCurs = {
				std::abs(cursor.x - border.x) ,
				std::abs(cursor.y + border.y) };

			if (Wrapping == WrapDown)
			{
				aCurs.x -= spacing.x;
				aCurs.y += wrapSize;
			}
			else
			{
				aCurs.y -= spacing.y;
				aCurs.x += wrapSize;
			}

			contentSize.x = aCurs.x > contentSize.x ? aCurs.x : contentSize.x;
			contentSize.y = aCurs.y > contentSize.y ? aCurs.y : contentSize.y;
		}

		//Stretch to fit content
		if (!fixedSize.Contains(Canvas::Horizontal))
		{
			float stretchx = contentSize.x + border.x * 2;
			size.x = stretchx < sizeLimit.x ? stretchx : sizeLimit.x;
		}
		if (!fixedSize.Contains(Canvas::Vertical))
		{
			float stretchy = contentSize.y + border.y * 2;
			size.y = stretchy < sizeLimit.y ? stretchy : sizeLimit.y;
		}


		//Apply alignement
		if (LineMoveH != nullptr || LineMoveV != nullptr)
		{
			for (int i = 0; i < wrapLines.size(); i++)
			{
				auto& elems = wrapLines[i].elems;
				for (int j = 0; j < elems.size(); j++)
				{
					if (LineMoveH != nullptr) LineMoveH(elems[j], wrapLines[i], contentSize, size - border * 2);
					if (LineMoveV != nullptr) LineMoveV(elems[j], wrapLines[i], contentSize, size - border * 2);
				}
			}
		}
		//Update world position cause it may have changed with stretching
		SetPosition(position);
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