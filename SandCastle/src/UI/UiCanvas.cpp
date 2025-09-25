#include "pch.h"
#include "SandCastle/UI/UiCanvas.h"
#include "SandCastle/UI/UiFrame.h"
#include "SandCastle/Render/Transform.h"

namespace SandCastle
{
	UiCanvas::~UiCanvas()
	{
		destroyed = true;
		for (auto& child : children)
		{
			delete child.second;
		}
	}

	void UiCanvas::MustUpdate()
	{
		if (!mustUpdate)
		{
			mustUpdate = true;
			mustUpdateSignal.Send(this);
		}
	}

	UiElem::Type UiCanvas::GetType() const
	{
		return UiElem::Type::Canvas;
	}
	Vec2f UiCanvas::AnchorOffset() const
	{
		Vec2f offset(0, 0);
		switch (anchor)
		{
		case CanvasAnchor::TopLeft:
			offset.x += margin.x;
			offset.y -= margin.y;
			break;
		case CanvasAnchor::TopCenter:
			offset.x -= size.x * 0.5f;
			offset.y -= margin.y;
			break;
		case CanvasAnchor::TopRight:
			offset.x -= size.x + margin.x;
			offset.y -= margin.y;
			break;
		case CanvasAnchor::BotLeft:
			offset.x += margin.x;
			offset.y += size.y + margin.y;
			break;
		case CanvasAnchor::BotCenter:
			offset.x -= size.x * 0.5f;
			offset.y += size.y + margin.y;
			break;
		case CanvasAnchor::BotRight:
			offset.x -= size.x + margin.x;
			offset.y += size.y + margin.y;
			break;
		case CanvasAnchor::MiddleCenter:
			offset.x -= size.x * 0.5f;
			offset.y += size.y * 0.5f;
			break;
		case CanvasAnchor::MiddleRight:
			offset.x -= size.x + margin.x;
			offset.y += size.y * 0.5f;
			break;
		case CanvasAnchor::MiddleLeft:
			offset.x += margin.x;
			offset.y += size.y * 0.5f;
			break;
		default:
			break;
		}
		return offset;
	}

	Vec2f UiCanvas::GetPosition() const
	{
		auto pos = position + AnchorOffset();
		if (parent != nullptr)
			return parent->GetPosition() + pos;
		return pos;
	}

	void UiCanvas::SetPosition(Vec2f pos)
	{
		position = pos;
		if (parent != nullptr)
			pos += parent->GetPosition();
		pos += AnchorOffset();
		pos = Vec2f(std::round(pos.x), std::round(pos.y));
		root.gtr()->SetPosition(pos.x, pos.y, z);
		//update children hitboxes:
		for (auto& child_kvp : children)
		{
			auto child = child_kvp.second;
			child->SetPosition(child->position);
			child->ComputeHitbox();
		}
		ComputeHitbox();
	}

	void UiCanvas::AddElem(UiElem* elem)
	{
		//root.AddChild(elem->root);
		children.insert(std::make_pair(elem->id, elem));
		elem->destroySignal.Listen(&UiCanvas::OnDestroy, this, SignalPriority::low);
		if (elem->GetType() == UiElem::Type::Canvas)
		{
			static_cast<UiCanvas*>(elem)->mustUpdateSignal.Listen(&UiCanvas::OnChildMustUpdate, this);
		}
	}

	void UiCanvas::RemoveElem(UiElem* elem)
	{

	}

	void UiCanvas::OnChildMustUpdate(UiCanvas* child)
	{
		MustUpdate();
	}

	void UiCanvas::OnDestroy(UiElem* elem)
	{
		//When a children gets destroyed
		if (destroyed)
			return;
		children.erase(elem->id);
		MustUpdate();
	}

	void UiCanvas::UpdateLayout()
	{
		//Set the position of every children according to layout
		//And contentSize the canvas when needed

		/*- lambda definitions -*/

		void (*WrapSize)(Vec2f childSize, float& wrapSize) = nullptr;
		bool (*WrapCondition)(Bitmask8 fixedSize, Vec2f childSize, Vec2f cursor, Vec2f size) = nullptr;
		void (*Wrapping)(float wrapSize, Vec2f & cursor, Vec2f padding) = nullptr;
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
		auto WrapDown = [](float wrapSize, Vec2f& cursor, Vec2f padding) -> void
			{
				cursor.y -= wrapSize;
				cursor.x = padding.x;
			};
		auto WrapRight = [](float wrapSize, Vec2f& cursor, Vec2f padding) -> void
			{
				cursor.x += wrapSize;
				cursor.y = -padding.y;
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
		switch (context.layoutDir)
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
			LOG_ERROR("UiCanvas::MakeLayout, Unknown Layout dir!");
			break;
		}
		struct Line
		{
			std::vector<UiElem*> elems;
			//float start = 0.f;
			//float end = 0.f;
			float wrapSize = 0.f;
			float length = 0.f;
			float center = 0.f;
		};
		void (*LineMoveH)(UiElem * elem, const Line & line, Vec2f contentSize, Vec2f canvas) = nullptr;
		void (*LineMoveV)(UiElem * elem, const Line & line, Vec2f contentSize, Vec2f canvas) = nullptr;


		switch (context.layoutDir)
		{
		case LayoutDir::LeftRight:

			switch (context.layoutAlignH)
			{
			case LayoutAlign::Begin:
				break;
			case LayoutAlign::Middle:
				LineMoveH = [](UiElem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f((canvas.x - line.length) * 0.5f, 0.f);
						offset.x = std::round(offset.x);
						elem->Move(offset);
					};

				break;
			case LayoutAlign::End:
				LineMoveH = [](UiElem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f((canvas.x - line.length), 0.f);
						offset.x = std::round(offset.x);
						elem->Move(offset);
					};
				break;
			}

			switch (context.layoutAlignV)
			{
			case LayoutAlign::Begin:
				break;
			case LayoutAlign::Middle:
				LineMoveV = [](UiElem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f(0.f, -(line.wrapSize - elem->size.y) * 0.5f);
						offset += Vec2f(0.f, -(canvas.y - contentSize.y) * 0.5f);
						offset.y = std::round(offset.y);
						elem->Move(offset);
					};
				break;
			case LayoutAlign::End:
				LineMoveV = [](UiElem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f(0.f, -(line.wrapSize - elem->size.y));
						offset += Vec2f(0.f, -(canvas.y - contentSize.y));
						offset.y = std::round(offset.y);
						elem->Move(offset);
					};
				break;
			}
			break; //LeftRight

		case LayoutDir::TopDown:

			switch (context.layoutAlignH)
			{
			case LayoutAlign::Begin:
				break;
			case LayoutAlign::Middle:
				LineMoveH = [](UiElem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f((line.wrapSize - elem->size.x) * 0.5f, 0.f);
						offset += Vec2f((canvas.x - contentSize.x) * 0.5f, 0.f);
						offset.x = std::round(offset.x);
						elem->Move(offset);
					};
				break;
			case LayoutAlign::End:
				LineMoveH = [](UiElem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f((line.wrapSize - elem->size.x), 0.f);
						offset += Vec2f((canvas.x - contentSize.x), 0.f);
						offset.x = std::round(offset.x);
						elem->Move(offset);
					};
				break;
			}

			switch (context.layoutAlignV)
			{
			case LayoutAlign::Begin:
				break;
			case LayoutAlign::Middle:
				LineMoveV = [](UiElem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f(0.f, -(canvas.y - line.length) * 0.5f);
						offset.y = std::round(offset.y);
						elem->Move(offset);
					};
				break;
			case LayoutAlign::End:
				LineMoveV = [](UiElem* elem, const Line& line, Vec2f contentSize, Vec2f canvas)
					{
						auto offset = Vec2f(0.f, -(canvas.y - line.length));
						offset.y = std::round(offset.y);
						elem->Move(offset);
					};
				break;
			}

			break; //Top Down
		}

		std::vector<Line> wrapLines;
		wrapLines.emplace_back(Line());

		/* - Apply layout - */
		Vec2f cursor = Vec2f(context.padding.x, -context.padding.y);
		float wrapSize = 0.f;
		Vec2f contentSize = Vec2f(0.f, 0.f);
		float totalWrap = 0.f;

		for (auto& child_kvp : children)
		{
			auto child = child_kvp.second;
			auto margedSize = child->size + child->margin * 2;
			float width = margedSize.x + context.padding.x;
			float height = margedSize.y + context.padding.y;

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
				float space = Wrapping == WrapDown ? context.spacing.y : context.spacing.x;
				totalWrap += space;
				//Create new line
				wrapLines.emplace_back(Line());
				//wrapLines.back().start = totalWrap;
				//Move cursor to new line
				Wrapping(wrapSize + space, cursor, context.padding);
				wrapSize = 0.f;
			}

			child->SetPosition(cursor);
			WrapSize(margedSize, wrapSize);

			//Line
			wrapLines.back().wrapSize = wrapSize;
			wrapLines.back().elems.emplace_back(child);
			Vec2f space = wrapLines.back().elems.size() == 1 ? 0.f : context.spacing; //no spacing for the first line's element
			Vec2f advance = margedSize + space;
			wrapLines.back().length += context.layoutDir == LayoutDir::LeftRight ? advance.x : advance.y;

			//Cursor moves
			MoveCursor(margedSize + context.spacing, cursor);

			//Update contentSize, based on cursor position
			Vec2f aCurs = {
				std::abs(cursor.x - context.padding.x) ,
				std::abs(cursor.y + context.padding.y) };

			if (Wrapping == WrapDown)
			{
				aCurs.x -= context.spacing.x;
				aCurs.y += wrapSize;
			}
			else
			{
				aCurs.y -= context.spacing.y;
				aCurs.x += wrapSize;
			}

			contentSize.x = aCurs.x > contentSize.x ? aCurs.x : contentSize.x;
			contentSize.y = aCurs.y > contentSize.y ? aCurs.y : contentSize.y;
		}

		//Stretch to fit content
		if (!fixedSize.Contains(UiCanvas::Horizontal))
		{
			float stretchx = contentSize.x + context.padding.x * 2;
			size.x = stretchx < sizeLimit.x ? stretchx : sizeLimit.x;
		}
		if (!fixedSize.Contains(UiCanvas::Vertical))
		{
			float stretchy = contentSize.y + context.padding.y * 2;
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
					if (LineMoveH != nullptr) LineMoveH(elems[j], wrapLines[i], contentSize, size - context.padding * 2);
					if (LineMoveV != nullptr) LineMoveV(elems[j], wrapLines[i], contentSize, size - context.padding * 2);
				}
			}
		}
		mustUpdate = false;
		//Update world position cause it may have changed with stretching
		SetPosition(position);
		if (frame)
		{
			auto frameSize = frame->Update(this, 1.f);
			if (std::abs(frameSize.x - size.x) > 0.5f
				|| std::abs(frameSize.y - size.y) > 0.5f)
			{
				size = frameSize;
			}
		}
	}
	void UiCanvas::SetSpacing(Vec2f Spacing)
	{
		context.spacing = Spacing;
		MustUpdate();
	}
	void UiCanvas::SetMargin(Vec2f Margin)
	{
		margin = Margin;
		SetPosition(position);
	}
}