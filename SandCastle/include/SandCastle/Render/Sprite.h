#pragma once

#include <vector>
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Render/Rect.h"
#include "SandCastle/Render/Texture.h"
#include "float16_t.hpp"

namespace SandCastle
{

	/// @brief A subregion of a Texture plus an origin (pivot) and cached UVs.
	/// Sprites are typically loaded by the Assets store from a `.texture` file
	/// describing their atlas layout. They are immutable from the renderer's
	/// point of view: once created, only their `orgX/orgY` origin can change.
	class Sprite
	{
	public:
		//To do add pivot point
		Sprite() {};
		/// @brief Sprite covering the full texture.
		Sprite(const Texture* texture);
		/// @brief Sprite cropped to `textureRect` (in texture pixels).
		Sprite(const Texture* texture, Rect textureRect);
		/// @brief Sprite cropped to `textureRect` with a normalized origin (ox, oy).
		Sprite(const Texture* texture, Rect textureRect, numeric::float16_t ox, numeric::float16_t oy);

		/// @brief Compute UVs for the four quad vertices given a pixel rect; resolution factor scales the rect for hi-DPI variants.
		void TextureCoordsRelative(Vec2f* coords, Rect rect, float resFactor = 1.f);
		/// @brief Recompute world-space dimensions from the texture's pixel-per-unit and rect.
		void ComputeDimensions();

		/// @brief Set the texture rect to crop the texture
		/// It will automatically calculate the textures coordinates relative to the texture size
		/// @param textureRect The rectangle to crop the texture
		/// @param resolutionFactor If you have multiple texture resolutions, you may enter a multiplication factor to the rectangle
		void SetTextureRect(Rect textureRect, float resolutionFactor = 1.f);
		/// @brief Set sprite origin point in normalized sprite coordinate
		/// 0, 0 is centered.
		inline const Texture* GetTexture() const
		{
			return m_texture;
		}
		inline Vec2f GetDimensions() const
		{
			return m_dimensions;
		}
		inline Rect GetTextureRect() const
		{
			return m_textureRect;
		}
		/// @brief Each individual vertex texture UV from vertex 0 to 3
		/// @param index 
		/// @return 
		inline Vec2f GetTextureCoords(int index) const
		{
			return m_textureCoords[index];
		}
		/// @brief Only top left/ bottom right corner UV
		/// @return 
		inline Vec4f GetUVs() const
		{
			return m_uvs;
		}
		numeric::float16_t orgX = 0.f;
		numeric::float16_t orgY = 0.f;

	private:
		Vec2f m_textureCoords[4];
		Vec2f m_dimensions = { 0.f, 0.f };
		Rect m_textureRect = { 0.f, 0.f, 0.f, 0.f };

		Vec4f m_uvs = { 0.f, 0.f, 0.f, 0.f }; //texture coord top left, and bottom right corners
		const Texture* m_texture = nullptr;

	};
}
