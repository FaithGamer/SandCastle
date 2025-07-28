#include "pch.h"
#include "SandCastle/Render/UniformBuffer.h"


namespace SandCastle
{
	UniformBuffer::UniformBuffer(GLsizeiptr size, GLuint binding)
	{
		glGenBuffers(1, &m_id);
		glBindBuffer(GL_UNIFORM_BUFFER, m_id);
		glBufferData(GL_UNIFORM_BUFFER, size, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, binding, m_id, 0, size);
	}

	UniformBuffer::~UniformBuffer()
	{
		glDeleteBuffers(1, &m_id);
	}

	void UniformBuffer::SetData(const void* data, GLsizeiptr size, GLuint offset)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_id);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}