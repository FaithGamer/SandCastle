#include "Launch.h"
#include "DrawSprite.h"
#include "DrawAnimation.h"
#include "BasicInputs.h"
#include "Rotation.h"
#include "WindowEvents.h"
#include "Serialization.h"
#include "BenchmarkLotOfSprites.h"
#include "Delegates.h"
#include <iostream>
#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{
	class NonMethodDelegate
	{

	};
	/// @brief Flexible delegate template.
	/// Holds a pointer to a method or function.
	/// Can be pre-bound or not, independently with arguments or object.
	/// Can't pre-bind reference argument.
	/// Warning: any pointer or reference you pre-bind is at user discretion to ensure lifetime.
	template<typename Ret, typename Obj = NonMethodDelegate, typename... Args>
	class Del
	{
		//To do: fynd a way to support reference/pointer as arguments ( the tuple default constructor is the problem here)
	public:
		/// @brief Free function 
		/// arguments provided when calling 
		/// use Call(...args)
		Del(Ret(*function)(Args...)) :
			m_function(function)
		{

		}
		/// @brief Free functions pre-bound arguments
		/// nothing to provide when calling
		/// use Call()
		template <typename... Ts>
		Del(Ret(*function)(Args...), Ts&&... args) :
			m_function(function)
		{
			m_args.emplace(std::forward<Ts>(args)...);
		}
		/// @brief Method
		/// object and arguments provided when calling 
		/// use CallOn(object_ptr, ...args)
		Del(Ret(Obj::* method)(Args...)) :
			m_method(method)
		{

		}
		/// @brief Method, pre-bound object
		/// arguments provided when calling 
		/// use Call(...args)
		Del(Ret(Obj::* method)(Args...), Obj* object) :
			m_method(method),
			m_obj(object)
		{
		}
		/// @brief Method, pre-bound arguments
		/// object provided when calling 
		/// use CallOn(object_ptr)
		template<typename... Ts>
		Del(Ret(Obj::* method)(Args...), Ts&&... args) :
			m_method(method)
		{
			m_args.emplace(std::forward<Ts>(args)...);
		}
		/// @brief Method, pre-bound object and arguments
		/// nothing to provide when calling
		/// use Call()
		template<typename... Ts>
		Del(Ret(Obj::* method)(Args...), Obj* object, Ts&&... args) :
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
	private:
		Ret(*m_function)(Args...) = nullptr;
		Ret(Obj::* m_method)(Args...) = nullptr;
		Obj* m_obj = nullptr;
		std::optional<std::tuple<Args...>> m_args;
	};
}

using namespace SandCastle;
struct Arg
{
	Arg() = default;
	Arg(String str) : s(str)
	{
	}
	Arg(const Arg& copy)
	{
		s = copy.s + "_Copy";
	}

	String s = "Default_Constructed_Arg";
};
class Foo
{
public:
	int Method(Arg arg, float f)
	{
		LOG_INFO("arg = {0}, f = {1}, obj.value = {2}", arg.s, f, value);
		return (int)b + value;
	}
	int value = 42;
};

String Function(String str, Arg arg)
{
	return str + " " + arg.s;
}

void FunctionWithRef(Arg& arg)
{
	LOG_INFO("arg = {0}", arg.s);
}

void Delegates()
{
	Engine::Init();

	//rvalue and lvalue can be mixed indenpendendtly

	//--Free function with no argument bound--
	{
		LOG_INFO("--Free function--");
		Arg lvalue("lvalue");
		Del delegate(&Function);
		auto r = delegate.Call("rvalue", lvalue);
		LOG_INFO({ "return: = {0}" }, r);
		std::cout << std::endl;
	}

	//--Free function with arguments bound--
	{
		LOG_INFO("--Free function, arg bound--");
		String lvalue = "lvalue";
		Del delegate(&Function, lvalue, Arg());
		auto r = delegate.Call();
		LOG_INFO({ "return: = {0}" }, r);
		std::cout << std::endl;
	}

	//--Method with no bounding--
	{
		LOG_INFO("--Method--");
		Del delegate(&Foo::Method);
		Foo obj;
		obj.value = 1;
		float lvalue = 2.1f;
		auto r = delegate.CallOn(&obj, Arg(), lvalue); //Notice when object is provided, method is called CallOn
		LOG_INFO({ "return: = {0}" }, r);
		std::cout << std::endl;
	}

	//--Method with object bound--
	{
		LOG_INFO("--Method obj bound--");
		Foo obj;
		obj.value = 2;
		Del delegate(&Foo::Method, &obj);
		Arg lvalue("lvalue");
		auto r = delegate.Call(lvalue, 3.1f);
		LOG_INFO({ "return: = {0}" }, r);
		std::cout << std::endl;
	}

	//--Method with arg bound--
	{
		LOG_INFO("--Method arg bound--");
		Arg lvalue("lvalue");
		Del delegate(&Foo::Method, lvalue, 4.1f);
		Foo obj;
		obj.value = 3;
		auto r = delegate.CallOn(&obj);
		LOG_INFO({ "return: = {0}" }, r);
		std::cout << std::endl;
	}

	//--Method with arg bound--
	{
		LOG_INFO("--Method arg & obj bound--");
		float lvalue = 5.1f;
		Foo obj;
		obj.value = 4;
		Del delegate(&Foo::Method, &obj, Arg(), lvalue);
		auto r = delegate.Call();
		LOG_INFO({ "return: = {0}" }, r);
		std::cout << std::endl;
	}

	// WARNING, if ref is destructed, del holds a dangling reference
	// Delegate can holds reference or pointer, but it's up to you to 
	// ensure their lifetime.
	{
		LOG_INFO("--Function, ref argument--");
		Arg ref;
		Del del(&FunctionWithRef, ref);
		del.Call();
	}
}


int main()
{
	//Launch();
	//DrawSprite();
	//DrawAnimation();
	//BasicInputs();
	//Rotation();
	//WindowEvents();
	//Serialization();
	//BenchmarkLotOfSprites();
	Delegates();
}