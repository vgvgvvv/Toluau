//
// Created by chen.dong on 2022/7/4.
//

#pragma once

#ifdef TOLUAUUNREAL_API
#include "CoreMinimal.h"
#endif
#include "Toluau/Class/ClassInfo.h"

namespace ToLuau
{

    struct BaseUserData
    {
    	void* RawPtr = nullptr;
    	const char* DebugName = nullptr;;
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
        		DebugName = nullptr;
        	}
        }

    };

}
