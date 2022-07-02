//
// Created by 35207 on 7/2/2022.
//

#include <iostream>
#include "TestRegister.h"

void FooClass::PrintIntMem()
{
	std::cout << IntMem << std::endl;
}

int32_t FooClass::SayHello(const std::string &Word)
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
