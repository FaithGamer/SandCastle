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
		virtual void SetPosition(Vec2f pos) = 0;
	public:
		Ui::Canvas* parent;
		Ui::ElemID id = 0;
		Entity root;
		float z = 0.f;
		Vec2f size = Vec2f(0, 0);
		Vec2f padding = Vec2f(0, 0);

	};
}
