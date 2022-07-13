//
// Created by 35207 on 2022/6/19 0019.
//

#include "ToLuau.h"
#include "lualib.h"
#include "Toluau/API/ToLuauLib.h"
#include "Toluau/API/ILuauChunkLoader.h"
#include "Toluau/API/IToLuauAPI.h"
#include "Toluau/API/IToLuauRegister.h"

namespace ToLuau
{
    std::map<lua_State*, const ILuauState*> ILuauState::AllLuauState;

    ILuauState::ILuauState()
    {
        L = luaL_newstate();
        luaL_openlibs(L);
        OpenToLuauLibs(L);
		StackAPI::InitStackAPI();
    	
        AllLuauState.insert(std::pair(L, this));

    }

    const ILuauState* ILuauState::GetByRawState(lua_State* TargetState)
    {
        auto It = AllLuauState.find(TargetState);
        if(It != AllLuauState.end())
        {
            return It->second;
        }
        return nullptr;
    }

    ToLuauState::ToLuauState() : ILuauState()
	{
		Loader = ILuauChunkLoader::Create(this);
		API = IToLuauAPI::Create(this);
		Register = IToLuauRegister::Create(this);
	}

	ToLuauState::~ToLuauState()
	{
		lua_close(L);
	}


}
