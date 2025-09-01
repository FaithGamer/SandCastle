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

	Ui::Ui()
	{
		auto uiLayer = Renderer2D::AddLayer("ui");
		auto uiMat = Renderer2D::CreateMaterial(Assets::Get<Shader>("ui.shader"));
		uiMat->SetFloat("uDpi", 1.f / 180.f);
		m_material.push(uiMat);
		m_layer.push(uiLayer);
		m_writer = new Writer(m_material.top(), m_layer.top());
	}
	Ui::~Ui()
	{
		delete m_writer;
	}
	
	Entity Ui::InstanceFrame(UiElemID id, String texture, Vec2f size)
	{
		auto it = m_frameTemplates.find(texture);
		if (it == m_frameTemplates.end())
		{
			LOG_ERROR("Frame template {0}, doesn't exists!", texture);
			return Entity();
		}

		auto& frame = it->second;
		
		//Dimensions
		Vec2f sDim = frame.cornerSpr[0]->GetDimensions();
		 
		//Ensure frame size is at least the size of the four corners.
		//Apply fixed step if relevant
		if (frame.fixedStep || size.x < sDim.x * 2)
			size.x = std::max(Math::NearestMultiple(size.x, sDim.x), sDim.x * 2.f);
		if (frame.fixedStep || size.y < sDim.y * 2)
			size.y = std::max(Math::NearestMultiple(size.y, sDim.y), sDim.y * 2.f);

		float ppu = frame.repeatTex[0]->GetPixelPerUnit();
		Vec2f pxDim = sDim / ppu;
		Vec2f pxSize = size / ppu;
		Vec2f hDim = sDim * 0.5f;

		//Instance border sprites at the right dimensions
		auto& borderSpr = m_borderSprites[id];
		borderSpr.clear();
		for (int i = 0; i < 5; i++)
		{
			Vec2f wDim;
			Rect rect;
			rect.left = 0;
			rect.top = 0;
			BorderSize(i, rect, wDim, pxSize, pxDim, sDim, ppu);
			borderSpr.emplace_back(BorderSprite(frame.repeatTex[i], rect, wDim));
		}

		//Create the sprites entities:
		auto entt = Entity::Create();
		entt.adc<Transform>();
		Anchor anchor = Anchor::TopLeft;
		
		//Corners
		for (int i = 0; i < 4; i++)
		{
			auto& sprite = frame.cornerSpr[i];
			auto e = Entity::Create();
			auto spr = e.adc<SpriteRender>();
			spr->SetLayer(m_layer.top());
			spr->SetMaterial(m_material.top()->GetID());
			auto tr = e.adc<Transform>();
			spr->SetSprite(sprite);
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
			entt.AddChild(e);
		}

		//Borders
		for (int i = 0; i < 5; i++)
		{
			//Top middle border
			auto& sprite = borderSpr[i].sprite;
			auto e = Entity::Create();
			auto spr = e.adc<SpriteRender>();
			spr->SetLayer(m_layer.top());
			spr->SetMaterial(m_material.top()->GetID());
			auto tr = e.adc<Transform>();
			spr->SetSprite(&sprite);
			auto re = sprite.GetTextureRect();
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
			entt.AddChild(e);
		}
		
		return entt;
	}

	/*---Initialization---*/
	
	void Ui::MakeFrameTemplate(String texture, bool fixedStep)
	{
		auto ins = Instance();
		if (ins->m_frameTemplates.find(texture) != ins->m_frameTemplates.end())
		{
			LOG_ERROR("Frame template {0}, already exists!", texture);
			return;
		}

		auto& frame = ins->m_frameTemplates[texture];
		frame.fixedStep = fixedStep;
		MakeBorderTex(texture, frame.repeatTex);

		//Load the corner sprites
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
			frame.cornerSpr.emplace_back(Assets::Get<Sprite>(spriteName));
		}
	}
	void Ui::MakeFont(String filename, String fancyName, int size, float outlineThickness, Vec4f outlineColor)
	{
	}

	/*---State---*/

	void Ui::PushMaterial(Material* material)
	{
		Instance()->m_material.push(material);
	}
	void Ui::PushFont(FontID font)
	{
		Instance()->m_font.push(font);
	}
	void Ui::PushLayer(LayerID layer)
	{
		Instance()->m_layer.push(layer);
	}
	void Ui::PopMaterial(Material* material)
	{
		if (Instance()->m_material.size() > 1)
			Instance()->m_material.pop();
		else
			LOG_WARN("Trying to pop material stack but only one element left.");
	}
	void Ui::PopFont(FontID font)
	{
		if (Instance()->m_font.size() > 1)
			Instance()->m_font.pop();
		else
			LOG_WARN("Trying to pop font stack but only one element left.");
	}
	void Ui::PopLayer(LayerID layer)
	{
		if (Instance()->m_layer.size() > 1)
			Instance()->m_layer.pop();
		else
			LOG_WARN("Trying to pop layer stack but only one element left.");
	}
	Vec3f Ui::UiToWorld(Vec3f uiPos)
	{
		return Vec3f();
	}
	Vec3f Ui::WorldToUi(Vec3f uiPos)
	{
		return Vec3f();
	}
	Writer* Ui::GetWriter()
	{
		return Instance()->m_writer;
	}
	Material* Ui::GetMaterial()
	{
		return Instance()->m_material.top();
	}
	FontID Ui::GetFont()
	{
		return Instance()->m_font.top();
	}
	LayerID Ui::GetLayer()
	{
		return Instance()->m_layer.top();
	}

	/*---Helpers---*/
	void Ui::MakeBorderTex(String texture, std::vector<Texture*>& tex)
	{
		//Create the repeating textures for frames
		tex.clear();
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
			tex.emplace_back(subTexture);
		}
	}

	void Ui::BorderSize(int i, Rect& rect, Vec2f& wDim, Vec2f pxSize, Vec2f pxDim, Vec2f sDim, float ppu)
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
}