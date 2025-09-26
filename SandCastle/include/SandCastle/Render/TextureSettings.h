#pragma once
#include <glad/glad.h>
#include "SandCastle/Core/Serialization.h"

namespace SandCastle
{
	typedef enum : GLint
	{
		Linear = GL_LINEAR,
		Nearest = GL_NEAREST
	}TextureFiltering;

	typedef enum : GLint
	{
		Clamp = GL_CLAMP_TO_EDGE,
		Repeat = GL_REPEAT
	}TextureWrapping;

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