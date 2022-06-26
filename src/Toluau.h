#pragma once

#include "ToLuau_API.h"
#include "API/IToLuauAPI.h"
#include "API/ILuauChunkLoader.h"
#include "API/IToLuauRegister.h"
#include "lua.h"

#include <vector>
#include <memory>
#include <string>

namespace ToLuau
{
	class IScriptLoader;

	class ToLuau_API ILuauState
	{
	public:
		~ILuauState() = default;
		lua_State* GetState() const { return L; }

	protected:
		lua_State* L = nullptr;
	};

	class ToLuau_API ToLuauState final : public ILuauState
	{
	public:
		ToLuauState();
		~ToLuauState();

		ILuauChunkLoader& GetLoader() const { return *Loader; }
		IToLuauAPI& GetAPI() const { return *API; }

	private:
		std::shared_ptr<ILuauChunkLoader> Loader;
		std::shared_ptr<IToLuauAPI> API;
		std::shared_ptr<IToLuauRegister> Register;
	};

}