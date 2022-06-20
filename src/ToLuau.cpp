//
// Created by 35207 on 2022/6/19 0019.
//

#include "Toluau.h"
#include "lualib.h"

namespace ToLuau
{
	void State::Init(lua_State *State)
	{
		this->L = State;
		luaL_openlibs(State);
	}
}
