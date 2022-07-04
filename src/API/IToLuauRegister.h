//
// Created by 35207 on 2022/6/26 0026.
//

#include <string>
#include <functional>

#include "lua.h"

#include "CppBinding.h"
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

        const ILuauState* GetOwner() const { return Owner; }

		static std::shared_ptr<IToLuauRegister> Create(ILuauState* InOwner);

		using LuaFunc = lua_CFunction;

		virtual void RegisterAll() = 0;

		virtual void BeginModule(const std::string& ModuleName) = 0;
		virtual void EndModule() = 0;

		void BeginClass(const std::string& ClassName) { BeginClass(ClassName, ""); };
		virtual void BeginClass(const std::string& ClassName, const std::string& SuperFullClassName) = 0;
		virtual void EndClass() = 0;

		virtual void BeginEnum(const std::string& EnumName) = 0;
		virtual void EndEnum() = 0;

		virtual void BeginStaticLib(const std::string& StaticLibName) = 0;
		virtual void EndStaticLib() = 0;

		virtual void RegFunction(const std::string& FuncName, LuaFunc Func) = 0;
		virtual void RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter) = 0;

        virtual int32_t GetEnumRef(const std::string& EnumName) const = 0;
        virtual int32_t GetStaticLibRef(const std::string& StaticLibName) const = 0;
        virtual int32_t GetClassMetaRef(const std::string& ClassName) const = 0;

	protected:
		ILuauState* Owner = nullptr;
	};

	class ToLuau_API ILuauStaticRegister
	{
	public:
		ILuauStaticRegister();
		virtual ~ILuauStaticRegister();
		virtual void LuaRegister(IToLuauRegister* Register) = 0;
	};

}

