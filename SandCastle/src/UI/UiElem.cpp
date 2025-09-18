#include "pch.h"
#include "SandCastle/UI/UiElem.h"
#include "SandCastle/UI/UiEnum.h"
#include "SandCastle/Render/Transform.h"

namespace SandCastle
{
	Ui::Elem::Elem()
	{
		root = Entity::Create();
		root.AddComponent<Transform>();
		LOG_INFO("elem default constructor");
	}

	Ui::Elem::Elem(Entity Root) : root(Root)
	{
		LOG_INFO("elem root constructor");
	}

	Ui::Elem::~Elem()
	{
		root.Destroy();
	}

	void Ui::Elem::SetPosition(Vec2f pos)
	{
		position = pos;

		//All elem anchor are top left (except canvas)
		Vec2f wPos = {
		pos.x + margin.x,
		pos.y - margin.y
		};
		root.gtr()->SetPosition(wPos.x, wPos.y, z);
	}
	Vec2f Ui::Elem::GetSize() const
	{
		return size;
	}
	void Ui::Elem::Move(Vec2f offset)
	{
		SetPosition(position + offset);
	}
	bool Ui::Elem::IsHovered()
	{
		return hitbox.PointInside(Ui::MousePos());
	}
}