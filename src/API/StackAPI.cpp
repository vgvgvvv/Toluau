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
        int32_t PushRefObj(lua_State *L, BaseUserData* Instance, const Class* LuaClass) // ud
        {
            auto Owner = ILuauState::GetByRawState(L);
            auto ClassRef = Owner->GetRegister().GetClassMetaRef(LuaClass->Name());
            if(ClassRef > 0)
            {
                lua_getref(L, ClassRef); // ud meta
                lua_setmetatable(L, -2); // ud
            }
            else
            {
                luaL_errorL(L, "cannot find class meta ref %s", LuaClass->Name());
            }
            return 1;
        }

        void* CheckRefObj(lua_State* L, int32_t Pos, const Class* LuaClass)
        {
            auto Owner = ILuauState::GetByRawState(L);
            auto ClassRef = Owner->GetRegister().GetClassMetaRef(LuaClass->Name());
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
                auto CurrentClass = LuaClass;
                while(true)
                {
                    if(std::strcmp(FullName, LuaClass->Name()) == 0)
                    {
                        break;
                    }
                    else
                    {
                        CurrentClass = CurrentClass->GetBaseClass();
                        if(!CurrentClass)
                        {
                            luaL_typeerrorL(L, Pos, LuaClass->Name());
                            return nullptr;
                        }
                    }
                }

	            lua_pop(L, 2);

                return reinterpret_cast<BaseUserData*>(lua_touserdata(L, Pos))->RawPtr;
            }
            else
            {
                luaL_errorL(L, "cannot find class meta ref %s", LuaClass->Name());
            }
            return nullptr;
        }

    }
}


