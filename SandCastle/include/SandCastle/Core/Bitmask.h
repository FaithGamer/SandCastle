#pragma once

#include <cstdint>
#include <unordered_map>
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	/// @brief Thin wrapper around an integer treated as a bitfield.
	/// Provides AddFlag/RemoveFlag/Contains in terms of bitwise OR/AND/NOT.
	/// Used heavily by the physics layer for collision masks.
	template <typename T>
	class Bitmask
	{
	public:
		Bitmask() : flags(0)
		{

		}
		Bitmask(T Flags) : flags(Flags)
		{

		}
		/// @brief Reset every bit to 0.
		void Clear()
		{
			flags = 0;
		}
		/// @brief Set the bits of `flag`.
		void AddFlag(T flag)
		{
			flags = flags | flag;
		}
		/// @brief Clear the bits of `flag`.
		void RemoveFlag(T flag)
		{
			flags = flags & ~flag;
		}
		/// @brief True if every bit of `flag` is set.
		bool Contains(T flag)
		{
			return (flags & flag) == flag;
		}
		T flags;
	};

	/// @brief 8-bit bitmask alias.
	typedef Bitmask<uint8_t> Bitmask8;
	/// @brief 16-bit bitmask alias (matches Box2D's collision filter width).
	typedef Bitmask<uint16_t> Bitmask16;
	/// @brief 32-bit bitmask alias.
	typedef Bitmask<uint32_t> Bitmask32;
	/// @brief 64-bit bitmask alias.
	typedef Bitmask<uint64_t> Bitmask64;

	/*class Bitmask16
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
	};*/


	/// @brief Dynamic collection of named flags, enabling the generation of bitmasks with custom names for each flag
	class Filter16
	{
	public:
		Filter16();
		/// @brief Register a new named flag and assign it the next free bit (max 16).
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

		/// @brief True if `mask` has the bit registered for the given flag name set.
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
