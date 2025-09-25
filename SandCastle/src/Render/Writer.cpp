
#include "pch.h"
#include "SandCastle/Render/Writer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include FT_STROKER_H 
#include "SandCastle/Core/Log.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Core/Random.h"

namespace fs = std::filesystem;

namespace SandCastle
{

	inline bool IsSpace(uint32_t cp)
	{
		switch (cp) {
		case 0x0020: // SPACE
		case 0x00A0: // NO-BREAK SPACE
		case 0x1680: // OGHAM SPACE MARK
		case 0x2000: // EN QUAD
		case 0x2001: // EM QUAD
		case 0x2002: // EN SPACE
		case 0x2003: // EM SPACE
		case 0x2004: // THREE-PER-EM SPACE
		case 0x2005: // FOUR-PER-EM SPACE
		case 0x2006: // SIX-PER-EM SPACE
		case 0x2007: // FIGURE SPACE
		case 0x2008: // PUNCTUATION SPACE
		case 0x2009: // THIN SPACE
		case 0x200A: // HAIR SPACE
		case 0x202F: // NARROW NO-BREAK SPACE
		case 0x205F: // MEDIUM MATHEMATICAL SPACE
		case 0x3000: // IDEOGRAPHIC SPACE
		case 0x200B: // ZERO WIDTH SPACE
		case 0x200C: // ZERO WIDTH NON-JOINER
		case 0x200D: // ZERO WIDTH JOINER
		case 0x2060: // WORD JOINER
		case 0xFEFF: // ZERO WIDTH NO-BREAK SPACE (BOM)
			return true;
		default:
			return false;
		}
	}
	static inline bool is_cont(uint8_t b) { return (b & 0xC0) == 0x80; }

	// Portable, table-less length decoder
	static inline uint8_t expected_len(uint8_t c) {
		if (c < 0x80) return 1;
		if (c < 0xC0) return 0;          // continuation as lead → invalid
		if (c < 0xE0) return 2;
		if (c < 0xF0) return 3;
		if (c <= 0xF4) return 4;         // F5..FF invalid in UTF-8
		return 0;
	}

	std::vector<uint32_t> Writer::Utf8ToCodepoints(std::string_view s)
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(s.data());
		const uint8_t* end = p + s.size();

		std::vector<uint32_t> out;
		out.reserve(s.size()); // worst case: all ASCII

		bool truncated = false;

		while (p < end) {
			// Fast path: ASCII run
			while (p < end && *p < 0x80) out.push_back(*p++);

			if (p >= end) break;

			uint8_t c = *p++;
			uint8_t L = expected_len(c);
			if (L == 0) { out.push_back(0xFFFD); continue; }
			if (L == 1) { out.push_back(c); continue; }

			if (end - p < static_cast<std::ptrdiff_t>(L - 1)) {
				// truncated multibyte sequence at end of string
				out.push_back(0xFFFD);
				truncated = true;
				break;
			}

			uint8_t b1 = p[0];
			if (!is_cont(b1)) { out.push_back(0xFFFD); continue; }

			if (L == 2) {
				if (c < 0xC2) { out.push_back(0xFFFD); continue; } // overlong
				out.push_back(((c & 0x1F) << 6) | (b1 & 0x3F));
				p += 1;
				continue;
			}

			uint8_t b2 = p[1];
			if (!is_cont(b2)) { out.push_back(0xFFFD); continue; }

			if (L == 3) {
				// E0: b1 >= A0 to avoid overlongs; ED: b1 < A0 to avoid surrogates
				if ((c == 0xE0 && b1 < 0xA0) || (c == 0xED && b1 >= 0xA0)) {
					out.push_back(0xFFFD); continue;
				}
				out.push_back(((c & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F));
				p += 2;
				continue;
			}

			// L == 4
			uint8_t b3 = p[2];
			if (!is_cont(b3)) { out.push_back(0xFFFD); continue; }
			if ((c == 0xF0 && b1 < 0x90) || (c == 0xF4 && b1 > 0x8F)) { // over/under long
				out.push_back(0xFFFD); continue;
			}
			uint32_t cp = ((c & 0x07) << 18) | ((b1 & 0x3F) << 12) |
				((b2 & 0x3F) << 6) | (b3 & 0x3F);
			if (cp > 0x10FFFF) { out.push_back(0xFFFD); continue; }

			out.push_back(cp);
			p += 3;
		}

		if (truncated)
			LOG_WARN("input string truncated in middle of UTF-8 sequence\n{0}", s);


		return out;
	}

	// ---------- Lifecycle ----------
	Writer::Writer(Material* material, LayerID layer)
		: m_material(material), m_layer(layer)
	{
		FT_Error err = FT_Init_FreeType(&m_ft);
		ASSERT_LOG_ERROR(err == 0, "FreeType init failed.");

		GLint maxTextureSize = 0;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
		m_maxAtlasSize = std::min(4096, maxTextureSize);
	}
	Writer::~Writer()
	{
		for (auto& font : m_fonts)
		{
			if (font.face)
				FT_Done_Face(font.face);
		}
		FT_Done_FreeType(m_ft);
	}

	FontID Writer::MakeFont(std::string filename, int size,
		float outlineThickness, Vec4f outlineColor)
	{
		// Load face by path
		String path = m_fontFolder + filename;
		Font font;
		{
			FT_Error err = FT_New_Face(m_ft, path.c_str(), 0, &font.face);
			ASSERT_LOG_ERROR(err == 0, std::string("Failed to load font: ") + path);
		}
		auto err = FT_Select_Charmap(font.face, FT_ENCODING_UNICODE);
		if (err)
		{
			LOG_ERROR("No Unicode charmap in {0} (err={1})", path.c_str(), (int)err);
		}
		FT_Set_Pixel_Sizes(font.face, 0, size);

		auto index = (FontID)m_fonts.size();
		font.ppu = m_ppu;
		font.id = index;
		font.size = size;
		font.name = filename;
		font.filtering = m_filtering;
		//font.spacesAdv = SpaceAdv(font.face, 1.f / font.ppu, outlineThickness);
		font.outlineThickness = std::max(0.f, outlineThickness);
		font.outlineColor = outlineColor;
		font.material = m_material;
		font.layer = m_layer;

		InitLazyPages(font); // start with an empty atlas we’ll grow on - demand
		BakeFallbackGlyph(font);

		m_fonts.emplace_back(std::move(font));

		return index;
	}

	void Writer::SetFontFolder(String path)
	{
		m_fontFolder = path;
	}

	void Writer::SetFiltering(TextureFiltering filtering)
	{
		m_filtering = filtering;
	}

	void Writer::NameFont(FontID font, String name)
	{
		m_fontFinder[name] = font;
	}

	void Writer::UseFont(FontID id)
	{
		if (m_fonts.size() <= id)
		{
			LOG_ERROR("Writer::UseFont, Invalid font Id");
			return;
		}
		m_current = id;
	}

	void Writer::UseFont(String name)
	{
		auto it = m_fontFinder.find(name);
		if (it == m_fontFinder.end())
		{
			LOG_ERROR("The font {0}, cannot be found.");
			return;
		}
		m_current = it->second;
	}
	void Writer::SetPPU(float ppu)
	{
		m_ppu = ppu;
	}
	void Writer::SetMaterial(Material* material)
	{
		m_material = material;
	}
	void Writer::SetMaxAtlasSize(int pixels)
	{
		m_maxAtlasSize = pixels;
	}

	void Writer::SetLineAdjustement(float adjustement)
	{
		m_adjustLine = adjustement;
	}

	void Writer::SetLayer(LayerID layer)
	{
		m_layer = layer;
	}

	Sentence Writer::Write(std::string_view utf8,
		float width,
		const Color& color,
		TextAlign textAlign,
		float lineSpacing)
	{
		if (m_fonts.size() <= (size_t)m_current)
		{
			LOG_ERROR("Writer::Write, invalid m_current (fontId out of range)");
			return Sentence();
		}
		return Write(utf8, m_current, color, m_fonts[m_current].material, m_fonts[m_current].layer, width, textAlign, lineSpacing);
	}

	Sentence Writer::Write(
		std::string_view utf8,
		const FontID fontId,
		const Color& color,
		const Material* material,
		const LayerID layer,
		float width,
		TextAlign textAlign,
		float lineSpacing)
	{
		if (m_fonts.size() <= (size_t)fontId)
		{
			LOG_ERROR("Writer::Write, invalid fontId (out of range)");
			return Sentence();
		}
		Font& font = m_fonts[(size_t)fontId];

		auto cps = Utf8ToCodepoints(utf8);
		EnsureGlyphs(font, cps);
		Sentence sent;
		sent.root = Entity::Create();
		sent.root.AddComponent<Transform>();

		Vec2f pen(0.f, 0.f); // baseline origin
		const float ppu = font.atlases.empty() ? (1.0f / std::max(1, font.size)) : font.atlases[0]->GetPixelPerUnit();
		const float lineStep = ((float)font.size + font.outlineThickness) * lineSpacing * ppu;

		uint32_t prevGlyphIndex = 0;

		//Helpers

		struct WordChar
		{
			Entity entt;
			float adv = 0.f;
		};
		struct Line
		{
			std::vector<WordChar> chars;
			float totalAdv = 0;
			float lastSpaceAdv = 0.f;
		};
		std::vector<Line> lines;
		lines.emplace_back(Line());
		struct CurrentWord
		{
			std::vector<WordChar> characters;
			float totalAdv = 0;
			float advStart = 0;
		};
		CurrentWord currentWord;
		auto NextLine = [&pen, &prevGlyphIndex, &lines, &lineStep]()
			{
				pen.x = 0.f;
				pen.y -= lineStep;
				prevGlyphIndex = 0;
				lines.emplace_back(Line());
			};
		auto RestartCurrWord = [&currentWord, &pen]()
			{
				currentWord.characters.clear();
				currentWord.totalAdv = 0.f;
				currentWord.advStart = pen.x;
			};
		auto CheckLineBounds = [&](float adv) -> bool
			{
				if (width <= 0.f)
					return false;
				if (adv > width)
					return false;
				if ((pen.x + adv) > width)
				{
					NextLine();

					if (currentWord.characters.empty()
						|| currentWord.totalAdv + adv > width)
					{
						return true;
					}
					auto& prevLine = lines[lines.size() - 2];
					auto& curLine = lines.back();
					prevLine.totalAdv -= prevLine.lastSpaceAdv;
					for (auto& ch : currentWord.characters)
					{
						ch.entt.gtr()->Move(-currentWord.advStart, -lineStep, 0);
						pen.x += ch.adv;
						if (!prevLine.chars.empty())
						{
							prevLine.totalAdv -= prevLine.chars.back().adv;
							prevLine.chars.pop_back();
						}
						curLine.chars.emplace_back(ch);
						curLine.totalAdv += ch.adv;
					}
					RestartCurrWord();
					return true;
				}
				return false;
			};
		auto CreateEntity = [&](const Glyph& g, float adv)
			{
				Vec3f pos(
					pen.x + (g.bearingPx.x + 0.5f * g.sizePx.x) * ppu,
					pen.y + (g.bearingPx.y - 0.5f * g.sizePx.y) * ppu - (float)font.size * ppu,
					0
				);
				pos.y += (float)font.size * ppu * m_adjustLine;
				Entity e = Entity::Create();
				auto tr = e.AddComponent<Transform>();
				tr->SetPosition(pos);
				auto ch = e.AddComponent<Character>();
				ch->originalPosition = pos;
				auto sr = e.AddComponent<SpriteRender>();
				sr->SetMaterial(material->GetID());
				sr->SetSprite(g.sprite.get());
				sr->SetLayer(layer);
				sr->color = color;
				sent.root.AddChild(e);
				sent.glyphEntities.push_back(e);
				currentWord.totalAdv += adv;
				currentWord.characters.emplace_back(WordChar(e, adv));
				lines.back().chars.emplace_back(WordChar(e, adv));
				lines.back().totalAdv += adv;
				pen.x += adv;
			};
		float maxPenX = 0.f;
		//Iterating sentence CPS
		for (uint32_t cp : cps)
		{
			if (cp == (uint32_t)'\n')
			{
				NextLine();
				RestartCurrWord();
				continue;
			}
			bool isSpace = IsSpace(cp);
			auto it = font.glyphs.find(cp);
			if (it == font.glyphs.end())
			{
				if (isSpace)
				{
					float adv = (float)font.size * ppu * 0.5f;
					pen.x += adv;
					RestartCurrWord();
					lines.back().totalAdv += adv;
					lines.back().lastSpaceAdv = adv;
				}
				else if (font.hasFallbackGlyph)
				{
					const Glyph& g = font.fallbackGlyph;
					float nextAdv = (float)g.advancePx * ppu;
					CheckLineBounds(nextAdv);
					CreateEntity(g, nextAdv);
					prevGlyphIndex = 0;
				}
				continue;
			}

			const Glyph& g = it->second;

			// Kerning
			float kernPx = 0.f;
			if (FT_HAS_KERNING(font.face) && prevGlyphIndex != 0)
			{
				uint32_t currIndex = FT_Get_Char_Index(font.face, cp);
				FT_Vector kern; kern.x = kern.y = 0;
				if (currIndex != 0)
				{
					FT_Get_Kerning(font.face, prevGlyphIndex, currIndex, FT_KERNING_DEFAULT, &kern);
					kernPx = (float)(kern.x >> 6);
				}
			}

			float nextAdv = (kernPx + (float)g.advancePx) * ppu;
			bool lineR = CheckLineBounds(nextAdv);
			if (!lineR)
				prevGlyphIndex = FT_Get_Char_Index(font.face, cp);
			else
				prevGlyphIndex = 0;

			//Reset word if space
			if (isSpace)
			{
				pen.x += nextAdv;
				RestartCurrWord();
				lines.back().totalAdv += nextAdv;
				lines.back().lastSpaceAdv = nextAdv;
			}
			else
			{
				CreateEntity(g, nextAdv);
			}
			maxPenX = pen.x > maxPenX ? pen.x : maxPenX;
		}
		sent.maxWidth = width;
		sent.size.x = width > 0.f ? width : maxPenX;
		sent.size.y = std::abs(pen.y) + ((float)font.size + font.outlineThickness) * ppu;

		//Lines alignement
		void (*Align)(Line & line, float w) = nullptr;
		switch (textAlign)
		{
		case TextAlign::Center:
			Align = [](Line& line, float w) -> void
				{
					float offset = (w - line.totalAdv) * 0.5f;
					for (int i = 0; i < line.chars.size(); i++)
					{
						line.chars[i].entt.gtr()->Move(offset, 0.f, 0.f);
					}
				};
			break;
		case TextAlign::Right:
			Align = [](Line& line, float w) -> void
				{
					float offset = w - line.totalAdv;
					for (int i = 0; i < line.chars.size(); i++)
					{
						line.chars[i].entt.gtr()->Move(offset, 0.f, 0.f);
					}
				};
			break;
		default:
			break;
		}

		if (width > 0.f && Align != nullptr)
		{
			for (int i = 0; i < lines.size(); i++)
			{
				Align(lines[i], sent.size.x);
			}
		}

		return sent;
	}
	float Writer::GetFontWorldSize(FontID font)
	{
		if (m_fonts.size() <= font)
		{
			LOG_ERROR("Writer::GetFontWorldSize, Invalid font ID");
			return 1.f;
		}

		float ppu = m_fonts[font].atlases[0]->GetPixelPerUnit();
		return (float)m_fonts[font].size * ppu;
	}

	float Writer::GetPPU() const
	{
		return m_ppu;
	}

	const Font* Writer::GetFont(String fancyName) const
	{
		auto font = m_fontFinder.find(fancyName);
		if (font == m_fontFinder.end())
		{
			LOG_ERROR("Writer::GetFont, the fancy name {0}, doesn't exists.", fancyName);
			return 0;
		}
		return &m_fonts[font->second];
	}

	const Font* Writer::GetFont(FontID font) const
	{
		if (m_fonts.size() > font)
			return &m_fonts[font];
		else
		{
			LOG_ERROR("Trying to get a font but fond id is out of range.");
			return nullptr;
		}
	}

	static int NextPow2(int x) { int p = 1; while (p < x) p <<= 1; return p; }

	void Writer::InitLazyPages(Font& font)
	{
		const int base = std::max(256, NextPow2(font.size * 16));
		const int side = std::min(m_maxAtlasSize, base);

		TextureImportSettings tis;
		tis.keepData = false;
		tis.useMipmaps = true;
		tis.filtering = font.filtering;
		tis.wrapping = TextureWrapping::Clamp;
		tis.pixelPerUnit = font.ppu;

		auto tex = makesptr<Texture>(side, side, tis);

		font.atlases.clear();
		font.atlasSizes.clear();
		font.atlasPaths.clear();
		font.atlases.push_back(tex);
		font.atlasSizes.push_back(Vec2i{ side, side });

		while (m_dynPages.size() <= (size_t)font.id)
			m_dynPages.emplace_back(std::vector<DynamicPage>());
		m_dynPages[font.id].clear();
		m_dynPages[font.id].push_back(DynamicPage{ side, side, 0, 0, 0 });
	}

	int Writer::PlaceOnPage(FontID id, int reqW, int reqH, int pad, Vec2i& outPos)
	{
		int pageIndex = 0;
		auto& pages = m_dynPages[id];
		for (int i = 0; i < (int)pages.size(); ++i)
		{
			auto& p = pages[i];
			if (reqW > p.w || reqH > p.h) continue;

			if (p.cursorX + reqW > p.w) { p.shelfY += p.shelfH; p.cursorX = 0; p.shelfH = 0; }
			if (p.shelfY + reqH > p.h) continue;

			pageIndex = i;
			outPos = { p.cursorX, p.shelfY };
			p.cursorX += reqW;
			if (reqH > p.shelfH) p.shelfH = reqH;
			return pageIndex;
		}

		const int needW = std::min(std::max(reqW, 256), m_maxAtlasSize);
		const int needH = std::min(std::max(reqH, 256), m_maxAtlasSize);
		const int side = std::min(m_maxAtlasSize, std::max(NextPow2(std::max(needW, needH)), 256));

		TextureImportSettings tis;
		auto font = m_fonts[id];
		tis.keepData = false;
		tis.useMipmaps = true;
		tis.filtering = font.filtering;
		tis.wrapping = TextureWrapping::Clamp;
		tis.pixelPerUnit = font.ppu;

		auto tex = makesptr<Texture>(side, side, tis);

		auto& pagesVec = m_dynPages[id];
		pagesVec.push_back(DynamicPage{ side, side, 0, 0, 0 });

		pageIndex = (int)pagesVec.size() - 1;
		outPos = { 0, 0 };
		pagesVec.back().cursorX = reqW;
		pagesVec.back().shelfH = reqH;

		Font& f = m_fonts[id];
		f.atlases.push_back(tex);
		f.atlasSizes.push_back(Vec2i{ side, side });
		return pageIndex;
	}

	std::vector<unsigned char> Writer::EdgeExtrudeRGBA(const unsigned char* srcAlpha, int w, int h, int pad)
	{
		std::vector<unsigned char> out;
		const int W = w + 2 * pad, H = h + 2 * pad;
		out.assign((size_t)W * (size_t)H * 4u, 0);

		auto px = [&](int X, int Y) -> unsigned char {
			X = std::clamp(X, 0, w - 1);
			Y = std::clamp(Y, 0, h - 1);
			return srcAlpha[Y * w + X];
			};

		for (int y = 0; y < H; y++)
		{
			for (int x = 0; x < W; x++)
			{
				const int srcX = x - pad;
				const int srcY = H - y - pad;
				const unsigned char a = (w > 0 && h > 0) ? px(srcX, srcY) : 0;

				const int dst = (y * W + x) * 4;
				out[dst + 0] = 255;
				out[dst + 1] = 255;
				out[dst + 2] = 255;
				out[dst + 3] = a;
			}
		}
		return out;
	}

	std::vector<unsigned char> Writer::PadRGBA(const unsigned char* srcRGBA, int w, int h, int pad)
	{
		std::vector<unsigned char> out;
		const int W = w + 2 * pad, H = h + 2 * pad;
		out.assign((size_t)W * (size_t)H * 4u, 0);

		auto copy_px = [&](int X, int Y, unsigned char* dst) {
			X = std::clamp(X, 0, w - 1);
			Y = std::clamp(Y, 0, h - 1);
			const int si = (Y * w + X) * 4;
			dst[0] = srcRGBA[si + 0];
			dst[1] = srcRGBA[si + 1];
			dst[2] = srcRGBA[si + 2];
			dst[3] = srcRGBA[si + 3];
			};

		for (int y = 0; y < H; ++y)
		{
			for (int x = 0; x < W; ++x)
			{
				const int srcX = x - pad;
				const int srcY = H - y - pad;
				unsigned char* d = &out[(y * W + x) * 4];
				if (w > 0 && h > 0) copy_px(srcX, srcY, d);
				else d[0] = d[1] = d[2] = d[3] = 0;
			}
		}
		return out;
	}

	void Writer::MakeTofuAlpha(int w, int h, int border, std::vector<unsigned char>& alpha)
	{
		w = std::max(w, 8);
		h = std::max(h, 8);
		border = std::max(border, 1);

		alpha.assign((size_t)w * (size_t)h, 0);
		auto setA = [&](int x, int y, unsigned char a)
			{
				if (x >= 0 && x < w && y >= 0 && y < h)
					alpha[(size_t)y * (size_t)w + (size_t)x] = a;
			};

		for (int x = 0; x < w; ++x)
			for (int b = 0; b < border; ++b)
			{
				setA(x, b, 255);
				setA(x, h - 1 - b, 255);
			}
		for (int y = 0; y < h; ++y)
			for (int b = 0; b < border; ++b)
			{
				setA(b, y, 255);
				setA(w - 1 - b, y, 255);
			}

		auto drawDiag = [&](bool reverse)
			{
				for (int y = border; y < h - border; ++y)
				{
					float t = (float)(y - border) / std::max(1, h - 2 * border - 1);
					int xi = border + (int)std::round(t * std::max(1, w - 2 * border - 1));
					int thickness = std::max(1, std::min(w, h) / 16);
					for (int dx = -thickness; dx <= thickness; ++dx)
						setA(xi + dx, y, 255);
				}
			};
		drawDiag(false);
		drawDiag(true);
	}

	bool Writer::BakeFallbackGlyph(Font& font)
	{
		const uint32_t REPLACEMENT_CP = 0xFFFD;
		if (FT_Get_Char_Index(font.face, REPLACEMENT_CP) != 0)
		{
			if (font.glyphs.find(REPLACEMENT_CP) == font.glyphs.end())
				BakeOneGlyph(font, REPLACEMENT_CP);

			auto it = font.glyphs.find(REPLACEMENT_CP);
			if (it != font.glyphs.end())
			{
				font.fallbackGlyph = it->second;
				font.hasFallbackGlyph = true;
				return true;
			}
		}

		const int asc = font.face->size ? (int)(font.face->size->metrics.ascender >> 6) : font.size;
		const int em = font.face->size ? (int)(font.face->size->metrics.y_ppem) : font.size;
		const int boxH = std::max(8, (int)std::round(em * 0.9f));
		const int boxW = boxH;
		const int border = std::max(1, boxH / 12);

		std::vector<unsigned char> alpha;
		MakeTofuAlpha(boxW, boxH, border, alpha);

		constexpr int kPad = 2;
		const int reqW = std::max(1, boxW + 2 * kPad);
		const int reqH = std::max(1, boxH + 2 * kPad);

		Vec2i pos{ 0,0 };
		int page = PlaceOnPage(font.id, reqW, reqH, kPad, pos);

		std::vector<unsigned char> rgba = EdgeExtrudeRGBA(alpha.data(), boxW, boxH, kPad);

		auto& tex = font.atlases[page];
		tex->UpdateRegion(pos.x, pos.y, reqW, reqH, rgba.data());

		Glyph g;
		g.codepoint = REPLACEMENT_CP;
		g.sizePx = Vec2i{ boxW, boxH };
		g.bearingPx = Vec2i{ 0, asc };
		g.advancePx = boxW + std::max(1, boxW / 8);
		g.atlasIndex = page;
		g.sprite = makesptr<Sprite>(font.atlases[g.atlasIndex].get());
		g.sprite->SetTextureRect(Rect{ (float)(pos.x + kPad), (float)(pos.y + kPad + 1), (float)boxW, (float)boxH }, 1.0f);

		font.fallbackGlyph = g;
		font.hasFallbackGlyph = true;
		return true;
	}

	static inline unsigned char f2ub(float v) { v = std::clamp(v, 0.0f, 1.0f); return (unsigned char)std::round(v * 255.0f); }

	bool Writer::BakeOneGlyph(Font& font, uint32_t cp)
	{
		FT_UInt gindex = FT_Get_Char_Index(font.face, cp);
		if (gindex == 0) return false;

		// We need advance regardless of rendering path


		// Outline path: render stroke + fill, composite into a single RGBA
		if (font.outlineThickness > 0.0f)
		{
			if (FT_Load_Glyph(font.face, gindex, FT_LOAD_DEFAULT))
				return false;
			int advancePx = (int)(font.face->glyph->advance.x >> 6);
			advancePx += font.outlineThickness;
			FT_Glyph glyphFill = nullptr;
			FT_Glyph glyphStroke = nullptr;
			FT_Stroker stroker = nullptr;

			auto cleanup = [&]() {
				if (glyphFill)   FT_Done_Glyph(glyphFill);
				if (glyphStroke) FT_Done_Glyph(glyphStroke);
				if (stroker)     FT_Stroker_Done(stroker);
				};

			// 3) Extract glyph outline twice: one for fill, one we’ll stroke
			if (FT_Get_Glyph(font.face->glyph, &glyphFill)) { cleanup(); return false; }
			if (FT_Glyph_Copy(glyphFill, &glyphStroke)) { cleanup(); return false; }

			// 4) Configure stroker (thickness in 26.6)
			if (FT_Stroker_New(m_ft, &stroker)) { cleanup(); return false; }
			FT_Stroker_Set(
				stroker,
				(FT_Fixed)std::lround(font.outlineThickness * 64.0), // thickness
				FT_STROKER_LINECAP_ROUND,
				FT_STROKER_LINEJOIN_ROUND,
				0 // miter limit (ignored for ROUND)
			);

			// 5) Apply outside stroke to the copy
			// destroy=1 lets FreeType free the source outline packed into glyphStroke
			if (FT_Glyph_StrokeBorder(&glyphStroke, stroker, 0, 1)) { cleanup(); return false; }

			// 6) Convert both to 8-bit coverage bitmaps (grayscale AA)
			if (FT_Glyph_To_Bitmap(&glyphFill, FT_RENDER_MODE_NORMAL, nullptr, 1)) { cleanup(); return false; }
			if (FT_Glyph_To_Bitmap(&glyphStroke, FT_RENDER_MODE_NORMAL, nullptr, 1)) { cleanup(); return false; }

			FT_BitmapGlyph bmf = reinterpret_cast<FT_BitmapGlyph>(glyphFill);
			FT_BitmapGlyph bms = reinterpret_cast<FT_BitmapGlyph>(glyphStroke);
			const FT_Bitmap& bf = bmf->bitmap; // fill bitmap
			const FT_Bitmap& bs = bms->bitmap; // stroke bitmap

			// 7) Union bounding box in pixel space (FreeType: left/top are bearing from baseline; top is upward)
			const int fill_left = bmf->left;
			const int fill_top = bmf->top;
			const int fill_right = bmf->left + (int)bf.width;
			const int fill_bottom = bmf->top - (int)bf.rows;

			const int stroke_left = bms->left;
			const int stroke_top = bms->top;
			const int stroke_right = bms->left + (int)bs.width;
			const int stroke_bottom = bms->top - (int)bs.rows;

			const int left = std::min(fill_left, stroke_left);
			const int right = std::max(fill_right, stroke_right);
			const int top = std::max(fill_top, stroke_top);
			const int bottom = std::min(fill_bottom, stroke_bottom);

			const int w = std::max(0, right - left);
			const int h = std::max(0, top - bottom);
			if (w == 0 || h == 0) { cleanup(); return false; }

			// 8) Offsets into the union image (y-down in destination buffer)
			const int dx_f = fill_left - left;
			const int dy_f = top - fill_top;
			const int dx_s = stroke_left - left;
			const int dy_s = top - stroke_top;

			// 9) Destination RGBA (premultiplied)
			std::vector<unsigned char> rgba((size_t)w * (size_t)h * 4u, 0);

			// Utility: handle positive/negative pitch safely
			auto row_ptr = [](const FT_Bitmap& b, int y) -> const unsigned char* {
				int pitch = b.pitch;
				if (pitch >= 0)
					return b.buffer + y * pitch;
				// Negative pitch: first row is at the end of the buffer
				return b.buffer + (b.rows - 1 - y) * (-pitch);
				};

			auto sampleCoverage = [&](const FT_Bitmap& b, int dx, int dy, int x, int y) -> float {
				const int xx = x - dx;
				const int yy = y - dy;
				if (xx < 0 || yy < 0 || xx >= (int)b.width || yy >= (int)b.rows) return 0.0f;
				const unsigned char* row = row_ptr(b, yy);
				return row[xx] / 255.0f; // coverage 0..1
				};

			// Colors (linear-ish). Your outlineColor should be in [0..1]
			const float Cs_r = std::clamp(font.outlineColor.x, 0.0f, 1.0f);
			const float Cs_g = std::clamp(font.outlineColor.y, 0.0f, 1.0f);
			const float Cs_b = std::clamp(font.outlineColor.z, 0.0f, 1.0f);

			// Fill color is white; change if needed
			constexpr float Cf_r = 1.0f, Cf_g = 1.0f, Cf_b = 1.0f;

			// 10) Composite FILL over STROKE into premultiplied RGBA
			for (int y = 0; y < h; ++y)
			{
				for (int x = 0; x < w; ++x)
				{
					const float as = sampleCoverage(bs, dx_s, dy_s, x, y);
					const float af = sampleCoverage(bf, dx_f, dy_f, x, y);

					// Porter-Duff OVER with premultiplied colors
					const float a = af + as * (1.0f - af);
					float Cp_r = Cf_r * af + Cs_r * as * (1.0f - af);
					float Cp_g = Cf_g * af + Cs_g * as * (1.0f - af);
					float Cp_b = Cf_b * af + Cs_b * as * (1.0f - af);

					// Clamp & store
					const size_t i = (size_t)(y * w + x) * 4u;
					rgba[i + 0] = (unsigned char)std::lround(std::clamp(Cp_r, 0.0f, 1.0f) * 255.0f);
					rgba[i + 1] = (unsigned char)std::lround(std::clamp(Cp_g, 0.0f, 1.0f) * 255.0f);
					rgba[i + 2] = (unsigned char)std::lround(std::clamp(Cp_b, 0.0f, 1.0f) * 255.0f);
					rgba[i + 3] = (unsigned char)std::lround(std::clamp(a, 0.0f, 1.0f) * 255.0f);
				}
			}

			// 11) Atlas placement with margin (edge clamp)
			constexpr int kPad = 2;
			const int reqW = std::max(1, w + 2 * kPad);
			const int reqH = std::max(1, h + 2 * kPad);

			Vec2i pos{ 0, 0 };
			int page = PlaceOnPage(font.id, reqW, reqH, kPad, pos);

			// You must provide PadRGBA(rgba, w, h, pad) -> vector<uint8_t> with clamped borders
			std::vector<unsigned char> padded = PadRGBA(rgba.data(), w, h, kPad);

			auto& tex = font.atlases[page];
			if (reqW > 0 && reqH > 0)
				tex->UpdateRegion(pos.x, pos.y, reqW, reqH, padded.data());

			// 12) Emit glyph metrics/sprite rect
			Glyph g;
			g.codepoint = cp;
			g.sizePx = Vec2i{ w, h };
			g.bearingPx = Vec2i{ left, top }; // left/top of union box (matches how we built it)
			g.advancePx = advancePx;
			g.atlasIndex = page;
			g.sprite = makesptr<Sprite>(font.atlases[g.atlasIndex].get());

			// Texture rect inside the padded region; keep your +1 if you need it for texel alignment
			g.sprite->SetTextureRect(
				Rect{ (float)(pos.x + kPad), (float)(pos.y + kPad + 1), (float)w, (float)h },
				1.0f
			);

			font.glyphs[cp] = g;

			cleanup();
			return true;
		}
		else
		{
			// Original fast path: render grayscale bitmap and expand to RGBA white
			if (FT_Load_Char(font.face, cp, FT_LOAD_RENDER))
				return false;
			int advancePx = (int)(font.face->glyph->advance.x >> 6);
			const FT_GlyphSlot slot = font.face->glyph;
			const int w = slot->bitmap.width;
			const int h = slot->bitmap.rows;

			const Vec2i bearing(slot->bitmap_left, slot->bitmap_top);

			constexpr int kPad = 2;
			const int reqW = std::max(1, w + 2 * kPad);
			const int reqH = std::max(1, h + 2 * kPad);

			Vec2i pos{ 0,0 };
			int page = PlaceOnPage(font.id, reqW, reqH, kPad, pos);

			std::vector<unsigned char> rgba;
			if (w > 0 && h > 0)
				rgba = EdgeExtrudeRGBA(slot->bitmap.buffer, w, h, kPad);
			else
				rgba.assign((size_t)reqW * (size_t)reqH * 4u, 0);

			auto& tex = font.atlases[page];
			if (reqW > 0 && reqH > 0)
				tex->UpdateRegion(pos.x, pos.y, reqW, reqH, rgba.data());

			Glyph g;
			g.codepoint = cp;
			g.sizePx = Vec2i{ w, h };
			g.bearingPx = bearing;
			g.advancePx = advancePx;
			g.atlasIndex = page;
			g.sprite = makesptr<Sprite>(font.atlases[g.atlasIndex].get());
			g.sprite->SetTextureRect(Rect{ (float)(pos.x + kPad), (float)(pos.y + kPad + 1), (float)w, (float)h }, 1.0f);

			font.glyphs[cp] = g;
			return true;
		}
	}

	void Writer::EnsureGlyphs(Font& font, const std::vector<uint32_t>& cps)
	{
		std::unordered_set<uint32_t> missing;
		missing.reserve(cps.size());

		for (uint32_t cp : cps)
		{
			if (cp == (uint32_t)'\n') continue;
			if (font.glyphs.find(cp) != font.glyphs.end()) continue;
			if (FT_Get_Char_Index(font.face, cp) != 0)
			{
				missing.insert(cp);
			}
			else
			{
				LOG_WARN("Missing char index, codepoint: {0}", cp);
			}
		}

		for (uint32_t cp : missing)
			BakeOneGlyph(font, cp);
	}
}
