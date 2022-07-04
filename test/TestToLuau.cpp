//
// Created by 35207 on 2022/6/19 0019.
//

#include <filesystem>
#include <iostream>

#include "ToLuau.h"
#include "Util/Util.h"
#include "API/StackAPI.h"
#include "API/ILuauChunkLoader.h"
#include "TestRegister.h"



int main()
{
    ToLuau::ToLuauState ToLuauState;

    std::filesystem::path CurrentPath(__FILE__);
    auto LuaPath = CurrentPath.parent_path().append("Lua");
    ToLuauState.GetLoader().AddLoadPath(LuaPath.string());

    ToLuauState.GetLoader().Require("main");


    auto L = ToLuauState.GetState();

    if(lua_isnil(L, -1))
    {
        LUAU_LOG("require failed")
         return 0;
    }

    lua_getfield(L, -1, "testNew"); // testClass
    if (lua_isnil(L, -1))
    {
        LUAU_LOG("cannot find entry function")
        return 0;
    }

    auto Foo = new FooClass();
    ToLuau::StackAPI::Push<FooClass*>(L, Foo); // testClass fooclass

    lua_call(L, 1, 0);

	return 0;
}