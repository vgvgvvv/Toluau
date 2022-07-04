//
// Created by chen.dong on 2022/7/4.
//

#pragma once

namespace ToLuau
{
    struct BaseUserData
    {
        void* RawPtr = nullptr;
        virtual ~BaseUserData() = default;
    };

    template<typename T>
    struct UserData : public BaseUserData
    {
        const T* GetValue() const
        {
            return reinterpret_cast<T*>(RawPtr);
        }

        ~UserData() override
        {
            auto RealValue = GetValue();
            if(RealValue != nullptr)
            {
                delete RealValue;
                RawPtr = nullptr;
            }
        }
    };
}
