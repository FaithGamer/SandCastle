
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
#include "SandCastle/Render/Text.h"
#include "SandCastle/Core/LangSignal.h"

namespace SandCastle
{
	class Ui;
	typedef std::pair<uint32_t, uint32_t> GlyphRange;
	
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
		std::vector<GlyphRange> ranges;
		std::vector<sptr<Texture>> atlases;          // one or more atlas textures
		std::vector<Vec2i> atlasSizes;               // same length as atlases
		std::vector<std::string> atlasPaths;         // PNG files for pages
		Glyph fallbackGlyph;              // baked replacement/tofu glyph
		bool  hasFallbackGlyph = false;   // true once baked
		float outlineThickness = 0.f;     // in pixels (stroke radius)
		Vec4f outlineColor = { 0, 0, 0, 1 };
		LayerID layer = 0;
		TextureFiltering filtering = TextureFiltering::Nearest;
		float scale = 1.f;
		float lineHeight = 1.f;
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

		FontID MakeFont(const String& filename,
			int size,
			float scale = 1.f,
			float lineHeight = 1.f,
			float outlineThickness = 0.f,
			Vec4f outlineColor = { 0,0,0,1 });
		void AddIcon(String id, Sprite* sprite, Vec2f offset = 0.f);
		/// @brief Give a fancy name to the font to find it easily later 
		/// across all your project.
		void NameFont(FontID font, const String& name, const std::vector<String> langs = {});
		/// @brief Set the font that will be used for every subsequent Write
		void UseFont(FontID id);
		/// @brief Set the font that will be used for every subsequent Write, 
		/// using its fancy name, also enable localized version 
		void UseFont(const String& name, const String& lang = "");
		/// @brief Set the folder path for the font assets
		void SetFontFolder(const String& path);
		/// @brief Set atlas texture filtering (default is nearest)
		void SetFiltering(TextureFiltering filtering);
		/// @brief Set the PPU that will be used for every subsequent MakeFont
		void SetPPU(float ppu);
		/// @brief Set the material that will be used for every subsequent MakeFont
		void SetMaterial(Material* material);
		void SetMaxAtlasSize(int pixels);
		/// @brief Set an offset for the line height.
		/// 0.f = normal
		/// 1.f = lines are 1.f*fontSize higher
		void SetLineAdjustement(float adjustement);
		/// @brief Set the Layer that will be used for every subsequent MakeFont
		void SetLayer(LayerID layer);

		
		Sentence Write(std::string_view utf8,
			float width = -1.f,
			const Color& color = Color(255, 255, 255, 255),
			TextAlign textAlign = TextAlign::Left,
			float lineSpacing = 1.0f);

		Sentence Write(std::string_view utf8,
			const FontID font,
			const Color& color,
			const Material* material,
			const LayerID layer,
			float width = -1.f,
			TextAlign textAlign = TextAlign::Left,
			float lineSpacing = 1.0f);

		Color GetColor() const;
		float GetFontWorldSize(FontID font);
		float GetPPU() const;
		const Font* GetFont(const String& fancyName, const String& lang = "");
		const Font* GetFont(FontID font) const;
		FontID GetFontID(const String& name, const String& lang = "");

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
		void OnLang(LangSignal* signal);

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
		std::unordered_map<String, FontID> m_localized;
		std::unordered_map<String, FontID> m_fontFinder;
		FontID m_current = 0;
		int m_maxAtlasSize = 4096;
		float m_ppu = 1.f;
		float m_adjustLine = 0.1f;
		Material* m_material = nullptr;
		LayerID m_layer = 0;
		TextureFiltering m_filtering = TextureFiltering::Nearest;
		String m_fontFolder = "fonts/";
		std::unordered_map<String, Glyph> m_icons;
	};
}