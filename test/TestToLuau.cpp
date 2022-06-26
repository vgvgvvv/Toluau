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

	ToLuauState.GetAPI().CallFunc("main");
	
	return 0;
}