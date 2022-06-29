//
// Created by 35207 on 2022/6/26 0026.
//

#include <string>
#include <functional>
#include "ToLuau_API.h"

#pragma once

struct lua_State;

namespace ToLuau
{

	class ILuauState;

	class ToLuau_API IToLuauRegister
	{
	protected:
		explicit IToLuauRegister(ILuauState* InOwner) : Owner(InOwner) {}
	public:

		virtual ~IToLuauRegister() = default;

		static std::shared_ptr<IToLuauRegister> Create(ILuauState* InOwner);

		using LuaFunc = std::function<int(lua_State*)>;

		virtual void BeginModule(const std::string& ModuleName) = 0;
		virtual void EndModule() = 0;

		virtual void BeginClass(const std::string& ClassName, const std::string& SuperClassName) = 0;
		virtual void EndClass() = 0;

		virtual void BeginEnum(const std::string& EnumName) = 0;
		virtual void EndEnum() = 0;

		virtual void RegFunction(const std::string& FuncName, LuaFunc Func) = 0;

		virtual void RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter) = 0;

	protected:
		ILuauState* Owner = nullptr;
	};

}
