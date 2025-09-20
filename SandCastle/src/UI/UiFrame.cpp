#include "pch.h"
#include "SandCastle/UI/UiFrame.h"
#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/Core/Math.h"
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Render/SpriteRender.h"

namespace SandCastle
{
	/* --- Helpers --- */

	void MakeBorderTex(String texture, std::vector<Texture*>& tex)
	{
		//Create the repeating textures for frame
		tex.clear();
		for (int i = 0; i < 5; i++)
		{
			String x = "_";
			String y = "_";
			auto type = (UiFrame::TexBorder)i;
			switch (type)
			{
			case UiFrame::TexBorder::Top:
				x += "1";
				y += "0";
				break;
			case UiFrame::TexBorder::Left:
				x += "0";
				y += "1";
				break;
			case UiFrame::TexBorder::Mid:
				x += "1";
				y += "1";
				break;
			case UiFrame::TexBorder::Right:
				x += "2";
				y += "1";
				break;
			case UiFrame::TexBorder::Bot:
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
			tex.emplace_back(subTexture);
		}
	}

	void BorderSize(int i, Rect& rect, Vec2f& wDim, Vec2f pxSize, Vec2f pxDim, Vec2f sDim, float ppu)
	{
		//Calculate the sprite rect and world size of a sprite border (with texture repeat)
		auto type = (UiFrame::TexBorder)i;
		if (type == UiFrame::TexBorder::Top || type == UiFrame::TexBorder::Bot)
		{
			rect.width = pxSize.x - pxDim.x * 2; //pixel length
			rect.height = pxDim.y; //pixel length
			wDim.x = rect.width * ppu; //world length
			wDim.y = sDim.y;
		}
		else if (type == UiFrame::TexBorder::Left || type == UiFrame::TexBorder::Right)
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

	/*--- Border Sprite ---*/
	UiFrame::BorderSprite::BorderSprite(Texture* tex, Rect rect, Vec2f worldDim)
		: sprite(tex, rect),
		wDim(worldDim)
	{

	}

	/*--- UiFrame ---*/


	UiFrame::UiFrame()
	{
	}
	UiFrame::UiFrame(UiFrame::Template* Templ, Material* Material, LayerID Layer)
		: templ(Templ), material(Material), layer(Layer)
	{
	}

	void UiFrame::SetColor(Color color)
	{
		auto children = root.GetComponent<Children>();
		for (auto child : children->children)
		{
			auto spr = Entity(child).GetComponent<SpriteRender>();
			spr->color.r = color.r;
			spr->color.g = color.g;
			spr->color.b = color.b;
		}
	}

	void UiFrame::SetAlpha(unsigned int alpha)
	{
		auto children = root.GetComponent<Children>();
		for (auto& child : children->children)
		{
			Entity(child).GetComponent<SpriteRender>()->color.a = alpha;
		}
	}

	void UiFrame::MakeTemplate(Template& frame, const std::string& texture, bool fixedStep)
	{
		frame.fixedStep = fixedStep;
		MakeBorderTex(texture, frame.repeatTex);

		//Load the corner sprites
		for (int i = 0; i < 4; i++)
		{
			String x = "_";
			String y = "_";
			auto type = (UiFrame::SpriteCorner)i;
			switch (type)
			{
			case UiFrame::SpriteCorner::TopLeft:
				x += "0";
				y += "0";
				break;
			case UiFrame::SpriteCorner::TopRight:
				x += "2";
				y += "0";
				break;
			case UiFrame::SpriteCorner::BotLeft:
				x += "0";
				y += "2";
				break;
			case UiFrame::SpriteCorner::BotRight:
				x += "2";
				y += "2";
				break;
			}
			String spriteName = texture + y + x;
			frame.cornerSpr.emplace_back(Assets::Get<Sprite>(spriteName));
		}
	}

	void UiFrame::Update(UiElem* elem, float z)
	{
		//Dimensions
		Vec2f sDim = templ->cornerSpr[0]->GetDimensions();
		if (root.Valid())
			root.Destroy();
		root = Entity::Create();
		//Ensure frame size is at least the size of the four corners.
		//Apply fixed step if relevant
		size = elem->GetSize();
		if (templ->fixedStep || size.x < sDim.x * 2)
			size.x = std::max(Math::CeilMultiple(size.x, sDim.x), sDim.x * 2.f);
		if (templ->fixedStep || size.y < sDim.y * 2)
			size.y = std::max(Math::CeilMultiple(size.y, sDim.y), sDim.y * 2.f);

		float ppu = templ->repeatTex[0]->GetPixelPerUnit();
		Vec2f pxDim = sDim / ppu;
		Vec2f pxSize = size / ppu;
		Vec2f hDim = sDim * 0.5f;

		//Create the sprites entities:
		auto rootTr = root.adc<Transform>();
		rootTr->SetPosition(0, 0, z);

		//Instance border sprites at the right dimensions
		auto borderSpr = root.AddComponent<BorderSprites>();
		for (int i = 0; i < 5; i++)
		{
			Vec2f wDim;
			Rect rect;
			rect.left = 0;
			rect.top = 0;
			BorderSize(i, rect, wDim, pxSize, pxDim, sDim, ppu);
			borderSpr->sprites[i] = BorderSprite(templ->repeatTex[i], rect, wDim);
		}

		//Corners
		for (int i = 0; i < 4; i++)
		{
			auto& sprite = templ->cornerSpr[i];
			auto e = Entity::Create();
			auto spr = e.adc<SpriteRender>();
			spr->SetLayer(layer);
			spr->SetMaterial(material->GetID());
			auto tr = e.adc<Transform>();
			spr->SetSprite(sprite);
			auto type = (UiFrame::SpriteCorner)i;
			switch (type)
			{
			case UiFrame::SpriteCorner::TopLeft:
				tr->SetPosition(hDim.x, -hDim.y, 0);
				break;
			case UiFrame::SpriteCorner::TopRight:
				tr->SetPosition(hDim.x + (size.x - sDim.x), -hDim.y, 0);
				break;
			case UiFrame::SpriteCorner::BotLeft:
				tr->SetPosition(hDim.x, -size.y + hDim.y, 0);
				break;
			case UiFrame::SpriteCorner::BotRight:
				tr->SetPosition(hDim.x + (size.x - sDim.x), -size.y + hDim.y, 0);
				break;
			default:
				break;
			}
			root.AddChild(e);
		}

		//Borders
		for (int i = 0; i < 5; i++)
		{
			//Top middle border
			auto& sprite = borderSpr->sprites[i].sprite;
			auto e = Entity::Create();
			auto spr = e.adc<SpriteRender>();
			spr->SetLayer(layer);
			spr->SetMaterial(material->GetID());
			auto tr = e.adc<Transform>();
			spr->SetSprite(&sprite);
			auto re = sprite.GetTextureRect();
			auto type = (UiFrame::TexBorder)i;
			switch (type)
			{
			case UiFrame::TexBorder::Top:
				tr->SetPosition(size.x * 0.5f, -hDim.y, 0);
				break;
			case UiFrame::TexBorder::Left:
				tr->SetPosition(hDim.x, -size.y * 0.5f, 0);
				break;
			case UiFrame::TexBorder::Mid:
				tr->SetPosition(size.x * 0.5f, -size.y * 0.5f, 0);
				break;
			case UiFrame::TexBorder::Right:
				tr->SetPosition(size.x - hDim.x, -size.y * 0.5f, 0);
				break;
			case UiFrame::TexBorder::Bot:
				tr->SetPosition(size.x * 0.5f, -size.y + hDim.y, 0);
				break;
			default:
				break;
			}
			root.AddChild(e);
		}
		elem->root.AddChild(root);
	}
}