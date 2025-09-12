
#pragma once

#include <vector>
#include <string>
#include <set>
#include <unordered_map>
#include <cstdint>
#include <utility>
#include <limits>

#define __STDC_LIB_EXT1__
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ADVANCES_H 

#include "SandCastle/Core/Vec.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/ECS/System.h"

namespace SandCastle
{
	class Ui;
	using FontID = uint32_t;
	typedef std::pair<uint32_t, uint32_t> GlyphRange;
	enum class TextAlign
	{
		Left,
		Center,
		Right
	};
	struct Glyph
	{
		uint32_t codepoint = 0;
		Vec2i    sizePx{ 0,0 };     // bitmap size in pixels
		Vec2i    bearingPx{ 0,0 };  // left/top bearing in pixels
		int      advancePx = 0;     // advance in pixels (FT 26.6 to px)
		int      atlasIndex = 0;    // which atlas texture/page this glyph lives on
		sptr<Sprite> sprite = nullptr;
	};

	struct Font
	{
		FontID id = 0;
		FT_Face face;
		std::string name;
		float ppu = 50;
		Material* material;
		int size = 0;                                // pixel height
		std::unordered_map<uint32_t, Glyph> glyphs;  // codepoint -> glyph
		std::unordered_map<uint32_t, float> spacesAdv;
		std::vector<GlyphRange> ranges;
		std::vector<sptr<Texture>> atlases;          // one or more atlas textures
		std::vector<Vec2i> atlasSizes;               // same length as atlases
		std::vector<std::string> atlasPaths;         // PNG files for pages
		Glyph fallbackGlyph;              // baked replacement/tofu glyph
		bool  hasFallbackGlyph = false;   // true once baked
		float outlineThickness = 0.f;     // in pixels (stroke radius)
		Vec4f outlineColor = { 0, 0, 0, 1 };
		LayerID layer = 0;
	};

	struct Sentence
	{
		Entity root;                           // parent entity for the sentence
		std::vector<Entity> glyphEntities;     // child entities (one per glyph)
		Vec2f size;
	};

	// NEW: store each character's "normal" computed position so FX can move them later.
	struct Character
	{
		Vec3f originalPosition{ 0,0,0 };
	};

	class Writer
	{
	public:
		
		~Writer();

		FontID MakeFont(std::string filename,
			int size,
			float outlineThickness = 0.f,
			Vec4f outlineColor = { 0,0,0,1 });
		/// @brief Give a fancy name to the font to find it easily later 
		/// across all your project.
		void NameFont(FontID font, String name);
		/// @brief Set the font that will be used for every subsequent Write
		void UseFont(FontID id);
		/// @brief Set the font that will be used for every subsequent Write, using its fancy name
		void UseFont(String name);
		/// @brief Set the folder path for the font assets
		void SetFontFolder(String path);
		/// @brief Set the PPU that will be used for every subsequent MakeFont
		void SetPPU(float ppu);
		/// @brief Set the material that will be used for every subsequent MakeFont
		void SetMaterial(Material* material);
		void SetMaxAtlasSize(int pixels);
		/// @brief Set the Layer that will be used for every subsequent MakeFont
		void SetLayer(LayerID layer);

		Sentence Write(std::string_view utf8,
			float maxWidth = -1.f,
			TextAlign textAlign = TextAlign::Left,
			float lineSpacing = 1.0f);

		Sentence Write(std::string_view utf8,
			FontID font,
			Material* material,
			LayerID layer,
			float maxWidth = -1.f,
			TextAlign textAlign = TextAlign::Left,
			float lineSpacing = 1.0f);

		float GetFontWorldSize(FontID font);
		float GetPPU() const;
		FontID GetFont(String fancyName) const;

	private:
		friend Ui;
		Writer(Material* material, LayerID layer);
		// Helpers
		static std::vector<uint32_t> Utf8ToCodepoints(std::string_view s);
		void InitLazyPages(Font& font);
		void EnsureGlyphs(Font& font, const std::vector<uint32_t>& cps);
		bool BakeOneGlyph(Font& font, uint32_t cp);
		int PlaceOnPage(FontID id, int reqW, int reqH, int pad, Vec2i& outPos);
		std::vector<unsigned char> EdgeExtrudeRGBA(const unsigned char* srcAlpha, int w, int h, int pad);
		std::vector<unsigned char> PadRGBA(const unsigned char* srcRGBA, int w, int h, int pad);
		void MakeTofuAlpha(int w, int h, int border, std::vector<unsigned char>& alpha);
		bool BakeFallbackGlyph(Font& font);

		struct ShelfPacker
		{
			int width, height;
			int shelfY = 0, shelfH = 0, cursorX = 0;
			explicit ShelfPacker(int w, int h) : width(w), height(h) {}
			Vec2i place(int w, int h)
			{
				if (w > width || h > height) return { -1,-1 };
				if (cursorX + w > width) { shelfY += shelfH; cursorX = 0; shelfH = 0; }
				if (shelfY + h > height) return { -1,-1 };
				Vec2i pos{ cursorX, shelfY };
				cursorX += w;
				if (h > shelfH) shelfH = h;
				return pos;
			}
		};

		struct DynamicPage {
			int w = 0, h = 0;
			int shelfY = 0, shelfH = 0, cursorX = 0;
		};

	private:
		FT_Library m_ft = nullptr;
		std::vector<std::vector<DynamicPage>> m_dynPages;
		std::vector<Font> m_fonts;
		std::unordered_map<String, FontID> m_fontFinder;
		FontID m_current = 0;
		int m_maxAtlasSize = 4096;
		float m_ppu = 1.f;
		Material* m_material = nullptr;
		LayerID m_layer = 0;
		String m_fontFolder = "assets/fonts/";
	};
}