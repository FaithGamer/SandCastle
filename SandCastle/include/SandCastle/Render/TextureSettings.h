#pragma once
#include <glad/glad.h>
#include "SandCastle/Core/Serialization.h"

namespace SandCastle
{
	/// @brief GL texture filtering mode (Linear smooths, Nearest preserves pixels).
	typedef enum : GLint
	{
		Linear = GL_LINEAR,
		Nearest = GL_NEAREST
	}TextureFiltering;

	/// @brief GL texture wrap mode for UVs outside [0,1].
	typedef enum : GLint
	{
		Clamp = GL_CLAMP_TO_EDGE,
		Repeat = GL_REPEAT
	}TextureWrapping;

	/// @brief Per-texture import options serialized as JSON in `.texture` files.
	/// Controls filtering, wrapping, pixel-per-unit, mipmap generation, and
	/// whether to keep the CPU-side pixel buffer accessible after upload.
	struct TextureImportSettings : public Serializable
	{
		static TextureImportSettings defaultSettings;
		TextureImportSettings() = default;
		TextureImportSettings(
			TextureFiltering TextureFiltering,
			TextureWrapping Wrapping,
			float PixelPerUnit,
			bool UseMipmaps,
			bool KeepData);

		TextureImportSettings(Serialized& config);
		void Deserialize(Serialized& config);
		Serialized Serialize() override;
		bool DeserializationError() override;

		TextureFiltering filtering = TextureFiltering::Linear;
		GLint wrapping = TextureWrapping::Clamp;
		float pixelPerUnit = 1;
		bool useMipmaps = true;
		bool keepData = false;
		int lodMin = -1000;
		int lodMax = 1000;
		bool valid = true;
	};
}