#pragma once

#include <SandCastle.h>
#include <iostream>
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
		return (int)f + value;
	}
	int NoArg()
	{
		return 43;
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

int NoArg()
{
	return 33;
}

void Delegates()
{
	Engine::Init();

	//rvalue and lvalue can be mixed indenpendendtly

	//--Free function with no argument at all--
	{
		LOG_INFO("--Free function no arg--");
		Arg lvalue("lvalue");
		Delegate delegate(&NoArg);
		auto r = delegate.Call();
		LOG_INFO({ "return: = {0}" }, r);
		std::cout << std::endl;
	}

	//--Method with no argument at all--
	{
		LOG_INFO("--Method no arg--");
		Foo obj;
		Delegate delegate(&Foo::NoArg, &obj);
		auto r = delegate.Call();
		LOG_INFO({ "return: = {0}" }, r);
		std::cout << std::endl;
	}

	//--Free function with no argument bound--
	{
		LOG_INFO("--Free function--");
		Arg lvalue("lvalue");
		Delegate delegate(&Function);
		auto r = delegate.Call("rvalue", lvalue);
		LOG_INFO({ "return: = {0}" }, r);
		std::cout << std::endl;
	}

	//--Free function with arguments bound--
	{
		LOG_INFO("--Free function, arg bound--");
		String lvalue = "lvalue";
		Delegate delegate(&Function, lvalue, Arg());
		auto r = delegate.Call();
		LOG_INFO({ "return: = {0}" }, r);
		std::cout << std::endl;
	}

	//--Method with no bounding--
	{
		LOG_INFO("--Method--");
		Delegate delegate(&Foo::Method);
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
		Delegate delegate(&Foo::Method, &obj);
		Arg lvalue("lvalue");
		auto r = delegate.Call(lvalue, 3.1f);
		LOG_INFO({ "return: = {0}" }, r);
		std::cout << std::endl;
	}

	//--Method with arg bound--
	{
		LOG_INFO("--Method arg bound--");
		Arg lvalue("lvalue");
		Delegate delegate(&Foo::Method, lvalue, 4.1f);
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
		Delegate delegate(&Foo::Method, &obj, Arg(), lvalue);
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
		Delegate del(&FunctionWithRef, ref);
		del.Call();
	}
}