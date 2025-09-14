#include "pch.h"
#include "SandCastle/Render/UiImg.h"
#include "SandCastle/Render/Sprite.h"

namespace SandCastle
{
	Ui::Elem::Type Ui::Img::GetType() const
	{
		return Ui::Elem::Type::Image;
	}

	void Ui::Img::SetPosition(Vec2f pos)
	{
		//Making sure that no matter what is the sprite origin
		//The origin point is always top left.
		auto dim = sprite->GetDimensions();
		Vec2f offset = {
			((float)sprite->orgX + 0.5f) * dim.x,
			((float)sprite->orgY - 0.5f) * dim.y
		};
		pos += offset;
		Ui::Elem::SetPosition(pos);

	}
}