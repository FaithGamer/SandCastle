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
			Button,
			Image
		};

	public:
		Elem();
		virtual Type GetType() const = 0;
		virtual void SetPosition(Vec2f pos);
		virtual void Move(Vec2f offset);
		
	protected:
		friend Canvas;
		friend Ui;
		Ui::Canvas* parent = nullptr;
		Ui::ElemID id = 0;
		Entity root;
		float z = 0.f;
		Vec2f size = Vec2f(0, 0);
		Vec2f padding = Vec2f(0, 0);
		Vec2f position = Vec2f(0, 0);
		Ui::Anchor anchor;
	};
}
