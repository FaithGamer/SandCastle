#pragma once

#include "SandCastle/Render/Ui.h"
#include "SandCastle/Render/UiElem.h"
#include "SandCastle/Render/UiEnum.h"
#include "SandCastle/Core/Bitmask.h"

namespace SandCastle
{
	class Ui::Canvas : public Ui::Elem
	{
	public:
		typedef enum : uint8_t
		{
			Horizontal = 1,
			Vertical = 2
		}SizeLimit;
	public:
		Ui::Elem::Type GetType() const override;
		void MakeLayout();
		void SetAnchor(Ui::Anchor anchor);
		/// @brief Spacing between element inside the canvas
		void SetSpacing(Vec2f spacing);
		void SetBorder(Vec2f border);
		void SetMargin(Vec2f margin);
	public:
		Ui::LayoutDir layoutDir = Ui::LayoutDir::TopDown;
		Ui::LayoutWrap layoutWrap = Ui::LayoutWrap::Normal;
		Ui::LayoutAlignH layoutAlignH = Ui::LayoutAlignH::Left;
		Ui::LayoutAlignV layoutAlignV = Ui::LayoutAlignV::Top;
	private:
		friend Ui;
		void OffsetRange(int begin, int end, Vec2f offset);
	private:
		bool hasFrame = false;
		Entity frame;
		Bitmask8 fixedSize = 0;
		std::vector<Ui::Elem*> children;
		Vec2f spacing = Vec2f(0.f, 0.f);
		Vec2f border = Vec2f(0.f, 0.f);
		Vec2f sizeLimit = Vec2f(9999999.f, 9999999.f);
	};
}