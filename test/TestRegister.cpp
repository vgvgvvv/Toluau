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
//		Register->BeginClass("FooClass");
//		Register->RegFunction("PrintIntMem", &FooClass::PrintIntMem);
//		Register->EndClass();
	}
};

void FooClass::PrintIntMem()
{
	std::cout << IntMem << std::endl;
}



template<typename TOwner>
using MemberFunc = void(TOwner::*)();

template<typename TOwner, typename T, T Func>
lua_CFunction Convert()
{
	static auto Instance =
			[](lua_State* L) -> int32_t
			{
				auto Owner = ToLuau::StackAPI::Check<TOwner*>(L, -1);
				(Owner->*Func)();
				return 0;
			};
	return Instance;
}

void Test()
{
	auto Result = Convert<FooClass, decltype(&FooClass::PrintIntMem), &FooClass::PrintIntMem>();
}


void FooClass::SayHello(const std::string &Word)
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
