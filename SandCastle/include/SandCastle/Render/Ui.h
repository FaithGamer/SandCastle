#pragma once
#include <stack>
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Render/Writer.h"


namespace SandCastle
{
	class Ui : public Singleton<Ui>
	{
	public:

		/*---Elements---*/

		/// @brief base class of every Ui objects.
		class Elem;
		/// @brief Container for every Ui objects, can contain other canvas.
		class Canvas;
		/// @brief Text
		class Txt;

		/*--- Types ---*/
		//definitions in UiEnum.h

		typedef uint32_t ElemID;
		typedef uint32_t FrameID;
		enum class Anchor;
		enum class LayoutDir; 
		enum class LayoutWrap;
		enum class LayoutAlign;
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
		static void MakeFrameTemplate(String texture, bool fixedStep);
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

		/*---Ui creation---*/

		static ElemID BeginCanvas(Vec2f size = { 0, 0 }, bool frame = true);
		static ElemID Text(std::string_view utf8, float maxWidth = -1.f);
		static void EndCanvas();
		static void Delete(ElemID uiElem);

		/*---State---*/

		/// @brief Set the material that will be used for every subsequent ui creation
		static void SetMaterial(Material* material);
		/// @brief Set the font that will be used for every subsequent ui creation
		static void SetFont(FontID font);
		/// @brief Set the font that will be used for every subsequent ui creation
		static void SetFont(String fancyName);
		/// @brief Set the layer that will be used for every subsequent ui creation
		static void SetLayer(LayerID layer);
		/// @brief Set the frame that will be used for every subsequent canvas creation
		static void SetCanvasFrame(String texture);
		/// @brief Set the frame that will be used for every subsequent button creation
		static void SetButtonFrame(String texture);
		/// @brief Set the text alignement that will be used for every subsequent text creation
		static void SetTextAlign(TextAlign textAlign);
		/// @brief Set the padding that will be used for every subsequent element creation.
		static void SetPadding(Vec2f padding);

		/*---Utility---*/

		/// @brief Convert UI position to world position
		static Vec3f UiToWorld(Vec3f uiPos);
		/// @brief Convert world position to UI position
		static Vec3f WorldToUi(Vec3f uiPos);

		/*---Accessors---*/

		/// @brief Get the object used to create fonts and write text.
		static Writer* GetWriter();
		/// @brief Get the material used (top of the stack)
		static Material* GetMaterial();
		static FontID GetFont();
		static LayerID GetLayer();

		//Public only for test
	private:

		/*---Structs---*/
		struct RepeatTextures
		{

		};

		struct BorderSprite
		{
			BorderSprite(Texture* tex, Rect rect, Vec2f worldDim);
			Sprite sprite;
			Vec2f wDim;
		};

		struct FrameTemplate
		{
			//3x3 stretchable sprites.
			//Used for buttons or frames.
			bool fixedStep = false;
			std::vector<Sprite*> cornerSpr;
			std::vector<Texture*> repeatTex;
		};

		/*---Helpers---*/
		static void MakeBorderTex(String texture, std::vector<Texture*>& tex);
		static void BorderSize(int i, Rect& rect, Vec2f& wDim, Vec2f pxSize, Vec2f pxDim, Vec2f sDim, float ppu);
		/*---Instantiation---*/
		ElemID InstanceElem(Elem* elem, Canvas* canvas);
		Entity InstanceFrame(ElemID id, FrameTemplate* frame, Vec2f size);

	private:
		//Helper
		Writer* m_writer = nullptr;

		//Data
		std::unordered_map<String, FrameTemplate> m_frameTemplates;
		std::unordered_map<ElemID, std::vector<BorderSprite>> m_borderSprites;

		//State (creation of new elements)
		Material* m_material = nullptr;
		FontID m_font;
		LayerID m_layer;
		FrameTemplate* m_canvasFrame = nullptr;
		FrameTemplate* m_buttonFrame = nullptr;
		TextAlign m_textAlign = TextAlign::Left;
		std::stack<Canvas*> m_canvas;
		std::unordered_map<ElemID, Elem*> m_elems;
		float m_z = 0.f;
		Vec2f m_padding = 0.f;

		static ElemID m_nextId;
		const float zStep = 1.f;

	};
}