//
// Created by 35207 on 2022/6/19 0019.
//

#include "Toluau.h"
#include "lualib.h"
#include "API/ToLuauLib.h"

namespace ToLuau
{

	ToLuauState::ToLuauState()
	{
		L = luaL_newstate();
		luaL_openlibs(L);
		ToLuau::OpenToLuauLibs(L);

		Loader = ILuauChunkLoader::Create(this);
		API = IToLuauAPI::Create(this);
		Register = IToLuauRegister::Create(this);
		
	}

	ToLuauState::~ToLuauState()
	{
		lua_close(L);
	}

}
