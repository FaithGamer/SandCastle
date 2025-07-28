#include "SandCastle/Core/Container.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	namespace Container
	{
		template <typename T, typename V>
		int Remove(std::vector<T>& container, V toRemove)
		{
			int removedCount = 0;
			size_t i = 0;
			std::vector<size_t> is;
			for (auto& element : container)
			{
				if (element == toRemove)
				{
					is.emplace_back(i);
				}
				i++;
			}

			for (size_t j : is)
			{
				if (j >= container.size())
				{
					break;
				}
				//find the first value that differs from the one removed.
				//pop_back as long has a different value is not found.
				for (size_t r = container.size() - 1; r >= 0; r--)
				{
					if (j >= container.size())
					{
						break;
					}
					if (container.at(r) != container.at(j))
					{
						container.at(j) = container.at(r);
						container.pop_back();
						removedCount++;
						break;
					}

					container.pop_back();
					removedCount++;
				}
			}
			return removedCount;
		}

		template <typename T>
		bool RemoveAt(std::vector<T>& container, size_t index)
		{
			if (index >= container.size())
			{
				return false;
			}
			container.at(index) = container.back();
			container.pop_back();
			return true;
		}

		template <typename T, typename V>
		bool PushIfDoesntContains(std::vector<T>& container, const V& value)
		{
			auto container_it = std::find(container.begin(), container.end(), value);
			if (container_it == container.end())
			{
				container.push_back(value);
			}
		}

		template <typename T, typename V>
		bool Contains(std::vector<T>& container, const V& value)
		{
			auto container_it = std::find(container.begin(), container.end(), value);
			if (container_it != container.end())
			{
				return true;
			}
			return false;
		}

		template <typename T, typename V>
		int64_t FindIndex(std::vector<T>& container, const V& value)
		{
			for (size_t i = 0; i < container.size(); i++)
			{
				if (container[i] == value)
					return (int64_t)i;
			}
			return -1;
		}
	}
}