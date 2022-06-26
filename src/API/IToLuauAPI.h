#pragma once

#include <string>
#include <functional>

#include "lua.h"
#include "StackAPI.h"

namespace ToLuau
{
	class ILuauState;

	class IToLuauAPI
	{
	protected:
		explicit IToLuauAPI(ILuauState* InOwner) : Owner(InOwner) {}
	public:
		virtual ~IToLuauAPI() = default;

		using LuaFunc = std::function<int32_t(lua_State*)>;

		static std::shared_ptr<IToLuauAPI> Create(ILuauState* InOwner);

		#pragma region Function

		virtual bool GetFuncGlobal(const std::string& LuaFunc, bool& bIsClassFunc) = 0;

		#pragma endregion

	protected:
		ILuauState* Owner = nullptr;
	};
}
