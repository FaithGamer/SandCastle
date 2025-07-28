#pragma once

#include "SandCastle/Core/Vec.h"
#include "SandCastle/Render/VertexBuffer.h"
#include "SandCastle/Render/VertexArray.h"
#include "SandCastle/Core/std_macros.h"

#define LINE_WIDTH_INDICES 5

namespace SandCastle
{
	/// @brief Render lines with customisable width
	/// Line renderer doesn't compute scales transformation correctly.
	class LineRenderer
	{
	public:
		LineRenderer(unsigned int maxPoints);
		void AddPoint(Vec3f point);
		void PopPoint();
		void SetPointPosition(size_t index, Vec3f position);
		void SetLayer(uint32_t layer);
		void SetColor(Vec4f color);
		/// @brief Set the width of the line at different indices.
		/// Width indices count is fixed and is NOT line point count.
		/// @param index range between 0 and LINE_WIDTH_INDICES-1, 0 = start point, LINE_WIDTH_INDICES-1 = end point.
		/// using a value under or above will set every index to the same width.
		void SetWidth(float width, int index = -1);
		/// @brief Set the number of vertices for the round shape and the tips. 
		/// @param count 0 for flat. max 21
		void SetEndCapVertices(int count);
		void Reverse();
		void Bind();
		uint32_t GetLayer() const;
		Vec4f GetColor() const;
		Vec3f GetPointPosition(size_t index) const;
		size_t GetPointCount() const;
		int GetEndCapVertices() const;
		/// @brief Return an array of size LINE_WIDTH_INDICES with the width set
		float* GetWidthArray();
		/// @brief Width at a given distance
		/// @param distance 0 = begin 1 = end
		float Width(float distance) const;
	
	private:
		struct LinePoint
		{
			Vec3f point;
			float index;
		};
		void UpdateBuffer();
		sptr<VertexBuffer> m_vertexBuffer;
		sptr<VertexArray> m_vertexArray;
		std::vector<LinePoint> m_points;
		std::vector<uint32_t> m_indices;
		uint32_t m_layer;
		unsigned int m_maxPoints;
		bool m_needUpdateBuffer;
		float m_width[LINE_WIDTH_INDICES];
		Vec4f m_color;
		int m_endCapVertices;

	};
}
