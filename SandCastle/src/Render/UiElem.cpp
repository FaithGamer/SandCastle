#include "pch.h"
#include "SandCastle/Render/UiElem.h"
#include "SandCastle/Render/UiEnum.h"

namespace SandCastle
{
	Ui::Elem::Elem() : anchor(Ui::Anchor::TopLeft)
	{
	}

	void Ui::Elem::SetPosition(Vec2f pos)
	{
		switch (anchor)
		{
		case Ui::Anchor::TopLeft:
			pos.x += padding.x;
			pos.y -= padding.y;
			root.gtr()->SetPosition(pos.x, pos.y, z);
			break;
		case Ui::Anchor::TopRight:
			pos.x -= size.x - padding.x;
			pos.y -= padding.y;
			root.gtr()->SetPosition(pos.x, pos.y, z);
			break;
		case Ui::Anchor::BotLeft:
			pos.x += padding.x;
			pos.y += size.y - padding.y;
			root.gtr()->SetPosition(pos.x, pos.y, z);
			break;
		case Ui::Anchor::BotRight:
			pos.x -= size.x - padding.x;
			pos.y += size.y - padding.y;
			root.gtr()->SetPosition(pos.x, pos.y, z);
			break;
		}
	}
}