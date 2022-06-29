//
// Created by 35207 on 2022/6/19 0019.
//

#include "lualib.h"
#include "Toluau.h"
#include <filesystem>

int main()
{
	ToLuau::ToLuauState ToLuauState;

	std::filesystem::path CurrentPath(__FILE__);
	auto LuaPath = CurrentPath.parent_path().append("Lua");
	ToLuauState.GetLoader().AddLoadPath(LuaPath.string());

	ToLuauState.GetLoader().Require("main");

    auto L = ToLuauState.GetState();

    luaL_findtable(L, LUA_REGISTRYINDEX, "_MODULES", 1);
    lua_getfield(L, -1, "main");
    lua_getfield(L, -1, "main");
    lua_call(L, 0, 0);
    lua_settop(L, 0);

	return 0;
}