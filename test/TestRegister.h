//
// Created by 35207 on 7/2/2022.
//

#pragma once

#include <cstdint>
#include <string>
#include "Class/ClassInfo.h"

enum class FooEnum
{
	Bar,
	Foo
};

class FooClass
{
	DEFINE_LUA_CLASS(FooClass)

public:

	static FooClass* create(int32_t Int, const std::string& str)
	{
		auto Result = new FooClass;
		Result->IntMem = Int;
		Result->StrMem = str;
		return Result;
	}

	void PrintIntMem();
	void SayHello(const std::string& Word);
	~FooClass()
	{
		std::cout << "release FooClass" << std::endl;
	}

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

class DerivedFooClass : public FooClass
{
	DEFINE_LUA_DERIVED_CLASS(DerivedFooClass, FooClass)

};


class FooStaticLib
{
public:
	static int32_t Add(int32_t A, int32_t B);
	static void FooFunc();
};


