#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/UI/UiFrame.h"
#include "SandCastle/Core/Bitmask.h"

namespace SandCastle
{
	class Ui;
	class UiCanvas : public UiElem
	{
	public:
		/// @brief What is the point of origin for the movement of a canvas.
		enum class LayoutDir : int
		{
			TopDown,
			//DownTop,
			LeftRight
			//RightLeft
		};

		enum class LayoutAlignH : int
		{
			Left,
			Center,
			Right
		};

		enum class LayoutAlignV : int
		{
			Top,
			Center,
			Bot
		};
		typedef enum : uint8_t
		{
			Horizontal = 1,
			Vertical = 2
		}SizeLimit;
		enum class Anchor
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
	public:
		~UiCanvas();
		UiElem::Type GetType() const override;
		void SetPosition(Vec2f pos) override;
		void AddElem(UiElem* elem);

		void SetAnchor(Anchor anchor);
		/// @brief Spacing between element inside the canvas
		void SetSpacing(Vec2f spacing);
		void SetBorder(Vec2f border);
		void SetMargin(Vec2f margin);
	public:
		LayoutDir layoutDir = LayoutDir::TopDown;
		LayoutAlignH layoutAlignH = LayoutAlignH::Left;
		LayoutAlignV layoutAlignV = LayoutAlignV::Top;
	private:
		friend Ui;
		void UpdateLayout();
	private:
		std::optional<UiFrame> frame;
		Bitmask8 fixedSize = 0;
		std::list<UiElem*> children;
		Vec2f spacing = Vec2f(0.f, 0.f);
		Vec2f border = Vec2f(0.f, 0.f);
		Vec2f sizeLimit = Vec2f(9999999.f, 9999999.f);
		Anchor anchor;
	};
}