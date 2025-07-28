#pragma once

#include <glad/glad.h>
#include "RenderTarget.h"

namespace SandCastle
{
	class Texture;
	/// @brief wrapper around an OpenGL FrameBuffer, RenderBuffer, and Texture Buffer
	///Together they make for an offscreen rendering target allowing for depth and stencil test.
	class RenderTexture : public RenderTarget
	{
	public:
		RenderTexture(Vec2u size);
		void BindTexture(uint32_t);
		void Bind() override;
		void Clear() override;
		void SetSize(Vec2u size) override;
		sptr<Texture> GetTexture() const;
		sptr<Texture> GetTexture(float pixelPerUnit) const;
	private:
		GLuint m_frameBufferId;
		GLuint m_renderBufferId;
		GLuint m_textureId;
		Vec2u m_size;
	};
}
