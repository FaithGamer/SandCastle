#pragma once

#include <glad/glad.h>
#include "SandCastle/Core/Vec.h"

namespace SandCastle
{
	struct QuadRenderData
	{
		/// @brief this data structure must remain <= 64bytes for cache speed
		int type; //0 colored quad, 1 sprite
		Vec3f pos;
		Vec4f uvOrColor;
		Vec2f size;
		float rotation;
		GLuint textureID;
		float texturePPU;
		uint32_t layerID;
		uint32_t batchID;
		float alpha;
	};
}