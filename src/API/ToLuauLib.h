//
// Created by 35207 on 2022/6/26 0026.
//

#pragma once

#include "lualib.h"

#define LUA_TOLUAULIBNAME "toluau"

namespace ToLuau
{
	int32_t TOLUAU_PRELOAD_REF = 21;
	int32_t TOLUAU_LOADED_REF = 22;
	int32_t TOLUAU_MAINTHRAD_REF = 23;
	int32_t TOLUAU_GLOBAL_REF = 24;
	int32_t TOLUAU_REQUIRE_REF = 25;

	void OpenToLuauLibs(lua_State* L);
}
