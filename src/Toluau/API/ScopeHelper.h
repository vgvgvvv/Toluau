#pragma once

#include <string>
#include "Toluau/ToLuauDefine.h"

struct lua_State;

namespace ToLuau
{
	namespace StackAPI
	{
		struct ToLuau_API TableScope
		{
			TableScope(lua_State* InL, const std::string& InName, bool bCreateIfNil);
			~TableScope();

			const std::string& GetName() const;
			bool IsValid() const;
		private:
			lua_State* L = nullptr;
			std::string Name;
			bool bIsValid = false;
		};

	}
}

