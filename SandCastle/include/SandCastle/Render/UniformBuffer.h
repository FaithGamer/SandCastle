#pragma once
#include <glad/glad.h>

namespace SandCastle
{
	/// @brief Wrapper around an OpenGL Uniform Buffer Object bound to a fixed binding point. Used for shared scene data (camera matrices, viewport size, etc.).
	class UniformBuffer
	{
	public:
		//To do: something to calculate size based on the uniform buffer content
		/// @brief Allocate `size` bytes and bind to the given UBO binding point.
		UniformBuffer(GLsizeiptr size, GLuint binding);
		~UniformBuffer();
		/// @brief Upload `size` bytes from `data` at `offset` within the UBO.
		void SetData(const void* data, GLsizeiptr size, GLuint offset);
	private:
		GLuint m_id;
	};
}