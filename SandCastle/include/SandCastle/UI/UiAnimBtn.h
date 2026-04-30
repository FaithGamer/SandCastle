#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/UI/UiContext.h"
#include "SandCastle/UI/UiBtn.h"

namespace SandCastle
{
	class Ui;
	/// @brief Variant of UiBtn whose four states (idle/hover/pressed/disabled)
	/// are sprite animations rather than 9-slice frames. Configure via
	/// Ui::SetAnimBtnIdle / Hover / Pressed / Disabled in a context.
	class UiAnimBtn : public UiBtn
	{
	public:
		~UiAnimBtn();
		UiElem::Type GetType() const override;
		virtual void SetPosition(Vec2f pos) override;
		virtual void ComputeHitbox() override;
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
