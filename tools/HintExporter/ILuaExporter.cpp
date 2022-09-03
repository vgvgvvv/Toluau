#include "ILuaExporter.h"

#if ToLuauTool

#include "Toluau/ToLuau.h"
#include "Toluau/API/IToLuauRegister.h"

namespace ToLuau
{
	inline bool ends_with(std::string const & value, std::string const & ending)
	{
		if (ending.size() > value.size()) return false;
		return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
	}
	
	bool ILuaExporter::IsValid(const ClassMetaData& Class) const
	{
		auto Name = Class.GetName();
		auto It = Name.find("__PythonCallable");
		if(It != std::string::npos)
		{
			return false;
		}

		bool EndWith_C = ends_with(Name, "_C");
		if(EndWith_C)
		{
			return false;
		}

		return true;
	}

	void ExportLua(ILuaExporter& Exporter, const std::string& OutPath)
	{
		Exporter.Export(OutPath);
	}
}
#endif