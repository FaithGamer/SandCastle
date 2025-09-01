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
	typedef uint32_t UiElemID;

	class UiElem
	{

	};

	/// @brief Container for every Ui objects, can contain other canvas.
	class Canvas : public UiElem
	{
	public:
		Entity root;
		Vec2f size;
	};

	class Ui : public Singleton<Ui>
	{
	public:

		typedef uint32_t FrameID;
		enum class Anchor
		{
			TopLeft,
			TopCenter,
			TopRight,
			MiddleLeft,
			MiddleCenter,
			MiddleRight,
			BotLeft,
			BotCenter,
			BotRight
		};

		enum class Align : int
		{
			Normal,
			Inverse
		};

		enum class AlignDirection : int
		{
			TopDown,
			DownTop,
			LeftRight,
			RightLeft
		};

		typedef enum : int
		{
			Top,
			Left,
			Mid,
			Right,
			Bot
		}TexBorder;

		typedef enum : int
		{
			TopLeft,
			TopRight,
			BotLeft,
			BotRight

		}SpriteCorner;

	public:
		Ui();
		~Ui();

		//static FontID MakeFont(String font, )
		// 
		/*---Initialization---*/

		static void MakeFrameTemplate(String texture, bool fixedStep);
		static void MakeFont(String filename,
			String fancyName,
			int size,
			float outlineThickness = 0.f,
			Vec4f outlineColor = { 0,0,0,1 });

		/*---Ui creation---*/

		static void BeginCanvas();

		/*---State---*/

		/// @brief Set the material that will be used for every subsequent ui element creation
		static void PushMaterial(Material* material);
		/// @brief Set the font that will be used for every subsequent ui element creation
		static void PushFont(FontID font);
		/// @brief Set the layer that will be used for every subsequent ui element creation
		static void PushLayer(LayerID layer);
		/// @brief Remove the top of the material stack (can't remove the last element)
		static void PopMaterial(Material* material);
		/// @brief Remove the top of the font stack (can't remove the last element)
		static void PopFont(FontID font);
		/// @brief Remove the top of the layer stack (can't remove the last element)
		static void PopLayer(LayerID layer);

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
		Entity InstanceFrame(UiElemID id, String texture, Vec2f size);
	private:
		//Helpers
		static void MakeBorderTex(String texture, std::vector<Texture*>& tex);
		static void BorderSize(int i, Rect& rect, Vec2f& wDim, Vec2f pxSize, Vec2f pxDim, Vec2f sDim, float ppu);
		//Instantiation

	
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

		Writer* m_writer;
		std::stack<Material*> m_material;
		std::stack<FontID> m_font;
		std::stack<LayerID> m_layer;
		std::unordered_map<String, FrameTemplate> m_frameTemplates;
		std::unordered_map<UiElemID, std::vector<BorderSprite>> m_borderSprites; //
	};
}