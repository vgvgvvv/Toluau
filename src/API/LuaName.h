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

    template<typename T>
    const char* GetLuaClassName()
    {
        return LuaClassName<T>::GetName();
    }

#pragma endregion

}

#define DEF_LUA_CLASSNAME(ClassName, RegisterName)      \
    template<>                                          \
    struct ToLuau::LuaClassName<ClassName>              \
    {                                                   \
        static const char* GetName()                    \
        {                                               \
            return #RegisterName;                       \
        }                                               \
    };

