#include "EmmyLuaExporter.h"
#include "ILuaExporter.h"
#if ToLuauTool
#include "Toluau/ToLuau.h"
#include "Toluau/API/IToLuauRegister.h"
#include "Toluau/API/MetaData.h"

namespace ToLuau
{
	namespace EmmyLua
	{

		class EmmyLuaExporter : public ILuaExporter
		{
		public:
			virtual std::string GetLuaTypeName(const std::string& TypeName) const override;
			virtual void GenSingleLuaClassProperties(const ToLuau::FieldMetaData& Field, std::string& Builder) const override;
			virtual void GenSingleLuaClassFunctions(const ClassMetaData& Class, const FunctionGroupMetaData& Function, std::string& Builder) const override;
			virtual void GenLuaSingleClass(const ClassMetaData& Class, const std::string& OutPath) const override;
			virtual void GenLuaClassDefines(const std::map<std::string, std::shared_ptr<ClassMetaData>>& Classes, const std::string& OutPath) const override;
			virtual void GenLuaSingleEnum(const EnumMetaData& Enum, const std::string& OutPath) const override;
			virtual void GenLuaEnumDefines(const std::map<std::string, std::shared_ptr<EnumMetaData>>& Enums, const std::string& OutPath) const override;
			virtual void Export(const std::string& OutPath) const override;
		};

		std::string EmmyLuaExporter::GetLuaTypeName(const std::string& TypeName) const
		{
			if(TypeName == "float" ||
					TypeName == "double" )
			{
				return "number";
			}
			if(TypeName.rfind("int", 0) == 0)
			{
				return "number";
			}
			if(TypeName.rfind("uint", 0) == 0)
			{
				return "number";
			}
			if(TypeName == "bool")
			{
				return "boolean";
			}
			if(TypeName == "FString" ||
				TypeName == "FName" ||
				TypeName == "FText")
			{
				return "string";
			}

			if(TypeName == GetClassName<LuaAnyType>())
			{
				return "any";
			}

			if(TypeName.rfind("TArray") != std::string_view::npos)
			{
				return "ToLuauArray";
			}

			if(TypeName.rfind("TMap") != std::string_view::npos)
			{
				return "ToLuauMap";
			}

			if(TypeName.rfind("TSubclassOf", 0) == 0)
			{
				return "UClass";
			}

			if(TypeName.rfind("TEnumAsByte", 0) == 0)
			{
				auto Index = TypeName.find_first_of("<");
				auto LastIndex = TypeName.find_last_of(">");
				auto EnumName = TypeName.substr(Index + 1, LastIndex - Index - 1);
				StringEx::ReplaceAll(EnumName, "::Type", "");
				return GetLuaTypeName(EnumName);
			}
				
			if(TypeName.rfind("<", 0) != std::string_view::npos)
			{
				auto Index = TypeName.find_first_of("<");
				return TypeName.substr(0, Index);
			}
				
			auto TempTypeName = TypeName;
			StringEx::ReplaceAll(TempTypeName, "*", "");
			StringEx::ReplaceAll(TempTypeName, "::", ".");
			return TempTypeName;
		}

		void EmmyLuaExporter::GenSingleLuaClassProperties(const ToLuau::FieldMetaData& Field, std::string& Builder) const
		{
			Builder += "---@field " + Field.Name + " " + GetLuaTypeName(Field.Type) + "\n";
		}

		void EmmyLuaExporter::GenSingleLuaClassFunctions(const ClassMetaData& Class, const FunctionGroupMetaData& Function, std::string& Builder) const
		{
			auto FunctionName = Function.Name;
			auto bIsStatic = Function.bIsStatic;
			// not support override func
			auto DefaultFunc = Function.Group[0];
			for (auto& Pair : DefaultFunc->ParamTypes)
			{
				Builder += "---@param " + std::get<0>(Pair) + " " + GetLuaTypeName(std::get<1>(Pair)) + "\n";
			}
			if(!DefaultFunc->ReturnType.empty())
			{
				auto ReturnType = DefaultFunc->ReturnType;
				Builder += "---@return " + GetLuaTypeName(ReturnType) + "\n";
			}
				
			auto Dot = bIsStatic ? "." : ":";

			std::string ParamBuilder;
			bool bIsFirst = true;
			for (auto& Pair : DefaultFunc->ParamTypes)
			{
				if(!bIsFirst)
				{
					ParamBuilder += ", ";
				}
				ParamBuilder += std::get<0>(Pair);
				bIsFirst = false;
			}
				
			Builder += "function " + Class.GetName() + Dot + FunctionName + "(" + ParamBuilder + ") end \n";
		}

		void EmmyLuaExporter::GenLuaSingleClass(const ClassMetaData& Class, const std::string& OutPath) const
		{
			std::string Builder;
			auto superClassName = Class.GetSuperClass();
			if(superClassName.empty())
			{
				Builder += "---@class " + Class.GetName() + "\n";
			}
			else
			{
				Builder += "---@class " + Class.GetName() + " : " + Class.GetSuperClass() + "\n";
			}

			for (auto& Pair : Class.GetFields())
			{
				GenSingleLuaClassProperties(*Pair.second, Builder);
			}

			Builder += Class.GetName() + " = {} \n";

			for (auto& Pair : Class.GetFunctions())
			{
				GenSingleLuaClassFunctions(Class, *Pair.second, Builder);
			}

			FileEx::CreateDir(OutPath);

			auto FinalPath = PathHelper::Combine(OutPath, Class.GetName() + ".lua");
			FileEx::WriteFile(FinalPath, Builder);
		}

		void EmmyLuaExporter::GenLuaClassDefines(const std::map<std::string, std::shared_ptr<ClassMetaData>>& Classes, const std::string& OutPath) const
		{
			for (auto& Pair : Classes)
			{
				auto Class = Pair.second;
				if(!IsValid(*Class))
				{
					continue;
				}
				GenLuaSingleClass(*Class, OutPath);
			}
		}

		void EmmyLuaExporter::GenLuaSingleEnum(const EnumMetaData& Enum, const std::string& OutPath) const
		{
			std::string Builder;
			Builder += "---@class " + Enum.GetName() + "\n";
			for (auto& Pair : Enum.GetFields())
			{
				auto Name = Pair.first;
				Builder += "---@field " + Name + + " number \n";
			}

			Builder += Enum.GetName() + " = {} \n";
				
			FileEx::CreateDir(OutPath);

			auto FinalPath = PathHelper::Combine(OutPath, Enum.GetName() + ".lua");
			FileEx::WriteFile(FinalPath, Builder);
		}

		void EmmyLuaExporter::GenLuaEnumDefines(const std::map<std::string, std::shared_ptr<EnumMetaData>>& Enums, const std::string& OutPath) const
		{
			for (auto& Pair : Enums)
			{
				auto Enum = Pair.second;
				GenLuaSingleEnum(*Enum, OutPath);
			}
		}

		void EmmyLuaExporter::Export(const std::string& OutPath) const
		{
#if PLATFORM_WINDOWS

			if(FileEx::DirExist(OutPath))
			{
				FileEx::RemoveDir(OutPath);
			}
			FileEx::CreateDir(OutPath);
			
			ToLuauState State;
			auto& MetaData = State.GetRegister().GetMetaData();
			auto GlobalMetaData = MetaData.GetGlobalMetaData();

			if(!GlobalMetaData)
			{
				return;
			}

			GenLuaClassDefines(GlobalMetaData->GetClasses(), OutPath);
			GenLuaEnumDefines(GlobalMetaData->GetEnums(), OutPath);

			auto ZipPath = PathHelper::Combine(OutPath, "../EmmyLuaLib.zip");
			if(FileEx::FileExist(ZipPath))
			{
				FileEx::RemoveFile(ZipPath);
			}
			auto Command = "zip -r -j " + ZipPath + " " + OutPath;
			system(Command.c_str());
#else
			UE_LOG(LogTemp, Error, TEXT("only support windows to export lua lib"));
#endif
		}
	}
}

#ifdef TOLUAUUNREAL_API
FAutoConsoleCommand GExportEmmyLua(TEXT("toluau.exportlib.emmylua"), TEXT("Export Emmy Lua library for toluau"),
FConsoleCommandDelegate::CreateLambda([]()
{
	auto Path = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("EmmyLua"));
	ToLuau::EmmyLua::EmmyLuaExporter Exporter;
	ExportLua(Exporter, ToLuau::StringEx::FStringToStdString(Path));
}));
#endif

#endif