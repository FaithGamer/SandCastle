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
		virtual void SetPosition(Vec2f pos);
		virtual void Move(Vec2f offset);
		Vec2f GetSize() const;
		bool IsInside(Vec2f uiPos);
		void ComputeHitbox();

		template<typename T>
		void ListenHover(void(T::* listener)(Elem* signal), T* obj)
		{
			hoverSignal.Listen(listener, obj);
			Ui::RegisterHoverable(this);
		}
		template<typename T>
		void ListenUnhover(void(T::* listener)(Elem* signal), T* obj)
		{
			unhoverSignal.Listen(listener, obj);
			Ui::RegisterHoverable(this);
		}
		template<typename T>
		void ListenClickPressed(void(T::* listener)(Elem* signal), T* obj)
		{
			clickPressSignal.Listen(listener, obj);
			clickable = true;
			Ui::RegisterHoverable(this);
		}
		template<typename T>
		void ListenClickReleased(void(T::* listener)(Elem* signal), T* obj)
		{
			clickReleasedSignal.Listen(listener, obj);
			clickable = true;
			Ui::RegisterHoverable(this);
		}
	protected:
		virtual void OnHover() {};
		virtual void OnUnHover() {};
		virtual void OnClickPressed() {};
		virtual void OnClickReleased() {};

	private:
		friend Canvas;
		friend Ui;
		void Hover();
		void UnHover();
		void ClickPressed();
		void ClickReleased();

	protected:
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
		bool pressed = false;

		Signal<Elem*> hoverSignal;
		Signal<Elem*> unhoverSignal;
		Signal<Elem*> clickPressSignal;
		Signal<Elem*> clickReleasedSignal;
	};
}
