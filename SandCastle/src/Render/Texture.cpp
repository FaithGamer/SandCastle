#include "pch.h"

#include "SandCastle/Core/Log.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Render/Texture.h"

#include <stb/stb_image.h>

namespace SandCastle
{

	Texture::Texture() : m_id(0), m_pixels(nullptr), m_nbChannels(0), m_importSettings(TextureImportSettings()), m_size(1, 1)
	{
		m_importSettings.pixelPerUnit = 1.f / m_importSettings.pixelPerUnit;
	}

	Texture::Texture(std::string path, TextureImportSettings importSettings)
		: m_size(0, 0), m_nbChannels(0), m_pixels(nullptr), m_id(0), m_importSettings(importSettings)
	{
		//Texture from file
		m_importSettings.pixelPerUnit = 1.f / m_importSettings.pixelPerUnit;

		if (path == "white")
		{
			Create1x1White();
			return;
		}

		LoadFromFile(path);
		Generate(importSettings);
	}

	Texture::Texture(unsigned char* buffer, int size, TextureImportSettings importSettings)
		: m_size(0, 0), m_nbChannels(0), m_pixels(nullptr), m_id(0), m_importSettings(importSettings)
	{
		//Texture from memory
		m_importSettings.pixelPerUnit = 1.f / m_importSettings.pixelPerUnit;
		LoadFromMemory(buffer, size);
		Generate(importSettings);
	}

	inline void Texture::LoadFromMemory(unsigned char* buffer, int size)
	{
		stbi_set_flip_vertically_on_load(true);
		m_pixels = stbi_load_from_memory(buffer, size, &m_size.x, &m_size.y, &m_nbChannels, 4);
		ASSERT_LOG_ERROR(m_pixels, "Failed to load a texture from memory.");
	}

	inline void Texture::LoadFromFile(std::string path)
	{
		stbi_set_flip_vertically_on_load(true);
		m_pixels = stbi_load(path.c_str(), &m_size.x, &m_size.y, &m_nbChannels, 4);
		ASSERT_LOG_ERROR(m_pixels, "Failed to load texture: " + path);
	}

	inline void Texture::Generate(TextureImportSettings importSettings)
	{
		//Generate and bind an OpenGL texture
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);

		//Send the texture data to OpenGL
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_size.x, (GLsizei)m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels);

		//Texture Wrapping
		GLfloat borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f }; // transparent
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, importSettings.wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, importSettings.wrapping);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		//Texture filtering
		GLint minFilter = importSettings.useMipmaps ? GL_LINEAR_MIPMAP_LINEAR : importSettings.filtering;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, importSettings.filtering);

		if (importSettings.useMipmaps)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, importSettings.lodMax);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, importSettings.lodMin);
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		if (!importSettings.keepData)
		{
			stbi_image_free(m_pixels);
		}

		//Unbind since we are done configuring this texture
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::Create1x1White()
	{
		//generate 1x1 white texture
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);
		m_pixels = new unsigned char(0xd);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)1, (GLsizei)1, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels);

		//Texture Wrapping
		GLfloat borderColor[] = { 1.0f, 0.0f, 1.0f, 1.0f };
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_importSettings.wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_importSettings.wrapping);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		//Texture filtering
		GLint minFilter = m_importSettings.useMipmaps ? GL_LINEAR_MIPMAP_LINEAR : m_importSettings.filtering;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_importSettings.filtering);

		if (m_importSettings.useMipmaps)
		{
			//glGenerateMipmap(GL_TEXTURE_2D);
		}

		delete m_pixels;

		//Unbind since we are done configuring this texture
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::Reload(std::string path, TextureImportSettings importSettings)
	{
		glDeleteTextures(1, &m_id);
		if (m_importSettings.keepData)
		{
			stbi_image_free(m_pixels);
		}
		LoadFromFile(path);
		Generate(importSettings);
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &m_id);
		if (m_importSettings.keepData)
		{
			stbi_image_free(m_pixels);
		}
	}

	void Texture::Bind(uint32_t textureUnit) const
	{
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, m_id);
	}

	void Texture::SetPixelPerUnit(float ppu)
	{
		m_importSettings.pixelPerUnit = 1.f / ppu;
	}

	GLuint Texture::GetId() const
	{
		return m_id;
	}

	Vec2i Texture::GetSize() const
	{
		return m_size;
	}


	float Texture::GetPixelPerUnit() const
	{
		return m_importSettings.pixelPerUnit;
	}

	//
	//
	//
	// Texture Import Settings
	//
	//
	//

	TextureImportSettings::TextureImportSettings(TextureFiltering Filtering, TextureWrapping Wrapping, float PixelPerUnit, bool UseMipmaps, bool KeepData)
		: filtering(Filtering), wrapping(Wrapping), pixelPerUnit(PixelPerUnit), useMipmaps(UseMipmaps), keepData(KeepData)
	{

	}
	TextureImportSettings::TextureImportSettings(Serialized& parameters) : TextureImportSettings()
	{
		Deserialize(parameters);
	}

	void TextureImportSettings::Deserialize(Serialized& parameters)
	{
		String Filtering = parameters.GetString("Filtering");
		if (Filtering == "Linear")
			filtering = TextureFiltering::Linear;
		else if (Filtering == "Nearest")
			filtering = TextureFiltering::Nearest;
		else
		{
			LOG_WARN("Unknown texture filtering parameters: " + Filtering + ", in file: " + parameters.GetReadPath());
		}

		String Wrapping = parameters.GetString("Wrapping");
		if (Wrapping == "Clamp")
			wrapping = TextureWrapping::Clamp;
		else if (Wrapping == "Repeat")
			wrapping = TextureWrapping::Repeat;
		else
		{
			LOG_WARN("Unknown texture wrapping parameters: " + Wrapping + ", in file: " + parameters.GetReadPath());
		}

		pixelPerUnit = parameters.GetFloat("PixelPerUnit");
		useMipmaps = parameters.GetBool("Mipmaps");
		keepData = parameters.GetBool("KeepData");
		if (parameters.HaveField("LodMin"))
			lodMin = (int)parameters.GetInt("LodMin");
		if (parameters.HaveField("LodMax"))
			lodMax = (int)parameters.GetInt("LodMax");
		valid = parameters.HadGetError();
	}

	Serialized TextureImportSettings::Serialize()
	{
		Serialized settings;
		if (filtering == TextureFiltering::Linear)
			settings["Filtering"] = "Linear";
		else
			settings["Filtering"] = "Nearest";

		if (wrapping == TextureWrapping::Clamp)
			settings["Wrapping"] = "Clamp";
		else
			settings["Wrapping"] = "Repeat";

		settings["Mipmaps"] = useMipmaps;
		settings["KeepData"] = keepData;
		settings["PixelPerUnit"] = pixelPerUnit;

		return settings;
	}

	bool TextureImportSettings::DeserializationError()
	{
		return valid;
	}

}