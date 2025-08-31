#pragma once
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Render/Writer.h"
namespace SandCastle
{
	class Ui : public Singleton<Ui>
	{
	public:
		
		typedef uint32_t FrameID;
		enum class Anchor
		{
			TopLeft,
			TopCenter,
			TopRight,
			MiddleLeft,
			MiddleCenter,
			MiddleRight,
			BotLeft,
			BotCenter,
			BotRight
		};

		typedef enum : int
		{
			Top,
			Left,
			Mid,
			Right,
			Bot
		}TexBorder;

		typedef enum : int
		{
			TopLeft,
			TopRight,
			BotLeft,
			BotRight

		}SpriteCorner;

	public:
		Ui();
		~Ui();
		Entity MakeFrameSprites(FrameID id, Vec2f size, String texture, bool fixedStep);
		static Writer* GetWriter();
	private:
		void MakeBorderTex(String texture);

		struct RepeatTextures
		{
			std::vector<Texture*> tex;
		};
		struct BorderSprite
		{
			BorderSprite(Texture* tex, Rect rect, Vec2f worldDim);
			Sprite sprite;
			Vec2f wDim;
		};
		struct Frame
		{
			std::vector<BorderSprite> borderSprites;
			Entity root;
			Vec2f size;
		};
		std::unordered_map<String, RepeatTextures> m_bordersTex;
		std::unordered_map<FrameID, Frame> m_frames;

		Writer* m_writer;
	};
}