#include "ILuauChunkLoader.h"

#include "Toluau.h"
#include "Util/Util.h"
#include "Luau/Common.h"
#include "Luau/Compiler.h"

namespace ToLuau
{
	class DefaultScriptLoader final : public IScriptLoader
	{
	public:
		explicit DefaultScriptLoader(ILuauChunkLoader* InOwner)
			: IScriptLoader(InOwner)
		{
		}

		bool Load(const std::string& path) override;
	};

	bool DefaultScriptLoader::Load(const std::string& path)
	{
		for (auto& LoadPath : Owner->GetLoadPaths())
		{
			if(Owner->RequireFromFile(LoadPath))
			{
				return true;
			}
		}
		return false;
	}

	class LuauChunkLoader : public ILuauChunkLoader
	{
	public:

		explicit LuauChunkLoader(ILuauState* InOwner);

		bool Require(const std::string& Path) const override;

		bool RequireFromFile(const std::string& Path) const override;

	};

	LuauChunkLoader::LuauChunkLoader(ILuauState* InOwner) : ILuauChunkLoader(InOwner)
	{
		AddLoader(std::make_shared<DefaultScriptLoader>(this));
	}

	bool LuauChunkLoader::Require(const std::string& Path) const
	{
		for(auto& Loader : Loaders)
		{
			if(Loader->Load(Path))
			{
				break;
			}
		}
		return false;
	}

	bool LuauChunkLoader::RequireFromFile(const std::string& Path) const
	{
		auto L = Owner->GetState();

		std::string Name = Path;
		StringEx::ReplaceAll(Name, ".", "/");

		auto ChunkName = "=" + Name;

		auto FinishRequire = [](lua_State* L)->int32_t{
			if (lua_isstring(L, -1))
				lua_error(L);
			return 1;
		};

		luaL_findtable(L, LUA_REGISTRYINDEX, "_MODULES", 1);

		// return the module from the cache
		lua_getfield(L, -1, Name.c_str());
		if (!lua_isnil(L, -1))
			return FinishRequire(L);
		lua_pop(L, 1);


		std::string Content;
		for (const std::string &LoadPath: LoadPaths)
		{
			std::string FinalLoadPath = PathHelper::Combine(LoadPath, Name);

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
			if(TryReadFile(LuaPath))
			{
				break;
			}

			auto LuauPath = FinalLoadPath + ".luau";
			if(TryReadFile(LuauPath))
			{
				break;
			}
		}

		if(Content.empty())
		{
			return false;
		}

		std::string ByteCode = Luau::compile(Content, CompileOptions);
		if (luau_load(L, ChunkName.c_str(), ByteCode.data(), ByteCode.size(), 0) == 0)
		{
			auto status = lua_resume(L, NULL, 0);
			if (status == 0)
			{
				if (lua_gettop(L) == 0)
					lua_pushstring(L, "module must return a value");
				else if (!lua_istable(L, -1) && !lua_isfunction(L, -1))
					lua_pushstring(L, "module must return a table or function");
			}
			else if (status == LUA_YIELD)
			{
				lua_pushstring(L, "module can not yield");
			}
			else if (!lua_isstring(L, -1))
			{
				lua_pushstring(L, "unknown error while running module");
			}
		}

		lua_pushvalue(L, -1);
		lua_setfield(L, -4, Name.c_str());

		FinishRequire(L);

		return true;
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
