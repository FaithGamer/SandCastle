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
		void SetPosition(Vec2f pos) override;
		void MakeLayout();
	public:
		bool hasFrame = false;
		Entity root;
		Entity frame;
		Bitmask8 fixedSize = 0;
		std::vector<Ui::Elem*> children;
		Vec2f cursor = Vec2f(0, 0); //remove ?
		Ui::LayoutDir layoutDir = Ui::LayoutDir::LeftRight;
		Ui::LayoutWrap layoutWrap = Ui::LayoutWrap::Normal;
		Ui::LayoutAlign layoutAlign = Ui::LayoutAlign::Left;
		Ui::Anchor anchor = Ui::Anchor::TopLeft;
	};

}