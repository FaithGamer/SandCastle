#include "pch.h"
#include "SandCastle/UI/UiImg.h"
#include "SandCastle/UI/UiCanvas.h"
#include "SandCastle/Render/Sprite.h"

namespace SandCastle
{
	UiElem::Type UiImg::GetType() const
	{
		return UiElem::Type::Image;
	}
	void UiImg::SetPosition(Vec2f pos)
	{
		position = pos;
		//Offset sprite to make top left anchor no matter the sprite origin
		auto tr = root.GetComponent<Transform>();
		auto rd = root.GetComponent<SpriteRender>();
		auto spr = rd->GetSprite();
		auto dim = spr->GetDimensions();
		Vec2f offset = {
			((float)spr->orgX + 0.5f) * dim.x,
			((float)spr->orgY - 0.5f) * dim.y
		};

		pos += offset;
		

		if (parent != nullptr)
		{
			pos += parent->GetPosition();
		}

		pos = {
		std::round(pos.x + margin.x),
		std::round(pos.y - margin.y)
		};
		root.gtr()->SetPosition(pos.x, pos.y, z);
	}
}