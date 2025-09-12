#pragma once

#include "SandCastle/Render/UiElem.h"


namespace SandCastle
{
	class Ui::Txt : public Ui::Elem
	{
	public:

	public:
		Ui::Elem::Type GetType() const override;
		void SetPosition(Vec2f pos) override;

	public:
		Sentence sentence;

	};
}
