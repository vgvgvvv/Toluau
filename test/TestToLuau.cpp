//
// Created by 35207 on 2022/6/19 0019.
//

#include <filesystem>
#include <iostream>

#include "ToLuau.h"
#include "Util/Util.h"
#include "API/ILuauChunkLoader.h"

int main()
{
    ToLuau::ToLuauState ToLuauState;

    std::filesystem::path CurrentPath(__FILE__);
    auto LuaPath = CurrentPath.parent_path().append("Lua");
    ToLuauState.GetLoader().AddLoadPath(LuaPath.string());

    ToLuauState.GetLoader().Require("main");

    auto L = ToLuauState.GetState();

    auto Result = luaL_findtable(L, LUA_REGISTRYINDEX, "_MODULES", 1);
    if(Result)
    {
        LUAU_LOG("_MODULES is not a table")
    }

    lua_getfield(L, -1, "main");
    if (lua_isnil(L, -1))
    {
        std::cout << "cannot find main module" << std::endl;
        return 0;
    }

    lua_call(L, 0, 1);

    ToLuau::Lua::DumpStack(L);

    lua_getfield(L, -1, "entry");
    if (lua_isnil(L, -1))
    {
        std::cout << "cannot find entry function" << std::endl;
        return 0;
    }

    lua_call(L, 0, 0);

	return 0;
}