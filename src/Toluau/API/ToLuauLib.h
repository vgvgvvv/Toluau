//
// Created by 35207 on 2022/6/26 0026.
//

#pragma once

#include "lualib.h"
#include "Toluau/ToLuauDefine.h"

#define LUA_TOLUAULIBNAME "toluau"

namespace ToLuau
{
	extern ToLuau_API int32_t TOLUAU_PRELOAD_REF;
	extern ToLuau_API int32_t TOLUAU_LOADED_REF;
	extern ToLuau_API int32_t TOLUAU_MAIN_THREAD_REF;
	extern ToLuau_API int32_t TOLUAU_GLOBAL_REF;
	extern ToLuau_API int32_t TOLUAU_REQUIRE_REF;
    extern ToLuau_API int32_t TOLUAU_REGISTER_REF;
	extern ToLuau_API int32_t TOLUAU_ERROR_HANDLER_REF;

	void OpenToLuauLibs(lua_State* L);
	
}
