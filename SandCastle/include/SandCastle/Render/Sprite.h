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
		Sprite(const Texture* texture);
		Sprite(const Texture* texture, Rect textureRect);
		Sprite(const Texture* texture, Rect textureRect, Vec2f origin);
	
		void TextureCoordsRelative(Vec2f* coords, Rect rect, float resFactor = 1.f);
		void ComputeDimensions();

		/// @brief Set the texture rect to crop the texture
		/// It will automatically calculate the textures coordinates relative to the texture size
		/// @param textureRect The rectangle to crop the texture
		/// @param resolutionFactor If you have multiple texture resolutions, you may enter a multiplication factor to the rectangle
		void SetTextureRect(Rect textureRect, float resolutionFactor = 1.f);
		/// @brief Set sprite origin point in normalized sprite coordinate
		/// 0, 0 is centered.
		void SetOrigin(Vec2f origin);

		inline Vec2f GetOrigin() const
		{
			return m_origin;
		}
		inline const Texture* GetTexture() const
		{
			return m_texture;
		}
		inline Vec2f GetDimensions() const
		{
			return m_dimensions;
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

	private:
		Vec2f m_textureCoords[4];
		Vec2f m_dimensions;
		Vec2f m_origin;
		Vec4f m_uvs; //texture coord top left, and bottom right corners
		const Texture* m_texture;

	};
}
