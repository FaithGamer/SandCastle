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
#include "SandCastle/UI/UiCheckbox.h"
#include "SandCastle/Input/Mouse.h"
#include "SandCastle/Render/Window.h"
#include "SandCastle/Render/Camera.h"
#include "SandCastle/Input/Inputs.h"
#include "SandCastle/Input/ButtonInput.h"
#include "SandCastle/Core/Container.h"

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
		m_context.material = uiMat;
		m_context.layer = uiLayer;
		m_writer = new Writer(m_context.material, m_context.layer);

		/*auto input = Inputs::CreateInputMap("UI");
		auto click = input->CreateButtonInput("Click");
		click->BindMouse(Mouse::Button::Left);
		click->SetSignalOnRelease(true);
		click->signal.Listen(&Ui::OnClick, this);*/
	}

	Ui::~Ui()
	{
		delete m_writer;
	}

	bool Ui::OnEvent(SDL_Event& event)
	{
		switch (event.type)
		{
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			return OnClick(true);
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			return OnClick(false);
			break;
		}
		return false;
	}

	void Ui::Update()
	{
		HoverableUpdate();
		ValuesUpdate();
		LayoutUpdate();
		DestroyUpdate();
	}

	void Ui::LayoutUpdate()
	{
		std::sort(m_layoutUpdate.begin(), m_layoutUpdate.end(),
			[](UiCanvas* a, UiCanvas* b)->bool
			{
				return a->GetParentCount() > b->GetParentCount();
			});
		for (auto canvas : m_layoutUpdate)
		{
			canvas->UpdateLayout();
		}
		m_layoutUpdate.clear();
	}
	void Ui::DestroyUpdate()
	{
		auto ins = Instance();
		for (int i = 0; i < m_destroy.size(); i++)
		{
			delete m_destroy[i];
		}
		m_destroy.clear();
	}
	void RemoveHelper(std::vector<UiElem*>& container, UiElem::ID id)
	{
		int remH = -1;
		for (int j = 0; j < container.size(); j++)
		{
			if (container[j]->GetID() == id)
			{
				remH = j;
				break;
			}
		}
		if (remH != -1)
		{
			container[remH] = container.back();
			container.pop_back();
		}
	}
	void RemoveHelper(std::vector<UiTxt*>& container, UiElem::ID id)
	{
		int remH = -1;
		for (int j = 0; j < container.size(); j++)
		{
			if (container[j]->GetID() == id)
			{
				remH = j;
				break;
			}
		}
		if (remH != -1)
		{
			container[remH] = container.back();
			container.pop_back();
		}
	}
	void Ui::OnDestroy(UiElem* elem)
	{
		auto id = elem->id;
		auto it = m_roots.find(id);
		if (it != m_roots.end())
		{
			m_roots.erase(it);
			Container::Remove(m_fastRoots, it->second);
		}
		if (m_hovered && m_hovered->id == id)
			m_hovered = nullptr;
		if (m_pressed && m_pressed->id == id)
			m_pressed = nullptr;

		RemoveHelper(m_hoverables, id);
		RemoveHelper(m_values, id);
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

	bool Ui::OnClick(bool pressed)
	{
		//Are we over any ui canvas ?
		bool hover = m_hovered != nullptr;
		if (!hover)
		{
			for (int i = 0; i < m_fastRoots.size(); i++)
			{
				if (m_fastRoots[i]->IsInside(MousePos()))
				{
					hover = true;
					break;
				}
			}
		}

		//Handling clickable UI
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

		//Return true if mouse is over any canvas
		return hover;
	}

	void Ui::OnCanvasMustUpdate(UiCanvas* canvas)
	{
		Instance()->m_layoutUpdate.emplace_back(canvas);
	}

	void Ui::NewElem(UiElem* elem, UiCanvas* canvas)
	{
		auto id = m_nextId++;
		elem->id = id;
		elem->parent = canvas;
		elem->z = -(float)m_canvas.size();
		elem->margin = m_context.rootMargin;
		if (canvas != nullptr)
		{
			elem->margin = m_context.margin;
			canvas->AddElem(elem);
		}
		elem->destroySignal.Listen(&Ui::OnDestroy, this);
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
		ins->m_writer->SetLayer(ins->m_context.layer);
		auto font = ins->m_writer->MakeFont(filename, pxSize, pxOutline, outlineColor);
		ins->m_writer->NameFont(font, fancyName);
	}

	void Ui::DefaultMaterial(Material* material)
	{
		Instance()->m_defaultMaterial = material;
		Instance()->m_context.material = material;
	}

	void Ui::PPU(float ppu)
	{
		ppu = 1.f / ppu;
		Instance()->m_ppu = ppu;
		Instance()->m_defaultMaterial->SetFloat("uPPu", ppu * 2.f);
		Instance()->m_context.material->SetFloat("uPPu", ppu * 2.f);
	}

	UiCanvas* Ui::Begin(Vec2f size, bool frame)
	{
		auto i = Instance();
		auto canvas = new UiCanvas();
		canvas->context = i->m_context.canvas;
		canvas->root = Entity::Create();
		canvas->root.AddComponent<Transform>();
		auto parent = i->m_canvas.empty() ? nullptr : i->m_canvas.top();
		if (frame)
			canvas->frame = UiFrame(canvas->context.frame, i->m_context.material, i->m_context.layer);
		else
			canvas->context.frame = nullptr;
		//Differentiate limit from size
		if (size.x > 0.f)
			canvas->fixedSize.AddFlag(UiCanvas::Horizontal);
		if (size.y > 0.f)
			canvas->fixedSize.AddFlag(UiCanvas::Vertical);
		canvas->sizeLimit.x = size.x > 0.f ? size.x : canvas->sizeLimit.x;
		canvas->sizeLimit.y = size.y > 0.f ? size.y : canvas->sizeLimit.y;
		if (parent)
		{
			auto parentLimit = Vec2f{
				parent->sizeLimit.x - parent->context.padding.x * 2,
				parent->sizeLimit.y - parent->context.padding.y * 2 };

			canvas->sizeLimit.x = parentLimit.x < canvas->sizeLimit.x ? parentLimit.x : canvas->sizeLimit.x;
			canvas->sizeLimit.y = parentLimit.y < canvas->sizeLimit.y ? parentLimit.y : canvas->sizeLimit.y;
			canvas->anchor = CanvasAnchor::TopLeft;
		}
		else
		{
			canvas->anchor = i->m_context.rootAnchor;
		}
		size.x = size.x > canvas->sizeLimit.x ? canvas->sizeLimit.x : size.x;
		size.y = size.y > canvas->sizeLimit.y ? canvas->sizeLimit.y : size.y;
		canvas->size = size;
		i->NewElem(canvas, parent);
		i->m_canvas.push(canvas);
		canvas->mustUpdateSignal.Listen(&Ui::OnCanvasMustUpdate, i.get(), SignalPriority::high);
		if (parent == nullptr)
		{
			//root
			i->m_roots.insert(std::make_pair(canvas->id, canvas));
			i->m_fastRoots.emplace_back(canvas);
		}
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
		text->context = i->m_context.text;
		text->utf8 = utf8;
		i->CreateText(text, utf8, width);
		i->NewElem(text, canvas);

		return text;
	}
	void Ui::CreateText(UiTxt* text, std::string_view utf8, float width)
	{
		if (text->root.Valid())
			text->root.Destroy();
		auto canvas = text->parent;
		float limit = canvas->sizeLimit.x - canvas->context.padding.x * 2;
		if (limit < 8888888.f)
			//There is a size limit
			width = width > 0.f ? std::min(limit, width) : limit;
		else
			//There is no size limit
			width = width > 0.f ? std::min(limit, width) : 0.f;

		auto font = m_writer->GetFont(text->context.font);
		text->sentence = m_writer->Write
		(
			utf8,
			text->context.font,
			text->context.color,
			font->material,
			font->layer,
			width,
			text->context.align,
			1.f
		);

		text->size = text->sentence.size;
		text->root = text->sentence.root;
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
		image->margin = i->m_context.margin;
		image->root = Entity::CreateSprite(sprite);
		auto render = image->root.GetComponent<SpriteRender>();

		render->SetLayer(i->m_context.layer);
		render->SetMaterial(i->m_context.material->GetID());
		auto spr = render->GetSprite();
		image->sprite = spr;
		image->size = image->sprite->GetDimensions();

		i->NewElem(image, canvas);
		return image;
	}

	UiBtn* Ui::Button(std::string_view utf8)
	{
		auto i = Instance();
		ASSERT_LOG_ERROR(!i->m_canvas.empty(), "Trying to create Button without active canvas");
		auto canvas = i->m_canvas.top();

		UiBtn* button = new UiBtn();
		button->root = Entity::Create();
		button->root.AddComponent<Transform>();
		button->margin = i->m_context.margin;
		auto font = i->m_writer->GetFont(i->m_context.button.font);
		button->label = i->m_writer->Write(utf8, font->id, i->m_context.button.textColor, font->material, font->layer, 0.f, TextAlign::Center, 1.f);
		button->label.root.GetComponent<Transform>()->Move(i->m_context.button.padding.x, -i->m_context.button.padding.y, -3.f);
		button->root.AddChild(button->label.root);
		button->size.x = button->label.size.x + i->m_context.button.padding.x * 2;
		button->size.y = button->label.size.y + i->m_context.button.padding.y * 2;
		button->frameIdle = UiFrame(
			i->m_context.button.frameIdle,
			i->m_context.material,
			i->m_context.layer);
		button->frameHover = UiFrame(
			i->m_context.button.frameHover,
			i->m_context.material,
			i->m_context.layer);
		button->framePressed = UiFrame(
			i->m_context.button.framePressed,
			i->m_context.material,
			i->m_context.layer);
		i->NewElem(button, canvas);
		button->UpdateFrames();
		RegisterHoverable(button);
		return button;
	}

	UiCheckbox* Ui::Checkbox()
	{
		auto i = Instance();
		ASSERT_LOG_ERROR(!i->m_canvas.empty(), "Trying to create Checkbox without active canvas");
		auto canvas = i->m_canvas.top();
		ASSERT_LOG_ERROR((i->m_context.checkbox.texture != ""), "Trying to create Checkbox without checkbox sprite");

		auto checkbox = new UiCheckbox();
		checkbox->root = Entity::Create();
		auto tr = checkbox->root.AddComponent<Transform>();
		for (int j = 0; j < 3; j++)
		{
			String spriteName = i->m_context.checkbox.texture + "_0_" + std::to_string(j);
			auto entt = Entity::CreateSprite(spriteName);
			auto rd = entt.GetComponent<SpriteRender>();
			rd->SetLayer(i->m_context.layer);
			rd->SetMaterial(i->m_context.material->GetID());
			auto spr = rd->GetSprite();
			auto dim = spr->GetDimensions();
			Vec3f offset = {
			((float)spr->orgX + 0.5f) * dim.x,
			((float)spr->orgY - 0.5f) * dim.y,
			0.f
			};
			entt.gtr()->Move(offset);
			checkbox->sprites.emplace_back(entt);
			checkbox->root.AddChild(entt);
		}
		auto rd = checkbox->sprites[0].GetComponent<SpriteRender>();
		auto spr = rd->GetSprite();
		checkbox->size = spr->GetDimensions();
		checkbox->margin = i->m_context.margin;
		checkbox->clickable = true;
		i->NewElem(checkbox, canvas);
		checkbox->UpdateVisual();
		RegisterHoverable(checkbox);
		return checkbox;

	}

	void Ui::End()
	{
		auto i = Instance();
		if (i->m_canvas.empty())
		{
			LOG_ERROR("EndCanvas called without an active canvas.");
			return;
		}
		auto canvas = i->m_canvas.top();
		canvas->MustUpdate();
		i->m_canvas.pop();
	}

	void Ui::Destroy(UiElem* elem)
	{
		Instance()->m_destroy.emplace_back(elem);
	}

	/*---Contect---*/

	void Ui::SnapshotContext(String name)
	{
		auto i = Instance();
		auto it = i->m_contextSnapshots.find(name);
		if (it != i->m_contextSnapshots.end())
		{
			LOG_ERROR("Canvas context snapshot with the name {0}, already exists.");
			return;
		}
		i->m_contextSnapshots[name] = i->m_context;
	}
	void Ui::Context(String name)
	{
		auto i = Instance();
		auto it = i->m_contextSnapshots.find(name);
		if (it == i->m_contextSnapshots.end())
		{
			LOG_ERROR("The following canvas context doesnt exists: {0}", name);
			return;
		}
		i->m_context = it->second;
	}
	void Ui::SetMaterial(Material* material)
	{
		material->SetFloat("uPpu", Instance()->m_ppu * 2.f);
		Instance()->m_context.material = material;
	}
	void Ui::SetTextFont(FontID font)
	{
		Instance()->m_context.text.font = font;
	}
	void Ui::SetTextFont(String fancyName)
	{
		auto ins = Instance();
		ins->m_context.text.font = ins->m_writer->GetFont(fancyName)->id;
	}
	void Ui::SetButtonFont(FontID font)
	{
		Instance()->m_context.button.font = font;
	}
	void Ui::SetButtonFont(String fancyName)
	{
		auto ins = Instance();
		ins->m_context.button.font = ins->m_writer->GetFont(fancyName)->id;
	}
	void Ui::SetTextColor(Color color)
	{
		Instance()->m_context.text.color = color;
	}
	void Ui::SetButtonTextColor(Color color)
	{
		Instance()->m_context.button.textColor = color;
	}
	void Ui::SetButtonPadding(Vec2f padding)
	{
		Instance()->m_context.button.padding = padding;
	}
	void Ui::SetLayer(LayerID layer)
	{
		Instance()->m_context.layer = layer;
	}

	void Ui::SetCanvasFrame(String texture)
	{
		Instance()->SetFrame(&Instance()->m_context.canvas.frame, texture);
	}
	void Ui::SetCanvasPadding(Vec2f padding)
	{
		Instance()->m_context.canvas.padding = padding;
	}
	void Ui::SetCanvasSpacing(Vec2f spacing)
	{
		Instance()->m_context.canvas.spacing = spacing;
	}
	void Ui::SetCanvasLayoutDir(LayoutDir dir)
	{
		Instance()->m_context.canvas.layoutDir = dir;
	}
	void Ui::SetCanvasLayoutAlignH(LayoutAlign alignH)
	{
		Instance()->m_context.canvas.layoutAlignH = alignH;
	}
	void Ui::SetCanvasLayoutAlignV(LayoutAlign alignV)
	{
		Instance()->m_context.canvas.layoutAlignV = alignV;
	}
	void Ui::SetButtonFrame(String texture)
	{
		Instance()->SetFrame(&Instance()->m_context.button.frameIdle, texture);
	}
	void Ui::SetButtonFrameHover(String texture)
	{
		Instance()->SetFrame(&Instance()->m_context.button.frameHover, texture);
	}
	void Ui::SetButtonFramePressed(String texture)
	{
		Instance()->SetFrame(&Instance()->m_context.button.framePressed, texture);
	}
	void Ui::SetCheckboxSprites(String texture)
	{
		Instance()->m_context.checkbox.texture = texture;
	}
	void Ui::SetTextAlign(TextAlign textAlign)
	{
		Instance()->m_context.text.align = textAlign;
	}
	void Ui::SetMargin(Vec2f margin)
	{
		Instance()->m_context.margin = margin;
	}
	void Ui::SetRootMargin(Vec2f margin)
	{
		Instance()->m_context.rootMargin = margin;
	}
	void Ui::SetRootAnchor(CanvasAnchor anchor)
	{
		Instance()->m_context.rootAnchor = anchor;
	}
	void Ui::ResetMaterial(Material* material)
	{
		Instance()->m_context.material = Instance()->m_defaultMaterial;
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
		return Instance()->m_context.material;
	}
	FontID Ui::GetFont()
	{
		return Instance()->m_context.text.font;
	}
	LayerID Ui::GetLayer()
	{
		return Instance()->m_context.layer;
	}

	std::unordered_map<UiElem::ID, UiCanvas*> Ui::GetCanvases()
	{
		return Instance()->m_roots;
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