#include "Launch.h"
#include "DrawSprite.h"
#include "DrawAnimation.h"
#include "BasicInputs.h"
#include "Rotation.h"
#include "WindowEvents.h"
#include "Serialization.h"
#include "BenchmarkLotOfSprites.h"
#include "Delegates.h"

#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{

	//to do: boolean instead of type erasure  

	/// @brief Holds function pointer to either free function of member function
	/// do not support function with pointer or reference as argument yet
	class Dummy
	{

	};
	template<typename Ret, typename Obj = Dummy, typename... Args>
	class Del
	{
		//To do: fynd a way to support reference/pointer as arguments ( the tuple default constructor is the problem here)
	public:
		/// @brief Free function, arguments provided when calling
		Del(Ret(*function)(Args...)) :
			m_function(function)
		{

		}
		/// @brief Free functions, arguments pre-provided
		template <typename... Ts>
		Del(Ret(*function)(Args...), Ts&&... args)
			: m_args(std::forward<Ts>(args)...),
			m_function(function)
		{

		}
		/// @brief Method, object and arguments provided when calling
		Del(Ret(Obj::* method)(Args...)) :
			m_method(method)
		{

		}
		/// @brief Method, arguments provided when calling
		Del(Ret(Obj::* method)(Args...), Obj* object) :
			m_method(method),
			m_obj(object)
		{
		}
		/// @brief Method, object provided when calling
		template<typename... Ts>
		Del(Ret(Obj::* method)(Args...), Ts&&... args)
			: m_args(std::forward<Ts>(args)...),
			m_method(method)
		{

		}
		/// @brief Method, nothing to provide when calling
		template<typename... Ts>
		Del(Ret(Obj::* method)(Args...), Obj* object, Ts&&... args)
			: m_args(std::forward<Ts>(args)...),
			m_method(method),
			m_obj(object)
		{

		}



		/// @brief Call the function with arguments provided in the delegate's constructor.
		/// If the delegate function is a member function, the function will be called upon the object provided in constructor
		/// The behaviour is undefined if no arguments were provided in the delegate's constructor
		/// @return return of the function
		Ret Call()
		{
			if (m_method != nullptr)
			{
				ASSERT_LOG_ERROR((int)(m_obj != nullptr), "Calling a method delegate without object.");
				return[this] <std::size_t... I>
					(std::index_sequence<I...>)
				{
					return (static_cast<Obj*>(m_obj)->*m_method)(std::forward<Args>(std::get<I>(m_args))...);
				}
				(std::make_index_sequence<sizeof...(Args)>());
			}
			else
			{
				return[this] <std::size_t... I>
					(std::index_sequence<I...>)
				{
					return m_function(std::forward<Args>(std::get<I>(m_args))...);
				}
				(std::make_index_sequence<sizeof...(Args)>());
			}
			
		}

		/// @brief Call the function with arguments.
		/// If the delegate function is a member function, the function will be called upon the object provided in constructor
		/// @param ...args arguments
		/// @return return of the function
		template<typename... Ts>
		Ret Call(Ts&&... args)
		{
			if (m_method != nullptr)
			{
				ASSERT_LOG_ERROR((int)(m_obj != nullptr), "Calling a method delegate without object.");
				/*return[this, args...] <std::size_t... I>
					(std::index_sequence<I...>)
				{
					return (static_cast<Obj*>(m_obj)->*m_method)(std::forward<Args>(std::get<I>(args))...);
				}
				(std::make_index_sequence<sizeof...(Args)>());*/
				return (static_cast<Obj*>(m_obj)->*m_method)(std::forward<Ts>(args)...);
			}
			else
			{
				return m_function(std::forward<Ts>(args)...);
			}
		}


		template<typename... Ts>
		Ret CallOn(void* const object, Ts&&... args)
		{
			return (static_cast<Obj*>(object)->*m_method)(std::forward<Ts>(args)...);
		}

		/// @brief Call the member function on object with arguments provided in the delegate's constructor.
		/// The behaviour is undefined if no arguments were provided in the delegate's constructor
		/// @param object object to call the member function upon
		/// @return return of the function
		Ret CallOn(void* const object)
		{
			return[this, object] <std::size_t... I>
				(std::index_sequence<I...>)
			{
				return (static_cast<Obj*>(object)->*m_method)(std::forward<Args>(std::get<I>(m_args))...);
			}
			(std::make_index_sequence<sizeof...(Args)>());
		}

	private:
		//sptr<Callable> m_callable;
		Ret(*m_function)(Args...) = nullptr;
		Ret(Obj::* m_method)(Args...) = nullptr;
		Obj* m_obj = nullptr;
		std::tuple<Args...> m_args;
		/*	struct Callable
			{
				virtual ~Callable() {};
				template<typename... Ts>
				Ret Call(Ts&& args)
				{
					if(member)
				}
				virtual Ret CallWithObject(void* const object, Args... args) = 0;
				virtual Ret Call(Args... args) = 0;
				bool member = false;
				sptr<Callable> that;
			};

			template <typename Obj>
			struct MemberFunction
			{
				MemberFunction(Ret(Obj::* method)(Args...), Obj* object)
					: m_function(method), m_obj(object)
				{

				}
				MemberFunction(Ret(Obj::* method)(Args...))
					: m_function(method), m_obj(nullptr)
				{

				}

				Ret CallWithObject(void* const object, Args... args) override
				{
					return (static_cast<Obj*>(object)->*m_function)(std::forward<Args>(args)...);
				}
				Ret Call(Args... args) override
				{
					return (m_obj->*m_function)(std::forward<Args>(args)...);
				}


				Ret(Obj::* m_function)(Args...);
				Obj* m_obj;
			};

			struct FreeFunction
			{
				FreeFunction(Ret(*function)(Args...))
					: m_function(function)
				{

				}
				FreeFunction(Ret(*function))
					: m_function(function)
				{

				}
				template <typename... Ts>
				Ret Call(Ts&&... args) override
				{
					return (*m_function)(std::forward<Args>(args)...);
				}
				template <typename... Ts>
				Ret CallWithObject(void* const object, Ts&&... args) override
				{
					//no object of free function
					return (*m_function)(std::forward<Args>(args)...);
				}
				bool IsSameFunction(Ret(*function)(Args...)) const override
				{
					return m_function == function;
				}

				Ret(*m_function)(Args...);
			};*/

	};
}

using namespace SandCastle;
struct Arg
{
	Arg() = default;
	Arg(const Arg& copy)
	{
		LOG_INFO("copied");
		i = copy.i;
	}

	int i = 15;
};
class Foo
{
public:
	void Method(Arg prout, float pouet)
	{
		LOG_INFO("prout = {0}, pouet = {1}, ez = {2}", prout.i, pouet, ez);
	}
	int Meth()
	{
		LOG_INFO("meth");
		return 12;
	}
	int ez = 42;
};

int fun(Foo a, Foo b)
{
	LOG_INFO("a = {0}, b = {1}", a.ez, b.ez);
	return a.ez + b.ez;
}
void funfun()
{
	LOG_INFO("free function no argument");
}
void Delegates()
{
	Engine::Init();
	Foo foo;
	foo.ez = 66;
	Arg az;

	Del free(&funfun);
	free.Call();

	Del freeFun(&fun);
	freeFun.Call(foo, foo);

	//Del freeFunctionArg(&fun, Foo(), foo);
	//int r = freeFunctionArg.Call();
	//LOG_INFO("free function return r = {0}", r);

	Del method(&Foo::Method);
	method.CallOn(&foo, az, 423.f);

	//Del methodArg(&Foo::Method, az, 32.f);
	//methodArg.CallOn(&foo);

	Del methodObj(&Foo::Method, &foo);
	methodObj.Call(az, 32.f);

	//Del methodArgObj(&Foo::Method, &foo, az, 32.f);
	//methodArgObj.Call();


	/*Arg fo;
	fo.i = 999;
	del.CallOn(&foo, Arg(), 2);
	int deux = 2;
	del.CallOn(&foo, fo, deux);*/

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