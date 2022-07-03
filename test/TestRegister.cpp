//
// Created by 35207 on 7/2/2022.
//

#include <iostream>
#include "TestRegister.h"
#include "API/IToLuauRegister.h"
#include "API/StackAPI.h"


class TestLuaRegister : public ToLuau::ILuauStaticRegister
{
public:
	void LuaRegister(ToLuau::IToLuauRegister *Register) override
	{
		Register->BeginClass("FooClass");
		Register->RegFunction("PrintIntMem",
							  &ToLuau::LuaCppBinding<decltype(&FooClass::PrintIntMem), &FooClass::PrintIntMem>::LuaCFunction);
		Register->RegFunction("SayHello",
		                      &ToLuau::LuaCppBinding<decltype(&FooClass::SayHello), &FooClass::SayHello>::LuaCFunction);
		Register->EndClass();

		Register->BeginEnum("FooEnum");
		Register->RegVar("Bar", [](lua_State* L){ ToLuau::StackAPI::Push(L, FooEnum::Bar); return 1;}, nullptr);
		Register->RegVar("Foo", [](lua_State* L){ ToLuau::StackAPI::Push(L, FooEnum::Foo); return 1;}, nullptr);
		Register->EndEnum();

		Register->BeginStaticLib("FooStaticLib");
		Register->RegFunction("Add", &ToLuau::LuaCppBinding<decltype(&FooStaticLib::Add), &FooStaticLib::Add>::LuaCFunction);
		Register->RegFunction("FooFunc", &ToLuau::LuaCppBinding<decltype(&FooStaticLib::FooFunc), &FooStaticLib::FooFunc>::LuaCFunction);
		Register->EndStaticLib();
	}
};

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
