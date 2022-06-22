//
// Created by 35207 on 2022/6/20 0020.
//

#include "IToLuauAPI.h"

namespace ToLuau
{
	class ToLuauAPI : public IToLuauAPI
	{
	protected:
		ToLuauAPI(ILuauState* InOwner) : IToLuauAPI(InOwner) {}
	public:
		void BeginModule(const std::string& ModuleName) override;
		void EndModule() override;
		void BeginClass(const std::string& ClassName, const std::string& SuperClassName) override;
		void EndClass() override;
		void BeginEnum(const std::string& EnumName) override;
		void EndEnum() override;
		void RegFunction(const std::string& FuncName, LuaFunc Func) override;
		void RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter) override;
		bool Call(const std::string& GlobalFunctionName) override;
	};

	std::shared_ptr<IToLuauAPI> IToLuauAPI::Create(ILuauState* InOwner)
	{
		return std::make_shared<ToLuauAPI>(InOwner);
	}

	void ToLuauAPI::BeginModule(const std::string& ModuleName)
	{
	}

	void ToLuauAPI::EndModule()
	{
	}

	void ToLuauAPI::BeginClass(const std::string& ClassName, const std::string& SuperClassName)
	{
	}

	void ToLuauAPI::EndClass()
	{
	}

	void ToLuauAPI::BeginEnum(const std::string& EnumName)
	{
	}

	void ToLuauAPI::EndEnum()
	{
	}

	void ToLuauAPI::RegFunction(const std::string& FuncName, LuaFunc Func)
	{
	}

	void ToLuauAPI::RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter)
	{
	}

	bool ToLuauAPI::Call(const std::string& GlobalFunctionName)
	{
		return false;
	}

}
