#pragma once
#include <glad/glad.h>
#include <stdint.h>

namespace SandCastle
{

	/// @brief Wrapper around OpenGL Element Array Buffer
	class IndexBuffer
	{
	public:
		/// @brief Make an index buffer with dynamic_draw
		/// @param indices
		/// @param count indices count
		IndexBuffer(uint32_t* indices, uint32_t count);
		IndexBuffer(uint32_t* indices, uint32_t count, GLenum type);
		~IndexBuffer();

		void Bind();
		GLsizei GetCount() const;
	private:
		GLuint m_id;
		GLsizei m_count;
	}; 
}