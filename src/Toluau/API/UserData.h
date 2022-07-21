//
// Created by chen.dong on 2022/7/4.
//

#pragma once

#include "lua.h"

#ifdef TOLUAUUNREAL_API
#include "CoreMinimal.h"
#endif
#include "Toluau/Class/ClassInfo.h"

namespace ToLuau
{

    struct BaseUserData
    {
    	void* RawPtr = nullptr;
#if ToLuauDebug
    	const char* DebugName = "NULL";
#endif
        virtual ~BaseUserData() = default;
    };

    template<typename T>
    struct UserData : public BaseUserData
    {
        T* GetValue() const
        {
            return reinterpret_cast<T*>(RawPtr);
        }

		~UserData()
        {
        	auto RealValue = GetValue();
        	if(RealValue != nullptr)
        	{
        		RawPtr = nullptr;
#if ToLuauDebug
        		DebugName = nullptr;
#endif
        	}
        }

    };

    struct LuaRef
    {
		LuaRef(lua_State* L, int32_t Pos)
		{
			Ref = lua_ref(L, Pos);
			OwnerState = L;
		}
    	
    	virtual ~LuaRef()
		{
			if(OwnerState)
			{
				lua_unref(OwnerState, Ref);
			}
		}

    	bool IsValid() const
    	{
    		return Ref != LUA_NOREF;
    	}
    	
    	void Push() const
    	{
    		lua_getref(OwnerState, Ref);
    	}
    	
    	int32_t Ref;
    	lua_State* OwnerState;
    };
	
}
