#pragma once

namespace SandCastle
{
	/// @brief Per-material GL pipeline state (currently only depth test).
	/// The material binds its options before each draw to ensure consistent state.
	class RenderOptions
	{
	public:
		//todo stencil
		RenderOptions();
		/// @brief Apply the options to the current GL state.
		void Bind();
		/// @brief Toggle depth testing for this material.
		void SetDepthTest(bool active);
		/// @brief Engine-side stable id (assigned once on creation).
		uint32_t GetID();
		bool GetDepthTest();
	private:
		bool m_depthTest = true;
	};
}
