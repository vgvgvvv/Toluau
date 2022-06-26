//
// Created by 35207 on 2022/6/26 0026.
//

#include <iostream>
#include "Util.h"

namespace ToLuau
{
	namespace StringEx
	{
		void StringEx::ReplaceAll(std::string& str, const std::string& from, const std::string& to)
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
				std::cout << Log << std::endl;
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
				std::cerr << Error << std::endl;
			}
		}
	}
}