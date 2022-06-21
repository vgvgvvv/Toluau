//
// Created by 35207 on 2022/6/20 0020.
//

#include "IToLuauAPI.h"

namespace ToLuau
{
	void IToLuauAPI::BeginModule(const std::string& ModuleName)
	{
	}

	void IToLuauAPI::EndModule()
	{
	}

	void IToLuauAPI::BeginClass(const std::string& ClassName, const std::string& SuperClassName)
	{
	}

	void IToLuauAPI::EndClass()
	{
	}

	int IToLuauAPI::BeginEnum(const std::string& EnumName)
	{
	}

	void IToLuauAPI::EndEnum()
	{
	}

	void IToLuauAPI::RegFunction(const std::string& FuncName, LuaFunc Func)
	{
	}

	void IToLuauAPI::RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter)
	{
	}
}
