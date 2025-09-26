#include "pch.h"

#include "SandCastle/Core/Log.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Render/Texture.h"

#include <stb/stb_image.h>

namespace SandCastle
{

	Texture::Texture() : m_id(0), m_pixels(nullptr), m_nbChannels(0), m_importSettings(TextureImportSettings()), m_size(1, 1)
	{
		m_ppu = 1.f / m_importSettings.pixelPerUnit;
	}

	Texture::Texture(const Texture& texture)
	{
		Copy(texture);
	}

	Texture::Texture(std::string path, TextureImportSettings importSettings)
		: m_size(0, 0), m_nbChannels(0), m_pixels(nullptr), m_id(0), m_importSettings(importSettings)
	{
		//Texture from file
		m_ppu = 1.f / importSettings.pixelPerUnit;

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
		m_ppu = 1.f / importSettings.pixelPerUnit;
		LoadFromMemory(buffer, size);
		Generate(importSettings);
	}

	Texture::Texture(int width, int height, TextureImportSettings importSettings)
		: m_size(width, height), m_nbChannels(4), m_pixels(nullptr), m_id(0), m_importSettings(importSettings)
	{
		m_ppu = 1.f / importSettings.pixelPerUnit;
		GenerateEmpty(importSettings);
	}

	Texture::Texture(const Texture* sourceTexture, const Rect region)
		: m_size(region.width, region.height), m_nbChannels(4), m_pixels(nullptr), m_id(0)
	{
		m_importSettings = sourceTexture->m_importSettings;
		m_ppu = 1.f / m_importSettings.pixelPerUnit;
		ASSERT_LOG_ERROR(sourceTexture, "Null sourceTexture");
		const GLuint srcTex = sourceTexture->GetId();
		ASSERT_LOG_ERROR(srcTex != 0, "Invalid source texture");

		// Save a bit of state we’ll touch
		GLint prevTex = 0, prevReadFBO = 0, prevDrawFBO = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTex);
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFBO);
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDrawFBO);

		// Query source size at its base level
		glBindTexture(GL_TEXTURE_2D, srcTex);
		GLint baseLevel = 0;
		glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, &baseLevel);

		GLint srcW = 0, srcH = 0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, baseLevel, GL_TEXTURE_WIDTH, &srcW);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, baseLevel, GL_TEXTURE_HEIGHT, &srcH);
		ASSERT_LOG_ERROR(srcW > 0 && srcH > 0, "Source texture has no pixels at base level");

		// Compute GL source rect (Rect is top-left based)
		GLint sx0 = region.left;
		GLint sy0 = region.top;
		GLint sx1 = region.left + region.width;
		GLint sy1 = (region.top + region.height);

		// Clamp to source bounds
		sx0 = std::max(0, std::min(sx0, srcW));
		sx1 = std::max(0, std::min(sx1, srcW));
		sy0 = std::max(0, std::min(sy0, srcH));
		sy1 = std::max(0, std::min(sy1, srcH));
		ASSERT_LOG_ERROR(sx1 > sx0 && sy1 > sy0, "Empty/out-of-bounds copy region");

		// Create destination texture
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_size.x, m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		// Basic params
		const GLint minFilter = m_importSettings.useMipmaps ? GL_LINEAR_MIPMAP_LINEAR : m_importSettings.filtering;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_importSettings.filtering);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_importSettings.wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_importSettings.wrapping);

		// Build simple READ/DRAW FBOs
		GLuint readFBO = 0, drawFBO = 0;
		glGenFramebuffers(1, &readFBO);
		glGenFramebuffers(1, &drawFBO);

		// READ: source
		glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTex, baseLevel);
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		// DRAW: destination
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_id, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		// Check both FBOs
		const GLenum readStatus = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
		const GLenum drawStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		ASSERT_LOG_ERROR(readStatus == GL_FRAMEBUFFER_COMPLETE, "READ FBO incomplete");
		ASSERT_LOG_ERROR(drawStatus == GL_FRAMEBUFFER_COMPLETE, "DRAW FBO incomplete");

		// Blit (nearest = exact copy)
		glBlitFramebuffer(
			sx0, sy0, sx1, sy1,          // source rect (flipped+clamped)
			0, 0, m_size.x, m_size.y,// destination rect (full)
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST
		);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFBO);
		glDeleteFramebuffers(1, &readFBO);
		glDeleteFramebuffers(1, &drawFBO);

		if (m_importSettings.useMipmaps)
		{
			glBindTexture(GL_TEXTURE_2D, m_id);
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		SignalReady();
	}
	inline void Texture::LoadFromMemory(unsigned char* buffer, int size)
	{
		m_pixels = stbi_load_from_memory(buffer, size, &m_size.x, &m_size.y, &m_nbChannels, 4);
		ASSERT_LOG_ERROR(m_pixels, "Failed to load a texture from memory.");
	}

	inline void Texture::LoadFromFile(std::string path)
	{
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
		SignalReady();
	}
	inline void Texture::SignalReady()
	{
		// Create a fence so other contexts can see when uploads are done
		GLsync f = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glFlush(); // make the fence visible across shared contexts

		// Replace any previous fence (if any)
		GLsync old = m_pendingFence.exchange(f, std::memory_order_acq_rel);
		if (old) glDeleteSync(old);
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
		SignalReady();
	}
	void Texture::Create1x1White()
	{
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);

		const unsigned char whiteRGBA[4] = { 255, 255, 255, 255 }; // pure white
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whiteRGBA);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_importSettings.wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_importSettings.wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_importSettings.filtering);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_importSettings.filtering);

		glBindTexture(GL_TEXTURE_2D, 0);
		SignalReady();
	}
	void Texture::Reload(std::string path, TextureImportSettings importSettings)
	{
		WaitIfPending();
		glDeleteTextures(1, &m_id);
		if (m_importSettings.keepData)
		{
			stbi_image_free(m_pixels);
		}
		LoadFromFile(path);
		Generate(importSettings);
	}

	void Texture::Copy(const Texture& texture)
	{
		m_importSettings = texture.m_importSettings;
		m_ppu = texture.m_ppu;
		m_size = texture.m_size;
		m_nbChannels = texture.m_nbChannels;
		m_id = texture.m_id;
		m_pbos[0] = 0;
		m_pbos[1] = 0;
		m_currentPBO = 0;
		m_pboSize = 0;
		m_pboEnabled = false;
		m_pendingFence = { 0 };
	}

	void Texture::Destroy()
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
		WaitIfPending();
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, m_id);
	}

	void Texture::SetPixelPerUnit(float ppu)
	{
		m_importSettings.pixelPerUnit = ppu;
		m_ppu = 1.f / m_importSettings.pixelPerUnit;
	}
	void Texture::SetWrapping(TextureWrapping wrapping)
	{
		WaitIfPending();
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
		glBindTexture(GL_TEXTURE_2D, 0);
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
		return m_ppu;
	}

	void Texture::SetPBOStreaming(bool enable)
	{
		if (enable == m_pboEnabled)
			return;
		m_pboSize = static_cast<size_t>(m_size.x) * static_cast<size_t>(m_size.y) * 4; // RGBA8
		if (m_pboSize == 0)
		{
			LOG_ERROR("Texture PBO size = 0");
			return;
		}
		if (enable)
		{
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
		}
	}

	void Texture::UpdateRegion(int x, int y, int w, int h, const void* rgba8)
	{
		if (w <= 0 || h <= 0)
		{
			LOG_ERROR("Texture::UpdateRegion, w or h is zero.");
			return;
		}

		glBindTexture(GL_TEXTURE_2D, m_id);

		if (!m_pboEnabled)
			SetPBOStreaming(true);

		GLsizeiptr regionSize = (GLsizeiptr)w * (GLsizeiptr)h * (GLsizeiptr)4;

		//Bind PBO
		GLuint pbo = m_pbos[m_currentPBO];
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
		// Orphan previous storage to avoid stalls
		glBufferData(GL_PIXEL_UNPACK_BUFFER, (GLsizeiptr)m_pboSize, nullptr, GL_STREAM_DRAW);
		//Map pbo (dst is a pointer to the memory region we want to update)
		void* dst = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, regionSize,
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

		if (dst)
		{
			// Upload from PBO to texture
			memcpy(dst, rgba8, regionSize);
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
			glBindTexture(GL_TEXTURE_2D, m_id);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*)0);

			if (m_importSettings.useMipmaps)
				glGenerateMipmap(GL_TEXTURE_2D);

			//Clean state
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

			//Swap double buffer
			m_currentPBO = 1 - m_currentPBO;
		}
		else
		{
			LOG_ERROR("Texture::UpdateRegion, could not map region.");
			return;
		}

		SignalReady();
	}

	inline void Texture::WaitIfPending() const
	{
		if (GLsync f = m_pendingFence.exchange(0, std::memory_order_acq_rel)) {
			glClientWaitSync(f, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
			glDeleteSync(f);
		}
	}

}