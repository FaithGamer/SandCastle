#include "pch.h"
#include "SandCastle/UI/Ui.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/RenderOptions.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Render/Animator.h"
#include "SandCastle/UI/UiCanvas.h"
#include "SandCastle/UI/UiTxt.h"
#include "SandCastle/UI/UiImg.h"
#include "SandCastle/UI/UiBtn.h"
#include "SandCastle/UI/UiAnimBtn.h"
#include "SandCastle/UI/UiCheckbox.h"
#include "SandCastle/Input/Mouse.h"
#include "SandCastle/Render/Window.h"
#include "SandCastle/Render/Camera.h"

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
		m_interactionGroups[0] = true;
	}

	Ui::~Ui()
	{
		delete m_writer;
	}

	bool Ui::OnEvent(SDL_Event& event)
	{
		switch (event.type)
		{
			//Handle left click
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (event.button.button == SDL_BUTTON_LEFT)
				return OnClick(true);
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (event.button.button == SDL_BUTTON_LEFT)
				return OnClick(false);
			break;
		default:
			break;
		}
		return false;
	}

	void Ui::Update()
	{
		HoverableUpdate();
		ValuesUpdate();
		DestroyUpdate();
		LayoutUpdate();
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
		if (m_destroy.empty())
			return;
		auto ins = Instance();

		for (int i = 0; i < m_destroy.size(); i++)
		{
			//Remove from layout update
			auto it_f = std::find_if(m_layoutUpdate.begin(), m_layoutUpdate.end(),
			[&](const UiCanvas* obj) 
				{
					return obj->GetID() == m_destroy[i]->GetID(); 
				});
			if (it_f != m_layoutUpdate.end())
				m_layoutUpdate.erase(it_f);
			//If it was hovered, remove from hovered.
			if (m_hovered != nullptr && m_destroy[i]->GetID() == m_hovered->GetID())
			{
				m_hovered->UnHover();
				m_hovered = nullptr;
			}

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
		}
		if (m_hovered && m_hovered->id == id)
			m_hovered = nullptr;
		if (m_pressed && m_pressed->id == id)
			m_pressed = nullptr;

		RemoveHelper(m_hoverables, id);
		RemoveHelper(m_values, id);
	}

	void Ui::OnTxtLang(UiTxt* txt)
	{
		UpdateText(txt, *Assets::Get<Textual>(txt->keyLoc), true);
	}

	void Ui::OnBtnLang(UiBtn* btn)
	{
		UpdateBtn(btn, *Assets::Get<Textual>(btn->keyLoc));
	}

	void Ui::HoverableUpdate()
	{
		//Unhover
		if (m_hovered != nullptr && !m_hovered->IsInside(MousePos()))
		{
			//Mouse is no longer inside
			m_hovered->UnHover();
			m_hovered = nullptr;
		}
		if (m_hovered != nullptr)
			return;
		
		//Hover
		for (int i = 0; i < m_hoverables.size(); i++)
		{
			auto candidate = m_hoverables[i];
			if (candidate->disabled && !candidate->hoverableWhenDisabled)
				continue;
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
			for (auto& kvp : m_roots)
			{
				if (kvp.second->IsInside(MousePos()))
				{
					hover = true;
					break;
				}
			}
		}

		//Handling clickable UI
		if (m_hovered != nullptr && m_hovered->clickable && !m_hovered->disabled)
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
		elem->interactionGroup = m_context.interactionGroup;
		elem->id = id;
		if (canvas != nullptr)
		{
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
	void Ui::MakeFont(String filename, String fancyName, float uiSize, float scale, float lineHeight, std::vector<String> langs, float outlineThickness, Vec4f outlineColor)
	{
		auto ins = Instance();
		float ppu = ins->m_writer->GetPPU();
		int pxSize = (int)std::round(uiSize * ppu);
		int pxOutline = outlineThickness > 0.f ? (int)std::max(1, (int)std::round(outlineThickness * ppu)) : 0;
		ins->m_writer->SetLayer(ins->m_context.layer);
		auto font = ins->m_writer->MakeFont(filename, pxSize, scale, lineHeight, pxOutline, outlineColor);
		ins->m_writer->NameFont(font, fancyName, langs);
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
		canvas->root.Add<Transform>();
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
		canvas->z = i->m_context.z;
		if (parent == nullptr)
			canvas->margin = i->m_context.rootMargin;
		i->NewElem(canvas, parent);
		i->m_canvas.push(canvas);
		canvas->mustUpdateSignal.Listen(&Ui::OnCanvasMustUpdate, i.get(), SignalPriority::high);
		if (parent == nullptr)
		{
			//root
			i->m_roots.insert(std::make_pair(canvas->id, canvas));
		}
		return canvas;
	}

	UiTxt* Ui::TextLoc(const String& key, float width)
	{
		auto text = Text(*Assets::Get<Textual>(key), width);
		text->keyLoc = key;
		text->langSignal.Listen(&Ui::OnTxtLang, Instance().get());
		Assets::Instance()->langSignal.Listen(&UiTxt::OnLang, text);
		return text;
	}
	void Ui::CreateText(UiTxt* text, std::string_view utf8, float width)
	{
		if (text->root.Valid())
			text->root.Destroy();
		float limit = 8888888.f;
		if (text->parent != nullptr)
		{
			limit = text->parent->sizeLimit.x - text->parent->context.padding.x * 2;
		}
		if (limit < 8888888.f)
			//There is a size limit
			width = width > 0.f ? std::min(limit, width) : limit;
		else
			//There is no size limit
			width = width > 0.f ? std::min(limit, width) : 0.f;
		String lang = Assets::GetLang();
		auto font = m_writer->GetFont(text->context.fontName, lang);
		text->sentence = m_writer->Write
		(
			utf8,
			font->id,
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

	void Ui::UpdateBtn(UiBtn* button, std::string_view utf8)
	{
		auto i = Instance();
		auto lang = Assets::GetLang();
		auto font = i->m_writer->GetFont(button->context.fontName, lang);
		button->label.root.Destroy();
		button->label = i->m_writer->Write(utf8, font->id, button->context.textColor, font->material, font->layer, 0.f, TextAlign::Center, 1.f);
		button->label.root.Get<Transform>()->Move(button->context.padding.x, -button->context.padding.y, -3.f);
		button->root.AddChild(button->label.root);
		button->size.x = button->label.size.x + button->context.padding.x * 2;
		button->size.y = button->label.size.y + button->context.padding.y * 2;
		button->UpdateFrames();
		button->parent->MustUpdate();
	}
	UiImg* Ui::Image(Sprite* sprite)
	{
		auto i = Instance();
		/*ASSERT_LOG_ERROR(!i->m_canvas.empty(), "Trying to create Ui Text without active canvas");
		auto canvas = i->m_canvas.top();*/
		UiCanvas* parent = nullptr;
		if (!i->m_canvas.empty())
			parent = i->m_canvas.top();

		UiImg* image = new UiImg();
		image->margin = i->m_context.margin;
		image->root = Entity::Create();
		image->root.Add<Transform>();
		image->root.Add<SpriteRender>();
		auto render = image->root.Get<SpriteRender>();

		render->SetSprite(sprite);
		render->SetLayer(i->m_context.layer);
		render->SetMaterial(i->m_context.material->GetID());
		auto spr = render->GetSprite();
		image->sprite = spr;
		image->size = image->sprite->GetDimensions();

		i->NewElem(image, parent);
		return image;
	}
	UiImg* Ui::Image(String sprite)
	{
		return Image(Assets::Get<Sprite>(sprite));
	}

	UiBtn* Ui::Button(std::string_view utf8)
	{
		auto i = Instance();

		UiCanvas* parent = nullptr;
		if (!i->m_canvas.empty())
			parent = i->m_canvas.top();

		UiBtn* button = new UiBtn();
		button->root = Entity::Create();
		button->root.Add<Transform>();
		button->context = i->m_context.button;
		button->margin = i->m_context.margin;
		auto lang = Assets::GetLang();
		auto font = i->m_writer->GetFont(button->context.fontName, lang);
		button->label = i->m_writer->Write(utf8, font->id, button->context.textColor, font->material, font->layer, 0.f, TextAlign::Center, 1.f);
		button->label.root.Get<Transform>()->Move(button->context.padding.x, -button->context.padding.y, -4.f);
		button->root.AddChild(button->label.root);
		button->size.x = button->label.size.x + button->context.padding.x * 2;
		button->size.y = button->label.size.y + button->context.padding.y * 2;

		ASSERT_LOG_ERROR((button->context.frameIdle != nullptr), "Trying to create a button without at least a frame idle context.");
		button->frameIdle = UiFrame(
			button->context.frameIdle,
			i->m_context.material,
			i->m_context.layer);
		button->frameHover = UiFrame(
			button->context.frameHover == nullptr ? button->context.frameIdle : button->context.frameHover,
			i->m_context.material,
			i->m_context.layer);
		button->framePressed = UiFrame(
			button->context.framePressed == nullptr ? button->context.frameIdle : button->context.framePressed,
			i->m_context.material,
			i->m_context.layer);
		button->frameDisabled = UiFrame(
			button->context.frameDisabled == nullptr ? button->context.frameIdle : button->context.frameDisabled,
			i->m_context.material,
			i->m_context.layer);
		i->NewElem(button, parent);
		button->UpdateFrames();
		RegisterHoverable(button);
		return button;
	}

	UiBtn* Ui::ButtonLoc(const String& key)
	{
		auto btn = Button(*Assets::Get<Textual>(key));
		btn->keyLoc = key;
		btn->langSignal.Listen(&Ui::OnBtnLang, Instance().get());
		Assets::Instance()->langSignal.Listen(&UiBtn::OnLang, btn);
		return btn;
	}

	UiAnimBtn* Ui::AnimButton(std::string_view utf8)
	{
		auto i = Instance();

		UiCanvas* parent = nullptr;
		if (!i->m_canvas.empty())
			parent = i->m_canvas.top();

		UiAnimBtn* button = new UiAnimBtn();
		button->root = Entity::Create();
		button->root.Add<Transform>();
		button->root.Add<SpriteRender>();
		auto render = button->root.Get<SpriteRender>();
		render->SetLayer(i->m_context.layer);
		render->SetMaterial(i->m_context.material->GetID());
		auto animator = button->root.AddGet<Animator>();
		auto& animContext = i->m_context.animButton;
		ASSERT_LOG_ERROR((animContext.idle != nullptr), "Trying to create anim button without at least an idle animation context.");
		animator->AddAnimation("idle", animContext.idle);
		animator->AddAnimation("hover", animContext.hover == nullptr ? animContext.idle : animContext.hover);
		animator->AddAnimation("pressed", animContext.pressed == nullptr ? animContext.idle : animContext.pressed);
		animator->AddAnimation("disabled", animContext.disabled == nullptr ? animContext.idle : animContext.disabled);
		animator->SetAnimation("idle");
		render->SetSprite(animator->currentState->animation->frames[0].sprite);
		button->context = i->m_context.button;
		button->animContext = animContext;
		button->margin = i->m_context.margin;
		auto lang = Assets::GetLang();
		auto font = i->m_writer->GetFont(button->context.fontName, lang);
		button->label = i->m_writer->Write(utf8, font->id, button->context.textColor, font->material, font->layer, 0.f, TextAlign::Center, 1.f);
		button->label.root.Get<Transform>()->Move(button->context.padding.x, -button->context.padding.y, -4.f);
		button->root.AddChild(button->label.root);
		button->size = animContext.idle->frames[0].sprite->GetDimensions();

		i->NewElem(button, parent);

		RegisterHoverable(button);
		return button;
	}

	UiAnimBtn* Ui::AnimButtonLoc(const String& key)
	{
		auto btn = AnimButton(*Assets::Get<Textual>(key));
		btn->keyLoc = key;
		btn->langSignal.Listen(&Ui::OnBtnLang, Instance().get());
		Assets::Instance()->langSignal.Listen(&UiBtn::OnLang, (UiBtn*)btn);
		return btn;
	}

	UiCheckbox* Ui::Checkbox(bool* value)
	{
		auto i = Instance();
		/*ASSERT_LOG_ERROR(!i->m_canvas.empty(), "Trying to create Ui Text without active canvas");
		auto canvas = i->m_canvas.top();*/
		UiCanvas* parent = nullptr;
		if (!i->m_canvas.empty())
			parent = i->m_canvas.top();
		ASSERT_LOG_ERROR((i->m_context.checkbox.texture != ""), "Trying to create Checkbox without checkbox sprite");

		auto checkbox = new UiCheckbox();
		checkbox->root = Entity::Create();
		checkbox->value = value;
		checkbox->root.Add<Transform>();
		for (int j = 0; j < 3; j++)
		{
			String spriteName = i->m_context.checkbox.texture + "_0_" + std::to_string(j);
			auto entt = Entity::CreateSprite(spriteName);
			auto rd = entt.Get<SpriteRender>();
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
		auto rd = checkbox->sprites[0].Get<SpriteRender>();
		auto spr = rd->GetSprite();
		checkbox->size = spr->GetDimensions();
		checkbox->margin = i->m_context.margin;
		checkbox->clickable = true;
		i->NewElem(checkbox, parent);
		checkbox->UpdateVisual();
		RegisterHoverable(checkbox);
		if (value != nullptr)
		{
			checkbox->SetChecked(*value);
		}
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

	void Ui::Destroy(UiElem*& elem)
	{
		Instance()->DestroyHelper(elem);
		elem = nullptr;
	}

	void Ui::Destroy(UiCanvas*& elem)
	{
		Instance()->DestroyHelper(static_cast<UiElem*>(elem));
		elem = nullptr;
	}

	void Ui::Destroy(UiTxt*& elem)
	{
		Instance()->DestroyHelper(static_cast<UiElem*>(elem));
		elem = nullptr;
	}

	void Ui::Destroy(UiImg*& elem)
	{
		Instance()->DestroyHelper(static_cast<UiElem*>(elem));
		elem = nullptr;
	}

	void Ui::Destroy(UiBtn*& elem)
	{
		Instance()->DestroyHelper(static_cast<UiElem*>(elem));
		elem = nullptr;
	}

	void Ui::Destroy(UiCheckbox*& elem)
	{
		Instance()->DestroyHelper(static_cast<UiElem*>(elem));
		elem = nullptr;
	}

	/*---Context---*/

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
	void Ui::SetOrder(float z)
	{
		Instance()->m_context.z = z;
	}
	void Ui::SetInteractionGroup(int group)
	{
		auto i = Instance();
		i->m_context.interactionGroup = group;
		auto f_it = i->m_interactionGroups.find(group);
		if (f_it == i->m_interactionGroups.end())
			i->m_interactionGroups[group] = true;

	}
	void Ui::SetMaterial(Material* material)
	{
		material->SetFloat("uPpu", Instance()->m_ppu * 2.f);
		Instance()->m_context.material = material;
	}
	void Ui::SetTextFont(String fancyName)
	{
		auto ins = Instance();
		auto lang = Assets::GetLang();
		ins->m_writer->GetFont(fancyName, lang); // for error message if font doesn't exists.
		ins->m_context.text.fontName = fancyName;
	}
	void Ui::SetButtonFont(String fancyName)
	{
		auto ins = Instance();
		auto lang = Assets::GetLang();
		ins->m_writer->GetFont(fancyName, lang); // for error message if font doesn't exists.
		ins->m_context.button.fontName = fancyName;
	}
	void Ui::SetTextColor(Color color)
	{
		Instance()->m_context.text.color = color;
	}
	void Ui::SetButtonTextColor(Color color)
	{
		Instance()->m_context.button.textColor = color;
	}
	void Ui::SetButtonDisabledTextColor(Color color)
	{
		Instance()->m_context.button.textColorDisabled = color;
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
	void Ui::SetSpacing(Vec2f spacing)
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
	void Ui::SetButtonFrameDisabled(String texture)
	{
		Instance()->SetFrame(&Instance()->m_context.button.frameDisabled, texture);
	}
	void Ui::SetButtonPressSound(Sound* sound)
	{
		Instance()->m_context.button.pressSound = sound;
	}
	void Ui::SetButtonReleaseSound(Sound* sound)
	{
		Instance()->m_context.button.releaseSound = sound;
	}
	void Ui::SetAnimBtnIdle(Animation* anim)
	{
		Instance()->m_context.animButton.idle = anim;
	}
	void Ui::SetAnimBtnPressed(Animation* anim)
	{
		Instance()->m_context.animButton.pressed = anim;
	}
	void Ui::SetAnimBtnHover(Animation* anim)
	{
		Instance()->m_context.animButton.hover = anim;
	}
	void Ui::SetAnimBtnDisabled(Animation* anim)
	{
		Instance()->m_context.animButton.disabled = anim;
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
	void Ui::EnableGroup(int group)
	{
		auto i = Instance();
		auto f_it = i->m_interactionGroups.find(group);
		if (f_it == i->m_interactionGroups.end())
		{
			LOG_ERROR("EnableGroup, interaction group {0}, doesn't exists");
			return;
		}
		f_it->second = true;
		i->OnEnable(group);
	}

	void Ui::OnDisable(int group)
	{
		for (int j = 0; j < m_hoverables.size(); j++)
		{
			auto& hoverable = m_hoverables[j];
			if (hoverable->interactionGroup == group)
			{
				hoverable->Disable();
			}
		}
	}
	void Ui::OnEnable(int group)
	{
		for (int j = 0; j < m_hoverables.size(); j++)
		{
			auto& hoverable = m_hoverables[j];
			if (hoverable->interactionGroup == group)
			{
				hoverable->Enable();
			}
		}
	}
	void Ui::DisableGroup(int group)
	{
		auto i = Instance();
		auto f_it = i->m_interactionGroups.find(group);
		if (f_it == i->m_interactionGroups.end())
		{
			LOG_ERROR("DisableGroup, interaction group {0}, doesn't exists");
			return;
		}
		f_it->second = false;
		i->OnDisable(group);

	}
	void Ui::EnableOnlyGroup(int group)
	{
		auto i = Instance();
		auto f_it = i->m_interactionGroups.find(group);
		if (f_it == i->m_interactionGroups.end())
		{
			LOG_ERROR("EnableOnlyGroup, interaction group {0}, doesn't exists");
		}
		else
		{
			f_it->second = true;
			i->OnEnable(f_it->first);
		}
		for (auto& g : i->m_interactionGroups)
		{
			if (g.first != group)
			{
				g.second = false;
				i->OnDisable(g.first);
			}
		}
	}
	void Ui::EnableAllGroups()
	{
		auto i = Instance();
		for (auto& g : i->m_interactionGroups)
		{
			g.second = true;
			i->OnEnable(g.first);
		}
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
	String Ui::GetFont()
	{
		return Instance()->m_context.text.fontName;
	}
	LayerID Ui::GetLayer()
	{
		return Instance()->m_context.layer;
	}

	bool Ui::IsGroupEnabled(int group)
	{
		auto i = Instance();
		auto f_it = i->m_interactionGroups.find(group);
		if (f_it == i->m_interactionGroups.end())
			return false;
		return f_it->second;
	}

	std::unordered_map<UiElem::ID, UiCanvas*> Ui::GetCanvases()
	{
		return Instance()->m_roots;
	}

	/*---Helpers---*/

	void Ui::DestroyHelper(UiElem* elem)
	{
		if (elem == nullptr)
			return;
		if (elem->IsDestroyed())
			return;
		elem->destroyed = true;
		Instance()->m_destroy.emplace_back(elem);
	}

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