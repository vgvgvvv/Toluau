#pragma once

#include "ToLuau_API.h"
#include "IToLuauAPI.h"
#include "lua.h"

#include <vector>
#include <memory>
#include <string>

namespace ToLuau
{
	class IScriptLoader;

	class State : public IToLuauAPI
	{
		void Init(lua_State* State);

	private:
		lua_State* L;
		std::vector<std::shared_ptr<IScriptLoader>> Loaders;
	};

	class IScriptLoader
	{
		virtual bool Load(const std::string& path) = 0;
	};
}