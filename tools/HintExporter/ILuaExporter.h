#pragma once

#include <string>

#include "Toluau/API/MetaData.h"

#if ToLuauTool

namespace ToLuau
{
	class ILuaExporter
	{
	public:
		virtual ~ILuaExporter() = default;
		virtual std::string GetLuaTypeName(const std::string& TypeName) const = 0;
		virtual void GenSingleLuaClassProperties(const ToLuau::FieldMetaData& Field, std::string& Builder) const = 0;
		virtual void GenSingleLuaClassFunctions(const ClassMetaData& Class, const FunctionGroupMetaData& Function, std::string& Builder) const = 0;
		virtual void GenLuaSingleClass(const ClassMetaData& Class, const std::string& OutPath) const = 0;
		virtual void GenLuaClassDefines(const std::map<std::string, std::shared_ptr<ClassMetaData>>& Classes, const std::string& OutPath) const = 0;
		virtual void GenLuaSingleEnum(const EnumMetaData& Enum, const std::string& OutPath) const = 0;
		virtual void GenLuaEnumDefines(const std::map<std::string, std::shared_ptr<EnumMetaData>>& Enums, const std::string& OutPath) const = 0;
		virtual void Export(const std::string& OutPath) const = 0;
		virtual bool IsValid(const ClassMetaData& Class) const;
	};
	
	ToLuau_API void ExportLua(ILuaExporter& Exporter, const std::string& OutPath);

}

#endif