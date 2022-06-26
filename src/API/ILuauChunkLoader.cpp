#include "ILuauChunkLoader.h"

#include "Toluau.h"


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
		//TODO
		return false;
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
