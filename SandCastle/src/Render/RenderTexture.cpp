
#include "pch.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Render/RenderTexture.h"
#include "SandCastle/Render/Texture.h"
#include "stb/stb_image_write.h"

namespace SandCastle
{
	RenderTexture::RenderTexture(Vec2u size)
	{
		m_size = size;
		//Framebuffer
		glGenFramebuffers(1, &m_frameBufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);

		//Texture buffer attachment
		glGenTextures(1, &m_textureId);
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureId, 0);

		//Render buffer attachement for depth and stencil testing
		glGenRenderbuffers(1, &m_renderBufferId);
		glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferId);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y); // use a single renderbuffer object for both a depth AND stencil buffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBufferId); // now actually attach it

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

	void RenderTexture::Bind()
	{
		glViewport(0, 0, m_size.x, m_size.y);
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);
	}

	void RenderTexture::Clear()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void RenderTexture::SetSize(Vec2u size)
	{
		m_size = size;
		//Delete frame/render buffer (because I'm not sure resizing would work otherwise)
		glDeleteFramebuffers(1, &m_frameBufferId);
		glDeleteRenderbuffers(1, &m_renderBufferId);

		//Resizing the texture (should be fine for the texture)
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		//Regen frame/render buffer
		glGenFramebuffers(1, &m_frameBufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);

		glGenRenderbuffers(1, &m_renderBufferId);
		glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferId);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y); // use a single renderbuffer object for both a depth AND stencil buffer.

		//Attach texture and frame buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureId, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBufferId);
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