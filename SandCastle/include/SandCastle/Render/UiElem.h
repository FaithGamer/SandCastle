#pragma once

#include "SandCastle/Render/Ui.h"

namespace SandCastle
{
	class Ui::Elem
	{
	public:
		enum class Type
		{
			Canvas,
			Text,
			Button
		};

	public:
		virtual Type GetType() const = 0;
		virtual void SetPosition(Vec2f pos)
		{
			pos.x += padding.x;
			pos.y -= padding.y;
			root.gtr()->SetPosition(pos.x, pos.y, z);
		}
	private:
		friend Ui;
		Ui::Canvas* parent;
		Ui::ElemID id = 0;
		Entity root;
		float z = 0.f;
		Vec2f size = Vec2f(0, 0);
		Vec2f padding = Vec2f(0, 0);
		Vec2f position = Vec2f(0, 0);

	};
}
