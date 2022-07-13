//
// Created by 35207 on 2022/6/26 0026.
//

#include "Util.h"

#include <iostream>
#include "lua.h"
#include "Luau/Common.h"

#ifndef TOLUAUUNREAL_API

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#endif
#else
#include "ToluauUnreal.h"
#endif

namespace ToLuau
{
	namespace StringEx
	{
		void ReplaceAll(std::string& str, const std::string& from, const std::string& to)
		{
			if (from.empty())
				return;
			size_t start_pos = 0;
			while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
				str.replace(start_pos, from.length(), to);
				start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
			}
		}

		std::vector<std::string> Split(const std::string& str, const std::string& delimiter)
		{
			std::vector<std::string> result;
			std::string s = str;

			size_t pos = 0;
			std::string token;
			while ((pos = s.find(delimiter)) != std::string::npos) {
				token = s.substr(0, pos);
				result.push_back(token);
				s.erase(0, pos + delimiter.length());
			}
			result.push_back(s);
			return result;
		}

#ifdef TOLUAUUNREAL_API
		std::string FStringToStdString(const FString& Str)
		{
			return std::string(TCHAR_TO_UTF8(*Str));
		}

		FString StdStringToFString(const std::string& Str)
		{
			return FString(Str.c_str());
		}
#endif


	}

	namespace PathHelper
	{
		std::string Combine(const std::string &p1, const std::string &p2)
		{
			if(p1.empty())
			{
				return p2;
			}
			if(p2.empty())
			{
				return p1;
			}
			char sep = '/';
			std::string tmp = p1;

#ifdef PLATFORM_WINDOWS
			sep = '\\';
			StringEx::ReplaceAll(tmp, "/", "\\");
#endif


			if (tmp[tmp.length()-1] != sep)
			{ // Need to add a
				tmp += sep;                // path separator
				return(tmp + p2);
			}
			else
			{
				return(p1 + p2);
			}
		}
	}

#ifndef TOLUAUUNREAL_API
	namespace FileEx
	{
		#ifdef _WIN32
		static std::wstring FromUtf8(const std::string& path)
		{
			size_t result = MultiByteToWideChar(CP_UTF8, 0, path.data(), int(path.size()), nullptr, 0);
			LUAU_ASSERT(result);

			std::wstring buf(result, L'\0');
			MultiByteToWideChar(CP_UTF8, 0, path.data(), int(path.size()), &buf[0], int(buf.size()));

			return buf;
		}

		static std::string ToUtf8(const std::wstring& path)
		{
			size_t result = WideCharToMultiByte(CP_UTF8, 0, path.data(), int(path.size()), nullptr, 0, nullptr, nullptr);
			LUAU_ASSERT(result);

			std::string buf(result, '\0');
			WideCharToMultiByte(CP_UTF8, 0, path.data(), int(path.size()), &buf[0], int(buf.size()), nullptr, nullptr);

			return buf;
		}
		#endif

		std::optional<std::string> ReadFile(const std::string& name)
		{
#ifdef _WIN32
			FILE* file = _wfopen(FromUtf8(name).c_str(), L"rb");
#else
			FILE* file = fopen(name.c_str(), "rb");
#endif

			if (!file)
				return std::nullopt;

			fseek(file, 0, SEEK_END);
			long length = ftell(file);
			if (length < 0)
			{
				fclose(file);
				return std::nullopt;
			}
			fseek(file, 0, SEEK_SET);

			std::string result(length, 0);

			size_t read = fread(result.data(), 1, length, file);
			fclose(file);

			if (read != size_t(length))
				return std::nullopt;

			// Skip first line if it's a shebang
			if (length > 2 && result[0] == '#' && result[1] == '!')
				result.erase(0, result.find('\n'));

			return result;
		}
	}
#endif
	
	namespace Lua
	{
#ifdef TOLUAUUNREAL_API
		TOptional<std::function<void(const std::string&)>> OnLog;
		TOptional<std::function<void(const std::string&)>> OnError;
#else
		std::optional<std::function<void(const std::string&)>> OnLog;
		std::optional<std::function<void(const std::string&)>> OnError;
#endif

		void Log(const std::string &Log)
		{
#ifdef TOLUAUUNREAL_API
			if(OnLog.IsSet())
			{
				OnLog.GetValue()(Log);
			}
#else
			if(OnLog.has_value())
			{
				OnLog.value()(Log);
			}
#endif
			else
			{
#ifdef TOLUAUUNREAL_API
				UE_LOG(LogToLuau, Log, TEXT("%s"), *StringEx::StdStringToFString(Log))
#else
				std::cout << Log << std::endl;
#endif
			}
		}

		void Error(const std::string &Error)
		{
#ifdef TOLUAUUNREAL_API
			if(OnError.IsSet())
			{
				OnError.GetValue()(Error);
			}
#else
			if(OnError.has_value())
			{
				OnError.value()(Error);
			}
#endif
			else
			{
#ifdef TOLUAUUNREAL_API
				UE_LOG(LogToLuau, Error, TEXT("%s"), *StringEx::StdStringToFString(Error))
#else
				std::cerr << Error << std::endl;
#endif
			}
		}

        void DumpStack(lua_State* L, const std::string& Title)
        {
            if(!Title.empty())
            {
                LUAU_LOG(Title);
            }
            auto Top = lua_gettop(L);
            for(int i = 1; i <= Top; i ++)
            {
                lua_pushvalue(L, i);
                auto TypeId = lua_type(L, -1);
                auto TypeName = lua_typename(L, TypeId);

                if(lua_isnumber(L, -1) || lua_isstring(L, -1))
                {
                    auto CharStr = lua_tostring(L, -1);
                    LUAU_LOG_F("%d - %s (%s)", i, TypeName, CharStr);
                }
                else if(lua_istable(L, -1))
                {
                    LUAU_LOG_F("%d - %s", i, TypeName);
                    DumpTable(L, -1);
                }
                else
                {
                    LUAU_LOG_F("%d - %s", i, TypeName);
                }
                lua_pop(L, 1);
            }
        }

        // ref from https://stackoverflow.com/questions/6137684/iterate-through-lua-table
        void DumpTable(lua_State *L, int32_t Index)
        {
            // Push another reference to the table on top of the stack (so we know
            // where it is, and this function can work for negative, positive and
            // pseudo indices
            lua_pushvalue(L, Index);
            // stack now contains: -1 => table
            lua_pushnil(L);
            // stack now contains: -1 => nil; -2 => table
            while (lua_next(L, -2))
            {
                // stack now contains: -1 => value; -2 => key; -3 => table
                // copy the key so that lua_tostring does not modify the original
                lua_pushvalue(L, -2);
                // stack now contains: -1 => key; -2 => value; -3 => key; -4 => table
                const char *key = lua_tostring(L, -1);
                auto TypeId = lua_type(L, -2);
                auto TypeName = lua_typename(L, TypeId);
                if(lua_isnumber(L, -2) || lua_isstring(L, -2))
                {
                    const char *value = lua_tostring(L, -2);
                    LUAU_LOG_F("%s => %s(%s)\n", key, TypeName, value);
                }
                else
                {
                    const char *value = lua_tostring(L, -2);
                    LUAU_LOG_F("%s => %s\n", key, TypeName);
                }
                // pop value + copy of key, leaving original key
                lua_pop(L, 2);
                // stack now contains: -1 => key; -2 => table
            }
            // stack now contains: -1 => table (when lua_next returns 0 it pops the key
            // but does not push anything.)
            // Pop table
            lua_pop(L, 1);
            // Stack is now the same as it was on entry to this function

        }

    }

}