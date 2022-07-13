//
// Created by 35207 on 2022/6/26 0026.
//

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <stdexcept>
#include "ToLuau_API.h"
#ifdef TOLUAUUNREAL_API
#include "CoreMinimal.h"
#endif

struct lua_State;

namespace ToLuau
{
	namespace StringEx
	{
		
        ToLuau_API void ReplaceAll(std::string& str, const std::string& from, const std::string& to);

        ToLuau_API std::vector<std::string> Split(const std::string& str, const std::string& split);

        template<typename ... Args>
        std::string Format( const std::string& format, Args ... args )
        {
            int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
            if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
            auto size = static_cast<size_t>( size_s );
            std::unique_ptr<char[]> buf( new char[ size ] );
            std::snprintf( buf.get(), size, format.c_str(), args ... );
            return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
        }

#ifdef TOLUAUUNREAL_API
		ToLuau_API std::string FStringToStdString(const FString& Str);

		ToLuau_API FString StdStringToFString(const std::string& Str);
#endif
    };

	namespace PathHelper
	{
        ToLuau_API std::string Combine(const std::string& p1, const std::string& p2);
	}

#ifndef TOLUAUUNREAL_API
	namespace FileEx
	{
        ToLuau_API std::optional<std::string> ReadFile(const std::string& name);
	}
#endif

	namespace Lua
	{
#ifdef TOLUAUUNREAL_API
		extern TOptional<std::function<void(const std::string&)>> OnLog;
		extern TOptional<std::function<void(const std::string&)>> OnError;
#else
		extern std::optional<std::function<void(const std::string&)>> OnLog;
		extern std::optional<std::function<void(const std::string&)>> OnError;
#endif
		
        ToLuau_API void Log(const std::string& Log);

        ToLuau_API void Error(const std::string& Error);

        ToLuau_API void DumpStack(lua_State* L, const std::string& Title = "");

        ToLuau_API void DumpTable(lua_State* L, int32_t Index);
	}
}

#define LUAU_LOG(Str) \
    ToLuau::Lua::Log(Str);

#define LUAU_ERROR(Str) \
    ToLuau::Lua::Error(Str);

#define LUAU_LOG_F(format, ...) \
    {                                   \
        auto Log = ToLuau::StringEx::Format(format, ##__VA_ARGS__); \
        ToLuau::Lua::Log(Log);                                    \
    }

#define LUAU_ERROR_F(format, ...) \
    {                                     \
        auto Error = ToLuau::StringEx::Format(format, ##__VA_ARGS__); \
        ToLuau::Lua::Error(Error);\
    }