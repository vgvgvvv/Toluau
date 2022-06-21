#pragma once

#include <string>
#include <functional>

#include "lua.h"

namespace ToLuau
{
	class IToLuauAPI
	{
	public:

		using LuaFunc = std::function<int(lua_State*)>;

		void BeginModule(const std::string& ModuleName);
		void EndModule();

		void BeginClass(const std::string& ClassName, const std::string& SuperClassName);
		void EndClass();

		int BeginEnum(const std::string& EnumName);
		void EndEnum();

		void RegFunction(const std::string& FuncName, LuaFunc Func);

		void RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter);
	};
}
