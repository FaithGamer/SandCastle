#pragma once

namespace SandCastle
{
	///@brief Bind any Type with a unique identifier.
	/// id is determined the first time GetId is called
	class TypeId
	{
	public:
		/// @brief Get a stable, process-wide unique integer id for type T.
		/// The id is allocated lazily the first time GetId<T>() is called and
		/// is not stable across builds — do NOT serialize it.
		template <typename T>
		static int32_t GetId()
		{
			static const int32_t id = currentTypeId++;
			return id;
		}
		static int32_t currentTypeId;
	};
}