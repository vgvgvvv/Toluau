#pragma once

#include <vector>
#include <memory>
#include <string>
#include <map>

#include "lua.h"
#include "lualib.h"

#include "ToLuau_API.h"


namespace ToLuau
{
	class IScriptLoader;
    class ILuauChunkLoader;
    class IToLuauAPI;
    class IToLuauRegister;


	class ToLuau_API ILuauState
	{
	public:
        ILuauState();
		~ILuauState() = default;
		lua_State* GetState() const { return L; }

        virtual ILuauChunkLoader& GetLoader() const = 0;
        virtual IToLuauAPI& GetAPI() const = 0;
        virtual IToLuauRegister& GetRegister() const = 0;

        static const ILuauState* GetByRawState(lua_State* TargetState);

	protected:
		lua_State* L = nullptr;
        static std::map<lua_State*, const ILuauState*> AllLuauState;
	};

	class ToLuau_API ToLuauState final : public ILuauState
	{
	public:
		ToLuauState();
		~ToLuauState();

		ILuauChunkLoader& GetLoader() const override { return *Loader; }
		IToLuauAPI& GetAPI() const override { return *API; }
        IToLuauRegister& GetRegister() const override { return *Register; };

	private:
		std::shared_ptr<ILuauChunkLoader> Loader;
		std::shared_ptr<IToLuauAPI> API;
		std::shared_ptr<IToLuauRegister> Register;
	};

}
