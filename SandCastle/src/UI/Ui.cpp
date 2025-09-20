#include "pch.h"
#include "SandCastle/UI/Ui.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/Core/Math.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/RenderOptions.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/UI/UiCanvas.h"
#include "SandCastle/UI/UiTxt.h"
#include "SandCastle/UI/UiImg.h"
#include "SandCastle/UI/UiBtn.h"
#include "SandCastle/Input/Mouse.h"
#include "SandCastle/Render/Window.h"
#include "SandCastle/Render/Camera.h"
#include "SandCastle/Input/Inputs.h"
#include "SandCastle/Input/ButtonInput.h"

namespace SandCastle
{
	UiElem::ID Ui::m_nextId = 0;

	Ui::Ui()
	{
		auto uiLayer = Renderer2D::AddLayer("ui");
		Renderer2D::SetLayerSortZ(uiLayer, true);
		auto uiMat = Renderer2D::CreateMaterial(Assets::Get<Shader>("ui.shader"));
		uiMat->GetRenderOptions()->SetDepthTest(false);
		uiMat->SetFloat("uPpu", m_ppu * 2.f);
		m_defaultMaterial = uiMat;
		m_material = uiMat;
		m_layer = uiLayer;
		m_writer = new Writer(m_material, m_layer);
		m_writer->SetPPU(3.f);

		auto input = Inputs::CreateInputMap("UI");
		auto click = input->CreateButtonInput("Click");
		click->BindMouse(Mouse::Button::Left);
		click->SetSignalOnRelease(true);
		click->signal.Listen(&Ui::OnClick, this);
	}

	Ui::~Ui()
	{
		delete m_writer;
	}

	void Ui::Update()
	{
		HoverableUpdate();
		ValuesUpdate();
	}

	void Ui::HoverableUpdate()
	{
		if (m_hovered != nullptr && !m_hovered->IsInside(MousePos()))
		{
			m_hovered->UnHover();
			m_hovered = nullptr;
		}
		if (m_hovered != nullptr)
			return;
		for (int i = 0; i < m_hoverables.size(); i++)
		{
			auto candidate = m_hoverables[i];
			if (candidate->IsInside(MousePos()))
			{
				candidate->Hover();
				m_hovered = candidate;
				break;
			}
		}
	}

	void Ui::ValuesUpdate()
	{
		for (int i = 0; i < m_values.size(); i++)
		{
			if (m_values[i]->DataChanged())
			{
				UpdateText(m_values[i], m_values[i]->Format(), false);
			}
		}
	}

	void Ui::OnClick(InputSignal* signal)
	{
		bool pressed = signal->GetBool();
		if (m_hovered != nullptr && m_hovered->clickable)
		{
			if (pressed)
			{
				m_pressed = m_hovered;
				m_hovered->ClickPressed();
			}
			else
			{
				m_pressed = nullptr;
				m_hovered->ClickReleased();
			}
		}
	}

	void Ui::NewElem(UiElem* elem, UiCanvas* canvas)
	{
		auto id = m_nextId++;
		elem->id = id;
		elem->parent = canvas;
		elem->z = -1.f;
		elem->margin = m_rootMargin;
		if (canvas != nullptr)
		{
			elem->margin = m_margin;
			canvas->AddElem(elem);
		}
	}

	/*---Initialization---*/

	void Ui::MakeFrameTemplate(const String& texture, bool fixedStep)
	{
		auto i = Instance();
		if (i->m_frameTemplates.find(texture) != i->m_frameTemplates.end())
		{
			LOG_ERROR("Frame template {0}, already exists!", texture);
			return;
		}

		UiFrame::MakeTemplate(i->m_frameTemplates[texture], texture, fixedStep);

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

	void Ui::SetDefaultMaterial(Material* material)
	{
		Instance()->m_defaultMaterial = material;
		Instance()->m_material = material;
	}

	void Ui::SetPPU(float ppu)
	{
		ppu = 1.f / ppu;
		Instance()->m_ppu = ppu;
		Instance()->m_defaultMaterial->SetFloat("uPPu", ppu * 2.f);
		Instance()->m_material->SetFloat("uPPu", ppu * 2.f);
	}

	UiCanvas* Ui::BeginCanvas(Vec2f size, bool frame)
	{
		auto i = Instance();
		auto canvas = new UiCanvas();
		auto parent = i->m_canvas.empty() ? nullptr : i->m_canvas.top();
		if (frame)
			canvas->frame = UiFrame(i->m_canvasFrame, i->m_material, i->m_layer);
		//Differentiante limit from size
		if (size.x > 0.f)
			canvas->fixedSize.AddFlag(UiCanvas::Horizontal);
		if (size.y > 0.f)
			canvas->fixedSize.AddFlag(UiCanvas::Vertical);
		canvas->sizeLimit.x = size.x > 0.f ? size.x : canvas->sizeLimit.x;
		canvas->sizeLimit.y = size.y > 0.f ? size.y : canvas->sizeLimit.y;
		if (parent)
		{
			auto parentLimit = Vec2f{
				parent->sizeLimit.x - parent->border.x * 2,
				parent->sizeLimit.y - parent->border.y * 2 };

			canvas->sizeLimit.x = parentLimit.x < canvas->sizeLimit.x ? parentLimit.x : canvas->sizeLimit.x;
			canvas->sizeLimit.y = parentLimit.y < canvas->sizeLimit.y ? parentLimit.y : canvas->sizeLimit.y;
		}
		size.x = size.x > canvas->sizeLimit.x ? canvas->sizeLimit.x : size.x;
		size.y = size.y > canvas->sizeLimit.y ? canvas->sizeLimit.y : size.y;
		canvas->size = size;
		i->m_canvas.push(canvas);
		i->NewElem(canvas, parent);
		return canvas;
	}

	UiTxt* Ui::Text(std::string_view utf8, float width)
	{
		auto i = Instance();
		ASSERT_LOG_ERROR(!i->m_canvas.empty(), "Trying to create Ui Text without active canvas");
		auto canvas = i->m_canvas.top();

		//Instantiation
		UiTxt* text = new UiTxt();
		text->parent = canvas;
		text->font = i->m_font;
		text->align = i->m_textAlign;
		text->color = i->m_txtColor;
		text->utf8 = utf8;
		i->CreateText(text, utf8, width);
		i->NewElem(text, canvas);

		return text;
	}
	void Ui::CreateText(UiTxt* text, std::string_view utf8, float width)
	{
		if (text->sentence.root.Valid())
			text->sentence.root.Destroy();
		auto canvas = text->parent;
		if (canvas->sizeLimit.x < 8888888.f)
			//There is a size limit
			width = width > 0.f ? std::min(canvas->sizeLimit.x, width) : canvas->sizeLimit.x;
		else
			//There is no size limit
			width = width > 0.f ? std::min(canvas->sizeLimit.x, width) : 0.f;

		auto font = m_writer->GetFont(text->font);
		text->sentence = m_writer->Write
		(
			utf8,
			text->font,
			text->color,
			font->material,
			font->layer,
			width,
			text->align,
			1.f
		);

		text->size = text->sentence.size;
		text->root = text->sentence.root;
		text->parent->root.AddChild(text->root);

	}
	void Ui::UpdateText(UiTxt* text, std::string_view utf8, bool replaceUtf8)
	{
		auto prevSize = text->sentence.size;
		if (replaceUtf8)
			text->utf8 = utf8;
		Instance()->CreateText(text, text->Format(), text->sentence.maxWidth);
		if (std::abs(prevSize.x - text->size.x) > 0.01f
			|| std::abs(prevSize.y - text->size.y) > 0.01f)
			text->parent->UpdateLayout();
		else
			text->SetPosition(text->position);
	}

	UiImg* Ui::Image(String sprite)
	{
		auto i = Instance();
		ASSERT_LOG_ERROR(!i->m_canvas.empty(), "Trying to create Ui Image without active canvas");
		auto canvas = i->m_canvas.top();

		UiImg* image = new UiImg();
		image->margin = i->m_margin;
		image->entt = Entity::CreateSprite(sprite);
		auto render = image->entt.GetComponent<SpriteRender>();

		render->SetLayer(i->m_layer);
		render->SetMaterial(i->m_material->GetID());
		auto spr = render->GetSprite();
		image->sprite = spr;
		image->size = image->sprite->GetDimensions();

		//Offset sprite to make top left anchor no matter the sprite origin
		auto dim = spr->GetDimensions();
		Vec2f offset = {
			((float)spr->orgX + 0.5f) * dim.x,
			((float)spr->orgY - 0.5f) * dim.y
		};
		image->entt.GetComponent<Transform>()->Move(offset);
		image->root.AddChild(image->entt);

		i->NewElem(image, canvas);
		return image;
	}

	UiBtn* Ui::Button(std::string_view utf8, Vec2f padding)
	{
		auto i = Instance();
		ASSERT_LOG_ERROR(!i->m_canvas.empty(), "Trying to create Button without active canvas");
		auto canvas = i->m_canvas.top();

		UiBtn* button = new UiBtn();
		button->margin = i->m_margin;
		auto font = i->m_writer->GetFont(i->m_font);
		button->label = i->m_writer->Write(utf8, font->id, i->m_txtColor, font->material, font->layer, 0.f, TextAlign::Center, 1.f);
		button->label.root.GetComponent<Transform>()->Move(padding.x, -padding.y, -3.f);
		button->root.AddChild(button->label.root);
		button->size.x = button->label.size.x + padding.x * 2;
		button->size.y = button->label.size.y + padding.y * 2;
		button->frameIdle = UiFrame(i->m_buttonFrame, i->m_material, i->m_layer);
		button->frameHover = UiFrame(i->m_buttonFrameHover, i->m_material, i->m_layer);
		button->framePressed = UiFrame(i->m_buttonFramePressed, i->m_material, i->m_layer);
		button->UpdateFrames();
		RegisterHoverable(button);
		i->NewElem(button, canvas);
		return button;
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
		canvas->UpdateLayout();
		i->m_canvas.pop();
	}

	void Ui::Delete(UiElem::ID uiElem)
	{


	}

	/*---State---*/

	void Ui::SetMaterial(Material* material)
	{
		material->SetFloat("uPpu", Instance()->m_ppu * 2.f);
		Instance()->m_material = material;
	}
	void Ui::SetFont(FontID font)
	{
		Instance()->m_font = font;
	}
	void Ui::SetFont(String fancyName)
	{
		auto ins = Instance();
		ins->m_font = ins->m_writer->GetFont(fancyName)->id;
		ins->m_writer->UseFont(ins->m_font);
	}
	void Ui::SetTextColor(Color color)
	{
		Instance()->m_txtColor = color;
	}
	void Ui::SetLayer(LayerID layer)
	{
		Instance()->m_layer = layer;
	}
	void Ui::SetCanvasFrame(String texture)
	{
		Instance()->SetFrame(&Instance()->m_canvasFrame, texture);
	}
	void Ui::SetButtonFrame(String texture)
	{
		Instance()->SetFrame(&Instance()->m_buttonFrame, texture);
	}
	void Ui::SetButtonFrameHover(String texture)
	{
		Instance()->SetFrame(&Instance()->m_buttonFrameHover, texture);
	}
	void Ui::SetButtonFramePressed(String texture)
	{
		Instance()->SetFrame(&Instance()->m_buttonFramePressed, texture);
	}
	void Ui::SetTextAlign(TextAlign textAlign)
	{
		Instance()->m_textAlign = textAlign;
	}
	void Ui::SetMargin(Vec2f margin)
	{
		Instance()->m_margin = margin;
	}
	void Ui::SetRootMargin(Vec2f margin)
	{
		Instance()->m_rootMargin = margin;
	}
	void Ui::ResetMaterial(Material* material)
	{
		Instance()->m_material = Instance()->m_defaultMaterial;
	}
	Vec3f Ui::UiToWorld(Vec2f uiPos)
	{
		auto ppu = Instance()->m_ppu;
		auto cam = Camera::main;
		Vec2f screen = Window::GetSize();
		Vec2f midScreen(screen.x * 0.5f, screen.y * 0.5f);
		Vec3f mid = cam->ScreenToWorld(midScreen, screen);

		Vec3f p;
		float scale = ppu / cam->zoom;
		p.z = 0.f;
		p.x = mid.x + (uiPos.x * scale);
		p.y = mid.y + (uiPos.y * scale);

		return p;
	}
	Vec2f Ui::WorldToUi(Vec3f worldPos)
	{
		auto ppu = Instance()->m_ppu;
		auto cam = Camera::main;
		Vec2f screen = Window::GetSize();
		Vec2f midScreen(screen.x * 0.5f, screen.y * 0.5f);
		Vec3f mid = cam->ScreenToWorld(midScreen, screen);

		Vec2f ui;
		float scale = ppu / cam->zoom;
		ui.x = (worldPos.x - mid.x) / scale;
		ui.y = (worldPos.y - mid.y) / scale;

		return ui;
	}
	Vec2f Ui::ScreenToUi(Vec2f pos)
	{
		auto ppu = Instance()->m_ppu;
		Vec2f screen = Window::GetSize();
		float ratio = (screen.x / screen.y);
		Vec2f norm = {
			pos.x / screen.x - 0.5f,
			pos.y / screen.y - 0.5f
		};
		auto red = Camera::main->GetReduction();
		return Vec2f{
		norm.x * ratio / ppu / red,
		-norm.y / ppu / red };
	}
	Vec2f Ui::MousePos()
	{
		return ScreenToUi(Mouse::GetPosition());
	}
	void Ui::RegisterHoverable(UiElem* elem)
	{
		if (elem->hoverable)
			return;
		auto i = Instance();
		auto& h = i->m_hoverables;
		if (std::find(h.begin(), h.end(), elem) == h.end())
		{
			elem->hoverable = true;
			h.emplace_back(elem);
		}
		else
		{
			LOG_WARN("Trying to make hoverable an elem that is already in the list.");
		}
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

	void Ui::SetFrame(UiFrame::Template** frame, String texture)
	{
		auto it = m_frameTemplates.find(texture);
		if (it == m_frameTemplates.end())
		{
			LOG_ERROR("Ui::SetFrame, the frame template {0}, doesn't exists.", texture);
			return;
		}
		*frame = &it->second;
	}
}