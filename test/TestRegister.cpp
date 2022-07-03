//
// Created by 35207 on 7/2/2022.
//

#include <iostream>
#include "TestRegister.h"
#include "API/IToLuauRegister.h"
#include "API/StackAPI.h"


namespace
{
	using namespace ToLuau;
	class TestLuaRegister : public ILuauStaticRegister
	{
	public:
		void LuaRegister(IToLuauRegister *Register) override
		{

			Register->BeginClass("FooClass");
			Register->RegFunction("PrintIntMem",
			                      &LuaCppBinding<decltype(&FooClass::PrintIntMem), &FooClass::PrintIntMem>::LuaCFunction);
			Register->RegFunction("SayHello",
			                      &LuaCppBinding<decltype(&FooClass::SayHello), &FooClass::SayHello>::LuaCFunction);
	//		Register->RegVar("IntMem", &ToLuau::LuaCppBinding<decltype(&FooClass::IntMem), FooClass::IntMem>::LuaCFunction);
			Register->EndClass();

			Register->BeginEnum("FooEnum");
			Register->RegVar("Bar", nullptr, [](lua_State* L){ StackAPI::Push(L, FooEnum::Bar); return 1;});
			Register->RegVar("Foo", nullptr, [](lua_State* L){ StackAPI::Push(L, FooEnum::Foo); return 1;});
			Register->EndEnum();

			Register->BeginStaticLib("FooStaticLib");
			Register->RegFunction("Add", &LuaCppBinding<decltype(&FooStaticLib::Add), &FooStaticLib::Add>::LuaCFunction);
			Register->RegFunction("FooFunc", &LuaCppBinding<decltype(&FooStaticLib::FooFunc), &FooStaticLib::FooFunc>::LuaCFunction);
			Register->EndStaticLib();
		}
	};
}


void FooClass::PrintIntMem()
{
	std::cout << IntMem << std::endl;
}

void FooClass::SayHello(const std::string& Word)
{
	std::cout << Word << " : " << StrMem << std::endl;
}

int32_t FooStaticLib::Add(int32_t A, int32_t B)
{
	return A + B;
}

void FooStaticLib::FooFunc()
{
	std::cout << __FUNCTION__ << std::endl;
}
