//
// Created by 35207 on 2022/6/19 0019.
//

#include <filesystem>
#include <iostream>

#include "Toluau/ToLuau.h"
#include "Toluau/Util/Util.h"
#include "Toluau/API/StackAPI.h"
#include "Toluau/API/ILuauChunkLoader.h"
#include "TestRegister.h"
#include "Toluau/API/IToLuauAPI.h"
#include "Toluau/Util/Util.h"


int main()
{
    ToLuau::ToLuauState ToLuauState;

    auto L = ToLuauState.GetState();

    std::filesystem::path CurrentPath(__FILE__);
    auto LuaPath = CurrentPath.parent_path().append("Lua");
    ToLuauState.GetLoader().AddLoadPath(LuaPath.string());

    ToLuauState.GetLoader().Require("main");
    lua_pop(L, 1);

    ToLuau::Lua::DumpStack(L, "before call");

    FooClass Foo;
    ToLuauState.GetAPI().CallFunc("main.testEnum", &Foo);

	return 0;
}