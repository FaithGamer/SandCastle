#pragma once

#include "SandCastle/Render/Ui.h"
#include "SandCastle/Render/UiElem.h"
#include "SandCastle/Render/UiEnum.h"
#include "SandCastle/Core/Bitmask.h"


namespace SandCastle
{
	class Sprite;
	class Ui::Img : public Ui::Elem
	{

	public:
		Ui::Elem::Type GetType() const override;
		void SetPosition(Vec2f pos) override;

	private:
		friend Ui;
		Sprite* sprite;
	};

}