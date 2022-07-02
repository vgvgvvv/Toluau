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

		void BeginClass(const std::string& ClassName, const std::string& SuperFullClassName) override;
		void EndClass() override;

		static int32_t ClassNewEvent(lua_State* L);
		static int32_t ClassIndexEvent(lua_State* L);
		static int32_t ClassNewIndexEvent(lua_State* L);

		void BeginEnum(const std::string& EnumName) override;
		void EndEnum() override;

        static int32_t EnumIndexEvent(lua_State* L);
        static int32_t EnumNewIndexEvent(lua_State* L);

		void BeginStaticLib(const std::string &StaticLibName) override;
		void EndStaticLib() override;

        static int32_t StaticLibIndexEvent(lua_State* L);
        static int32_t StaticLibNewIndexEvent(lua_State* L);

		void RegFunction(const std::string& FuncName, LuaFunc Func) override;
		void RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter) override;
    private:

        static void PushFullName(const ToLuaRegister* Owner, int32_t Pos);

        static void AddToLoaded(const ToLuaRegister* Owner);

		static bool GetFromPreload(lua_State* L);

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
	        lua_getref(L, TOLUAU_PRELOAD_REF); // table key space preload
	        lua_pushvalue(L, -2); // table key space preload space
	        lua_pushstring(L, "."); // table key space preload space "."
	        lua_pushvalue(L, 2);  // table key space preload space "." key
	        lua_concat(L, 3); // table key space preload fullname
	        lua_pushvalue(L, -1); // table key space preload fullname fullname
	        lua_rawget(L, -3); // table key space preload fullname value

			if(!lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				lua_getref(L, TOLUAU_REQUIRE_REF);// table key space preload fullname require
				lua_pushvalue(L, -2); // table key space preload fullname reuqire fullname
				lua_call(L, 1, 1);
			}
			else
			{
				lua_pushnil(L);
			}
        }

		return 1;
	}

	void ToLuaRegister::BeginClass(const std::string& ClassName, const std::string& SuperFullClassName)
	{
        auto L = Owner->GetState();
        lua_pushstring(L, ClassName.c_str()); // table name
        lua_newtable(L); // table name classtable
		AddToLoaded(this);

		auto FullName = CurrentModuleName + "." + ClassName;


		auto ClassMetaRef_It = ClassMetaRefDict.find(FullName);
		int32_t ClassMetaRef = 0;
		if(ClassMetaRef_It == ClassMetaRefDict.end())
		{
			lua_newtable(L); // t k classtable mt
			lua_pushvalue(L, -1); // t k class table mt mt
			ClassMetaRef = lua_ref(L, LUA_REGISTRYINDEX); // t k classtable mt
			ClassMetaRefDict.insert(std::make_pair(FullName, ClassMetaRef));
		}
		else
		{
			ClassMetaRef = ClassMetaRef_It->second;
			lua_getref(L, ClassMetaRef);
		}

		// t k classtable mt

		if(!SuperFullClassName.empty())
		{
			auto BaseClassRef_It = ClassMetaRefDict.find(SuperFullClassName);
			if(BaseClassRef_It == ClassMetaRefDict.end())
			{
				lua_newtable(L);     // t k classtable mt bmt
				lua_pushvalue(L, -1);   //t k classtable mt bmt bmt
				auto BaseClassRef = lua_ref(L, LUA_REGISTRYINDEX); // t k class mt bmt
				ClassMetaRefDict.insert(std::make_pair(SuperFullClassName, BaseClassRef));
				lua_setmetatable(L, -2); // t k class table mt
			}
			else
			{
				lua_getref(L, BaseClassRef_It->second);
				lua_setmetatable(L, -2);
			}
		}

		//table name classtable mt

		lua_pushstring(L, ".name"); // table name classtable mt .name
		PushFullName(this, -4);
		lua_rawset(L, -3);

		lua_pushstring(L, ".ref");
		lua_pushinteger(L, ClassMetaRef);
		lua_rawset(L, -3);

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::Call));
		lua_pushcfunction(L, ClassNewEvent, (FullName + "_Call").c_str());
		lua_rawset(L, -3);

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::Index));
		lua_pushcfunction(L, ClassIndexEvent, (FullName + "_Index").c_str());
		lua_rawset(L, -3);

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::NewIndex));
		lua_pushcfunction(L, ClassNewIndexEvent, (FullName + "_NewIndex").c_str());
		lua_rawset(L, -3);

		// TODO : destructor definition

	}

	void ToLuaRegister::EndClass()
	{
		auto L = Owner->GetState();
		lua_setmetatable(L, -2);
		lua_rawset(L, -3);
	}

	int32_t ToLuaRegister::ClassNewEvent(lua_State *L)
	{
		if(!lua_istable(L, 1))
		{
			luaL_typeerror(L, 1, "table");
			return 0;
		}

		int32_t count = lua_gettop(L); // table args...
		lua_pushvalue(L, 1); // table args... table

		if(lua_getmetatable(L, -1)) // table args... table mt
		{
			lua_remove(L, -2);
			lua_pushstring(L, "new"); // find newfunc // table args... mt newfunc
			lua_rawget(L, -2);

			if(lua_isfunction(L, -1))
			{
				for(int32_t i = 2; i <= count; i++)
				{
					lua_pushvalue(L, i);
				}
				// table args... mt newfunc args...
				lua_call(L, count-1, 1); // table args... mt result
			}
		}

		while(lua_gettop(L) != 2)
		{
			lua_remove(L, -1);
		}

		// table result

		return 1;
	}

	int32_t ToLuaRegister::ClassIndexEvent(lua_State *L)
	{
		// t k
		auto type = lua_type(L, 1);

		if(type == LUA_TUSERDATA)
		{

			lua_pushvalue(L, 1); // t k t
			while(lua_getmetatable(L, -1)) // t k t mt
			{
				lua_remove(L, -2); // t k mt
				lua_pushvalue(L, 2); // t k mt k
				lua_rawget(L, -2); // t k mt v

				if(!lua_isnil(L, -1))
				{
					lua_remove(L, -2); // t k v
					return 1;
				}

				lua_pop(L, 1);
				lua_pushstring(L, ".get"); // t k mt .get
				lua_rawget(L, -2); // t k mt tget

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2); // t k mt tget k
					lua_rawget(L, -2); // t k mt tget func

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1); // t k mt tget func table
						lua_call(L, 1, 1); // t k mt tget func v
						lua_remove(L, -2);
						lua_remove(L, -2);
						lua_remove(L, -2); // t k value
						return 1;
					}
				}


			}

			if(GetFromPreload(L))
			{
				return 1;
			}

			luaL_error(L, "field or property %s does not exist", lua_tostring(L, 2));
			return 0;
		}
		else if(type == LUA_TTABLE)
		{
			lua_pushvalue(L, 1); // t k t

			while(lua_getmetatable(L, -1)) // t k t mt
			{
				lua_remove(L, -2); // t k mt
				lua_pushvalue(L, 2); // t k mt k
				lua_rawget(L, -2);

				if(!lua_isnil(L,-1))
				{
					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 2); // t k m func k
						lua_pushvalue(L, -2); // t k m func k func
						lua_rawset(L, 1); // t k mt func
					}

					lua_remove(L, -2); // t k func

					return 1;
				}

				lua_pop(L, 1); // t k mt
				lua_pushstring(L, ".get"); // t k mt .get
				lua_rawget(L, -2); // t k mt tget

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2); // t k mt tget k
					lua_rawget(L, -2); // t k mt tget func

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1); // t k mt tget func t
						lua_call(L, 1, 1); // t k mt tget func v
						lua_remove(L, -2);
						lua_remove(L, -2);
						lua_remove(L, -2); // t k v
						return 1;
					}
				}

				lua_settop(L, 3);

			}

			if(GetFromPreload(L))
			{
				return 1;
			}

			luaL_error(L, "field or property %s does not exist", lua_tostring(L, 2));
			return 0;

		}

		lua_pushnil(L);
		return 1;
	}

	int32_t ToLuaRegister::ClassNewIndexEvent(lua_State *L)
	{
		auto type = lua_type(L, 1);

		if(type == LUA_TUSERDATA)
		{
			lua_getmetatable(L, 1);
			while(lua_istable(L, -1))
			{
				lua_pushstring(L, ".set");
				lua_rawget(L, -2);

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2);
					lua_rawget(L, -2);

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1);
						lua_pushvalue(L, 3);
						lua_call(L, 2, 0);
						return 0;
					}

					lua_pop(L, 1);
				}

				lua_pop(L, 1);

				if(!lua_getmetatable(L, -1))
				{
					lua_pushnil(L);
				}

				lua_remove(L, -2);
			}
		}
		else if(type == LUA_TTABLE)
		{
			lua_getmetatable(L, 1);
			while(lua_istable(L, -1))
			{
				lua_pushstring(L, ".set");
				lua_rawget(L, -2);

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2);
					lua_rawget(L, -2);

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1);
						lua_pushvalue(L, 3);
						lua_call(L, 2, 0);
						return 0;
					}

					lua_pop(L, 1);
				}

				lua_pop(L, 1);

				if(!lua_getmetatable(L, -1))
				{
					lua_pushnil(L);
				}

				lua_remove(L, -2);
			}
		}

		lua_settop(L, 3);
		luaL_error(L, "field or property %s does not exist", lua_tostring(L, 2));
		return 0;
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

        AddToLoaded(this); // enumname table
        lua_newtable(L); // enumname table metatable

        lua_pushstring(L, ".name"); // enumname table metatable .name
        PushFullName(this, -4); // enumname table metatable .name fullname
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
	    lua_getmetatable(L, 1); // table key meta

		if(lua_istable(L, -1))
		{
			lua_pushvalue(L, 2); // table key meta key
			lua_rawget(L, -2); // table key meta value

			if(!lua_isnil(L, -1))
			{
				lua_remove(L, -2); // table key value
				return 1;
			}

			lua_pop(L, 1);

			lua_pushstring(L, ".get"); // table key meta .get
			lua_rawget(L, -2);       // table key meta gettable

			if(lua_istable(L, -1))
			{
				lua_pushvalue(L, 2); // table key meta gettable key
				lua_rawget(L, -2); // table key meta gettable getfunc

				if(lua_isfunction(L, -1))
				{
					lua_call(L, 0, 1); // table key meta gettable value
					lua_pushvalue(L, 2); // table key meta gettable value key
					lua_pushvalue(L, -2); // table key meta gettable value key value
					lua_rawset(L, 3); // table key meta gettable value
					lua_remove(L, -2); // table key meta value
					lua_remove(L, -2); // table key value
					return 1;
				}

				lua_pop(L, 1);
			}
		}

	    lua_settop(L, 2);
	    lua_pushnil(L);
		
        return 1;
    }

    int32_t ToLuaRegister::EnumNewIndexEvent(lua_State *L)
    {
        luaL_errorL(L, "the left-hand side of an assignment must be a variable, a property or an indexer");
        return 0;
    }

	void ToLuaRegister::BeginStaticLib(const std::string &StaticLibName)
	{
		auto L = Owner->GetState();

		lua_pushstring(L, StaticLibName.c_str()); // name
		lua_newtable(L);                            // name table

		lua_pushvalue(L, -1);                   // name table table
		auto Ref = lua_ref(L, LUA_REGISTRYINDEX);
		StaticLibRefDict.insert(std::pair(StaticLibName, Ref));

		AddToLoaded(this);
		lua_pushvalue(L, -1); // name table table

		lua_pushstring(L, ".name"); // name table table ".name"
		PushFullName(this, -4); // name table table ".name" fullname
		lua_rawset(L, -3);

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::Index));
		lua_pushcfunction(L, StaticLibIndexEvent, (StaticLibName + "_Index").c_str());
		lua_rawset(L, -3);

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::NewIndex));
		lua_pushcfunction(L, StaticLibNewIndexEvent, (StaticLibName + "_NewIndex").c_str());
		lua_rawset(L, -3);

	}

	void ToLuaRegister::EndStaticLib()
	{
		auto L = Owner->GetState();
		lua_setmetatable(L, -2);
		lua_rawset(L, -3);
	}

	int32_t ToLuaRegister::StaticLibIndexEvent(lua_State *L)
	{
		lua_pushvalue(L, 2); // t k k
		lua_rawget(L, 1); // t k v

		if(!lua_isnil(L, -1))
		{
			return 1;
		}

		lua_pop(L, 1);
		lua_pushstring(L, ".get");
		lua_rawget(L, 1); // t k gettable

		if(lua_istable(L, -1))
		{
			lua_pushvalue(L, 2); // t k tget k
			lua_rawget(L, -2); // t k tget func

			if(lua_isfunction(L, -1))
			{
				lua_call(L, 0, 1); // t k tget value
				lua_remove(L, -2); // t k value
				return 1;
			}
		}

		lua_settop(L, 2);

		if(ToLuaRegister::GetFromPreload(L))
		{
			return 1;
		}

		luaL_error(L, "field or property %s does not exist", lua_tostring(L, 2));
		return 0;
	}

	int32_t ToLuaRegister::StaticLibNewIndexEvent(lua_State *L)
	{
		lua_pushstring(L, ".set"); // t k v .set
		lua_rawget(L, 1); // t k v table

		if(lua_istable(L, -1))
		{
			lua_pushvalue(L, 2); // t k v table k
			lua_rawget(L, -2); // t k v table func

			if(lua_isfunction(L, -1))
			{
				lua_pushvalue(L, 1); // t k v table func t
				lua_pushvalue(L, 3); // t k v table func t v
				lua_call(L, 2, 0);
				return 0;
			}
		}

		lua_settop(L, 3);

		luaL_errorL(L, "field or property %s does not exist", lua_tostring(L, 2));
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

    void ToLuaRegister::PushFullName(const ToLuaRegister* Owner, int32_t Pos)
    {
		auto L = Owner->GetOwner()->GetState();
        if(!Owner->CurrentModuleName.empty())
        {
            lua_pushstring(L, Owner->CurrentModuleName.c_str());
            lua_pushstring(L, ".");
            lua_pushvalue(L, Pos < 0 ? Pos - 2 : Pos + 2);
            lua_concat(L, 3);
        }
        else
        {
            lua_pushvalue(L, Pos);
        }
    }

    void ToLuaRegister::AddToLoaded(const ToLuaRegister* Owner)
    {
		auto L = Owner->GetOwner()->GetState();
        lua_getref(L, TOLUAU_LOADED_REF); // name table preload
        PushFullName(Owner, -3); // name table preload fullname
        lua_pushvalue(L, -3); // name table preload fullname table
        lua_rawset(L, -3); // name table preload
        lua_pop(L, 1);
    }

	bool ToLuaRegister::GetFromPreload(lua_State* L)
	{
		lua_settop(L, 2); // table key
		lua_getmetatable(L, 1); // table key meta
		lua_pushstring(L, ".name"); // table key meta ".name"
		lua_rawget(L, -2); // table key meta space

		if(!lua_isnil(L, -1))
		{
			lua_getref(L, TOLUAU_PRELOAD_REF); // table key meta space preload
			lua_pushvalue(L, -2); // table key meta space preload space
			lua_pushstring(L, "."); // table key meta space preload space "."
			lua_pushvalue(L, 2); // table key meta space preload space "." key
			lua_concat(L, 3); // table key meta space preload fullname
			lua_pushvalue(L, -1); // table key meta space preload fullname fullname
			lua_rawget(L, -3); // table key meta space preload fullname value

			if(!lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				lua_getref(L, TOLUAU_REQUIRE_REF);       // table key meta space preload fullname require
				lua_pushvalue(L, -2);                // table key meta space preload fullname require fullname
				lua_call(L, 1, 1);
				return true;
			}
		}

		return false;
	}


}