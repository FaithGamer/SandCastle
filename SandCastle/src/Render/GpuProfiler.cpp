#include "pch.h"

#include <algorithm>
#include "SandCastle/Render/GpuProfiler.h"

namespace SandCastle
{
	static constexpr float kWindowSeconds = 0.5f;

	void GpuProfiler::Init()
	{
		m_init = true;
		GLint bits = 0;
		glGetQueryiv(GL_TIMESTAMP, GL_QUERY_COUNTER_BITS, &bits);
		m_profile.supported = bits > 0;
		if (!m_profile.supported)
			return;
		//Never deleted: they live as long as the render context, which shutdown destroys.
		for (auto& frame : m_frames)
			glGenQueries(kStamps, frame.queries);
		m_window.Restart();
	}

	void GpuProfiler::BeginFrame()
	{
		if (!m_init)
			Init();
		if (!m_profile.supported)
			return;
		m_current = (m_current + 1) % kFrames;
		Frame& frame = m_frames[m_current];
		if (frame.pending)
			Collect(frame);
		frame.count = 0;
		frame.pending = false;
		Stamp("frame");
	}

	void GpuProfiler::Stamp(const char* group, const String& name)
	{
		if (m_current < 0)
			return;
		Frame& frame = m_frames[m_current];
		if (frame.count >= kStamps)
			return;
		glQueryCounter(frame.queries[frame.count], GL_TIMESTAMP);
		//Assigned in place, so a label keeps its buffer from one frame to the next.
		String& label = frame.labels[frame.count];
		label.assign(group);
		if (!name.empty())
		{
			label += ' ';
			label += name;
		}
		frame.count++;
	}

	void GpuProfiler::EndFrame()
	{
		if (m_current < 0)
			return;
		Frame& frame = m_frames[m_current];
		frame.pending = frame.count > 1;
	}

	void GpuProfiler::Collect(Frame& frame)
	{
		//Timestamps complete in submission order: the last one in means they all are.
		GLint available = 0;
		glGetQueryObjectiv(frame.queries[frame.count - 1], GL_QUERY_RESULT_AVAILABLE, &available);
		if (!available)
		{
			//Dropped. The next frame's interval would span two frames, so it goes too.
			m_lastStart = 0;
			return;
		}
		GLuint64 stamps[kStamps];
		for (int i = 0; i < frame.count; i++)
			glGetQueryObjectui64v(frame.queries[i], GL_QUERY_RESULT, &stamps[i]);
		auto elapsedMs = [](GLuint64 from, GLuint64 to) { return to > from ? (double)(to - from) * 1e-6 : 0.; };

		//A label new to this window goes right after the previous stamp's section, so
		//the sections keep frame order even when one comes and goes.
		size_t next = 0;
		for (int i = 1; i < frame.count; i++)
		{
			size_t at = 0;
			while (at < m_accum.size() && m_accum[at].label != frame.labels[i])
				at++;
			if (at == m_accum.size())
			{
				at = std::min(next, m_accum.size());
				m_accum.insert(m_accum.begin() + at, Accum{ frame.labels[i] });
			}
			m_accum[at].ms += elapsedMs(stamps[i - 1], stamps[i]);
			next = at + 1;
		}
		m_frameMs += elapsedMs(stamps[0], stamps[frame.count - 1]);
		//An interval longer than the averaging window isn't a frame period: the window
		//was minimized (nothing renders) between the two frames.
		const double interval = m_lastStart != 0 ? elapsedMs(m_lastStart, stamps[0]) : 0.;
		if (interval > 0. && interval < kWindowSeconds * 1000.)
		{
			m_intervalMs += interval;
			m_intervals++;
		}
		m_lastStart = stamps[0];
		m_collected++;
		if ((float)m_window.GetElapsed() >= kWindowSeconds)
			Publish();
	}

	void GpuProfiler::Publish()
	{
		m_profile.sections.resize(m_accum.size());
		for (size_t i = 0; i < m_accum.size(); i++)
		{
			m_profile.sections[i].label = m_accum[i].label;
			m_profile.sections[i].ms = (float)(m_accum[i].ms / m_collected);
		}
		m_profile.frameMs = (float)(m_frameMs / m_collected);
		m_profile.intervalMs = m_intervals > 0 ? (float)(m_intervalMs / m_intervals) : 0.f;
		m_profile.frames = m_collected;
		m_accum.clear();
		m_frameMs = 0.;
		m_intervalMs = 0.;
		m_intervals = 0;
		m_collected = 0;
		m_window.Restart();
	}
}
