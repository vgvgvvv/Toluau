//
// Created by 35207 on 2022/6/26 0026.
//

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <functional>
#include "ToLuau_API.h"

namespace ToLuau
{
	namespace StringEx
	{
        ToLuau_API void ReplaceAll(std::string& str, const std::string& from, const std::string& to);

        ToLuau_API std::vector<std::string> Split(const std::string& str, const std::string& split);
	};

	namespace PathHelper
	{
        ToLuau_API std::string Combine(const std::string& p1, const std::string& p2);
	}

	namespace FileEx
	{
        ToLuau_API std::optional<std::string> ReadFile(const std::string& name);
	}

	namespace Lua
	{
		extern std::optional<std::function<void(const std::string&)>> OnLog;
		extern std::optional<std::function<void(const std::string&)>> OnError;

        ToLuau_API void Log(const std::string& Log);

        ToLuau_API void Error(const std::string& Error);
	}
}
