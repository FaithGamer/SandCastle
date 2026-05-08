
#include "pch.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Render/RenderTexture.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Core/Math.h"
#include "stb/stb_image_write.h"

namespace SandCastle
{
	RenderTexture::RenderTexture(Vec2u size)
	{
		m_size = size;
		//Framebuffer
		glGenFramebuffers(1, &m_frameBufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);

		//Color texture attachment
		glGenTextures(1, &m_textureId);
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureId, 0);

		//Depth texture attachment (sampleable). Stencil is dropped - not used by the engine.
		glGenTextures(1, &m_depthTextureId);
		glBindTexture(GL_TEXTURE_2D, m_depthTextureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureId, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			LOG_ERROR("Frame Buffer incomplete");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void RenderTexture::BindTexture(uint32_t textureUnit)
	{
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, m_textureId);
	}

	void RenderTexture::BindDepthTexture(uint32_t textureUnit)
	{
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, m_depthTextureId);
	}

	void RenderTexture::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);
		glViewport(0, 0, Math::FloorToEven(m_size.x), Math::FloorToEven(m_size.y));
		//glViewport(0, 0, m_size.x, m_size.y);
	}

	void RenderTexture::Clear()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void RenderTexture::SetSize(Vec2u size)
	{
		m_size = size;
		//Delete existing GL objects (resizing them isn't a thing for textures and FBO attachments)
		glDeleteFramebuffers(1, &m_frameBufferId);
		glDeleteTextures(1, &m_depthTextureId);

		//Resize the color texture in place
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		//Regen FBO
		glGenFramebuffers(1, &m_frameBufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);

		//Regen depth texture
		glGenTextures(1, &m_depthTextureId);
		glBindTexture(GL_TEXTURE_2D, m_depthTextureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

		//Attach color texture and depth texture
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureId, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureId, 0);
	}

	sptr<Texture> RenderTexture::GetTexture() const
	{
		sptr<Texture> texture = makesptr<Texture>();
		texture->m_id = m_textureId;
		texture->m_size = m_size;
		return texture;
	}

	sptr<Texture> RenderTexture::GetTexture(float pixelPerUnit) const
	{
		sptr<Texture> texture = makesptr<Texture>();
		texture->m_id = m_textureId;
		texture->m_size = m_size;
		texture->m_importSettings.pixelPerUnit = 1.f / pixelPerUnit;
		return texture;
	}
}
