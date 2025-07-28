#include "pch.h"


#include <glad/glad.h>
#include "SandCastle/Render/RenderOptions.h"

namespace SandCastle
{

	RenderOptions::RenderOptions() : m_depthTest(true)
	{

	}
	void RenderOptions::Bind()
	{
		if (m_depthTest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}
	void RenderOptions::SetDepthTest(bool active)
	{
		m_depthTest = active;
	}
	uint32_t RenderOptions::GetID()
	{
		return 0;
	}
	bool RenderOptions::GetDepthTest()
	{
		return m_depthTest;
	}
}