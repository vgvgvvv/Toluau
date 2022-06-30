//
// Created by 35207 on 2022/6/26 0026.
//

#include "ToLuauLib.h"
#include "StackAPI.h"
#include "Util/Util.h"

namespace ToLuau
{
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
			// TODO
			return 0;
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

	void OpenToLuauLibs(lua_State *L)
	{
		const luaL_Reg* Lib = ToLuauLibs;
		for (; Lib->func; Lib++)
		{
			lua_pushcfunction(L, Lib->func, NULL);
			lua_pushstring(L, Lib->name);
			lua_call(L, 1, 0);
		}
	}
}

