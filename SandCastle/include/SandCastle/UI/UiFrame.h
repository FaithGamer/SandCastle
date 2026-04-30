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

	/// @brief 9-slice resizable frame used to draw canvas backgrounds, button
	/// backgrounds, load-bar frames, etc. A Template holds the 4 corner sprites
	/// plus the repeating border textures; a UiFrame instances that template at
	/// a specific size and updates the geometry when the size changes.
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

		/// @brief Reusable 9-slice description: 4 corner sprites + repeating border textures. Build with MakeTemplate.
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

		/// @brief Build a Template from a 3x3 spritesheet on disk. fixedStep clamps growth to multiples of the border textures.
		static void MakeTemplate(Template& templ, const std::string& texture, bool fixedStep);
		/// @brief Tint every slice with `color`.
		void SetColor(Color color);
		/// @brief Set the alpha channel only.
		void SetAlpha(unsigned int alpha);
		/// @brief Recompute the frame geometry to fit `elem` at depth z.
		Vec2f Update(UiElem* elem, float z);
	public:
		Entity root;
	private:
		friend Ui;
		Vec2f size;
		UiFrame::Template* templ = nullptr;
		Material* material = nullptr;
		LayerID layer = 0;
	};
}