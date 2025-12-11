#pragma once
#include <stack>
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Render/Writer.h"
#include "SandCastle/UI/UiTxt.h"
#include "SandCastle/UI/UiFrame.h"
#include "SandCastle/UI/UiEnum.h"
#include "SandCastle/UI/UiContext.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/Core/Textual.h"

namespace SandCastle
{
	class UiSystem;
	class UiCanvas;
	class UiElem;
	class UiImg;
	class UiBtn;
	class UiAnimBtn;
	class UiCheckbox;
	struct InputSignal;

	class Ui : public Singleton<Ui>
	{
	public:
		Ui();
		~Ui();

		/*---Initialization---*/

		/// @brief Create the sprites and textures needed to be used later for any type of frames (canvas, buttons...)
		/// The texture MUST already have 3x3 sprites (use the .texture file to set up properly).
		/// @param fixedStep Set true if your texture has a repeating pattern that must be consistent.
		/// The frame size will be constrainted to increase/decrease by stepped increment, according to the sprites size.
		static void MakeFrameTemplate(const String& texture, bool fixedStep = false);
		/// @brief Create a font that will be available for usage later at any time.
		/// @param filename font filename
		/// @param fancyName identification name of easy usage
		/// @param size font height in uiSize 
		/// (by default, ui full height is 360.f
		/// so a font of size 180.f would take half the screen.)
		static void MakeFont(String filename,
			String fancyName,
			float uiSize,
			float scale = 1.f,
			float lineHeight = 1.f,
			std::vector<String> langs = {},
			float outlineThickness = 0.f,
			Vec4f outlineColor = { 0,0,0,1 });
		/// @brief Override the default material.
		static void DefaultMaterial(Material* material);
		/// @brief Set the ui dimension.
		/// example, if ppu = 360 (default), an ui element of height 360 will fill up the screen.
		static void PPU(float ppu);

		/*---Ui creation---*/

		static UiCanvas* Begin(Vec2f size = { 0, 0 }, bool frame = true);
		static UiTxt* Text(std::string_view utf8, float width = -1.f)
		{
			auto i = Instance();
			/*ASSERT_LOG_ERROR(!i->m_canvas.empty(), "Trying to create Ui Text without active canvas");
			auto canvas = i->m_canvas.top();*/
			UiCanvas* parent = nullptr;
			if (!i->m_canvas.empty())
				parent = i->m_canvas.top();

			//Instantiation
			UiTxt* text = new UiTxt();
			i->NewElem(text, parent);
			text->context = i->m_context.text;
			text->utf8 = utf8;
			i->CreateText(text, utf8, width);

			return text;
		}
		static UiTxt* TextLoc(const String& utf8, float width = -1.f);
		template <typename... Ts>
		static UiTxt* Text(std::string_view utf8, float width = -1.f, Ts... args);
		template <typename... Ts>
		static UiTxt* TextLoc(const String& key, float width = -1.f, Ts... args);
		static UiImg* Image(String sprite);
		static UiBtn* Button(std::string_view utf8);
		static UiBtn* ButtonLoc(const String& key);
		static UiAnimBtn* AnimButton(std::string_view utf8);
		static UiAnimBtn* AnimButtonLoc(const String& key);
		static UiCheckbox* Checkbox(bool* value = nullptr);
		static void End();
		static void Destroy(UiElem*& elem);
		static void Destroy(UiCanvas*& elem);
		static void Destroy(UiTxt*& elem);
		static void Destroy(UiImg*& elem);
		static void Destroy(UiBtn*& elem);
		static void Destroy(UiCheckbox*& elem);

		/*---Ui update---*/

		static void UpdateText(UiTxt* text, std::string_view utf8, bool replaceUtf8 = true);
		static void UpdateBtn(UiBtn* button, std::string_view utf8);

		/*---Context---*/
		/// @brief Create a snapshot of the current context for later usage.
		static void SnapshotContext(String name);
		/// @brief Set the context for every subsequent canvas/element creation.
		/// The context must have been snapshoted with SnapshotContext.
		static void Context(String name);
		/// @brief Lower is in front of higher.
		/// @param z position for visual sorting.
		static void SetOrder(float z);
		/// @brief Later you can enable/disable interaction with specific groups
		static void SetInteractionGroup(int group);
		/// @brief Context setting.
		/// Set the material that will be used for ui creation
		static void SetMaterial(Material* material);
		/// @brief Context setting.
		/// Set the font that will be used for text creation
		static void SetTextFont(String fancyName);
		/// @brief Context setting.
		/// Set the font that will be used for button creation
		static void SetButtonFont(String fancyName);
		/// @brief Context setting.
		/// Set the color that will be sued for text creation
		static void SetTextColor(Color color);
		/// @brief Context setting.
		/// Set the text color that will be used for button creation 
		static void SetButtonTextColor(Color color);
		/// @brief Context setting.
		/// Set the text color that will be used for disabled button
		static void SetButtonDisabledTextColor(Color color);
		/// @brief Context setting.
		/// Set the padding (space between button text, and button edges)
		/// that will be used for  button creation.
		static void SetButtonPadding(Vec2f padding);
		/// @brief Context setting.
		/// Set the layer that will be used for ui creation
		static void SetLayer(LayerID layer);
		/// @brief Context setting.
		/// Set the frame that will be used for canvas creation
		static void SetCanvasFrame(String texture);
		/// @brief Context setting.
		/// Set the padding for  canvas creation
		static void SetCanvasPadding(Vec2f padding);
		/// @brief Context setting.
		/// Set spacing between element inside canvases.
		static void SetSpacing(Vec2f spacing);
		/// @brief Context setting.
		/// Set the layout direction for  canvas creation
		static void SetCanvasLayoutDir(LayoutDir dir);
		/// @brief Context setting.
		/// Set the horizontal alignement of element inside canvases
		static void SetCanvasLayoutAlignH(LayoutAlign alignH);
		/// @brief Context setting.
		/// Set the vertical alignement of element inside canvases
		static void SetCanvasLayoutAlignV(LayoutAlign alignV);
		/// @brief Context setting.
		/// Set the frame that will be used for button creation
		static void SetButtonFrame(String texture);
		/// @brief Context setting.
		/// Set the frame that will be used for button creation
		static void SetButtonFrameHover(String texture);
		/// @brief Context setting.
		/// Set the frame that will be used for  button creation
		static void SetButtonFramePressed(String texture);
		static void SetButtonFrameDisabled(String texture);
		static void SetButtonPressSound(Sound* sound);
		static void SetButtonReleaseSound(Sound* sound);
		static void SetAnimBtnIdle(Animation* anim);
		static void SetAnimBtnPressed(Animation* anim);
		static void SetAnimBtnHover(Animation* anim);
		static void SetAnimBtnDisabled(Animation* anim);
		/// @brief Context setting.
		/// The texture must have 3 horizontal sprites.
		/// respecting this order: unchecked, hovered, checked
		static void SetCheckboxSprites(String texture);
		/// @brief Context setting.
		/// Set the text alignement that will be used for  text creation
		static void SetTextAlign(TextAlign textAlign);
		/// @brief Context setting.
		/// Set the margin that will be used for  element creation.
		static void SetMargin(Vec2f margin);
		/// @brief Context setting.
		// Set the margin that will be used for every root canvas (non nested canvas).
		static void SetRootMargin(Vec2f margin);
		/// @brief Context setting.
		/// Set the anchor for the root canvases.
		/// The anchor of every other UiElem including UiCanvas is always top left.
		/// This applies only for non-nested canvases (canvases without parent).
		static void SetRootAnchor(CanvasAnchor anchor);
		/// @brief Set the material to default material.
		static void ResetMaterial(Material* material);

		/*---Utility---*/

		/// @brief Enable an interaction group
		/// @param group 
		static void EnableGroup(int group);
		/// @brief Disable an interaction group
		/// @param group 
		static void DisableGroup(int group);
		/// @brief Keep only one interaction group, disable all others
		/// @param group 
		static void EnableOnlyGroup(int group);
		/// @brief Enable all interaction group
		static void EnableAllGroups();
		/// @brief Convert UI position to world position
		static Vec3f UiToWorld(Vec2f uiPos);
		/// @brief Convert world position to UI position
		static Vec2f WorldToUi(Vec3f worldPos);
		/// @brief Convert a screen position to UI position
		static Vec2f ScreenToUi(Vec2f screenPos);
		/// @brief Get the mouse UI position
		static Vec2f MousePos();
		static void RegisterHoverable(UiElem* elem);

		/*---Accessors---*/

		/// @brief Get the object used to create fonts and write text.
		/// It could be used for non UI stuff.
		static Writer* GetWriter();
		/// @brief Get the material currently used 
		static Material* GetMaterial();
		/// @brief Get the font currently used
		static String GetFont();
		/// @brief Get the layer currently used
		static LayerID GetLayer();
		/// @brief Get all the root canvases.
		static std::unordered_map<UiElem::ID, UiCanvas*> GetCanvases();
	private:

		/*---Helpers---*/
		void OnDisable(int group);
		void OnEnable(int group);
		void DestroyHelper(UiElem* elem);
		void SetFrame(UiFrame::Template** frame, String texture);
		static void MakeBorderTex(String texture, std::vector<Texture*>& tex);

		void CreateText(UiTxt* text, std::string_view utf8, float width);
		/*---Instantiation---*/
		void NewElem(UiElem* elem, UiCanvas* canvas);

	private:
		friend Systems;
		bool OnEvent(SDL_Event& event);
		void Update();
		void LayoutUpdate();
		void DestroyUpdate();
		void HoverableUpdate();
		void ValuesUpdate();
		bool OnClick(bool pressed);
		void OnCanvasMustUpdate(UiCanvas* canvas);
		void OnDestroy(UiElem* elem);
		void OnTxtLang(UiTxt* txt);
		void OnBtnLang(UiBtn* btn);
		//Helper
		Writer* m_writer = nullptr;

		//Data
		std::unordered_map<String, UiFrame::Template> m_frameTemplates;
		std::unordered_map<String, UiContext> m_contextSnapshots;

		//State (creation of new elements)
		Material* m_defaultMaterial = nullptr;
		UiContext m_context;

		//Runtime
		std::vector<UiElem*> m_hoverables;
		std::vector<UiTxt*> m_values;
		std::vector<UiElem*> m_destroy;
		std::unordered_map<UiElem::ID, UiCanvas*> m_roots;
		std::vector<UiCanvas*> m_layoutUpdate;
		UiElem* m_hovered = nullptr;
		UiElem* m_pressed = nullptr;
		std::stack<UiCanvas*> m_canvas;
		float m_ppu = 1.f / 360.f;
		static UiElem::ID m_nextId;
		const float zStep = 1.f;
		std::unordered_map<int, bool> m_interactionGroups;

	};
	template<typename ...Ts>
	inline UiTxt* Ui::Text(std::string_view utf8, float width, Ts ...args)
	{
		auto i = Instance();
		/*ASSERT_LOG_ERROR(!i->m_canvas.empty(), "Trying to create Ui Text without active canvas");
		auto canvas = i->m_canvas.top();*/
		UiCanvas* parent = nullptr;
		if (!i->m_canvas.empty())
			parent = i->m_canvas.top();

		//Instantiation
		UiTxt* text = new UiTxt();
		text->AddData(args...);
		text->context = i->m_context.text;
		text->utf8 = utf8;
		i->NewElem(text, parent);
		i->CreateText(text, text->Format(), width);
		i->m_values.emplace_back(text);

		return text;
	}
	template<typename ...Ts>
	inline UiTxt* Ui::TextLoc(const String& key, float width, Ts ...args)
	{
		UiTxt* text = Ui::Text(*Assets::Get<Textual>(key), width, args...);
		text->keyLoc = key;
		text->langSignal.Listen(&Ui::OnTxtLang, Instance().get());
		Assets::Instance()->langSignal.Listen<UiTxt>(&UiTxt::OnLang, text);
		return text;
	}
}