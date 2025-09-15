#include "pch.h"
#include "SandCastle/Render/Ui.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/Core/Math.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/RenderOptions.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Render/UiEnum.h"
#include "SandCastle/Render/UiCanvas.h"
#include "SandCastle/Render/UiTxt.h"
#include "SandCastle/Render/UiImg.h"


namespace SandCastle
{
	Ui::BorderSprite::BorderSprite(Texture* tex, Rect rect, Vec2f worldDim)
		: sprite(tex, rect),
		wDim(worldDim)
	{

	}

	Ui::ElemID Ui::m_nextId = 0;

	Ui::Ui()
	{
		auto uiLayer = Renderer2D::AddLayer("ui");
		Renderer2D::SetLayerSortZ(uiLayer, true);
		auto uiMat = Renderer2D::CreateMaterial(Assets::Get<Shader>("ui.shader"));
		uiMat->GetRenderOptions()->SetDepthTest(false);
		uiMat->SetFloat("uDpi", 1.f / 180.f);
		m_material = uiMat;
		m_layer = uiLayer;
		m_writer = new Writer(m_material, m_layer);
		m_writer->SetPPU(3.f);
	}

	Ui::~Ui()
	{
		delete m_writer;
	}

	void Ui::AddElem(Elem* elem, Canvas* canvas)
	{
		auto id = m_nextId++;
		elem->id = id;
		elem->parent = canvas;
		elem->z = -1.f;
		elem->margin = elem->GetType() == Ui::Elem::Type::Canvas ? m_canvasMargin : m_margin;
		m_elems[id] = elem;
		if (canvas != nullptr)
		{
			canvas->root.AddChild(elem->root);
			canvas->children.emplace_back(elem);
		}
	}

	Entity Ui::InstanceFrame(Elem* elem, FrameTemplate* frame, Vec2f size)
	{
		//Dimensions
		Vec2f sDim = frame->cornerSpr[0]->GetDimensions();
		float z = 1.f;

		//Ensure frame size is at least the size of the four corners.
		//Apply fixed step if relevant
		if (frame->fixedStep || size.x < sDim.x * 2)
			size.x = std::max(Math::CeilMultiple(size.x, sDim.x), sDim.x * 2.f);
		if (frame->fixedStep || size.y < sDim.y * 2)
			size.y = std::max(Math::CeilMultiple(size.y, sDim.y), sDim.y * 2.f);

		float ppu = frame->repeatTex[0]->GetPixelPerUnit();
		Vec2f pxDim = sDim / ppu;
		Vec2f pxSize = size / ppu;
		Vec2f hDim = sDim * 0.5f;

		//Instance border sprites at the right dimensions
		auto& borderSpr = m_borderSprites[elem->id];
		borderSpr.clear();
		for (int i = 0; i < 5; i++)
		{
			Vec2f wDim;
			Rect rect;
			rect.left = 0;
			rect.top = 0;
			BorderSize(i, rect, wDim, pxSize, pxDim, sDim, ppu);
			borderSpr.emplace_back(BorderSprite(frame->repeatTex[i], rect, wDim));
		}

		//Create the sprites entities:
		auto entt = Entity::Create();
		auto rootTr = entt.adc<Transform>();
		rootTr->SetPosition(0, 0, z);
		Anchor anchor = Anchor::TopLeft;

		//Corners
		for (int i = 0; i < 4; i++)
		{
			auto& sprite = frame->cornerSpr[i];
			auto e = Entity::Create();
			auto spr = e.adc<SpriteRender>();
			spr->SetLayer(m_layer);
			spr->SetMaterial(m_material->GetID());
			auto tr = e.adc<Transform>();
			spr->SetSprite(sprite);
			auto type = (SpriteCorner)i;
			switch (type)
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
			spr->SetLayer(m_layer);
			spr->SetMaterial(m_material->GetID());
			auto tr = e.adc<Transform>();
			spr->SetSprite(&sprite);
			auto re = sprite.GetTextureRect();
			auto type = (TexBorder)i;
			switch (type)
			{
			case TexBorder::Top:
				tr->SetPosition(size.x * 0.5f, -hDim.y, 0);
				break;
			case TexBorder::Left:
				tr->SetPosition(hDim.x, -size.y * 0.5f, 0);
				break;
			case TexBorder::Mid:
				tr->SetPosition(size.x * 0.5f, -size.y * 0.5f, 0);
				break;
			case TexBorder::Right:
				tr->SetPosition(size.x - hDim.x, -size.y * 0.5f, 0);
				break;
			case TexBorder::Bot:
				tr->SetPosition(size.x * 0.5f, -size.y + hDim.y, 0);
				break;
			default:
				break;
			}
			entt.AddChild(e);
		}
		elem->root.AddChild(entt);
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
			auto type = (SpriteCorner)i;
			switch (type)
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
	void Ui::MakeFont(String filename, String fancyName, float uiSize, float outlineThickness, Vec4f outlineColor)
	{
		auto ins = Instance();
		float ppu = ins->m_writer->GetPPU();
		int pxSize = (int)std::round(uiSize * ppu);
		int pxOutline = outlineThickness > 0.f ? (int)std::max(1, (int)std::round(outlineThickness * ppu)) : 0;
		ins->m_writer->SetLayer(ins->m_layer);
		auto font = ins->m_writer->MakeFont(filename, pxSize, pxOutline, outlineColor);
		ins->m_writer->NameFont(font, fancyName);
	}

	Ui::Canvas* Ui::BeginCanvas(Vec2f size, bool frame)
	{
		auto i = Instance();
		auto canvas = new Canvas();
		canvas->root = Entity::Create();
		canvas->root.AddComponent<Transform>();
		canvas->size = size;
		auto parent = i->m_canvas.empty() ? nullptr : i->m_canvas.top();
		if (size.x > 0.f)
			canvas->fixedSize.AddFlag(Canvas::Horizontal);
		if (size.y > 0.f)
			canvas->fixedSize.AddFlag(Canvas::Vertical);
		canvas->hasFrame = frame;
		i->m_canvas.push(canvas);
		i->m_z -= i->zStep;
		i->AddElem(canvas, parent);
		return canvas;
	}

	Ui::Txt* Ui::Text(std::string_view utf8, float maxWidth)
	{
		auto i = Instance();
		ASSERT_LOG_ERROR(!i->m_canvas.empty(), "Trying to create Ui Text without active canvas");
		auto canvas = i->m_canvas.top();

		if (canvas->fixedSize.Contains(Canvas::Horizontal))
		{
			maxWidth = maxWidth > 0.f ? std::min(canvas->size.x, maxWidth) : canvas->size.x;
		}

		//Instantiation
		Txt* text = new Txt();
		text->sentence = i->m_writer->Write(utf8, maxWidth - i->m_margin.x * 2, i->m_textAlign);
		text->size = text->sentence.size;
		text->root = text->sentence.root;
		i->AddElem(text, canvas);

		return text;
	}

	Ui::Img* Ui::Image(String sprite)
	{
		auto i = Instance();
		ASSERT_LOG_ERROR(!i->m_canvas.empty(), "Trying to create Ui Image without active canvas");
		auto canvas = i->m_canvas.top();

		Img* image = new Img();
		image->margin = i->m_margin;
		image->root = Entity::CreateSprite(sprite);
		auto render = image->root.GetComponent<SpriteRender>();
		render->SetLayer(i->m_layer);
		render->SetMaterial(i->m_material->GetID());
		image->sprite = render->GetSprite();
		image->size = image->sprite->GetDimensions();
		i->AddElem(image, canvas);
		return image;
	}

	void Ui::EndCanvas()
	{
		auto i = Instance();
		if (i->m_canvas.empty())
		{
			LOG_ERROR("EndCanvas called without an active canvas.");
			return;
		}
		auto canvas = i->m_canvas.top();
		canvas->MakeLayout();
		i->m_z += i->zStep;
		if (canvas->hasFrame && i->m_canvasFrame != nullptr)
		{
			canvas->frame = i->InstanceFrame(canvas, i->m_canvasFrame, canvas->size);
		}
		i->m_canvas.pop();
	}

	void Ui::Delete(ElemID uiElem)
	{
		auto i = Instance();
		auto it = i->m_elems.find(uiElem);
		if (it == i->m_elems.end())
			return;

		/*auto elem = it->second;
		if (elem->parent)
		{
			elem->parent->children.remove(elem);
		}*/

	}

	/*---State---*/

	void Ui::SetMaterial(Material* material)
	{
		Instance()->m_material = material;
	}
	void Ui::SetFont(FontID font)
	{
		Instance()->m_font = font;
	}
	void Ui::SetFont(String fancyName)
	{
		auto ins = Instance();
		ins->m_font = ins->m_writer->GetFont(fancyName);
	}
	void Ui::SetLayer(LayerID layer)
	{
		Instance()->m_layer = layer;
	}
	void Ui::SetCanvasFrame(String texture)
	{
		auto ins = Instance();
		auto it = ins->m_frameTemplates.find(texture);
		if (it == ins->m_frameTemplates.end())
		{
			LOG_ERROR("Ui::SetCanvasFrame, the frame template {0}, doesn't exists.", texture);
			return;
		}
		Instance()->m_canvasFrame = &it->second;
	}
	void Ui::SetButtonFrame(String texture)
	{
		auto ins = Instance();
		auto it = ins->m_frameTemplates.find(texture);
		if (it == ins->m_frameTemplates.end())
		{
			LOG_ERROR("Ui::SetCanvasFrame, the frame template {0}, doesn't exists.", texture);
			return;
		}
		Instance()->m_buttonFrame = &it->second;
	}
	void Ui::SetTextAlign(TextAlign textAlign)
	{
		Instance()->m_textAlign = textAlign;
	}
	void Ui::SetMargin(Vec2f margin)
	{
		Instance()->m_margin = margin;
	}
	void Ui::SetCanvasMargin(Vec2f margin)
	{
		Instance()->m_canvasMargin = margin;
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
		return Instance()->m_material;
	}
	FontID Ui::GetFont()
	{
		return Instance()->m_font;
	}
	LayerID Ui::GetLayer()
	{
		return Instance()->m_layer;
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
			auto type = (TexBorder)i;
			switch (type)
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
		auto type = (TexBorder)i;
		if (type == TexBorder::Top || type == TexBorder::Bot)
		{
			rect.width = pxSize.x - pxDim.x * 2; //pixel length
			rect.height = pxDim.y; //pixel length
			wDim.x = rect.width * ppu; //world length
			wDim.y = sDim.y;
		}
		else if (type == TexBorder::Left || type == TexBorder::Right)
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