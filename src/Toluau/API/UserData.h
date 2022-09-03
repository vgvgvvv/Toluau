//
// Created by chen.dong on 2022/7/4.
//

#pragma once

#include "lua.h"

#ifdef TOLUAUUNREAL_API
#include "CoreMinimal.h"
#endif
#include "Toluau/Class/ClassName.h"
#include "Toluau/ToLuauDefine.h"
#include "Toluau/Class/ClassInfo.h"

namespace ToLuau
{
    struct UserData final
    {
    	UserData()
    		: SharedPtr(nullptr)
    	{
    	}

    	template<typename T>
        T* GetValue() const
        {
    		if(ClassName != GetClassName<T>())
    		{
    			return nullptr;
    		}
        	if(SharedPtr)
        	{
        		return static_cast<T*>(SharedPtr.get());
        	}
        	else if(RawPtr)
        	{
        		return static_cast<T*>(RawPtr);
        	}
            return nullptr;
        }

    	template<typename T>
    	void SetValue(const T* Value)
        {
    		SharedPtr.reset();
        	RawPtr = static_cast<void*>(const_cast<RawClass_T<T>*>(Value));
    		ClassName = GetClassName<T>(Value);
        }

    	template<typename T>
    	void SetValue(T Value)
        {
        	RawPtr = nullptr;
	        std::shared_ptr<T> SharedValue = std::make_shared<T>(Value);
        	SharedPtr = SharedValue;
    		ClassName = GetClassName<T>(&Value);
        }

    	bool IsValid() const
        {
	        return RawPtr != nullptr || SharedPtr != nullptr;
        }
    	
    	void SetDebugName(const char* InDebugName)
        {
	        DebugName = InDebugName;
        }

    	std::string GetName() const
        {
			return ClassName;
        }

    	~UserData()
        {
        	RawPtr = nullptr;
			SharedPtr = nullptr;
#if ToLuauDebug
        	DebugName = nullptr;
#endif
        }
    private:
    	const char* DebugName = "";
    	std::shared_ptr<void> SharedPtr;
    	void* RawPtr = nullptr;
    	std::string ClassName;
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
