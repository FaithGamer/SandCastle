#pragma once


#include "SandCastle/Render/Rect.h"
#include "SandCastle/Core/Signal.h"
#include "SandCastle/ECS/Entity.h"

namespace SandCastle
{
	class UiCanvas;
	class Ui;
	class UiElem
	{
	public:
		typedef uint32_t ID;
		enum class State
		{
			Idle,
			Hovered,
			Pressed
		};
		enum class Type
		{
			Canvas,
			Text,
			Button,
			AnimButton,
			Image,
			Checkbox
		};
		struct Component
		{
			UiElem* t;
		};

	public:
		virtual ~UiElem();
		virtual Type GetType() const = 0;
		virtual void SetPosition(Vec2f pos);
		virtual void Move(Vec2f offset);
		virtual void ComputeHitbox();
		void Disable();
		void Enable();
		virtual Vec2f GetPosition() const;
		Vec2f GetSize() const;
		Vec2f GetLocalPosition() const;
		State GetState() const;
		UiElem::ID GetID() const;
		UiCanvas* GetParent() const;
		int GetParentCount() const;
		bool IsInside(Vec2f uiPos);
		void SetAbsolutePos(bool absolute);
		void SetData(Entity entt);
		void SetZOffset(float offset);
		/// @brief Does it send callbacks when hovered if disabled?
		/// @param yesorno false by default
		void SetHoverableWhenDisabled(bool yesorno);
		Entity GetData();
		bool IsDestroyed() const;

		template<typename T>
		void ListenHover(void(T::* listener)(UiElem* signal), T* obj)
		{
			hoverSignal.Listen(listener, obj);
			Ui::RegisterHoverable(this);
		}
		template<typename T>
		void ListenUnhover(void(T::* listener)(UiElem* signal), T* obj)
		{
			unhoverSignal.Listen(listener, obj);
			Ui::RegisterHoverable(this);
		}
		template<typename T>
		void ListenClickPressed(void(T::* listener)(UiElem* signal), T* obj)
		{
			clickPressSignal.Listen(listener, obj);
			clickable = true;
			Ui::RegisterHoverable(this);
		}
		template<typename T>
		void ListenClickReleased(void(T::* listener)(UiElem* signal), T* obj)
		{
			clickReleasedSignal.Listen(listener, obj);
			clickable = true;
			Ui::RegisterHoverable(this);
		}
		Signal<UiElem*> destroySignal;
		/// @brief Use at your own risk
		Entity root;
	protected:
		virtual void OnHover() {};
		virtual void OnUnHover() {};
		virtual void OnClickPressed() {};
		virtual void OnClickReleased() {};
		virtual void OnDisable() {};
		virtual void OnEnable() {};

	private:
		friend UiCanvas;
		friend Ui;
		void Hover();
		void UnHover();
		void ClickPressed();
		void ClickReleased();

	protected:
		bool disabled = false;
		bool hoverableWhenDisabled = false;
		bool absolutePos = false;
		bool destroyed = false;
		int interactionGroup = 0;
		Entity data;
		int order = -1;
		State state = State::Idle;
		UiCanvas* parent = nullptr;
		UiElem::ID id = 0;
		float z = 0.f;
		float zOffset = 0.f;
		Vec2f size = Vec2f(0, 0);
		Vec2f margin = Vec2f(0, 0);
		Vec2f position = Vec2f(0, 0);

		Rect hitbox;
		bool hoverable = false;
		bool clickable = false;
		bool pressed = false;

		std::list<Entity> childrenEntities;

		Signal<UiElem*> hoverSignal;
		Signal<UiElem*> unhoverSignal;
		Signal<UiElem*> clickPressSignal;
		Signal<UiElem*> clickReleasedSignal;
	};
}
