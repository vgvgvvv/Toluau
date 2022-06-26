//
// Created by 35207 on 2022/6/26 0026.
//

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <functional>

namespace ToLuau
{
	namespace StringEx
	{
		static void ReplaceAll(std::string& str, const std::string& from, const std::string& to);

		static std::vector<std::string> Split(const std::string& str, const std::string& split);
	};

	namespace PathHelper
	{
		static std::string Combine(const std::string& p1, const std::string& p2);
	}

	namespace FileEx
	{
		static std::optional<std::string> ReadFile(const std::string& name);
	}

	namespace Lua
	{
		extern std::optional<std::function<void(const std::string&)>> OnLog;
		extern std::optional<std::function<void(const std::string&)>> OnError;

		static void Log(const std::string& Log);

		static void Error(const std::string& Error);
	}
}
