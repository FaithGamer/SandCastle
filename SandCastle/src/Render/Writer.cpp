
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
	static bool GetGlyphAdvance26_6(FT_Face face, FT_UInt gindex, FT_Pos& out_advance_26_6)
	{
		if (gindex == 0) return false; // glyph missing
		if (FT_Load_Glyph(face, gindex, FT_LOAD_DEFAULT)) return false;
		out_advance_26_6 = face->glyph->advance.x; // 26.6 fixed
		return true;
	}

	// Try to measure a specific codepoint's advance; if missing, return NaN.
	static float MeasureCodepointAdvancePx(FT_Face face, uint32_t cp)
	{
		FT_UInt gi = FT_Get_Char_Index(face, cp);
		FT_Pos adv_26_6 = 0;
		if (!GetGlyphAdvance26_6(face, gi, adv_26_6)) return std::numeric_limits<float>::quiet_NaN();
		return adv_26_6 >> 6;
	}

	// Compute an "em" width in pixels for fallbacks.
	// Uses the requested size in the face; call FT_Set_Pixel_Sizes or FT_Set_Char_Size first.
	static float EmPixels(FT_Face face)
	{
		if (face->size && face->size->metrics.x_ppem > 0)
			return static_cast<float>(face->size->metrics.x_ppem);

		FT_Fixed x_scale = face->size ? face->size->metrics.x_scale : (1 << 16);
		int em_in_px = FT_MulFix(face->units_per_EM ? face->units_per_EM : 2048, x_scale);
		return static_cast<float>(em_in_px);
	}

	static float FigureWidthPx(FT_Face face)
	{
		float w = MeasureCodepointAdvancePx(face, U'0');
		if (std::isnan(w)) w = MeasureCodepointAdvancePx(face, 0xFF10);
		return w;
	}
	static float PunctWidthPx(FT_Face face)
	{
		return MeasureCodepointAdvancePx(face, U'.');
	}

	std::unordered_map<uint32_t, float> SpaceAdv(FT_Face face, float ppu)
	{
		const uint32_t spaces[] = {
			0x0020,0x00A0,0x1680,0x2000,0x2001,0x2002,0x2003,0x2004,0x2005,0x2006,0x2007,0x2008,0x2009,0x200A,0x202F,0x205F,0x3000,
			0x200B,0x200C,0x200D,0x2060,0xFEFF,
		};

		std::unordered_map<uint32_t, float> out;
		const float em_px = EmPixels(face);
		const float space_px = MeasureCodepointAdvancePx(face, 0x0020);
		const float nbspace_px = MeasureCodepointAdvancePx(face, 0x00A0);
		const float digit_px = FigureWidthPx(face);
		const float punct_px = PunctWidthPx(face);
		auto emFrac = [&](float frac) -> float { return em_px * frac; };

		for (uint32_t cp : spaces) {
			double adv_px = MeasureCodepointAdvancePx(face, cp);

			if (!std::isnan(adv_px)) {
				out[cp] = adv_px;
				continue;
			}
			switch (cp) {
			case 0x0020: out[cp] = emFrac(0.25f); break;
			case 0x00A0: out[cp] = !std::isnan(space_px) ? space_px : emFrac(0.25f); break;
			case 0x1680: out[cp] = !std::isnan(space_px) ? space_px : emFrac(0.25f); break;
			case 0x2000:
			case 0x2002: out[cp] = emFrac(0.5f); break;
			case 0x2001:
			case 0x2003: out[cp] = emFrac(1.0f); break;
			case 0x2004: out[cp] = emFrac(1.0f / 3.0f); break;
			case 0x2005: out[cp] = emFrac(0.25f); break;
			case 0x2006: out[cp] = emFrac(1.0f / 6.0f); break;
			case 0x2007: out[cp] = !std::isnan(digit_px) ? digit_px : (!std::isnan(space_px) ? space_px : emFrac(0.5f)); break;
			case 0x2008: out[cp] = !std::isnan(punct_px) ? punct_px : (!std::isnan(space_px) ? space_px : emFrac(0.25f)); break;
			case 0x2009:
			case 0x202F: out[cp] = emFrac(0.2f); break;
			case 0x200A: out[cp] = emFrac(0.1f); break;
			case 0x205F: out[cp] = emFrac(2.0f / 9.0f); break;
			case 0x3000: out[cp] = emFrac(1.0f); break;
			case 0x200B:
			case 0x200C:
			case 0x200D:
			case 0x2060:
			case 0xFEFF: out[cp] = 0.0f; break;
			default: out[cp] = !std::isnan(space_px) ? space_px : emFrac(0.25f); break;
			}
		}
		if (out.count(0x0020) && out.count(0x00A0)) {
			if (out[0x00A0] < out[0x0020]) out[0x00A0] = out[0x0020];
		}
		for (auto& s : out) s.second *= ppu;
		return out;
	}
	std::vector<uint32_t> Writer::Utf8ToCodepoints(const std::string& s)
	{
		std::vector<uint32_t> cps;
		size_t i = 0, n = s.size();
		while (i < n) {
			uint8_t c = (uint8_t)s[i++];
			if ((c & 0x80) == 0) { cps.push_back(c); continue; }
			if ((c & 0xE0) == 0xC0 && i < n) { cps.push_back(((c & 0x1F) << 6) | (s[i++] & 0x3F)); continue; }
			if ((c & 0xF0) == 0xE0 && i + 1 < n) {
				uint32_t cp = ((c & 0x0F) << 12) | ((s[i] & 0x3F) << 6) | (s[i + 1] & 0x3F);
				i += 2; cps.push_back(cp); continue;
			}
			if ((c & 0xF8) == 0xF0 && i + 2 < n) {
				uint32_t cp = ((c & 0x07) << 18) | ((s[i] & 0x3F) << 12) | ((s[i + 1] & 0x3F) << 6) | (s[i + 2] & 0x3F);
				i += 3; cps.push_back(cp); continue;
			}
			cps.push_back(0xFFFD);
		}
		return cps;
	}

	// ---------- Lifecycle ----------
	Writer::Writer()
	{
		FT_Error err = FT_Init_FreeType(&m_ft);
		ASSERT_LOG_ERROR(err == 0, "FreeType init failed.");

		GLint maxTextureSize = 0;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
		m_maxAtlasSize = std::min(4096, maxTextureSize);
		m_defaultMaterial = Renderer2D::GetMaterial(0);
	}
	Writer::~Writer()
	{
		for (auto& kv : m_fonts)
		{
			if (kv.second.face)
				FT_Done_Face(kv.second.face);
		}
		FT_Done_FreeType(m_ft);
	}

	// ---------- MakeFont (primary: spec) ----------
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
		FT_Select_Charmap(font.face, FT_ENCODING_UNICODE);
		FT_Set_Pixel_Sizes(font.face, 0, size);


		font.ppu = m_ppu;
		font.material = m_material != nullptr ? m_material : m_defaultMaterial;
		font.id = ++m_nextId;
		font.size = size;
		font.name = filename;
		font.spacesAdv = SpaceAdv(font.face, 1.f / font.ppu);
		font.outlineThickness = std::max(0.f, outlineThickness);
		font.outlineColor = outlineColor;
		font.layer = m_layer;


		InitLazyPages(font); // start with an empty atlas we’ll grow on - demand
		BakeFallbackGlyph(font);

		m_fonts[font.id] = std::move(font);

		m_current = m_nextId;
		return m_current;
	}

	void Writer::SetFontFolder(String path)
	{
		m_fontFolder = path;
	}

	void Writer::UseFont(FontID id)
	{
		ASSERT_LOG_ERROR(m_fonts.count(id) != 0, "UseFont: invalid FontID");
		m_current = id;
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

	void Writer::SetLayer(uint32_t layer)
	{
		m_layer = layer;
	}

	// ---------- CreateSentence (kerning + wrapping + line spacing) ----------
	Sentence Writer::Write(const std::string& utf8,
		float maxWidth,
		float lineSpacing)
	{
		ASSERT_LOG_ERROR(m_fonts.count(m_current) != 0, "Write: no current font set");
		Font& font = m_fonts[m_current];

		auto cps = Utf8ToCodepoints(utf8);
		EnsureGlyphs(font, cps);
		Sentence sent;
		sent.root = Entity::Create();
		sent.root.AddComponent<Transform>();

		Vec2f pen(0.f, 0.f); // baseline origin
		const float ppu = font.atlases.empty() ? (1.0f / std::max(1, font.size)) : font.atlases[0]->GetPixelPerUnit();
		const float lineStep = ((float)font.size + font.outlineThickness) * lineSpacing * ppu;

		uint32_t prevGlyphIndex = 0;

		auto line = [&](Vec2f& pen, uint32_t& prevIndex, float max, float adv, float lineStep)
			{
				if (max > 0.f && (pen.x + adv) > max)
				{
					pen.x = 0.f;
					pen.y -= lineStep;
					prevIndex = 0;
				}
			};

		for (uint32_t cp : cps)
		{
			if (cp == (uint32_t)'\n')
			{
				pen.x = 0.f; pen.y -= lineStep; prevGlyphIndex = 0;
				continue;
			}

			auto it = font.glyphs.find(cp);
			if (it == font.glyphs.end())
			{
				auto it_space = font.spacesAdv.find(cp);
				if (it_space != font.spacesAdv.end())
				{
					float adv = it_space->second * ppu;
					line(pen, prevGlyphIndex, maxWidth, adv, lineStep);
					pen.x += adv;
					continue;
				}

				if (font.hasFallbackGlyph)
				{
					const Glyph& g = font.fallbackGlyph;
					float nextAdvanceWorld = (float)g.advancePx * ppu;
					line(pen, prevGlyphIndex, maxWidth, nextAdvanceWorld, lineStep);

					Vec3f pos(
						pen.x + (g.bearingPx.x + 0.5f * g.sizePx.x) * ppu,
						pen.y + (g.bearingPx.y - 0.5f * g.sizePx.y) * ppu,
						0
					);

					Entity e = Entity::Create();
					auto tr = e.AddComponent<Transform>();
					tr->SetPosition(pos);

					auto ch = e.AddComponent<Character>();
					ch->originalPosition = pos;

					auto sr = e.AddComponent<SpriteRender>();
					sr->SetMaterial(font.material->GetID());
					sr->SetSprite(g.sprite.get());
					sr->SetLayer(font.layer);

					sent.root.AddChild(e);
					sent.glyphEntities.push_back(e);
					pen.x += nextAdvanceWorld;
					prevGlyphIndex = 0;
					continue;
				}
				LOG_WARN("Invisible glyph has been drawn!");
				float adv = (font.size * 0.6f) * ppu;
				line(pen, prevGlyphIndex, maxWidth, adv, lineStep);
				pen.x += adv;
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

			float nextAdvanceWorld = (kernPx + (float)g.advancePx) * ppu;
			line(pen, prevGlyphIndex, maxWidth, nextAdvanceWorld, lineStep);

			Vec3f pos(
				pen.x + (g.bearingPx.x + 0.5f * g.sizePx.x) * ppu,
				pen.y + (g.bearingPx.y - 0.5f * g.sizePx.y) * ppu,
				0
			);

			Entity e = Entity::Create();
			auto tr = e.AddComponent<Transform>();
			tr->SetPosition(pos);
			auto ch = e.AddComponent<Character>();
			ch->originalPosition = pos;
			auto sr = e.AddComponent<SpriteRender>();
			sr->SetMaterial(font.material->GetID());
			sr->SetSprite(g.sprite.get());
			sr->SetLayer(font.layer);

			sent.root.AddChild(e);
			sent.glyphEntities.push_back(e);

			pen.x += (kernPx * ppu) + (g.advancePx * ppu);
			prevGlyphIndex = FT_Get_Char_Index(font.face, cp);
		}
		sent.width = pen.x;
		return sent;
	}

	float Writer::GetFontWorldSize(FontID font)
	{
		auto it = m_fonts.find(font);
		if (it == m_fonts.end())
		{
			LOG_ERROR("FontID {0}, does not exists", font);
			return 1;
		}

		float ppu = it->second.atlases[0]->GetPixelPerUnit();
		return (float)it->second.size * ppu;
	}

	static int NextPow2(int x) { int p = 1; while (p < x) p <<= 1; return p; }

	void Writer::InitLazyPages(Font& font)
	{
		const int base = std::max(256, NextPow2(font.size * 16));
		const int side = std::min(m_maxAtlasSize, base);

		TextureImportSettings tis;
		tis.keepData = false;
		tis.useMipmaps = true;
		tis.filtering = TextureFiltering::Linear;
		tis.wrapping = TextureWrapping::Clamp;
		tis.pixelPerUnit = font.ppu;

		auto tex = makesptr<Texture>(side, side, tis);

		font.atlases.clear();
		font.atlasSizes.clear();
		font.atlasPaths.clear();
		font.atlases.push_back(tex);
		font.atlasSizes.push_back(Vec2i{ side, side });

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
		tis.keepData = false;
		tis.useMipmaps = true;
		tis.filtering = TextureFiltering::Nearest;
		tis.wrapping = TextureWrapping::Clamp;
		tis.pixelPerUnit = m_fonts[id].ppu;

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

			// 11) Atlas placement with padding (edge clamp)
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
				missing.insert(cp);
		}

		for (uint32_t cp : missing)
			BakeOneGlyph(font, cp);
	}
}
