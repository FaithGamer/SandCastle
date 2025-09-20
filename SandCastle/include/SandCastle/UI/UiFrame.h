#pragma once

namespace SandCastle
{
	class Sprite;
	class Texture;

	class UiFrame
	{
	public:
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

		struct Template
		{
			//3x3 stretchable sprites.
			//Used for buttons or frames.
			bool fixedStep = false;
			std::vector<Sprite*> cornerSpr;
			std::vector<Texture*> repeatTex;
		};
	};
}