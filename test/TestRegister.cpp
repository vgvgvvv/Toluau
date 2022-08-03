//
// Created by 35207 on 7/2/2022.
//

#include <iostream>
#include "TestRegister.h"
#include "Toluau/API/IToLuauRegister.h"
#include "Toluau/API/StackAPI.h"
#include "Toluau/ToLuau.h"
#include "Toluau/API/RegisterMacro.h"

LUAU_BEGIN_CLASS(FooClass);

LUAU_REG_NEW_FUNC(FooClass, create);
LUAU_REG_FUNC(FooClass, PrintIntMem);
LUAU_REG_FUNC(FooClass, SayHello)
LUAU_REG_VAR(FooClass, IntMem)
LUAU_REG_VAR(FooClass, StrMem)
LUAU_END_CLASS(FooClass)

LUAU_BEGIN_ENUM(FooEnum)
LUAU_REG_CLASS_ENUM_VAR(FooEnum, Bar)
LUAU_REG_CLASS_ENUM_VAR(FooEnum, Foo)
LUAU_END_ENUM(FooEnum)

LUAU_BEGIN_STATIC_LIB(FooStaticLib)
LUAU_REG_FUNC(FooStaticLib, Add)
LUAU_REG_FUNC(FooStaticLib, FooFunc)
LUAU_END_STATIC_LIB(FooStaticLib)



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
