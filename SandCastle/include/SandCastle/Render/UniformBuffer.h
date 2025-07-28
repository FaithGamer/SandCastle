#pragma once
#include <glad/glad.h>

namespace SandCastle
{
	class UniformBuffer
	{
	public:
		//To do: something to calculate size based on the uniform buffer content
		UniformBuffer(GLsizeiptr size, GLuint binding);
		~UniformBuffer();
		void SetData(const void* data, GLsizeiptr size, GLuint offset);
	private:
		GLuint m_id;
	};
}