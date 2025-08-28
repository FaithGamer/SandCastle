#include "pch.h"
#include "SandCastle/Render/Ui.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/Core/Math.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/Render/Renderer2D.h"

namespace SandCastle
{
	Ui::BorderSprite::BorderSprite(Texture* tex, Rect rect, Vec2f worldDim)
		: sprite(tex, rect),
		wDim(worldDim)
	{

	}
	void Ui::MakeBorderTex(String texture)
	{
		//Create the repeating textures for frames
		m_bordersTex[texture].tex.clear();
		for (int i = 0; i < 5; i++)
		{
			String x = "_";
			String y = "_";
			switch (i)
			{
			case TexBorder::Top:
				x += "1";
				y += "0";
				break;
			case TexBorder::Left:
				x += "0";
				y += "1";
				break;
			case TexBorder::Mid:
				x += "1";
				y += "1";
				break;
			case TexBorder::Right:
				x += "2";
				y += "1";
				break;
			case TexBorder::Bot:
				x += "1";
				y += "2";
				break;
			default:
				break;
			}

			String spriteName = texture + y + x;
			auto sprite = Assets::Get<Sprite>(spriteName);
			auto subTexture = Renderer2D::CreateSubTexture(sprite->GetTexture(), sprite->GetTextureRect());
			subTexture->SetWrapping(TextureWrapping::Repeat);
			m_bordersTex[texture].tex.emplace_back(subTexture);
		}
	}
	void BorderSizeHelper(int i, Rect& rect, Vec2f& wDim, Vec2f pxSize, Vec2f pxDim, Vec2f sDim, float ppu)
	{
		//Calculate the sprite rect and world size of a sprite border (with texture repeat)

		if (i == Ui::TexBorder::Top || i == Ui::TexBorder::Bot)
		{
			rect.width = pxSize.x - pxDim.x * 2; //pixel length
			rect.height = pxDim.y; //pixel length
			wDim.x = rect.width * ppu; //world length
			wDim.y = sDim.y;
		}
		else if (i == Ui::TexBorder::Left || i == Ui::TexBorder::Right)
		{
			rect.width = pxDim.x; //pixel length
			rect.height = pxSize.y - pxDim.y * 2; //pixel length
			wDim.x = sDim.x;
			wDim.y = rect.height * ppu; //world length
		}
		else
		{
			//Middle
			rect.height = pxSize.y - pxDim.y * 2; //pixel length
			rect.width = pxSize.x - pxDim.x * 2; //pixel length
			wDim.x = rect.width * ppu; //world length
			wDim.y = rect.height * ppu; //world length
		}
	}
	Entity Ui::MakeFrameSprites(FrameID id, Vec2f size, String texture, bool fixedStep)
	{
		//Ensure the existence of border texture 
		if (m_bordersTex.find(texture) == m_bordersTex.end())
			MakeBorderTex(texture);

		//Load the corner sprites
		std::vector<Sprite*> cornerSpr;
		for (int i = 0; i < 4; i++)
		{
			String x = "_";
			String y = "_";
			switch (i)
			{
			case SpriteCorner::TopLeft:
				x += "0";
				y += "0";
				break;
			case SpriteCorner::TopRight:
				x += "2";
				y += "0";
				break;
			case SpriteCorner::BotLeft:
				x += "0";
				y += "2";
				break;
			case SpriteCorner::BotRight:
				x += "2";
				y += "2";
				break;
			}
			String spriteName = texture + y + x;
			cornerSpr.emplace_back(Assets::Get<Sprite>(spriteName));
		}

		Vec2f sDim = cornerSpr[0]->GetDimensions(); //sprite world dimension

		//Ensure frame size is at least the size of the four corners.
		if (fixedStep || size.x < sDim.x * 2)
			size.x = std::max(Math::NearestMultiple(size.x, sDim.x), sDim.x * 2.f);
		if (fixedStep || size.y < sDim.y * 2)
			size.y = std::max(Math::NearestMultiple(size.y, sDim.y), sDim.y * 2.f);

		Anchor anchor = Anchor::TopLeft;

		auto& frame = m_frames[id]; //Get create the frame

		auto& tex = m_bordersTex[texture].tex;
		if (tex.size() < 5)
		{
			LOG_ERROR("Border textures not created properly.");
		}

		float ppu = tex[0]->GetPixelPerUnit();
		Vec2f pxDim = sDim / ppu;
		Vec2f pxSize = size / ppu;

		if (frame.borderSprites.size() < 5 && frame.borderSprites.size() > 0)
		{
			//Useless security check.
			LOG_ERROR("Border sprite size between 1 and 3");
			frame.borderSprites.clear();
		}

		//Make border sprites
		for (int i = 0; i < 5; i++)
		{
			Vec2f wDim;
			Rect rect;
			rect.left = 0;
			rect.top = 0;

			BorderSizeHelper(i, rect, wDim, pxSize, pxDim, sDim, ppu);

			if (frame.borderSprites.size() <= i)
			{
				//Create if doesn't exists
				frame.borderSprites.emplace_back(BorderSprite(tex[i], rect, wDim));
			}
			else
			{
				//Update dimensions if exists.
				frame.borderSprites[i].sprite.SetTextureRect(rect);
				frame.borderSprites[i].wDim = wDim;
			}
		}

		//Create the sprites entities:

		frame.root = Entity::Create();
		frame.root.adc<Transform>();
		Vec2f hDim = sDim * 0.5f;
		//Corners
		for (int i = 0; i < 4; i++)
		{
			auto e = Entity::Create();
			auto spr = e.adc<SpriteRender>();
			auto tr = e.adc<Transform>();
			spr->SetSprite(cornerSpr[i]);
			switch (i)
			{
			case SpriteCorner::TopLeft:
				tr->SetPosition(hDim.x, -hDim.y, 0);
				break;
			case SpriteCorner::TopRight:
				tr->SetPosition(hDim.x + (size.x - sDim.x), -hDim.y, 0);
				break;
			case SpriteCorner::BotLeft:
				tr->SetPosition(hDim.x, -size.y + hDim.y, 0);
				break;
			case SpriteCorner::BotRight:
				tr->SetPosition(hDim.x + (size.x - sDim.x), -size.y + hDim.y, 0);
				break;
			default:
				break;
			}
			frame.root.AddChild(e);
		}
		//Borders
		for (int i = 0; i < 5; i++)
		{
			//Top middle border
			auto e = Entity::Create();
			auto spr = e.adc<SpriteRender>();
			auto tr = e.adc<Transform>();
			spr->SetSprite(&frame.borderSprites[i].sprite);
			auto re = frame.borderSprites[i].sprite.GetTextureRect();
			switch (i)
			{
			case TexBorder::Top:
				tr->SetPosition(size.x * 0.5f, -hDim.y, 0.f);
				break;
			case TexBorder::Left:
				tr->SetPosition(hDim.x, -size.y * 0.5f, 0.f);
				break;
			case TexBorder::Mid:
				tr->SetPosition(size.x * 0.5f, -size.y * 0.5f, 0.f);
				break;
			case TexBorder::Right:
				tr->SetPosition(size.x - hDim.x, -size.y * 0.5f, 0.f);
				break;
			case TexBorder::Bot:
				tr->SetPosition(size.x * 0.5f, -size.y + hDim.y, 0.f);
				break;
			default:
				break;
			}
			frame.root.AddChild(e);
		}
		return frame.root;
	}
}