#pragma once

namespace SandCastle
{
	class RenderOptions
	{
	public:
		//todo stencil
		RenderOptions();
		void Bind();
		void SetDepthTest(bool active);
		uint32_t GetID();
		bool GetDepthTest();
	private:
		bool m_depthTest;
	};
}
