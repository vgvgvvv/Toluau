//
// Created by 35207 on 2022/6/20 0020.
//

#include "IToLuauAPI.h"
#include "Util/Util.h"

#include "lua.h"
#include "Toluau.h"


namespace ToLuau
{
	class ToLuauAPI : public IToLuauAPI
	{
	public:
		explicit ToLuauAPI(ILuauState* InOwner) : IToLuauAPI(InOwner) {}

		void CallFunc(const std::string& FuncName) override;

		void CallFuncWithArg(const std::string& FuncName, const IArg& Arg) override;

	protected:
		bool GetFuncGlobal(const std::string& LuaFunc, bool* bIsClassFunc) override;
		void DoPCall(int32_t ArgNum, int32_t RetNum) override;

	};

	std::shared_ptr<IToLuauAPI> IToLuauAPI::Create(ILuauState* InOwner)
	{
		return std::make_shared<ToLuauAPI>(InOwner);
	}

	bool ToLuauAPI::GetFuncGlobal(const std::string& LuaFunc, bool* bIsClassFunc)
	{
		auto LuaFunName = LuaFunc;
		if(LuaFunName.empty())
		{
			*bIsClassFunc = false;
			return false;
		}

		*bIsClassFunc = LuaFunName.find(':') != std::string::npos;

		if(*bIsClassFunc)
		{
			StringEx::ReplaceAll(LuaFunName, ":", ".");
		}

		auto Names = StringEx::Split(LuaFunName, ".");
		auto L = Owner->GetState();
		lua_getglobal(L, Names[0].c_str());

		if(lua_isnil(L, -1))
		{
			lua_remove(L, -1);
			return false;
		}

		for(int32_t i = 0; i < Names.size(); ++i)
		{
			if(!lua_istable(L, -1))
			{
				lua_remove(L, -1);
				return false;
			}

			lua_getfield(L, -1, Names[i].c_str());
			if(lua_isnil(L, -1))
			{
				lua_remove(L, -1);
				lua_remove(L, -1);
			}

			if(*bIsClassFunc && i == Names.size() - 1)
			{
				lua_pushvalue(L, -2);
				lua_remove(L, -3);
			}
			else
			{
				lua_remove(L, -2);
			}
			return true;
		}

		return false;
	}

	void ToLuauAPI::DoPCall(int32_t ArgNum, int32_t RetNum)
	{
		auto L = Owner->GetState();
		if(lua_pcall(L, ArgNum, RetNum, 0) != LUA_OK)
		{
			auto msg = lua_tostring(L, lua_gettop(L));
			Lua::Error(msg);
			lua_pop(L, 1);
		}
	}

	void ToLuauAPI::CallFunc(const std::string &FuncName)
	{
		bool bIsClassFunc;
		if (!GetFuncGlobal(FuncName, &bIsClassFunc))
		{
			return;
		}

		DoPCall(bIsClassFunc ? 1 : 0, 0);
	}

	void ToLuauAPI::CallFuncWithArg(const std::string &FuncName, const IArg& Arg)
	{
		bool bIsClassFunc;
		if (!GetFuncGlobal(FuncName, &bIsClassFunc))
		{
			return;
		}

		Arg.PushLua(Owner->GetState());
		DoPCall(bIsClassFunc ? Arg.Count() + 1 : Arg.Count(), 0);
	}

}
