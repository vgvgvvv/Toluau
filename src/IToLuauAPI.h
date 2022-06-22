#pragma once

#include <string>
#include <functional>

#include "lua.h"

namespace ToLuau
{
	class ILuauState;

	class IToLuauAPI
	{
	protected:
		IToLuauAPI(ILuauState* InOwner) : Owner(InOwner) {}
	public:
		virtual ~IToLuauAPI() = default;

		static std::shared_ptr<IToLuauAPI> Create(ILuauState* InOwner);

		#pragma region Register

		using LuaFunc = std::function<int(lua_State*)>;

		virtual void BeginModule(const std::string& ModuleName) = 0;
		virtual void EndModule() = 0;

		virtual void BeginClass(const std::string& ClassName, const std::string& SuperClassName) = 0;
		virtual void EndClass() = 0;

		virtual void BeginEnum(const std::string& EnumName) = 0;
		virtual void EndEnum() = 0;

		virtual void RegFunction(const std::string& FuncName, LuaFunc Func) = 0;

		virtual void RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter) = 0;

		#pragma endregion

		#pragma region Function

		virtual bool Call(const std::string& GlobalFunctionName) = 0;

		#pragma endregion

	protected:
		ILuauState* Owner = nullptr;
	};
}
