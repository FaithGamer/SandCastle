#pragma once

#include "SandCastle/Core/Vec.h"
#include "SandCastle/Render/VertexBuffer.h"
#include "SandCastle/Render/VertexArray.h"
#include "SandCastle/Core/std_macros.h"


namespace SandCastle
{
	/// @brief Render lines of 1px width
	class WireRender
	{
	public:
		WireRender();
		WireRender(unsigned int maxPoints);
		void AddPoint(Vec3f point);
		void PopPoint();
		void SetPointPosition(size_t index, Vec3f position);
		void SetLayer(uint32_t layer);
		void SetColor(Vec4f color);

		void Bind();
		uint32_t GetLayer() const;
		Vec4f GetColor() const;
		Vec3f GetPointPosition(size_t index) const;
		size_t GetPointCount() const;


	
	private:
		struct LinePoint
		{
			Vec3f point;
		};
		void UpdateBuffer();
		sptr<VertexBuffer> m_vertexBuffer;
		sptr<VertexArray> m_vertexArray;
		std::vector<LinePoint> m_points;
		std::vector<uint32_t> m_indices;
		uint32_t m_layer;
		unsigned int m_maxPoints;
		bool m_needUpdateBuffer;
		Vec4f m_color;

	};
}
