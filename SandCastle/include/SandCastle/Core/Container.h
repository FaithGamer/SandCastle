#pragma once
#include <vector>
#include <unordered_set>

namespace SandCastle
{
    namespace Container
    {
        
        
        
        /// @brief Remove all occurences of value from a std::vector, Does NOT guarantee to maintain vector's order.
        /// it replaces the removed values with the back() value if any.
        /// @return the number of occurence removed
        template <typename T, typename V>
        int Remove(std::vector<T>& container, V value);

        /// @brief Remove a contained value at the given index. Does NOT maintain order.
        /// Replace the removed value with the back() value.
        /// @return true if deletion succeeds.
        template <typename T>
        bool RemoveAt(std::vector<T>& container, size_t index);

        /// @brief Push only if the std::vector doesnt contain the value.
        /// @return true if insertion has happened.
        template <typename T, typename V>
        bool PushIfDoesntContains(std::vector<T>& container, const V& value);
        
        /// @brief Check if std::vector contains the given value.
        template <typename T, typename V>
        bool Contains(std::vector<T>& container, const V& value);

        /// @brief Find the index of the given value if contained.
        /// @return Index of the value, -1 if vector doesn't contain the value.
        template <typename T, typename V>
        int64_t FindIndex(std::vector<T>& container, const V& value);


        template <typename T>
        class OrderedSet {
        public:
            bool Insert(const T& value) 
            {
                if (m_set.find(value) != m_set.end())
                    return false;
                m_set.insert(value);
                m_vec.emplace_back(value);
                return true;
            }

            const std::vector<T>& Data() const 
            { 
                return m_vec; 
            }

            void Clear()
            {
                m_set.clear();
                m_vec.clear();
            }

        private:
            std::vector<T> m_vec;
            std::unordered_set<T> m_set;
        };
       
    }
}

#include "Container.tpp"