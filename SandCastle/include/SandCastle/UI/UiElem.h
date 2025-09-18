#pragma once

#include "SandCastle/UI/Ui.h"
#include "SandCastle/Render/Rect.h"
#include "SandCastle/Core/Signal.h"
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
		Elem(Entity root);
		~Elem();
		virtual Type GetType() const = 0;
		virtual Vec2f GetSize() const;
		virtual void SetPosition(Vec2f pos);
		virtual void Move(Vec2f offset);
		virtual bool IsHovered();
		virtual void OnHover() {};
		virtual void OnUnHover() {};
		virtual void OnClickPressed() {};
		virtual void OnClickReleased() {};
		
	protected:
		friend Canvas;
		friend Ui;
		Ui::Canvas* parent = nullptr;
		Ui::ElemID id = 0;
		Entity root;
		float z = 0.f;
		Vec2f size = Vec2f(0, 0);
		Vec2f margin = Vec2f(0, 0);
		Vec2f position = Vec2f(0, 0);

		Rect hitbox;
		bool hoverable = false;
		bool clickable = false;

		Signal<Elem*> hoverSignal;
		Signal<Elem*> unhoverSignal;
		Signal<Elem*> clickPressSignal;
		Signal<Elem*> clickUnpressSignal;
	};
}
