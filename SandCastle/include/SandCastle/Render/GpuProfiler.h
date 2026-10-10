#pragma once

#include <glad/glad.h>
#include <vector>
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Core/Time.h"

namespace SandCastle
{
	/// @brief Where the GPU spends a frame, measured with GL timestamp queries.
	/// Renderer2D stamps the end of each phase of its render thread frame (the frame's
	/// clears, the sprite batches, each GPU particle pool, each layer composite, each
	/// post-process pass...), and a section is the GPU time between two stamps.
	/// Results are read back a few frames late and never waited on (a frame whose
	/// results aren't in yet is dropped), then averaged into a Profile every half
	/// second. Render thread only: read it from System::OnImGui. Renderer2D only
	/// stamps outside distrib builds, so the profile stays empty there.
	///
	/// Timestamps follow the GPU timeline, so a section also counts any time the GPU
	/// sat waiting for the render thread to submit the next commands. Renderer2D puts
	/// the render thread's two long CPU stretches in sections of their own, both in the
	/// "wait" group: "wait batching" (building the sprite batches; a batch that fills up
	/// early is drawn in a "sprites" section of its own) and "wait imgui" (building the
	/// dev UI). They read about 0 when the GPU is the bottleneck. A slow render thread
	/// (a Debug build) can still inflate the other sections a little.
	class GpuProfiler
	{
	public:
		/// @brief One phase of the frame.
		struct Section
		{
			/// @brief "group name", e.g. "composite horde" or "post bloom.shader".
			/// Stamps sharing a label in one frame are summed into one section.
			String label;
			/// @brief Average GPU time per frame, in ms. A frame the section was missing
			/// from counts 0, so it reads what the section costs on average.
			float ms = 0.f;
		};
		/// @brief Averages over the last half second of collected frames.
		struct Profile
		{
			/// @brief In frame order.
			std::vector<Section> sections;
			/// @brief GPU time from a frame's first stamp to its last.
			float frameMs = 0.f;
			/// @brief GPU time from one frame's first stamp to the next one's: the GPU's
			/// frame period. (frameMs - the "wait" sections) / intervalMs is how busy the
			/// GPU is.
			float intervalMs = 0.f;
			/// @brief Frames averaged (0 = nothing measured yet).
			int frames = 0;
			/// @brief False when the driver has no timestamp counter.
			bool supported = true;
		};

		/// @brief Open a frame: read back the oldest frame's results, then stamp the start.
		void BeginFrame();
		/// @brief Close the section labelled `group` (+ " " + `name` when given).
		void Stamp(const char* group, const String& name = String());
		/// @brief Close the frame. Its results are read back a few frames later.
		void EndFrame();
		const Profile& GetProfile() const { return m_profile; }

	private:
		static constexpr int kFrames = 6;  //frames in flight before a frame's queries are reused
		static constexpr int kStamps = 128; //stamps per frame, the extra ones are ignored
		struct Frame
		{
			GLuint queries[kStamps] = {};
			String labels[kStamps];
			int count = 0;
			bool pending = false;
		};
		struct Accum
		{
			String label;
			double ms = 0.;
		};
		void Init();
		void Collect(Frame& frame);
		void Publish();

		Frame m_frames[kFrames];
		int m_current = -1;
		bool m_init = false;
		GLuint64 m_lastStart = 0;
		std::vector<Accum> m_accum;
		double m_frameMs = 0.;
		double m_intervalMs = 0.;
		int m_intervals = 0;
		int m_collected = 0;
		Clock m_window;
		Profile m_profile;
	};
}
