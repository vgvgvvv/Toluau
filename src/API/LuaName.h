//
// Created by chen.dong on 2022/7/4.
//
#pragma once

namespace ToLuau
{
#pragma region LuaName

    template<typename T>
    struct LuaClassName
    {
        static const char* GetName()
        {
            return "UNKNOWN";
        }
    };

#define DEF_LUA_CLASSNAME(ClassName, RegisterName)      \
    template<>                                          \
    struct LuaClassName<ClassName>                      \
    {                                                   \
        static const char* GetName()                    \
        {                                               \
            return #RegisterName;                       \
        }                                               \
    };


    template<typename T>
    const char* GetLuaClassName()
    {
        return LuaClassName<T>::GetName();
    }

#pragma endregion

}

