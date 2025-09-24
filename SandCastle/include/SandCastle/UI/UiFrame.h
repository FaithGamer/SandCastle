#pragma once
#include "SandCastle/Render/Layer.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Render/Rect.h"
#include "SandCastle/Render/Color.h"
#include "SandCastle/ECS/Entity.h"


namespace SandCastle
{
	class Sprite;
	class Texture;
	class Material;
	class Ui;
	class UiElem;

	class UiFrame
	{
	public:

		struct BorderSprite
		{
			BorderSprite() {}
			BorderSprite(Texture* tex, Rect rect, Vec2f worldDim);
			Sprite sprite;
			Vec2f wDim;
		};

		struct BorderSprites
		{
			BorderSprites()
			{
				sprites.reserve(5);
			}
			std::vector<BorderSprite> sprites;
		};

		struct Template
		{
			//3x3 stretchable sprites.
			//Used for buttons or frames.
			bool fixedStep = false;
			std::vector<Sprite*> cornerSpr;
			std::vector<Texture*> repeatTex;
		};

	public:
		UiFrame();
		~UiFrame();
		UiFrame(UiFrame::Template* templ, Material* material, LayerID layer);
		enum class TexBorder : int
		{
			Top,
			Left,
			Mid,
			Right,
			Bot
		};

		enum class SpriteCorner : int
		{
			TopLeft,
			TopRight,
			BotLeft,
			BotRight
		};

		static void MakeTemplate(Template& templ, const std::string& texture, bool fixedStep);
		void SetColor(Color color);
		void SetAlpha(unsigned int alpha);
		Vec2f Update(UiElem* elem, float z);
	private:
		friend Ui;
		Entity root;
		Vec2f size;
		UiFrame::Template* templ = nullptr;
		Material* material = nullptr;
		LayerID layer = 0;
	};
}