#pragma once

#include <vector>
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Render/Rect.h"
#include "SandCastle/Render/Texture.h"

namespace SandCastle
{

	class Sprite
	{
	public:
		//To do add pivot point
		Sprite();
		Sprite(sptr<Texture> texture);
		Sprite(sptr<Texture> texture, Rect textureRect);
		Sprite(sptr<Texture> texture, Rect textureRect, Vec2f origin);
		/// @brief Construct sprite with normalized texture coords
		Sprite(sptr<Texture> texture, Vec2f* textureCoords);
		Sprite(sptr<Texture> texture, Vec2f* textureCoords, Vec2f origin);
		void TextureCoordsRelative(Vec2f* coords, Rect rect, float resFactor = 1.f);
		void ComputeDimensions();


		/// @brief Set the texture rect to crop the texture
		/// It will automatically calculate the textures coordinates relative to the texture size
		/// @param textureRect The rectangle to crop the texture
		/// @param resolutionFactor If you have multiple texture resolutions, you may enter a multiplication factor to the rectangle
		void SetTextureRect(Rect textureRect, float resolutionFactor = 1.f);
		void SetTexture(sptr<Texture> Texture);
		/// @brief Set sprite origin point in normalized sprite coordinate
		/// 0, 0 is centered.
		void SetOrigin(Vec2f origin);

		inline Vec2f GetOrigin() const
		{
			return m_origin;
		}
		inline sptr<Texture> GetTexture() const
		{
			return m_texture;
		}
		inline Vec2f GetDimensions() const
		{
			return m_dimensions;
		}
		inline Vec2f GetTextureCoords(int index) const
		{
			return m_textureCoords[index];
		}


	private:
		Vec2f m_textureCoords[4];
		Vec2f m_dimensions;
		Vec2f m_origin;
		sptr <Texture> m_texture;

	};
}
