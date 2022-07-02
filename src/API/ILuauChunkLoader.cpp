#include "ILuauChunkLoader.h"

#include "ToLuau.h"
#include "Util/Util.h"
#include "ToLuauLib.h"
#include "Luau/Common.h"
#include "Luau/Compiler.h"

namespace ToLuau
{

#pragma region file loader

	class FileScriptLoader final : public IScriptLoader
	{
	public:
		explicit FileScriptLoader(ILuauChunkLoader* InOwner)
			: IScriptLoader(InOwner)
		{
		}

		bool Load(const std::string& Name) override;
	};

	bool FileScriptLoader::Load(const std::string& Name)
	{


		for (auto& LoadPath : Owner->GetLoadPaths())
		{
			if(Owner->RequireFromFile(LoadPath, Name))
			{
				return true;
			}
		}
		return false;
	}

#pragma endregion


#pragma region package loader

    class PackageScriptLoader final : public IScriptLoader
    {
    public:
        explicit PackageScriptLoader(ILuauChunkLoader* InOwner)
        : IScriptLoader(InOwner)
                {
                }

        bool Load(const std::string& path) override;
    };

    bool PackageScriptLoader::Load(const std::string &path)
    {
        auto L = Owner->GetOwner()->GetState();
        lua_getref(L, TOLUAU_LOADED_REF); // preload
        lua_pushstring(L, path.c_str()); // preload path
        lua_rawget(L, -2); // preload value
        if(lua_isnil(L, -1))
        {
            lua_pop(L, 1);
            return false;
        }
        return true;
    }

#pragma endregion

	class LuauChunkLoader : public ILuauChunkLoader
	{
	public:

		explicit LuauChunkLoader(ILuauState* InOwner);

		bool Require(const std::string& Path) const override;

		bool RequireFromFile(const std::string& Path, const std::string& FileName) const override;

	};

	LuauChunkLoader::LuauChunkLoader(ILuauState* InOwner) : ILuauChunkLoader(InOwner)
	{
        AddLoader(std::make_shared<PackageScriptLoader>(this));
		AddLoader(std::make_shared<FileScriptLoader>(this));
	}

	bool LuauChunkLoader::Require(const std::string& Path) const
	{
		for(auto& Loader : Loaders)
		{
			if(Loader->Load(Path))
			{
				return true;
			}
		}
        // if require failed ,push nil
        lua_pushnil(Owner->GetState());
		return false;
	}

	bool LuauChunkLoader::RequireFromFile(const std::string& Path, const std::string& FileName) const
	{
		auto L = Owner->GetState();

		std::string FinalLoadPath = ToLuau::PathHelper::Combine(Path, FileName);
        std::string ModuleName = FileName;
		StringEx::ReplaceAll(ModuleName, ".", "/");

        Lua::Log(StringEx::Format("require name : %s", ModuleName.c_str()));

        auto ChunkName = "=" + ModuleName;

		auto FinishRequire = [](lua_State* L)->int32_t{
			if (lua_isstring(L, -1))
            {
                size_t Len;
                auto ErrorInfo = lua_tolstring(L, -1, &Len);
                std::string ErrorStr(ErrorInfo, Len);
                LUAU_LOG(ErrorStr);
                lua_pop(L, 1);
            }

			return 1;
		};

		luaL_findtable(L, LUA_REGISTRYINDEX, "_MODULES", 1);

		// return the module from the cache
		lua_getfield(L, -1, ModuleName.c_str());
		if (!lua_isnil(L, -1))
			return FinishRequire(L);
		lua_pop(L, 1);

        std::string Content;
        auto TryReadFile = [this, &Content](const std::string& FinalLoadPath)->bool {
            if(DefaultLoadFileFunc.has_value())
            {
                Content = DefaultLoadFileFunc.value()(FinalLoadPath);
                if(!Content.empty())
                {
                    return true;
                }
            }
            else
            {
                auto TempContent = FileEx::ReadFile(FinalLoadPath);
                if(TempContent.has_value())
                {
                    Content = TempContent.value();
                    return true;
                }
            }
            return false;
        };

        auto LuaPath = FinalLoadPath + ".lua";
        if(!TryReadFile(LuaPath))
        {
            auto LuauPath = FinalLoadPath + ".luau";
            if(!TryReadFile(LuauPath))
            {
                auto Error = StringEx::Format("cannot read file : %s !", FinalLoadPath.c_str());
                lua_pushstring(L, Error.c_str());
                FinishRequire(L);
                return false;
            }
        }

		if(Content.empty())
		{
            auto Error = StringEx::Format("file : %s is empty !", FinalLoadPath.c_str());
            lua_pushstring(L, Error.c_str());
            FinishRequire(L);
			return false;
		}

		std::string ByteCode = Luau::compile(Content, CompileOptions);
		if (luau_load(L, ChunkName.c_str(), ByteCode.data(), ByteCode.size(), 0) == 0)
		{
            bool Succ = true;
            if (lua_gettop(L) == 0)
            {
                lua_pushstring(L, "module must return a value");
                Succ = false;
            }
            else if (!lua_istable(L, -1) && !lua_isfunction(L, -1))
            {
                lua_pushstring(L, "module must return a table or function");
                Succ = false;
            }

            if(Succ)
            {
                lua_pushvalue(L, -1); // table function
                lua_call(L, 0, 1); // table function module
                lua_pushvalue(L, -1);   // table function module module
                lua_setfield(L, -4, ModuleName.c_str()); // table function module
                lua_remove(L, 1);
                lua_remove(L, 1);
            }

            FinishRequire(L);

            return Succ;

		}
        else
        {
            FinishRequire(L);
            return false;
        }

	}

	std::shared_ptr<ILuauChunkLoader> ILuauChunkLoader::Create(ILuauState* Owner)
	{
		return std::make_shared<LuauChunkLoader>(Owner);
	}

	const std::vector<std::string>& ILuauChunkLoader::GetLoadPaths() const
	{
		return LoadPaths;
	}

	void ILuauChunkLoader::AddLoader(LoaderPtr Loader)
	{
		Loaders.push_back(Loader);
	}

	void ILuauChunkLoader::RemoveLoader(LoaderPtr Loader)
	{
		auto end = 
			std::remove(Loaders.begin(), Loaders.end(), Loader);
	}

	void ILuauChunkLoader::AddLoadPath(const std::string& Path)
	{
		LoadPaths.push_back(Path);
	}

	void ILuauChunkLoader::RemoveLoadPath(const std::string& Path)
	{
		auto end = 
			std::remove(LoadPaths.begin(), LoadPaths.end(), Path);
	}

}
