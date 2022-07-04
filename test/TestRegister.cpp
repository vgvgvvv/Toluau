//
// Created by 35207 on 7/2/2022.
//

#include <iostream>
#include "TestRegister.h"
#include "API/IToLuauRegister.h"
#include "API/StackAPI.h"
#include "ToLuau.h"


namespace
{
	using namespace ToLuau;
	class TestLuaRegister : public ILuauStaticRegister
	{
	public:
		void LuaRegister(IToLuauRegister *Register) override
		{
            std::cout << "do test register" << std::endl;

			Register->BeginClass(FooClass::StaticLuaClass());
			Register->RegFunction("PrintIntMem",
			                      &LuaCppBinding<decltype(&FooClass::PrintIntMem), &FooClass::PrintIntMem>::LuaCFunction);
			Register->RegFunction("SayHello",
			                      &LuaCppBinding<decltype(&FooClass::SayHello), &FooClass::SayHello>::LuaCFunction);

            Lua::DumpStack(Register->GetOwner()->GetState(), "before int mem");

	        Register->RegVar("IntMem",
                             &ToLuau::LuaCppBinding<decltype(&FooClass::SetIntMem), &FooClass::SetIntMem>::LuaCFunction,
                             &ToLuau::LuaCppBinding<decltype(&FooClass::GetIntMem), &FooClass::GetIntMem>::LuaCFunction);

            Lua::DumpStack(Register->GetOwner()->GetState(), "after int mem");

            Register->RegVar("StrMem",
                             &ToLuau::LuaCppBinding<decltype(&FooClass::SetStrMem), &FooClass::SetStrMem>::LuaCFunction,
                             &ToLuau::LuaCppBinding<decltype(&FooClass::GetStrMem), &FooClass::GetStrMem>::LuaCFunction);

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

    TestLuaRegister TestRegister;
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
