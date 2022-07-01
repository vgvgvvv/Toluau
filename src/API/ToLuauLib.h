//
// Created by 35207 on 2022/6/26 0026.
//

#pragma once

#include "lualib.h"

#define LUA_TOLUAULIBNAME "toluau"

namespace ToLuau
{
    int32_t TOLUAU_LOADED_REF = 21;

	void OpenToLuauLibs(lua_State* L);
}
