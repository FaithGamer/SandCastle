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
		position = pos;
		switch (anchor)
		{
		case Ui::Anchor::TopLeft:
			pos.x += margin.x;
			pos.y -= margin.y;
			root.gtr()->SetPosition(pos.x, pos.y, z);
			break;
		case Ui::Anchor::TopRight:
			pos.x -= size.x + margin.x;
			pos.y -= margin.y;
			root.gtr()->SetPosition(pos.x, pos.y, z);
			break;
		case Ui::Anchor::BotLeft:
			pos.x += margin.x;
			pos.y += size.y + margin.y;
			root.gtr()->SetPosition(pos.x, pos.y, z);
			break;
		case Ui::Anchor::BotRight:
			pos.x -= size.x + margin.x;
			pos.y += size.y + margin.y;
			root.gtr()->SetPosition(pos.x, pos.y, z);
			break;
		}
	}
	void Ui::Elem::Move(Vec2f offset)
	{
		SetPosition(position + offset);
	}
}