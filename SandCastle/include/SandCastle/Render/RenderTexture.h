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
		/// @brief Create an offscreen render target of the given pixel size.
		RenderTexture(Vec2u size);
		/// @brief Bind the underlying color texture to a sampler slot.
		void BindTexture(uint32_t);
		void Bind() override;
		void Clear() override;
		/// @brief Resize the framebuffer (reallocates the GL textures).
		void SetSize(Vec2u size) override;
		/// @brief View the color attachment as a Texture you can draw with sprites.
		sptr<Texture> GetTexture() const;
		/// @brief Same as GetTexture() but with a custom pixel-per-unit setting.
		sptr<Texture> GetTexture(float pixelPerUnit) const;
	private:
		GLuint m_frameBufferId;
		GLuint m_renderBufferId;
		GLuint m_textureId;
		Vec2u m_size;
	};
}
