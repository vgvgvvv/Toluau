#pragma once
#include <memory>
#include <string>
#include <vector>

class lua_State;


namespace ToLuau
{
	class ILuauState;
	class ToLuauState;
	class ILuauChunkLoader;

	using LoaderPtr = std::shared_ptr<class IScriptLoader>;


	class IScriptLoader
	{
	public:
		IScriptLoader(ILuauChunkLoader* InOwner) : Owner(InOwner){}
		virtual ~IScriptLoader() = default;
		virtual bool Load(const std::string& path) = 0;
	protected:
		ILuauChunkLoader* Owner;
	};

	class ILuauChunkLoader
	{
	protected:
		ILuauChunkLoader(ILuauState* InOwner) : Owner(InOwner) {}
	public:
		virtual ~ILuauChunkLoader() = default;

		static std::shared_ptr<ILuauChunkLoader> Create(ILuauState* Owner);

		virtual bool Require(const std::string& Path) const = 0;

		virtual bool RequireFromFile(const std::string& Path) const = 0;

		const std::vector<std::string>& GetLoadPaths() const;

		void AddLoader(LoaderPtr Loader);

		void RemoveLoader(LoaderPtr Loader);

		void AddLoadPath(const std::string& Path);

		void RemoveLoadPath(const std::string& Path);



	protected:
		std::vector<std::string> LoadPaths;
		std::vector<LoaderPtr> Loaders;
		ILuauState* Owner;
	};
}
