#include "ILuaExporter.h"
#include "Toluau/Util/Util.h"
#if ToLuauTool
#include "Toluau/ToLuau.h"
#include "Toluau/API/IToLuauRegister.h"
#include "Toluau/Containers/ToLuauVar.h"


namespace ToLuau
{
	namespace LuauTypeFile
	{
		class LuauTypeExporter : public ILuaExporter
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
		private:
			mutable std::string FileContent;
			mutable std::string CurrentBuilder;
		};

		std::string LuauTypeExporter::GetLuaTypeName(const std::string& InTypeName) const
		{
			std::string TypeName = InTypeName;
			StringEx::ReplaceAll(TypeName, " ", "");
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
			if(TypeName == "luau_uint64")
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

			if(TypeName == GetClassName<LuaAnyType>() || TypeName == ToLuauVar::StaticLuaClass()->Name())
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

		void LuauTypeExporter::GenSingleLuaClassProperties(const ToLuau::FieldMetaData& Field, std::string& Builder) const
		{
			Builder += "\t" + Field.Name + " : " + GetLuaTypeName(Field.Type);
		}

		void LuauTypeExporter::GenSingleLuaClassFunctions(const ClassMetaData& Class, const FunctionGroupMetaData& Function, std::string& Builder) const
		{
			auto FunctionName = Function.Name;
			auto bIsStatic = Function.bIsStatic;
			auto DefaultFunc = Function.Group[0];

			std::string Params;
			bool IsFirst = true;
			if(!bIsStatic)
			{
				Params += "self : " + Class.GetName();
				IsFirst = false;
			}
			for (auto& Pair : DefaultFunc->ParamTypes)
			{
				if(!IsFirst)
				{
					Params += ", ";
				}
				Params += std::get<0>(Pair) + " : " + GetLuaTypeName(std::get<1>(Pair));
				IsFirst = false;
			}

			std::string Return;
			if(!DefaultFunc->ReturnType.empty())
			{
				auto ReturnType = GetLuaTypeName(DefaultFunc->ReturnType);
				if(Return != "void")
				{
					Return = ReturnType;
				}
			}

			Builder += "\t" + FunctionName + " : (" + Params + ") -> (" + Return + ")"; 
		}

		void LuauTypeExporter::GenLuaSingleClass(const ClassMetaData& Class, const std::string& OutPath) const
		{
			CurrentBuilder.clear();
			
			auto ClassName = Class.GetName();
			auto SuperClassName = Class.GetSuperClass();
			if(SuperClassName.empty())
			{
				CurrentBuilder += "export type " + ClassName + " = " + ClassName + "_Impl \n";
			}
			else
			{
				CurrentBuilder += "export type " + ClassName + " = " + ClassName + "_Impl & " + SuperClassName + "\n";
			}
			CurrentBuilder += "type " + ClassName + "_Impl = { \n";

			bool IsFirst = true;
			
			for (auto& Pair : Class.GetFields())
			{
				if(!IsFirst)
				{
					CurrentBuilder += ", \n";
				}
				GenSingleLuaClassProperties(*Pair.second, CurrentBuilder);
				IsFirst = false;
			}
			
			for (auto& Pair : Class.GetFunctions())
			{
				if(!IsFirst)
				{
					CurrentBuilder += ", \n";
				}
				GenSingleLuaClassFunctions(Class, *Pair.second, CurrentBuilder);
				IsFirst = false;
			}

			CurrentBuilder += "\n}\n\n";
			
			FileContent.append(CurrentBuilder);
		}

		void LuauTypeExporter::GenLuaClassDefines(const std::map<std::string, std::shared_ptr<ClassMetaData>>& Classes, const std::string& OutPath) const
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

		void LuauTypeExporter::GenLuaSingleEnum(const EnumMetaData& Enum, const std::string& OutPath) const
		{
			CurrentBuilder.clear();
			CurrentBuilder += "export type " + Enum.GetName() + " = {\n";

			bool IsFirst = true;
			for (auto& Pair : Enum.GetFields())
			{
				if(!IsFirst)
				{
					CurrentBuilder += ",\n";
				}
				auto Name = Pair.first;
				CurrentBuilder += "\t" + Name + " : "+ " number";
				IsFirst = false;
			}

			CurrentBuilder += "\n}\n\n";
			FileContent.append(CurrentBuilder);
		}

		void LuauTypeExporter::GenLuaEnumDefines(const std::map<std::string, std::shared_ptr<EnumMetaData>>& Enums, const std::string& OutPath) const
		{
			for (auto& Pair : Enums)
			{
				auto Enum = Pair.second;
				GenLuaSingleEnum(*Enum, OutPath);
			}
		}

		void LuauTypeExporter::Export(const std::string& OutPath) const
		{
			FileContent.clear();

			FileContent += "\n---generate file dont modify!!!\n---@diagnostic disable\n\n\n";
			
			if(!FileEx::DirExist(OutPath))
			{
				FileEx::CreateDir(OutPath);
			}

			ToLuauState State;
			auto& MetaData = State.GetRegister().GetMetaData();
			auto GlobalMetaData = MetaData.GetGlobalMetaData();

			if(!GlobalMetaData)
			{
				return;
			}

			GenLuaClassDefines(GlobalMetaData->GetClasses(), OutPath);
			GenLuaEnumDefines(GlobalMetaData->GetEnums(), OutPath);

			auto FilePath = PathHelper::Combine(OutPath, "UEExport.lua");
			if(FileEx::FileExist(FilePath))
			{
				FileEx::RemoveFile(FilePath);
			}
			FileEx::WriteFile(FilePath, FileContent);
		}
	}
}

#ifdef TOLUAUUNREAL_API
FAutoConsoleCommand GExportLuauTypeFile(TEXT("toluau.exportlib.luautype"), TEXT("Export Luau Type library for toluau"),
FConsoleCommandDelegate::CreateLambda([]()
{
	auto Path = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Luau/Definition"));
	ToLuau::LuauTypeFile::LuauTypeExporter Exporter;
	ExportLua(Exporter, ToLuau::StringEx::FStringToStdString(Path));
}));
#endif

#endif