#pragma once
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <functional>

#ifdef TOLUAUUNREAL_API
#include "Toluau/ToLuau_API.h"
#else
#include "ToLuau_API.h"
#endif

#include "Luau/Compiler.h"

namespace ToLuau
{
	class ILuauState;
	class ToLuauState;
	class ILuauChunkLoader;

	using LoaderPtr = std::shared_ptr<class IScriptLoader>;

	class ToLuau_API IScriptLoader
	{
	public:
		explicit IScriptLoader(ILuauChunkLoader* InOwner) : Owner(InOwner){}
		virtual ~IScriptLoader() = default;
		virtual bool Load(const std::string& path, bool ForceReload) = 0;
	protected:
		ILuauChunkLoader* Owner;
	};

	class ToLuau_API ILuauChunkLoader
	{
	protected:
		explicit ILuauChunkLoader(ILuauState* InOwner) : Owner(InOwner)
		{
			CompileOptions.debugLevel = 1;
			CompileOptions.optimizationLevel = 1;
			CompileOptions.coverageLevel = 0;
		}
	public:
		virtual ~ILuauChunkLoader() = default;

        const ILuauState* GetOwner() const { return Owner; }

		static std::shared_ptr<ILuauChunkLoader> Create(ILuauState* Owner);

		virtual bool Require(const std::string& Path, bool ForceReload = false) const = 0;

		virtual bool RequireFromFile(const std::string& Path, const std::string& FileName, bool ForceReload) const = 0;

		const std::vector<std::string>& GetLoadPaths() const;

		void AddLoader(LoaderPtr Loader);

		void RemoveLoader(LoaderPtr Loader);

		void AddLoadPath(const std::string& Path);

		void RemoveLoadPath(const std::string& Path);



	protected:
		std::vector<std::string> LoadPaths;
		std::vector<LoaderPtr> Loaders;
		ILuauState* Owner = nullptr;
		Luau::CompileOptions CompileOptions = {};

#ifdef TOLUAUUNREAL_API
		TOptional<std::function<std::string(const std::string&)>> DefaultLoadFileFunc;
#else
		std::optional<std::function<std::string(const std::string&)>> DefaultLoadFileFunc;
#endif
	};
}
