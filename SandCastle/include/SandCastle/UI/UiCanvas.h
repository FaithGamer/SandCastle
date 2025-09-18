#pragma once

#include "SandCastle/UI/Ui.h"
#include "SandCastle/UI/UiElem.h"
#include "SandCastle/UI/UiEnum.h"
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
		Ui::Elem::Type GetType() const override;
		void SetPosition(Vec2f pos) override;

		void MakeLayout();
		void SetAnchor(Anchor anchor);
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
		Anchor anchor;
	};
}