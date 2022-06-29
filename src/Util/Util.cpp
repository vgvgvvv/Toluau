//
// Created by 35207 on 2022/6/26 0026.
//

#include <iostream>
#include "Util.h"
#include "Luau/Common.h"

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

	namespace Lua
	{
		std::optional<std::function<void(const std::string&)>> OnLog;
		std::optional<std::function<void(const std::string&)>> OnError;

		void Log(const std::string &Log)
		{
			if(OnLog.has_value())
			{
				OnLog.value()(Log);
			}
			else
			{
				std::cout << Log;
			}
		}

		void Error(const std::string &Error)
		{
			if(OnError.has_value())
			{
				OnError.value()(Error);
			}
			else
			{
				std::cerr << Error;
			}
		}
	}


}