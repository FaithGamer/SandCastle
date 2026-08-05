#pragma once


#include "SandCastle/Render/Rect.h"
#include "SandCastle/Core/Signal.h"
#include "SandCastle/ECS/Entity.h"

namespace SandCastle
{
	class UiCanvas;
	class Ui;
	/// @brief Cardinal direction for gamepad/keyboard UI navigation.
	/// Pass to UiElem::AddNav and the per-direction Listen helpers.
	enum class NavDir : uint8_t
	{
		Left = 0,
		Right,
		Up,
		Down,
		Count
	};
	/// @brief Base class for every UI widget (Canvas, Text, Button, AnimButton, Image, Checkbox, LoadBar).
	/// Provides position/size, parent/child links, hover/click hit-testing, and Disable/Enable.
	/// Subscribe to ListenHover / ListenClickPressed / ListenClickReleased to react to user input.
	/// For gamepad navigation: AddNav wires per-direction targets, Navigate moves
	/// the Ui selector here, and the ListenNav* / ListenSelect* / ListenCancel* helpers
	/// fire when the matching Ui::Navigate* / Ui::Select / Ui::Cancel handler runs.
	class UiElem
	{
	public:
		typedef uint32_t ID;
		/// @brief Interaction state of a widget.
		enum class State
		{
			Idle,
			Hovered,
			Pressed
		};
		/// @brief Concrete widget kind, returned by GetType().
		enum class Type
		{
			Canvas,
			Text,
			Button,
			AnimButton,
			Image,
			Checkbox,
			LoadBar
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

		/// @brief Synthesize a press on this element from code, without the mouse or
		/// the gamepad selector being on it (e.g. a dedicated gamepad shortcut bound
		/// to a button that isn't the navigated one). Shows the pressed visual and
		/// fires the ListenClickPressed listeners. No-op if disabled or already held.
		void Press();
		/// @brief Release a code-driven Press: restores the resting visual and fires
		/// the ListenClickReleased listeners. No-op unless a Press is in flight; a
		/// press cancelled by a Disable() in between only clears the held state.
		void Release();

		/// @brief Set the element to navigate to when `dir` is pressed while this one is selected.
		/// Pass nullptr to clear the binding. Bindings are one-directional.
		void AddNav(NavDir dir, UiElem* target);
		/// @brief Get the element registered for direction `dir`, or nullptr if unset.
		UiElem* GetNav(NavDir dir) const;
		/// @brief Make this element the currently-navigated one (moves the Ui selector here).
		/// No-op if no gamepad selector texture has been set via Ui::SetGamepadSelector.
		void Navigate();

		template<typename T>
		void ListenNavPressed(NavDir dir, void(T::* listener)(UiElem* signal), T* obj)
		{
			navPressSignals[(int)dir].Listen(listener, obj);
		}
		template<typename T>
		void ListenNavReleased(NavDir dir, void(T::* listener)(UiElem* signal), T* obj)
		{
			navReleaseSignals[(int)dir].Listen(listener, obj);
		}
		template<typename T>
		void ListenSelectPressed(void(T::* listener)(UiElem* signal), T* obj)
		{
			selectPressSignal.Listen(listener, obj);
		}
		template<typename T>
		void ListenSelectReleased(void(T::* listener)(UiElem* signal), T* obj)
		{
			selectReleaseSignal.Listen(listener, obj);
		}
		template<typename T>
		void ListenCancelPressed(void(T::* listener)(UiElem* signal), T* obj)
		{
			cancelPressSignal.Listen(listener, obj);
		}
		template<typename T>
		void ListenCancelReleased(void(T::* listener)(UiElem* signal), T* obj)
		{
			cancelReleaseSignal.Listen(listener, obj);
		}

		Signal<UiElem*> destroySignal;
		/// @brief Multiplier on the gamepad selector ring's rectangle when this element
		/// is the navigated one. 1 = hug the laid-out size (the default, and what every
		/// element wants). Raise it when the element's ARTWORK is drawn bigger than its
		/// layout box — a card whose sprite transform is scaled up on hover, say — since
		/// the ring is sized from the layout and would otherwise end up underneath it.
		/// Scales about the element's centre, so the ring stays concentric.
		float selectorScale = 1.f;
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
		void NavPressed(NavDir dir);
		void NavReleased(NavDir dir);
		/// @brief Cancel an in-progress press (visual reset only, no released callback).
		/// Used when the gamepad selector navigates away from a held-down element.
		void ResetPress();
		void SelectPressed();
		void SelectReleased();
		void CancelPressed();
		void CancelReleased();

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

		UiElem* navTargets[(int)NavDir::Count] = { nullptr, nullptr, nullptr, nullptr };
		Signal<UiElem*> navPressSignals[(int)NavDir::Count];
		Signal<UiElem*> navReleaseSignals[(int)NavDir::Count];
		Signal<UiElem*> selectPressSignal;
		Signal<UiElem*> selectReleaseSignal;
		Signal<UiElem*> cancelPressSignal;
		Signal<UiElem*> cancelReleaseSignal;
	};
}
