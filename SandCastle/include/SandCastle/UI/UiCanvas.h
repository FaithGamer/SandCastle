#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/UI/UiFrame.h"
#include "SandCastle/UI/UiEnum.h"
#include "SandCastle/Core/Bitmask.h"
#include "SandCastle/UI/UiContext.h"

namespace SandCastle
{
	class Ui;
	/// @brief Layout container holding child UiElems. Children are arranged
	/// according to the canvas's CanvasContext (padding/spacing/direction/alignment).
	/// Root canvases (no parent) honor the rootAnchor; nested canvases anchor top-left.
	class UiCanvas : public UiElem
	{
	public:
		/// @brief What is the point of origin for the movement of a canvas.
		typedef enum : uint8_t
		{
			Horizontal = 1,
			Vertical = 2
		}SizeLimit;
		
	public:
		~UiCanvas();
		/// @brief Will Update layout and parent layout on the next Ui::Update
		void MustUpdate();
		UiElem::Type GetType() const override;
		Vec2f GetPosition() const override;
		void SetPosition(Vec2f pos) override;
		inline CanvasAnchor GetAnchor() const
		{
			return anchor;
		}
		Vec2f GetAnchoredPosition() const;
		void AddElem(UiElem* elem);
		void RemoveElem(UiElem* elem);
		/// @brief Spacing between element inside the canvas
		void SetSpacing(Vec2f spacing);
		void SetMargin(Vec2f margin);
		
	public:
		Signal<UiCanvas*> mustUpdateSignal;
	private:
		friend Ui;
		void OnChildMustUpdate(UiCanvas* child);
		void OnDestroy(UiElem* elem);
		void UpdateLayout();
		Vec2f AnchorOffset() const;
	private:
		bool hasDestroyed = false;
		CanvasContext context;
		std::optional<UiFrame> frame;
		Bitmask8 fixedSize = 0;
		std::unordered_map<UiElem::ID, UiElem*> children;
		Vec2f sizeLimit = Vec2f(9999999.f, 9999999.f);
		CanvasAnchor anchor;
		bool mustUpdate = false;
	};
}