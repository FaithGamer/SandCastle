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

	Texture::Texture(int width, int height, TextureImportSettings importSettings)
		: m_size(width, height), m_nbChannels(4), m_pixels(nullptr), m_id(0), m_importSettings(importSettings)
	{
		m_importSettings.pixelPerUnit = 1.f / m_importSettings.pixelPerUnit;
		GenerateEmpty(importSettings);
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
	inline void Texture::GenerateEmpty(TextureImportSettings importSettings)
	{
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);

		// Allocate immutable contents (RGBA8), no initial data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_size.x, (GLsizei)m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		// Wrapping
		GLfloat borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, importSettings.wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, importSettings.wrapping);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		// Filtering
		GLint minFilter = importSettings.useMipmaps ? GL_LINEAR_MIPMAP_LINEAR : importSettings.filtering;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, importSettings.filtering);

		if (importSettings.useMipmaps)
		{
			// We’ll generate after updates, no need now.
		}

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
		if (m_pbos[0] || m_pbos[1])
			glDeleteBuffers(2, m_pbos);
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

	void Texture::EnablePBOStreaming(bool enable, size_t pboSizeBytes)
	{
		if (enable == m_pboEnabled)
			return;

		if (enable)
		{
			if (pboSizeBytes == 0)
				pboSizeBytes = static_cast<size_t>(m_size.x) * static_cast<size_t>(m_size.y) * 4; // RGBA8

			m_pboSize = pboSizeBytes;
			glGenBuffers(2, m_pbos);

			for (int i = 0; i < 2; ++i)
			{
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbos[i]);
				glBufferData(GL_PIXEL_UNPACK_BUFFER, (GLsizeiptr)m_pboSize, nullptr, GL_STREAM_DRAW);
			}
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			m_currentPBO = 0;
			m_pboEnabled = true;
		}
		else
		{
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			if (m_pbos[0] || m_pbos[1])
				glDeleteBuffers(2, m_pbos);
			m_pbos[0] = m_pbos[1] = 0;
			m_currentPBO = 0;
			m_pboSize = 0;
			m_pboEnabled = false;
			m_pboMapped = false;
		}
	}

	void* Texture::MapUploadBuffer(size_t bytes)
	{
		if (!m_pboEnabled || bytes == 0 || bytes > m_pboSize)
			return nullptr;

		GLuint pbo = m_pbos[m_currentPBO];
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);

		// Orphan previous storage to avoid stalls
		glBufferData(GL_PIXEL_UNPACK_BUFFER, (GLsizeiptr)m_pboSize, nullptr, GL_STREAM_DRAW);

		// Fast write-only map. UNSYNCHRONIZED assumes you manage hazards (we orphaned so it's safe).
		void* ptr = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, (GLsizeiptr)bytes,
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

		m_pboMapped = (ptr != nullptr);
		return ptr;
	}

	void Texture::UnmapAndCommit(int x, int y, int w, int h, int srcStride)
	{
		if (!m_pboEnabled || !m_pboMapped)
			return;

		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		m_pboMapped = false;

		// Upload from PBO to texture
		glBindTexture(GL_TEXTURE_2D, m_id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (srcStride == 0 || srcStride == w * 4)
		{
			// Tightly packed
			glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*)0);
		}
		else
		{
			// If caller wrote row-strided data into the PBO (contiguously), use per-row uploads
			// Compute address per row by adjusting pointer offset before each call
			// (We keep it simple and do h calls; for large rects consider a temporary tight copy)
			for (int row = 0; row < h; ++row)
			{
				const GLvoid* rowPtr = (const GLvoid*)((uintptr_t)0 + (uintptr_t)row * (uintptr_t)srcStride);
				glTexSubImage2D(GL_TEXTURE_2D, 0, x, y + row, w, 1, GL_RGBA, GL_UNSIGNED_BYTE, rowPtr);
			}
		}

		if (m_importSettings.useMipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		// Switch PBO for next frame
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		m_currentPBO = 1 - m_currentPBO;
	}

	void Texture::UpdateRegion(int x, int y, int w, int h, const void* src, int srcStride)
	{
		if (w <= 0 || h <= 0) return;

		glBindTexture(GL_TEXTURE_2D, m_id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (!m_pboEnabled)
		{
			// Fallback: direct upload
			if (srcStride == 0 || srcStride == w * 4)
			{
				glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, src);
			}
			else
			{
				const uint8_t* row = static_cast<const uint8_t*>(src);
				for (int r = 0; r < h; ++r)
				{
					glTexSubImage2D(GL_TEXTURE_2D, 0, x, y + r, w, 1, GL_RGBA, GL_UNSIGNED_BYTE, row);
					row += srcStride;
				}
			}
		}
		else
		{
			const size_t needed = (srcStride ? (size_t)srcStride * (size_t)h
				: (size_t)w * 4ull * (size_t)h);
			void* dst = MapUploadBuffer(needed);
			if (dst)
			{
				if (srcStride == 0 || srcStride == w * 4)
				{
					memcpy(dst, src, needed);
					UnmapAndCommit(x, y, w, h, w * 4);
				}
				else
				{
					// Copy row by row into PBO linearly
					uint8_t* write = static_cast<uint8_t*>(dst);
					const uint8_t* read = static_cast<const uint8_t*>(src);
					for (int r = 0; r < h; ++r)
					{
						memcpy(write, read, (size_t)w * 4);
						write += (size_t)w * 4;
						read += srcStride;
					}
					UnmapAndCommit(x, y, w, h, w * 4);
				}
			}
			else
			{
				// Fallback if map failed
				if (srcStride == 0 || srcStride == w * 4)
				{
					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbos[m_currentPBO]);
					glBufferData(GL_PIXEL_UNPACK_BUFFER, (GLsizeiptr)needed, src, GL_STREAM_DRAW);
					glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*)0);
					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
					m_currentPBO = 1 - m_currentPBO;
				}
				else
				{
					const uint8_t* row = static_cast<const uint8_t*>(src);
					for (int r = 0; r < h; ++r)
					{
						glTexSubImage2D(GL_TEXTURE_2D, 0, x, y + r, w, 1, GL_RGBA, GL_UNSIGNED_BYTE, row);
						row += srcStride;
					}
				}
			}
		}

		if (m_importSettings.useMipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
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
		if(parameters.HaveField("LodMin"))
			lodMin = parameters.GetInt("LodMin");
		if(parameters.HaveField("LodMax"))
			lodMax = parameters.GetInt("LodMax");
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