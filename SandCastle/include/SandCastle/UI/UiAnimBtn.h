#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/UI/UiContext.h"
#include "SandCastle/UI/UiBtn.h"

namespace SandCastle
{
	class Ui;
	class UiAnimBtn : public UiBtn
	{
	public:
		~UiAnimBtn();
		UiElem::Type GetType() const override;

	protected:
		void OnHover() override;
		void OnUnHover() override;
		void OnClickPressed() override;
		void OnClickReleased() override;
		void OnDisable() override;
		void OnEnable() override;

	protected:
		friend Ui;
		AnimButtonContext animContext;
	private:
		void UpdateAnim();
	};
}
