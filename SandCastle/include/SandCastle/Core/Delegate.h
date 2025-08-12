#pragma once

#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{
	class FunctionDelegate
	{

	};
	/// @brief Flexible delegate template.
	/// Holds a pointer to a method or function.
	/// Can be pre-bound or not, independently with arguments or object.
	/// Can't pre-bind reference argument.
	/// Warning: any pointer or reference you pre-bind is at user discretion to ensure lifetime.
	template<typename Ret, typename Obj = FunctionDelegate, typename... Args>
	class Delegate
	{
		//To do: fynd a way to support reference/pointer as arguments ( the tuple default constructor is the problem here)
	public:
		/// @brief Free function 
		/// arguments provided when calling 
		/// use Call(...args)
		Delegate(Ret(*function)(Args...)) :
			m_function(function)
		{

		}
		/// @brief Free functions pre-bound arguments
		/// nothing to provide when calling
		/// use Call()
		template <typename... Ts>
		Delegate(Ret(*function)(Args...), Ts&&... args) :
			m_function(function)
		{
			m_args.emplace(std::forward<Ts>(args)...);
		}
		/// @brief Method
		/// object and arguments provided when calling 
		/// use CallOn(object_ptr, ...args)
		Delegate(Ret(Obj::* method)(Args...)) :
			m_method(method)
		{

		}
		/// @brief Method, pre-bound object
		/// arguments provided when calling 
		/// use Call(...args)
		Delegate(Ret(Obj::* method)(Args...), Obj* object) :
			m_method(method),
			m_obj(object)
		{
		}
		/// @brief Method, pre-bound arguments
		/// object provided when calling 
		/// use CallOn(object_ptr)
		template<typename... Ts>
		Delegate(Ret(Obj::* method)(Args...), Ts&&... args) :
			m_method(method)
		{
			m_args.emplace(std::forward<Ts>(args)...);
		}
		/// @brief Method, pre-bound object and arguments
		/// nothing to provide when calling
		/// use Call()
		template<typename... Ts>
		Delegate(Ret(Obj::* method)(Args...), Obj* object, Ts&&... args) :
			m_method(method),
			m_obj(object)
		{
			m_args.emplace(std::forward<Ts>(args)...);
		}
		/// @brief Call the function or method.
		/// If the delegate holds a method, it will be called on the object provided in constructor.
		/// @return return of the method/function
		Ret Call()
		{
			if (m_method != nullptr)
			{
				ASSERT_LOG_ERROR((int)(m_obj != nullptr), "Calling a method delegate without object.");
				return[this] <std::size_t... I>
					(std::index_sequence<I...>)
				{
					return (static_cast<Obj*>(m_obj)->*m_method)(std::forward<Args>(std::get<I>(*m_args))...);
				}
				(std::make_index_sequence<sizeof...(Args)>());
			}
			else
			{
				return[this] <std::size_t... I>
					(std::index_sequence<I...>)
				{
					return m_function(std::forward<Args>(std::get<I>(*m_args))...);
				}
				(std::make_index_sequence<sizeof...(Args)>());
			}

		}
		/// @brief Call the function or method.
		/// If the delegate holds a method, it will be called on the object provided in constructor.
		/// Probably crash if the delegate holds a method but no object instance.
		/// @param ...args arguments for the method/function
		/// @return return of the method/function
		template<typename... Ts>
		Ret Call(Ts&&... args)
		{
			if (m_method != nullptr)
			{
				ASSERT_LOG_ERROR((int)(m_obj != nullptr), "Calling a method delegate without object.");
				return (static_cast<Obj*>(m_obj)->*m_method)(std::forward<Ts>(args)...);
			}
			else
			{
				return m_function(std::forward<Ts>(args)...);
			}
		}
		/// @brief Call the method.
		/// @param object instance to call the member function on.
		/// @param ...args the arguments for the method.
		/// @return return of the method.
		template<typename... Ts>
		Ret CallOn(void* const object, Ts&&... args)
		{
			return (static_cast<Obj*>(object)->*m_method)(std::forward<Ts>(args)...);
		}
		/// @brief Call the the method with arguments provided in the delegate's constructor.
		/// The behaviour is undefined if no arguments were provided in the delegate's constructor.
		/// @param object object to call the member function upon
		/// @return return of the function
		Ret CallOn(void* const object)
		{
			return[this, object] <std::size_t... I>
				(std::index_sequence<I...>)
			{
				return (static_cast<Obj*>(object)->*m_method)(std::forward<Args>(std::get<I>(*m_args))...);
			}
			(std::make_index_sequence<sizeof...(Args)>());
		}
		Obj* GetObj()
		{
			return m_obj;
		}
		Ret(Obj::* GetMethod())(Args...)
		{
			return m_method;
		}
		Ret(*GetFunction())(Args...)
		{
			return m_function;
		}
		bool Equals(const Delegate<Ret, Obj, Args...>& other)
		{
			return m_function == other.m_function
				&& m_method == other.m_method
				&& m_obj == other.m_obj;
		}
	private:
		Ret(*m_function)(Args...) = nullptr;
		Ret(Obj::* m_method)(Args...) = nullptr;
		Obj* m_obj = nullptr;
		std::optional<std::tuple<Args...>> m_args;
	};
}