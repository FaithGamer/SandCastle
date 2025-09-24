#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/UI/UiFrame.h"
#include "SandCastle/Core/Bitmask.h"

namespace SandCastle
{
	class Ui;
	class UiCanvas : public UiElem
	{
	public:
		/// @brief What is the point of origin for the movement of a canvas.
		enum class LayoutDir : int
		{
			TopDown,
			//DownTop,
			LeftRight
			//RightLeft
		};

		enum class LayoutAlignH : int
		{
			Left,
			Center,
			Right
		};

		enum class LayoutAlignV : int
		{
			Top,
			Center,
			Bot
		};
		typedef enum : uint8_t
		{
			Horizontal = 1,
			Vertical = 2
		}SizeLimit;
		enum class Anchor
		{
			TopLeft,
			TopCenter,
			TopRight,
			MiddleLeft,
			MiddleCenter,
			MiddleRight,
			BotLeft,
			BotCenter,
			BotRight
		};
	public:
		~UiCanvas();
		/// @brief Will Update layout and parent layout on the next Ui::Update
		void MustUpdate();
		UiElem::Type GetType() const override;
		Vec2f GetPosition() const override;
		void SetPosition(Vec2f pos) override;
		void AddElem(UiElem* elem);
		void RemoveElem(UiElem* elem);
		void SetAnchor(Anchor anchor);
		/// @brief Spacing between element inside the canvas
		void SetSpacing(Vec2f spacing);
		void SetBorder(Vec2f border);
		void SetMargin(Vec2f margin);
		
	public:
		LayoutDir layoutDir = LayoutDir::TopDown;
		LayoutAlignH layoutAlignH = LayoutAlignH::Left;
		LayoutAlignV layoutAlignV = LayoutAlignV::Top;

		Signal<UiCanvas*> mustUpdateSignal;
	private:
		friend Ui;
		bool destroyed = false;
		void OnChildMustUpdate(UiCanvas* child);
		void OnDestroy(UiElem* elem);
		void UpdateLayout();
		Vec2f AnchorOffset() const;
	private:
		std::optional<UiFrame> frame;
		Bitmask8 fixedSize = 0;
		std::unordered_map<UiElem::ID, UiElem*> children;
		Vec2f spacing = Vec2f(0.f, 0.f);
		Vec2f border = Vec2f(0.f, 0.f);
		Vec2f sizeLimit = Vec2f(9999999.f, 9999999.f);
		Anchor anchor;
		bool mustUpdate = false;
	};
}