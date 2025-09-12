#pragma once
#include <stack>
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Render/Writer.h"


/*
Cahier des charges:

Initialization:
Creation des template visuels:
	frame
	bouton
	checkbox
Creation des fonts
Material et uniforms

Mise à jour d'elements, sans recréer toute la frame.
Donc mise a jour de la position de chaque éléments
Push global des template d'element visuel (frame, boutton, checkbox)
Bouttons avec callback < navigable
Checkbox avec callback < navigable
Textes avec alignements (gauche, centré)
Insertion d'icones en milieu de texte
Elements Navigables (selectionable)
Navigation globale (left, top, right, bot)
Frame stretch ou fixe ou hybride.
Quand fixe, contenu size peut pas depasser
Quand fixe, contenu position wrapping
Agencement flexible du contenu des frames:
	ancrage
	aligné gauche/droite/centré
	espace inter objets (padding)
	espace exterieur (margin)
	layout vertical ou horizontal
	layout direction normal (top down/left right) ou reverse
*/


/*class UiElem
{
	UiID id;
	Frame* parent;
	Entity entt;
	Vec2f size;
	Vec2f pos; //anchored pos
	Anchor anchor;

};
struct ButtonSignal
{

};
class Button
{
	Signal<ButtonSignal> signal;
};
class Navigable
{
	Navigable* top;
	Navigable* left;
	Navigable* right;
	Navigable* bot;
};
class Text : public UiElem
{
	Sentence text;
};
class Frame : public UiElem
{
	std::vector<BorderSprite> borderSprites;
	std::vector<UiElem*> children;

};*/

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
		enum class Layout;
		enum class LayoutDir;
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
		std::stack<Canvas*> m_canvas;
		std::unordered_map<ElemID, Elem*> m_elems;
		float m_z = 0.f;

		static ElemID m_nextId;
		const float zStep = 1.f;

	};
}