#include "pch.h"
#include "SandCastle/UI/UiLoadBar.h"
#include "SandCastle/UI/Ui.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/Core/Math.h"
#include "SandCastle/Render/Transform.h"

namespace SandCastle
{
	UiLoadBar::~UiLoadBar()
	{
	}

	UiElem::Type UiLoadBar::GetType() const
	{
		return UiElem::Type::LoadBar;
	}

	void UiLoadBar::SetProgress(double current, double goal)
	{
		this->current = current;
		this->goal = goal;
		UpdateFilling();
		UpdateLabel();
	}

	void UiLoadBar::SetFillingColor(Color color)
	{
		context.fillingColor = color;
		if (frameFilling.root.Valid())
			frameFilling.SetColor(color);
	}

	void UiLoadBar::SetUseCompact(bool useCompact)
	{
		useCompactFormat = useCompact;
		UpdateLabel();
	}

	double UiLoadBar::GetProgress() const
	{
		if (goal <= 0.0) return 0.0;
		double ratio = current / goal;
		return ratio < 0.0 ? 0.0 : (ratio > 1.0 ? 1.0 : ratio);
	}

	double UiLoadBar::GetCurrent() const
	{
		return current;
	}

	double UiLoadBar::GetGoal() const
	{
		return goal;
	}

	void UiLoadBar::UpdateFilling()
	{
		double ratio = GetProgress();
		Vec2f margin = context.fillingMargin;
		float fillWidth = (size.x - margin.x * 2.f) * (float)ratio;
		float fillHeight = size.y - margin.y * 2.f;

		// Temporarily override size for filling frame update
		Vec2f savedSize = size;
		size = Vec2f(fillWidth, fillHeight);
		frameFilling.Update(this, -1.f);
		size = savedSize;

		// Apply filling color (Update() rebuilds all entities, color must be re-applied)
		frameFilling.SetColor(context.fillingColor);

		// Reposition filling frame to account for margin
		if (frameFilling.root.Valid())
		{
			frameFilling.root.gtr()->Move(margin.x, -margin.y, 0.f);
		}
	}

	void UiLoadBar::UpdateLabel()
	{
		if (context.textMode == LoadBarTextMode::None)
			return;

		// Destroy old label
		if (label.root.Valid())
			label.root.Destroy();

		String text;
		if (context.textMode == LoadBarTextMode::Percent)
		{
			int pct = (int)(GetProgress() * 100.0);
			text = std::to_string(pct) + "%";
		}
		else // ValueGoal
		{
			if (useCompactFormat)
			{
				text = Math::FormatCompact(current) + "/" + Math::FormatCompact(goal);
			}
			else
			{
				text = std::to_string((long long)current) + "/" + std::to_string((long long)goal);
			}
		}

		auto writer = Ui::GetWriter();
		String lang = Assets::GetLang();
		auto font = writer->GetFont(context.fontName, lang);
		label = writer->Write(
			text,
			font->id,
			context.textColor,
			font->material,
			font->layer,
			0.f,
			TextAlign::Center,
			1.f
		);

		// Center label in the bar
		float labelX = (size.x - label.size.x) * 0.5f;
		float labelY = -(size.y - label.size.y) * 0.5f;
		label.root.Get<Transform>()->Move(labelX, labelY, -2.f);
		root.AddChild(label.root);
	}
}
