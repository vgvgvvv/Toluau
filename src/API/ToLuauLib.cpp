//
// Created by 35207 on 2022/6/26 0026.
//

#include "ToLuauLib.h"
#include "StackAPI.h"
#include "ToLuau.h"
#include "ILuauChunkLoader.h"
#include "Util/Util.h"

namespace ToLuau
{

	int32_t TOLUAU_PRELOAD_REF = 21;
	int32_t TOLUAU_LOADED_REF = 22;
	int32_t TOLUAU_MAINTHRAD_REF = 23;
	int32_t TOLUAU_GLOBAL_REF = 24;
	int32_t TOLUAU_REQUIRE_REF = 25;

	namespace ToluauDefaultLibs
	{
		static int Log(lua_State* L)
		{
			int32_t N = lua_gettop(L);
            std::string result;
			for(int i = 1; i <= N; i++)
			{
				size_t l;
				const char* s = luaL_tolstring(L, i, &l); /* convert to string using __tostring et al */
				if(i > 1)
				{
                    result += "\t";
				}
                result += s;
                lua_pop(L, 1); /* pop result */
			}
            Lua::Log(result);
			Lua::Log("\n");
			return 0;
		}

		static int Error(lua_State* L)
		{
			int32_t N = lua_gettop(L);
            std::string result;
			for(int i = 1; i <= N; i++)
			{
				size_t l;
				const char* s = luaL_tolstring(L, i, &l); /* convert to string using __tostring et al */
				if(i > 1)
				{
                    result += "\t";
				}
                result += s;
                lua_pop(L, 1); /* pop result */
			}
            Lua::Error(result);
			Lua::Error("\n");
			return 0;
		}

		static int Require(lua_State* L)
		{
            auto State = ILuauState::GetByRawState(L);
            if(!State)
            {
                lua_pushnil(L);
                return 1;
            }
            if(!lua_isstring(L, -1))
            {
                lua_pushnil(L);
                return 1;
            }
            auto Name = lua_tostring(L, -1);
            State->GetLoader().Require(Name);
			return 1;
		}

		static const luaL_Reg Reg[] = {
				{"log", Log},
				{"log_error", Error},
				{"require", Require},
                {NULL, NULL},
		};
	}

	int luaopen_toluau(lua_State* L)
	{
		luaL_register(L, LUA_TOLUAULIBNAME, ToluauDefaultLibs::Reg);
		return 1;
	}

	static const luaL_Reg ToLuauLibs[] = {
			{LUA_TOLUAULIBNAME, luaopen_toluau},
            {NULL, NULL}
	};

	void OpenCacheLuaVar(lua_State* L)
	{
		lua_pushthread(L);
		lua_rawseti(L, LUA_REGISTRYINDEX, TOLUAU_MAIN_THREAD_REF);

		lua_pushvalue(L, LUA_GLOBALSINDEX);
		lua_rawseti(L, LUA_REGISTRYINDEX, TOLUAU_GLOBAL_REF);

		if(luaL_findtable(L, LUA_REGISTRYINDEX, "_LOADED.toluau", 1) == nullptr) // toluau_lib
		{
			lua_getfield(L, -1, "require"); // toluau_lib require
			if(lua_isnil(L, -1))
			{
				LUAU_ERROR("cannot find _LOADED.toluau.require !! please check stack !!");
			}
			lua_rawseti(L, LUA_REGISTRYINDEX, TOLUAU_REQUIRE_REF); // toluau_lib

			lua_pushstring(L, "preload"); // toluau_lib "preload"
			lua_rawget(L, -2); // toluau_lib table
			if(lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				lua_pushstring(L, "preload"); // toluau_lib "preload"
				lua_newtable(L); // toluau_lib "preload" table
				lua_pushvalue(L, -1); // toluau_lib "preload" table table
				lua_rawseti(L, LUA_REGISTRYINDEX, TOLUAU_PRELOAD_REF); // toluau_lib "preload" table
				lua_rawset(L, -3); // toluau_lib
			}
			else // toluau_lib tpreload
			{
				lua_rawseti(L, LUA_REGISTRYINDEX, TOLUAU_PRELOAD_REF); // toluau_lib
			}

			lua_pushstring(L, "loaded");
			lua_rawget(L, -2);
			if(lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				lua_pushstring(L, "loaded");
				lua_newtable(L);
				lua_pushvalue(L, -1);
				lua_rawseti(L, LUA_REGISTRYINDEX, TOLUAU_LOADED_REF);
				lua_rawset(L, -3);
			}
			else
			{
				lua_rawseti(L, LUA_REGISTRYINDEX, TOLUAU_LOADED_REF);
			}
		}

	}

	void OpenToLuauLibs(lua_State *L)
	{
		const luaL_Reg* Lib = ToLuauLibs;
		for (; Lib->func; Lib++)
		{
			lua_pushcfunction(L, Lib->func, NULL);
			lua_pushstring(L, Lib->name);
			lua_call(L, 1, 0);
		}
		OpenCacheLuaVar(L);
	}
}

