#pragma once
#include <stack>
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Render/Writer.h"
#include "SandCastle/UI/UiTxt.h"
#include "SandCastle/UI/UiFrame.h"
#include "SandCastle/Core/Container.h"

namespace SandCastle
{
	class UiSystem;
	class UiCanvas;
	class UiElem;
	class UiImg;
	class UiBtn;
	struct InputSignal;

	class Ui : public Singleton<Ui>
	{
	public:

		/*---Elements---*/

		/*/// @brief base class of every Ui objects.
		class Elem;
		/// @brief Container for every Ui objects, can contain other canvas.
		class Canvas;
		/// @brief Text
		class Txt;
		/// @brief Image (sprite)
		class Img;
		class Btn;*/

		/*--- Types ---*/
		//definitions in UiEnum.h


		typedef uint32_t FrameID;
		enum class LayoutDir;
		enum class LayoutWrap;
		enum class LayoutAlignH;
		enum class LayoutAlignV;
		enum class TexBorder;
		enum class SpriteCorner;

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
			float outlineThickness = 0.f,
			Vec4f outlineColor = { 0,0,0,1 });
		/// @brief Override the default material.
		static void SetDefaultMaterial(Material* material);
		/// @brief Set the ui dimension.
		/// example, if ppu = 360 (default), an ui element of height 360 will fill up the screen.
		static void SetPPU(float ppu);

		/*---Ui creation---*/

		static UiCanvas* BeginCanvas(Vec2f size = { 0, 0 }, bool frame = true);
		static UiTxt* Text(std::string_view utf8, float width = -1.f);
		template <typename... Ts>
		static UiTxt* Text(std::string_view utf8, float width = -1.f, Ts... args);
		static UiImg* Image(String sprite);
		static UiBtn* Button(std::string_view utf8, Vec2f padding);
		static void EndCanvas();
		/// @brief Only root canvas can be deleted.
		static void Destroy(UiElem* elem);

		/*---Ui update---*/

		static void UpdateText(UiTxt* text, std::string_view utf8, bool replaceUtf8 = true);

		/*---State---*/

		/// @brief Set the material that will be used for every subsequent ui creation
		static void SetMaterial(Material* material);
		/// @brief Set the font that will be used for every subsequent ui creation
		static void SetFont(FontID font);
		/// @brief Set the font that will be used for every subsequent ui creation
		static void SetFont(String fancyName);
		/// @brief Set the color that will be sued for every subsequent text creation (including buttons)
		static void SetTextColor(Color color);
		/// @brief Set the layer that will be used for every subsequent ui creation
		static void SetLayer(LayerID layer);
		/// @brief Set the frame that will be used for every subsequent canvas creation
		static void SetCanvasFrame(String texture);
		/// @brief Set the frame that will be used for every subsequent button creation
		static void SetButtonFrame(String texture);
		/// @brief Set the frame that will be used for every subsequent button creation
		static void SetButtonFrameHover(String texture);
		/// @brief Set the frame that will be used for every subsequent button creation
		static void SetButtonFramePressed(String texture);
		/// @brief Set the text alignement that will be used for every subsequent text creation
		static void SetTextAlign(TextAlign textAlign);
		/// @brief Set the margin that will be used for every subsequent element creation.
		static void SetMargin(Vec2f margin);
		/// @brief Set the margin that will be used for every root canvas (non nested canvas).
		static void SetRootMargin(Vec2f margin);
		/// @brief Set the material to default material.
		static void ResetMaterial(Material* material);

		/*---Utility---*/

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
		static FontID GetFont();
		/// @brief Get the layer currently used
		static LayerID GetLayer();
		/// @brief Get all the root canvases.
		static std::unordered_map<UiElem::ID, UiCanvas*> GetCanvases();
	private:

		/*---Helpers---*/
		void SetFrame(UiFrame::Template** frame, String texture);
		static void MakeBorderTex(String texture, std::vector<Texture*>& tex);
		
		void CreateText(UiTxt* text, std::string_view utf8, float width);
		/*---Instantiation---*/
		void NewElem(UiElem* elem, UiCanvas* canvas);

	private:
		friend Systems;
		void Update();
		void LayoutUpdate();
		void DestroyUpdate();
		void HoverableUpdate();
		void ValuesUpdate();
		void OnClick(InputSignal* signal);
		void OnCanvasMustUpdate(UiCanvas* canvas);
		void OnDestroy(UiElem* elem);
		//Helper
		Writer* m_writer = nullptr;

		//Data
		std::unordered_map<String, UiFrame::Template> m_frameTemplates;

		//State (creation of new elements)
		Material* m_material = nullptr;
		Material* m_defaultMaterial = nullptr;
		FontID m_font = 0;
		LayerID m_layer = 0;
		UiFrame::Template* m_canvasFrame = nullptr;
		UiFrame::Template* m_buttonFrame = nullptr;
		UiFrame::Template* m_buttonFrameHover = nullptr;
		UiFrame::Template* m_buttonFramePressed = nullptr;
		TextAlign m_textAlign = TextAlign::Left;
		Color m_txtColor = Color(255, 255, 255, 255);

		//Runtime
		std::vector<UiElem*> m_hoverables;
		std::vector<UiTxt*> m_values;
		std::vector<UiElem*> m_destroy;
		std::unordered_map<UiElem::ID, UiCanvas*> m_roots;
		std::vector<UiCanvas*> m_layoutUpdate;
		UiElem* m_hovered = nullptr;
		UiElem* m_pressed = nullptr;
		std::stack<UiCanvas*> m_canvas;
		float m_z = 0.f;
		Vec2f m_margin = 0.f;
		Vec2f m_rootMargin = 0.f;
		float m_ppu = 1.f / 360.f;

		static UiElem::ID m_nextId;
		const float zStep = 1.f;

	};
	template<typename ...Ts>
	inline UiTxt* Ui::Text(std::string_view utf8, float width, Ts ...args)
	{
		auto i = Instance();
		ASSERT_LOG_ERROR(!i->m_canvas.empty(), "Trying to create Ui Text without active canvas");
		auto canvas = i->m_canvas.top();

		//Instantiation
		UiTxt* text = new UiTxt();
		text->AddData(args...);
		text->parent = canvas;
		text->font = i->m_font;
		text->align = i->m_textAlign;
		text->color = i->m_txtColor;
		text->utf8 = utf8;
		i->CreateText(text, text->Format(), width);
		i->NewElem(text, canvas);
		i->m_values.emplace_back(text);

		return text;
	}
}