#pragma once

#include "SandCastle/UI/Ui.h"
#include "SandCastle/UI/UiElem.h"
#include "SandCastle/UI/UiEnum.h"
#include "SandCastle/Core/Bitmask.h"


namespace SandCastle
{
	class Sprite;
	class Ui::Img : public Ui::Elem
	{

	public:
		Ui::Elem::Type GetType() const override;
	//	void SetPosition(Vec2f pos) override;

	private:
		friend Ui;
		Sprite* sprite;
		Entity entt;
	};

}