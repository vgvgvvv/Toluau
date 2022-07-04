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
	void SayHello(const std::string& Word);

    int32_t GetIntMem() const
    {
        return IntMem;
    }

    void SetIntMem(int32_t NewInt)
    {
        IntMem = NewInt;
    }

    int32_t GetStrMem() const
    {
        return IntMem;
    }

    void SetStrMem(const std::string& NewStr)
    {
        StrMem = NewStr;
    }

	int32_t IntMem;
	std::string StrMem;
};

class FooStaticLib
{
public:
	static int32_t Add(int32_t A, int32_t B);
	static void FooFunc();
};


