//
// Created by 35207 on 7/2/2022.
//

#pragma once

#include <cstdint>
#include <string>

enum class FooEnum
{
	Bar,
	Foo
};

class FooClass
{
public:

	void PrintIntMem();
	int32_t SayHello(const std::string& Word);

	int32_t IntMem;
	std::string StrMem;
};

class FooStaticLib
{
public:
	static int32_t Add(int32_t A, int32_t B);
	static void FooFunc();
};


