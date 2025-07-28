#include "pch.h"
#include "SandCastle/Physics/Bitmask.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	Filter16::Filter16() : m_maxFlags(15)
	{

	}
	void Filter16::AddFlag(String name)
	{
		if (m_flags.size() >= m_maxFlags)
		{
			LOG_WARN("Max flag count reached for filter when trying to add " + name);
		}
		else
		{
			int64_t flag = (int64_t)std::pow(2, m_flags.size());
			if (!m_flags.insert(MakePair(name, flag)).second)
			{
				LOG_WARN("Filter16 flag already exists: " + name);
			}
		}
	}

	bool Filter16::BitmaskContains(Bitmask16 mask, String flag)
	{
		auto find_it = m_flags.find(flag);
		if (find_it == m_flags.end())
		{
			LOG_WARN("Filter16's flag doesn't exists: " + flag + ", false returned.");
			return false;
		}
		return mask.Contains(find_it->second);
	}
}