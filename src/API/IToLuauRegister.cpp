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
#include "ToLuauLib.h"

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
		static int32_t ModuleIndexEvent(lua_State* L);

		void BeginClass(const std::string& ClassName, const std::string& SuperClassName) override;
		void EndClass() override;

		void BeginEnum(const std::string& EnumName) override;
		void EndEnum() override;

        static int32_t EnumIndexEvent(lua_State* L);
        static int32_t EnumNewIndexEvent(lua_State* L);

		void RegFunction(const std::string& FuncName, LuaFunc Func) override;
		void RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter) override;
    private:

        void PushFullName(int32_t Pos);

        void AddToLoaded();

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
				lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::Index)); // stack table __index
                lua_pushcfunction(L, ModuleIndexEvent, "ModuleIndexEvent"); // stack table __index function
				lua_rawset(L, -3); // stack table

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

	int ToLuaRegister::ModuleIndexEvent(lua_State *L) // table key
	{
        lua_pushvalue(L, 2); // table key key
        lua_rawget(L, 1); // table key value
        if(lua_isnil(L, -1))
        {
            return 1;
        }

        lua_pop(L, 1); // table key
        lua_pushstring(L, ".name"); // table key ".name"
        lua_rawget(L, 1); // table key namespace

        if(lua_isnil(L, -1))
        {
            // TODO Get From Preload
        }

		return 1;
	}

	void ToLuaRegister::BeginClass(const std::string& ClassName, const std::string& SuperClassName)
	{
        auto L = Owner->GetState();
        lua_pushstring(L, ClassName.c_str()); // table name
        lua_newtable(L); // table name classtable


	}

	void ToLuaRegister::EndClass()
	{
	}

	void ToLuaRegister::BeginEnum(const std::string& EnumName)
	{
        auto L = Owner->GetState();
        auto FullName = CurrentModuleName + "." + EnumName;
        lua_pushstring(L, EnumName.c_str()); // enumname
        lua_newtable(L); // enumname table

        lua_pushvalue(L, -1); // enumname table table
        auto Ref = lua_ref(L, LUA_REGISTRYINDEX); // enumname table
        EnumRefDict.insert(std::make_pair(FullName, Ref));

        AddToLoaded(); // enumname table
        lua_newtable(L); // enumname table metatable

        lua_pushstring(L, ".name"); // enumname table metatable .name
        PushFullName(-4); // enumname table metatable .name fullname
        lua_rawset(L, -3);

        lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::Index));
        lua_pushcfunction(L, EnumIndexEvent, (FullName + " Index").c_str());
        lua_rawset(L, -3);

        lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::NewIndex));
        lua_pushcfunction(L, EnumNewIndexEvent, (FullName + " NewIndex").c_str());
        lua_rawset(L, -3);

	}

	void ToLuaRegister::EndEnum()
	{
        auto L = Owner->GetState();
        lua_setmetatable(L, -2);
        lua_rawset(L, -3);
	}

    int32_t ToLuaRegister::EnumIndexEvent(lua_State *L)
    {
        // TODO
        return 0;
    }

    int32_t ToLuaRegister::EnumNewIndexEvent(lua_State *L)
    {
        // TODO
        return 0;
    }

	void ToLuaRegister::RegFunction(const std::string& FuncName, LuaFunc Func)
	{
		auto L = Owner->GetState();
		lua_pushstring(L, FuncName.c_str());
		lua_pushcfunction(L, Func, FuncName.c_str());
		lua_rawset(L, -3);
	}

	void ToLuaRegister::RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter)
	{
        auto L = Owner->GetState();
        if(Getter != nullptr)
        {
            lua_pushstring(L, ".get"); // table ".get"
            lua_rawget(L, -2); // table gettable

            if(!lua_istable(L, -1))
            {
                lua_pop(L, 1);
                lua_newtable(L);
                lua_pushstring(L, ".get");
                lua_pushvalue(L, -2);
                lua_rawset(L, -4);
            }

            lua_pushstring(L, VarName.c_str());
            lua_pushcfunction(L, Getter, (VarName + "Getter").c_str());
            lua_rawset(L, -3);
            lua_pop(L, 1);
        }

        if(Setter != nullptr)
        {
            lua_pushstring(L, ".set");
            lua_rawset(L, -2);

            if(!lua_istable(L, -1))
            {
                lua_pop(L, 1);
                lua_newtable(L);
                lua_pushstring(L, ".set");
                lua_pushvalue(L, -2);
                lua_rawset(L, -4);
            }

            lua_pushstring(L, VarName.c_str());
            lua_pushcfunction(L, Setter, (VarName + "Setter").c_str());
            lua_rawset(L, -3);
            lua_pop(L, 1);
        }
	}

    void ToLuaRegister::PushFullName(int32_t Pos)
    {
        auto L = Owner->GetState();
        if(!CurrentModuleName.empty())
        {
            lua_pushstring(L, CurrentModuleName.c_str());
            lua_pushstring(L, ".");
            lua_pushvalue(L, Pos < 0 ? Pos - 2 : Pos + 2);
            lua_concat(L, 3);
        }
        else
        {
            lua_pushvalue(L, Pos);
        }
    }

    void ToLuaRegister::AddToLoaded()
    {
        auto L = Owner->GetState();
        lua_getref(L, TOLUAU_LOADED_REF); // name table preload
        PushFullName(-3); // name table preload fullname
        lua_pushvalue(L, -3); // name table preload fullname table
        lua_rawset(L, -3); // name table preload
        lua_pop(L, 1);
    }




}