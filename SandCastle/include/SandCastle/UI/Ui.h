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
#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/Core/Textual.h"
#include "SandCastle/ECS/Entity.h"

namespace SandCastle
{
	class UiSystem;
	class UiCanvas;
	class UiImg;
	class UiBtn;
	class UiAnimBtn;
	class UiCheckbox;
	class UiLoadBar;
	class Sprite;
	class InputSignal;

	/// @brief Payload broadcast when an interaction group is enabled or disabled.
	struct UiGroupSignal
	{
		bool ennabled = false;
		int group = 0;
	};
	/// @brief UI singleton: builds canvases, text, buttons and other widgets,
	/// owns the styling context stack (fonts, colors, frame templates), runs
	/// the hover/click hit-testing, and dispatches localization signals.
	/// All ui-creation methods are static and operate on the singleton.
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

		/// @brief Open a new canvas (a layout container). Children created until the matching End() will be parented to this canvas.
		/// @param size Override size; (0,0) auto-sizes to fit children.
		/// @param frame Draw the background frame (using the current canvas frame template).
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
		/// @brief Localized text: looks up the Textual asset under `key` and updates automatically when the language changes.
		static UiTxt* TextLoc(const String& utf8, float width = -1.f);
		/// @brief Text bound to live data via std::format placeholders. Each arg must be a pointer to the value to display; the text re-formats whenever a value changes.
		template <typename... Ts>
		static UiTxt* Text(std::string_view utf8, float width = -1.f, Ts... args);
		/// @brief Localized + data-bound text. Combines TextLoc and the formatted Text overload.
		template <typename... Ts>
		static UiTxt* TextLoc(const String& key, float width = -1.f, Ts... args);
		/// @brief Image element loading the sprite from the Assets store by name.
		static UiImg* Image(String sprite);
		/// @brief Image element using an existing Sprite pointer.
		static UiImg* Image(Sprite* sprite);
		/// @brief Button with a text label.
		static UiBtn* Button(std::string_view utf8);
		/// @brief Button whose label is localized through Assets.
		static UiBtn* ButtonLoc(const String& key);
		/// @brief Animated button (sprite-based, no frame).
		static UiAnimBtn* AnimButton(std::string_view utf8);
		/// @brief Localized animated button.
		static UiAnimBtn* AnimButtonLoc(const String& key);
		/// @brief Checkbox optionally bound to an external bool*.
		static UiCheckbox* Checkbox(bool* value = nullptr);
		/// @brief Progress bar widget. `current` and `goal` set the initial value.
		static UiLoadBar* LoadBar(Vec2f size, double goal = 1.0, double current = 0.0);
		/// @brief Close the canvas opened by the most recent Begin().
		static void End();
		static void Destroy(UiElem*& elem);
		static void Destroy(UiCanvas*& elem);
		static void Destroy(UiTxt*& elem);
		static void Destroy(UiImg*& elem);
		static void Destroy(UiBtn*& elem);
		static void Destroy(UiAnimBtn*& elem);
		static void Destroy(UiCheckbox*& elem);
		static void Destroy(UiLoadBar*& elem);

		/*---Ui update---*/

		static void UpdateText(UiTxt* text, std::string_view utf8, bool replaceUtf8 = true);
		static void UpdateBtn(UiBtn* button, std::string_view utf8);
		static void UpdateLoadBar(UiLoadBar* loadBar, double current, double goal);

		/*---Context---*/
		/// @brief Create a snapshot of the current context for later usage.
		static void SnapshotContext(String name);
		/// @brief Create a snapeshot of the current button context for later usage.
		static void SnapshotBtnContext(String name);
		/// @brief Set the context for every subsequent canvas/element creation.
		/// The context must have been snapshoted with SnapshotContext.
		static void Context(String name);
		/// @brief Set the button context for every subsequent canvas/element creation.
		/// The context must have been snapshoted with SnapshotBtnContext.
		static void BtnContext(String name);
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
		/// Set the contour frame for load bar creation
		static void SetLoadBarFrameContour(String texture);
		/// @brief Context setting.
		/// Set the filling frame for load bar creation
		static void SetLoadBarFrameFilling(String texture);
		/// @brief Context setting.
		/// Set the margin between contour and filling in load bars
		static void SetLoadBarFillingMargin(Vec2f margin);
		/// @brief Context setting.
		/// Set the color of the filling frame in load bars
		static void SetLoadBarFillingColor(Color color);
		/// @brief Context setting.
		/// Set the text display mode for load bars (None, Percent, ValueGoal)
		static void SetLoadBarTextMode(LoadBarTextMode mode);
		/// @brief Context setting.
		/// Set the font for load bar text
		static void SetLoadBarFont(String fancyName);
		/// @brief Context setting.
		/// Set the text color for load bar text
		static void SetLoadBarTextColor(Color color);
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

		/*---Gamepad navigation---*/

		/// @brief Set the texture used to draw the gamepad-navigation selector ring.
		/// The texture must split into exactly 4 sprites (a 2x2 grid via its .texture
		/// file). Logs an error and does nothing if the texture is missing or has the
		/// wrong layout. The 4 sprites are placed at the four corners of the
		/// currently navigated UiElem; the texture's natural cell layout maps to
		/// position (top row → top corners, bottom row → bottom corners).
		/// `margin` is the gap (in UI units) between the navigated element and the
		/// selector ring on each side.
		static void SetGamepadSelector(const String& texture, Vec2f margin = Vec2f(2.f, 2.f));
		/// @brief Move the selector ring onto `elem`. Pass nullptr to clear.
		/// Same effect as calling elem->Navigate(). The selector is only drawn when
		/// gamepad/keyboard was the last input used.
		static void SetNavigated(UiElem* elem);
		/// @brief The currently-navigated UiElem, or nullptr.
		static UiElem* GetNavigated();
		/// @brief When true, listeners registered via ListenClickPressed/Released
		/// also fire when Select is pressed/released on the navigated element.
		/// Off by default.
		static void ClickIsSelect(bool enabled);
		/// @brief Current ClickIsSelect setting (used internally by UiElem dispatch).
		static bool IsClickIsSelect();

		/// @brief Send a Left navigation step to the navigated UiElem. Designed
		/// to be wired to a ButtonInput via input->signal.Listen(&Ui::NavigateLeft).
		/// On press: moves the selector to the navigated element's Left nav target
		/// (if any) and fires its ListenNavPressed(Left) listeners. On release:
		/// fires ListenNavReleased(Left).
		static void NavigateLeft(InputSignal* signal);
		static void NavigateRight(InputSignal* signal);
		static void NavigateUp(InputSignal* signal);
		static void NavigateDown(InputSignal* signal);
		/// @brief Send a Select press/release to the navigated UiElem. Reads the
		/// pressed/released state from the InputSignal so the input should be
		/// configured with both SetSignalOnPress(true) and SetSignalOnRelease(true).
		static void Select(InputSignal* signal);
		/// @brief Send a Cancel press/release to the navigated UiElem.
		static void Cancel(InputSignal* signal);

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
		static void ChangeFrame(UiCanvas* canvas, String frame);

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
		static bool IsGroupEnabled(int group);
		static Signal<UiGroupSignal>* GetGroupSignal();
		/// @brief Get all the root canvases.
		static std::unordered_map<UiElem::ID, UiCanvas*> GetCanvases();
		static UiContext GetContext();
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
		void SelectorUpdate();
		bool OnClick(bool pressed);
		void OnCanvasMustUpdate(UiCanvas* canvas);
		void OnDestroy(UiElem* elem);
		void OnTxtLang(UiTxt* txt);
		void OnBtnLang(UiBtn* btn);
		void NavigateInDir(NavDir dir, bool pressed);
		void RebuildSelector();
		void DestroySelector();
		//Helper
		Writer* m_writer = nullptr;

		//Data
		std::unordered_map<String, UiFrame::Template> m_frameTemplates;
		std::unordered_map<String, UiContext> m_contextSnapshots;
		std::unordered_map<String, ButtonContext> m_btnContextSnapshots;

		//State (creation of new elements)
		Material* m_defaultMaterial = nullptr;
		UiContext m_context;

		//Runtime
		std::vector<UiElem*> m_hoverables;
		std::vector<UiTxt*> m_values;
		std::vector<UiElem*> m_destroy;
		std::unordered_map<UiElem::ID, UiCanvas*> m_roots;
		std::vector<UiCanvas*> m_layoutUpdate;
		std::unordered_map<UiElem::ID, UiElem*> m_hovered;
		std::unordered_map<UiElem::ID, UiElem*> m_pressed;
		std::stack<UiCanvas*> m_canvas;
		float m_ppu = 1.f / 360.f;
		static UiElem::ID m_nextId;
		const float zStep = 1.f;
		std::unordered_map<int, bool> m_interactionGroups;

		//Signal
		Signal<UiGroupSignal> uiGroupSignal;

		//Gamepad selector
		Sprite* m_selectorCorners[4] = { nullptr, nullptr, nullptr, nullptr };
		Vec2f m_selectorMargin = Vec2f(2.f, 2.f);
		UiElem* m_navigated = nullptr;
		Entity m_selectorEntity;
		Vec3f m_selectorLastPos = Vec3f(0.f, 0.f, 0.f);
		//The navigated element's LAID-OUT size, which is what the ring is centred on.
		//UiElem::selectorScale is tracked beside it rather than folded in: it grows the
		//ring about that centre, so folding it into the size would drag the centre.
		Vec2f m_selectorLastSize = Vec2f(0.f, 0.f);
		float m_selectorLastScale = 1.f;
		float m_selectorBlinkElapsed = 0.f;
		bool m_selectorTextureValid = false;
		bool m_clickIsSelect = false;
		bool m_selectorWasGamepadMode = false;
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
		auto loc = Assets::Get<Textual>(key);
		UiTxt* text = Ui::Text(loc != nullptr ? *loc : key, width, args...);
		text->keyLoc = key;
		text->langSignal.Listen(&Ui::OnTxtLang, Instance().get());
		Assets::Instance()->langSignal.Listen<UiTxt>(&UiTxt::OnLang, text);
		return text;
	}
}