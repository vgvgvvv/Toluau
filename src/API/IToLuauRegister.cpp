//
// Created by 35207 on 2022/6/26 0026.
//

#include <memory>
#include <map>

#include "lua.h"
#include "lualib.h"

#include "ToLuau.h"
#include "IToLuauRegister.h"
#include "IToLuauAPI.h"
#include "Util/Util.h"

namespace ToLuau
{

	static const char* GlobalLuaRegisterName = "_GLOBAL_REGISTER_";

	class ToLuaRegister : public IToLuauRegister
	{
	public:
		explicit ToLuaRegister(ILuauState* InOwner) : IToLuauRegister(InOwner) {}
		void BeginModule(const std::string& ModuleName) override;
		void EndModule() override;

		void PushModuleName(const std::string& Name);
		static int ModuleIndexEvent(lua_State* L);

		void BeginClass(const std::string& ClassName, const std::string& SuperClassName) override;
		void EndClass() override;

		void BeginEnum(const std::string& EnumName) override;
		void EndEnum() override;

		void RegFunction(const std::string& FuncName, LuaFunc Func) override;
		void RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter) override;
    private:
        std::map<std::string, int32_t> EnumRefDict;
        std::map<std::string, int32_t> StaticLibRefDict;
        std::map<std::string, int32_t> ClassMetaRefDict;

		std::string CurrentModuleName;
	};

	std::shared_ptr<IToLuauRegister> IToLuauRegister::Create(ILuauState *InOwner)
	{
		return std::make_shared<ToLuaRegister>(InOwner);
	}

	void ToLuaRegister::BeginModule(const std::string& ModuleName)
	{
		auto L = Owner->GetState();
		if(!ModuleName.empty())
		{
			if(lua_type(L, -1) != LUA_TTABLE)
			{
				LUAU_ERROR_F("begin module %s", ModuleName.c_str())
				return;
			}

			lua_pushstring(L, ModuleName.c_str()); // stack name
			lua_rawget(L, -2); // stack value

			if(lua_isnil(L, -1))
			{
				lua_pop(L, 1); // stack
				lua_newtable(L); // stack table

				// TODO set index meta func
				// lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::Index)); // stack table __index
				// lua_pushfunction // stack table __index function
				// lua_rawset(L, -3) // stack table

				lua_pushstring(L, ModuleName.c_str()); // stack table name
				lua_pushstring(L, ".name"); // stack table name ".name"
				PushModuleName(ModuleName); //stack table name ".name" fullname

				lua_rawset(L, -4); // stack table name
				lua_pushvalue(L, -2); // stack table name table
				lua_rawset(L, -4); // stack table

				lua_pushvalue(L, -1); // stack table table
				lua_setmetatable(L, -2); // stack table
				return;
			}

		}
		else
		{
			luaL_findtable(L, LUA_REGISTRYINDEX, GlobalLuaRegisterName, 1);
		}
	}

	void ToLuaRegister::EndModule()
	{
		auto L = Owner->GetState();
		lua_pop(L, 1);
		auto index = find(CurrentModuleName.begin(), CurrentModuleName.end(), '.');
		if(index != CurrentModuleName.end())
		{
			CurrentModuleName = CurrentModuleName.substr(0, index - CurrentModuleName.begin());
		}
		else
		{
			CurrentModuleName = "";
		}
	}

	void ToLuaRegister::PushModuleName(const std::string &Name)
	{
		std::string NewModuleName = CurrentModuleName + "." + Name;
		lua_pushstring(Owner->GetState(), NewModuleName.c_str());
	}

	int ToLuaRegister::ModuleIndexEvent(lua_State *L)
	{
		// TODO Index Event
		return 0;
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