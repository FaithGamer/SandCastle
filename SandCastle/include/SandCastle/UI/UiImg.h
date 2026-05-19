#pragma once

#include "SandCastle/UI/Ui.h"
#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Core/Bitmask.h"
#include "SandCastle/Render/Color.h"


namespace SandCastle
{
	class Sprite;
	/// @brief UI widget displaying a single Sprite. Size auto-derives from the
	/// sprite's world dimensions; SetSprite swaps the visual at runtime.
	class UiImg : public UiElem
	{

	public:
		UiElem::Type GetType() const override;
		void SetPosition(Vec2f pos) override;
		/// @brief Replace the displayed sprite.
		void SetSprite(Sprite* sprite);
		/// @brief Tint the displayed sprite.
		void SetColor(const Color& color);

	private:
		friend Ui;
		Sprite* sprite = nullptr;
	};

}