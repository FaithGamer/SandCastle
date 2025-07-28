#include "pch.h"
#include "SandCastle/Render/WireRender.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	WireRender::WireRender() :
		m_needUpdateBuffer(true),
		m_maxPoints(0),
		m_layer(0),
		m_color({ 1, 1, 1, 1 })
	{
		m_vertexArray = makesptr<VertexArray>();
	}
	WireRender::WireRender(unsigned int maxPoints) :
		m_needUpdateBuffer(true),
		m_maxPoints(maxPoints),
		m_layer(0),
		m_color({ 1, 1, 1, 1 })
	{
		m_points.reserve(maxPoints);
		m_vertexBuffer = makesptr<VertexBuffer>(maxPoints * sizeof(LinePoint));
		AttributeLayout layout({
				{ ShaderDataType::Vec3f, "aPosition" },
			});
		
		m_vertexBuffer->SetLayout(layout);
		m_vertexArray = makesptr<VertexArray>();
		m_vertexArray->AddVertexBuffer(m_vertexBuffer);
	}

	void WireRender::AddPoint(Vec3f point)
	{
		if (m_points.size() >= m_maxPoints)
		{
			LOG_ERROR("Wire renderer, cannot add point over max point");
			return;
		}
		m_needUpdateBuffer = true;
		m_points.emplace_back(point);
	}

	void WireRender::PopPoint()
	{
		m_points.pop_back();
	}

	void WireRender::SetPointPosition(size_t index, Vec3f position)
	{
		if (index >= m_points.size())
		{
			LOG_ERROR("WireRender::SetPointPosition, point index is over point count.");
			return;
		}
		m_points[index].point = position;
		m_needUpdateBuffer = true;
	}

	void WireRender::SetLayer(uint32_t layer)
	{
		m_layer = layer;
	}

	void WireRender::SetColor(Vec4f color)
	{
		m_color = color;
	}

	void WireRender::Bind()
	{
		if (m_needUpdateBuffer)
		{
			UpdateBuffer();
			m_needUpdateBuffer = false;
		}
		m_vertexArray->Bind();
	}

	uint32_t WireRender::GetLayer() const
	{
		return m_layer;
	}

	Vec4f WireRender::GetColor() const
	{
		return m_color;
	}

	Vec3f WireRender::GetPointPosition(size_t index) const
	{
		if (index >= m_points.size())
		{
			LOG_ERROR("WireRender::GetPointPosition, point index is over point count.");
			return Vec3f(0);
		}
		return m_points[index].point;
	}

	size_t WireRender::GetPointCount() const
	{
		return m_points.size();
	}

	void WireRender::UpdateBuffer()
	{
		//Index buffer
		m_vertexBuffer->SetData(&m_points[0], (GLuint)sizeof(LinePoint) * (GLuint)m_points.size());
	}
}