//
// Created by 35207 on 2022/6/26 0026.
//

#include "IToLuauRegister.h"

namespace ToLuau
{
	class ToLuaRegister : public IToLuauRegister
	{
	public:
		explicit ToLuaRegister(ILuauState* InOwner) : IToLuauRegister(InOwner) {}
		void BeginModule(const std::string& ModuleName) override;
		void EndModule() override;
		void BeginClass(const std::string& ClassName, const std::string& SuperClassName) override;
		void EndClass() override;
		void BeginEnum(const std::string& EnumName) override;
		void EndEnum() override;

		void RegFunction(const std::string& FuncName, LuaFunc Func) override;
		void RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter) override;
	};

	std::shared_ptr<IToLuauRegister> IToLuauRegister::Create(ILuauState *InOwner)
	{
		return std::make_shared<ToLuaRegister>(InOwner);
	}

	void ToLuaRegister::BeginModule(const std::string& ModuleName)
	{
	}

	void ToLuaRegister::EndModule()
	{
	}

	void ToLuaRegister::BeginClass(const std::string& ClassName, const std::string& SuperClassName)
	{
	}

	void ToLuaRegister::EndClass()
	{
	}

	void ToLuaRegister::BeginEnum(const std::string& EnumName)
	{
	}

	void ToLuaRegister::EndEnum()
	{
	}

	void ToLuaRegister::RegFunction(const std::string& FuncName, LuaFunc Func)
	{
	}

	void ToLuaRegister::RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter)
	{
	}

}