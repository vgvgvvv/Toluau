//
// Created by 35207 on 2022/6/26 0026.
//

#include "lua.h"

#include "StackAPI.h"
#include "ToLuau.h"
#include "IToLuauRegister.h"

namespace ToLuau
{
    namespace StackAPI
    {
        int32_t PushRefObj(lua_State *L, void* Instance, const std::string &ClassName)
        {
            auto Owner = ILuauState::GetByRawState(L);
            auto ClassRef = Owner->GetRegister().GetClassMetaRef(ClassName);
            if(ClassRef > 0)
            {
            }
            else
            {
                lua_pushlightuserdata(L, Instance);
            }
            return 1;
        }

        void* CheckRefObj(lua_State* L, int32_t Pos, const std::string& ClassName)
        {
            auto Owner = ILuauState::GetByRawState(L);
            auto ClassRef = Owner->GetRegister().GetClassMetaRef(ClassName);
            if(ClassRef > 0)
            {
                if(!lua_isuserdata(L, Pos))
                {
                    luaL_typeerror(L, Pos, "userdata");
                    return nullptr;
                }
                lua_getref(L, ClassRef); // ref
                lua_getfield(L, -1, ".name"); // ref fullname
                auto FullName = lua_tostring(L, -1);
                if(FullName != ClassName)
                {
                    luaL_typeerror(L, Pos, FullName);
                    return nullptr;
                }

                return lua_touserdata(L, Pos);
            }
            else
            {
                if(!lua_islightuserdata(L, Pos))
                {
                    luaL_typeerror(L, Pos, "lightuserdata");
                    return nullptr;
                }
                return lua_tolightuserdata(L, Pos);
            }
            return nullptr;
        }

    }
}


