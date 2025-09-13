#include "pch.h"
#include "SandCastle/Render/UiTxt.h"

namespace SandCastle
{
	Ui::Elem::Type Ui::Txt::GetType() const
	{
		return Ui::Elem::Type::Text;
	}
	void Ui::Txt::SetPosition(Vec2f pos)
	{
		pos.x += padding.x;
		pos.y -= padding.y;
		sentence.root.gtr()->SetPosition(pos.x, pos.y, z);
	}
}