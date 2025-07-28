#include "pch.h"
#include <glad/glad.h>

#include "SandCastle/Core/Log.h"
#include "SandCastle/Render/VertexArray.h"


namespace SandCastle
{
	VertexArray::VertexArray()
	{
		glGenVertexArrays(1, &m_id);
	}

	VertexArray::VertexArray(const sptr<VertexBuffer>& vertexBuffer, const sptr<IndexBuffer>& indexBuffer)
	{
		glGenVertexArrays(1, &m_id);
		AddVertexBuffer(vertexBuffer);
		SetIndexBuffer(indexBuffer);
	}

	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &m_id);
	}

	void VertexArray::AddVertexBuffer(const sptr<VertexBuffer>& buffer)
	{
		glBindVertexArray(m_id);
		buffer->Bind();

		ASSERT_LOG_ERROR(buffer->GetLayout().GetElements().size(), "VertexBuffer layout is empty");
		//We need to tell OpenGL how to interpret the data in the Vertex Array Buffer
		GLuint layoutIndex = 0;
		const auto& layout = buffer->GetLayout();
		for (const auto& element : layout)
		{
			glVertexAttribPointer(layoutIndex,
				ShaderDataTypeCount(element.type),
				ShaderDataTypeGLType(element.type),
				element.normalized,
				layout.GetStride(),
				(const void*)element.offset);
			glEnableVertexAttribArray(layoutIndex);

			layoutIndex++;
		}

		m_vertexBuffer.push_back(buffer);
		glBindVertexArray(0);
	}

	

	void VertexArray::SetIndexBuffer(const sptr<IndexBuffer>& buffer)
	{
		glBindVertexArray(m_id);
		buffer->Bind();
		m_indexBuffer = buffer;
		glBindVertexArray(0);
	}

	void VertexArray::Bind()
	{
		glBindVertexArray(m_id);
		if (m_indexBuffer)
			m_indexBuffer->Bind();
	}

	const sptr<IndexBuffer>& VertexArray::GetIndexBuffer() const
	{
		return m_indexBuffer;
	}
}