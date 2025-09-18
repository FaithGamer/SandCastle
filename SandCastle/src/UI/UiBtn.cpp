#include "pch.h"
#include "SandCastle/UI/UiBtn.h"
#include "SandCastle/Render/SpriteRender.h"

namespace SandCastle
{
	void Alpha(Entity& entity, unsigned char alpha) 
	{
		auto children = entity.GetComponent<Children>();
		for (auto& child : children->children)
		{
			Entity(child).GetComponent<SpriteRender>()->color.a = alpha;
		}
	};
	Ui::Elem::Type Ui::Btn::GetType() const
	{
		return Ui::Elem::Type::Button;
	}
	void Ui::Btn::OnHover()
	{
		Alpha(frameIdle, 0);
		Alpha(framePressed, 0);
		Alpha(frameHover, 255);
	}
	void Ui::Btn::OnUnHover()
	{
		Alpha(frameIdle, 255);
		Alpha(framePressed, 0);
		Alpha(frameHover, 0);
	}
	void Ui::Btn::OnClickPressed()
	{
		Alpha(frameIdle, 0);
		Alpha(framePressed, 255);
		Alpha(frameHover, 0);
		if (!signalOnRelease)
			signal.Send(this);
	}
	void Ui::Btn::OnClickReleased()
	{
		Alpha(frameIdle, 0);
		Alpha(framePressed, 0);
		Alpha(frameHover, 255);
		if (signalOnRelease)
			signal.Send(this);
		LOG_INFO("OnClickReleased");
	}
}