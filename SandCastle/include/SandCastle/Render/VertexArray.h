#pragma once

#include "SandCastle/Render/Buffer.h"
#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{
	/// @brief Wrapper around an OpenGL VAO bundling one or more VertexBuffers and an IndexBuffer.
	class VertexArray
	{
	public:
		VertexArray();
		VertexArray(const sptr<VertexBuffer>& vertexBuffer, const sptr<IndexBuffer>& indexBuffer);
		~VertexArray();
		/// @brief Attach an additional VBO and configure its attribute layout.
		void AddVertexBuffer(const sptr<VertexBuffer>& buffer);
		/// @brief Set the index buffer used for indexed draws.
		void SetIndexBuffer(const sptr<IndexBuffer>& buffer);
		/// @brief Bind this VAO for the next draw call.
		void Bind();

		const sptr<IndexBuffer>& GetIndexBuffer() const;

	private:
		GLuint m_layoutIndex = 0;
		std::vector<sptr<VertexBuffer>> m_vertexBuffer;
		sptr<IndexBuffer> m_indexBuffer;
		GLuint m_id;
	};

}