#pragma once

#include <glad/glad.h>
#include "SandCastle/Core/Vec.h"
#include <float16_t.hpp>
#include "SandCastle/Render/Layer.h"
#include "SandCastle/Render/Material.h"
#include "SandCastle/Render/Color.h"

namespace SandCastle
{
	struct QuadRenderData
	{
		/// @brief this data structure must remain <= 64bytes for cache speed
		int type; //0 colored quad, 1 sprite
		Vec3f pos;
		numeric::float16_t orgX;
		numeric::float16_t orgY;
		Vec4f uvs;
		Vec2f size;
		float rotation;
		GLuint textureID;
		LayerID layerID;
		MaterialID materialID;
		Color color;
	};
}