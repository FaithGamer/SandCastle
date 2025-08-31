#pragma once

#include "SandCastle/Core/Vec.h"

namespace SandCastle
{
	class Color
	{
	public:
		Color()
		{
		}
		Color(unsigned char R, unsigned char G, unsigned char B, unsigned char A)
			: r(R), g(G), b(B), a(A)
		{
		}
		Color(const Vec4f& vec)
		{
			FromFloat(vec);
		}


		inline uint32_t PackColorRGBA()
		{
			return  ((uint32_t)r << 24) |
				((uint32_t)g << 16) |
				((uint32_t)b << 8) |
				((uint32_t)a);
		}

		inline void UnpackColorRGBA(uint32_t color)
		{
			r = (uint8_t)((color >> 24) & 0xFF);
			g = (uint8_t)((color >> 16) & 0xFF);
			b = (uint8_t)((color >> 8) & 0xFF);
			a = (uint8_t)(color & 0xFF);
		}
		inline operator Vec4f() const {
			constexpr float inv255 = 1.0f / 255.0f;
			return Vec4f{ r * inv255, g * inv255, b * inv255, a * inv255 };
		}
		inline Color operator=(const Vec4f& vec)
		{
			FromFloat(vec);
			return *this;
		}

		unsigned char r = 255;
		unsigned char g = 255;
		unsigned char b = 255;
		unsigned char a = 255;
	private:
		static unsigned char ToByte(float f) {
			if (f <= 0.0f) return 0;
			if (f >= 1.0f) return 255;
			return static_cast<unsigned char>(std::lround(f * 255.0f));
		}
		void FromFloat(const Vec4f& v) {
			r = ToByte(v.x);
			g = ToByte(v.y);
			b = ToByte(v.z);
			a = ToByte(v.w);
		}
	};
}
