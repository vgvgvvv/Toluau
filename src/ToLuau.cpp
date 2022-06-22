//
// Created by 35207 on 2022/6/19 0019.
//

#include "Toluau.h"
#include "lualib.h"

namespace ToLuau
{

	ToLuauState::ToLuauState()
	{
		L = luaL_newstate();
		luaL_openlibs(L);

		Loader = ILuauChunkLoader::Create(this);
		API = IToLuauAPI::Create(this);
		
	}

	ToLuauState::~ToLuauState()
	{
		lua_close(L);
	}

	ToLuauSandbox::ToLuauSandbox()
	{
		L = luaL_newstate();
		luaL_openlibs(L);

		Loader = ILuauChunkLoader::Create(this);
		API = IToLuauAPI::Create(this);
	}

	ToLuauSandbox::~ToLuauSandbox()
	{
		lua_close(L);
	}
}
