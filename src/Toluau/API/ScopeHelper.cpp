#include "ScopeHelper.h"

#include <lua.h>
#include <lualib.h>
#include "ToLuauLib.h"

namespace ToLuau
{
	namespace StackAPI
	{
		TableScope::TableScope(lua_State* InL, const std::string& InName, bool bCreateIfNil)
		{
			Name = InName;
			this->L = InL;
			if(Name.empty())
			{
				lua_getref(L, TOLUAU_GLOBAL_REF);
				bIsValid = true;
			}
			else
			{
				lua_pushstring(L, Name.c_str()); // stack name
				lua_rawget(L, -2); // stack value
				if(lua_isnil(L, -1))
				{
					if(bCreateIfNil)
					{
						lua_pop(L, -1);
						lua_newtable(L);
						lua_pushstring(L, Name.c_str());
						lua_pushvalue(L, -2);
						lua_rawset(L, -4);
						bIsValid = true;
					}
					else
					{
						bIsValid = false;
					}
				}
				else if(lua_istable(L, -1))
				{
					bIsValid = true;
				}
				else
				{
					bIsValid = false;
				}
			}
		}

		TableScope::~TableScope()
		{
			lua_pop(L, 1);
		}

		const std::string& TableScope::GetName() const
		{
			return Name;
		}

		bool TableScope::IsValid() const
		{
			return L != nullptr && bIsValid;
		}
	};
}
