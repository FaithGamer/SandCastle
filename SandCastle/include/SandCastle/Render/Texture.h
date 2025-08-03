#pragma once
#include <glad/glad.h>
#include "SandCastle/Core/Vec.h"
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

	class RenderTexture;

	class Texture
	{
	public:
		Texture();
		Texture(std::string path, TextureImportSettings importSettings = TextureImportSettings());
		Texture(unsigned char* buffer, int size, TextureImportSettings importSettings = TextureImportSettings());
		void Reload(std::string path, TextureImportSettings importSettings = TextureImportSettings());
		~Texture();

		void Bind(uint32_t textureUnit = 0) const;
		void SetPixelPerUnit(float ppu);

		GLuint GetId() const;
		Vec2i GetSize() const;

		/// @brief The scale of the texture.
		/// @return The amount of world unit for one pixel of the texture ( 1.0f / pixelPerUnit ) 
		float GetPixelPerUnit() const;

	private:
		inline void LoadFromMemory(unsigned char* buffer, int size);
		inline void LoadFromFile(std::string paths);
		inline void Generate(TextureImportSettings importSettings);
		
		void Create1x1White();
		friend RenderTexture;
		TextureImportSettings m_importSettings;
		Vec2i m_size;
		int m_nbChannels;
		unsigned char* m_pixels;
		GLuint m_id;
	};
}