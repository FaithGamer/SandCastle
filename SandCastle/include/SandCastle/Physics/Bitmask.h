#pragma once

#include <cstdint>
#include <unordered_map>
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	/// @brief 15 flags
	class Bitmask16
	{
	public:
		Bitmask16() : flags(0)
		{

		}
		Bitmask16(uint16_t Flags) : flags(Flags)
		{

		}
		void Clear()
		{
			flags = 0;
		}
		void AddFlag(uint16_t flag)
		{
			flags = flags | flag;
		}
		void RemoveFlag(uint16_t flag)
		{
			flags = flags & ~flag;
		}
		bool Contains(uint16_t flag)
		{
			return (flags & flag) == flag;
		}
		uint16_t flags;
	};
	class Bitmask32
	{
	public:
		Bitmask32() : flags(0)
		{

		}
		Bitmask32(uint32_t Flags) : flags(Flags)
		{

		}
		void Clear()
		{
			flags = 0;
		}
		void AddFlag(uint32_t flag)
		{
			flags = flags | flag;
		}
		void RemoveFlag(uint32_t flag)
		{
			flags = flags & ~flag;
		}
		bool Contains(uint32_t flag)
		{
			return (flags & flag) == flag;
		}
		uint32_t flags;
	};
	class Bitmask64
	{
	public:
		Bitmask64() : flags(0)
		{

		}
		Bitmask64(uint64_t Flags) : flags(Flags)
		{

		}
		void Clear()
		{
			flags = 0;
		}
		void AddFlag(uint64_t flag)
		{
			flags = flags | flag;
		}
		void RemoveFlag(uint64_t flag)
		{
			flags = flags & ~flag;
		}
		bool Contains(uint64_t flag)
		{
			return (flags & flag) == flag;
		}
		uint64_t flags;
	};


	/// @brief Dynamic collection of named flags, enabling the generation of bitmasks with custom names for each flag
	class Filter16 
	{
	public:
		Filter16();
		void AddFlag(String name);

		/// @brief Generate a bitmask for the given flags names. 
		/// Flags must have been created first with the AddFlag method.
		/// @tparam ...str bewteen 1 and 31 String corresponding to the flags
		template<typename... Str>
		Bitmask16 GetMask(Str ...filters)
		{
			Bitmask16 mask;
			SetMask(mask, filters...);
			return mask;
		}
		inline std::unordered_map<String, uint16_t> GetFlags()
		{
			return m_flags;
		}

		bool BitmaskContains(Bitmask16 mask, String flag);

	private:
		template<typename... str>
		void SetMask(Bitmask16& mask, String filter, str ...filters)
		{
			auto find_it = m_flags.find(filter);
			if (find_it == m_flags.end())
			{
				LOG_ERROR("Filter16's flag doesn't exists: " + filter + ", couldn't generate a proper bitmask.");
			}
			else
			{
				uint16_t flag = find_it->second;
				mask.AddFlag(flag);
				SetMask(mask, filters...);
			}
		}
		void SetMask(Bitmask16& mask)
		{

		}

		Bitmask16 m_allFlags;
		size_t m_maxFlags;
		std::unordered_map<String, uint16_t> m_flags;
	};


}
