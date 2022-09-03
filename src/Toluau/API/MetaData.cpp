//
// Created by chen.dong on 7/25/2022.
//
#include "MetaData.h"
#include <cassert>
#include <memory>

#include "Toluau/Util/Util.h"

namespace ToLuau
{

	
	class LuaMetaData : public ILuaMetaData
	{
		public:
		LuaMetaData()
		{
			GlobalMetaData = std::make_shared<NamespaceMetaData>("");
			ScopeStack.push_back(GlobalMetaData);
		}

		virtual NamespaceMetaData& PushNamespace(const std::string& Name) override;
		virtual ClassMetaData& PushClass(const std::string& Name, const std::string& SuperName) override;
		virtual EnumMetaData& PushEnum(const std::string& Name) override;

		virtual void RegVar(bool IsStatic, const std::string& TypeName, const std::string& VarName) override;
		virtual void RegFunction(bool IsStatic, const std::string& FunctionName, const FunctionGroupMetaData& FunctionInfo) override;
		virtual void RegFunction(std::shared_ptr<FunctionGroupMetaData> MetaData) override;
    	
		public:
    	
		virtual const NamespaceMetaData* GetGlobalMetaData() const override;

#ifdef TOLUAUUNREAL_API
		virtual void RegAllUEClass() override;
		virtual ClassMetaData& PushUEClass(UClass* Class) override;
		virtual ClassMetaData& PushUEStruct(UScriptStruct* Struct) override;
		virtual void ExportClassProperties(UStruct* Class) override;
		virtual void ExportClassFunctions(UStruct* Class) override;
		virtual void RegAllUEEnum() override;
#endif

		virtual void Pop() override;
		protected:
		std::shared_ptr<NamespaceMetaData> GlobalMetaData;
		std::vector<std::shared_ptr<IMetaDataScope>> ScopeStack;
	};

	class DummyMetaData : public ILuaMetaData
	{
		virtual NamespaceMetaData& PushNamespace(const std::string& Name) override { static NamespaceMetaData Meta; return Meta; }
		virtual ClassMetaData& PushClass(const std::string& Name, const std::string& SuperName) override { static ClassMetaData Meta; return Meta; }
		virtual EnumMetaData& PushEnum(const std::string& Name) override { static EnumMetaData Meta; return Meta;};

		virtual void RegVar(bool IsStatic, const std::string& TypeName, const std::string& VarName) override {}
		virtual void RegFunction(bool IsStatic, const std::string& FunctionName, const FunctionGroupMetaData& FunctionInfo) override{}
		virtual void RegFunction(std::shared_ptr<FunctionGroupMetaData> MetaData) override{}
    	
	public:
    	
		virtual const NamespaceMetaData* GetGlobalMetaData() const override { return nullptr; }

#ifdef TOLUAUUNREAL_API
		virtual void RegAllUEClass() override {}
		virtual ClassMetaData& PushUEClass(UClass* Class) override { static ClassMetaData Meta; return Meta; }
		virtual ClassMetaData& PushUEStruct(UScriptStruct* Struct) override { static ClassMetaData Meta; return Meta; }
		virtual void ExportClassProperties(UStruct* Class) override {}
		virtual void ExportClassFunctions(UStruct* Class) override {}
		virtual void RegAllUEEnum() override{}
#endif

		virtual void Pop() override {}
	};
	
	NamespaceMetaData& LuaMetaData::PushNamespace(const std::string &Name)
	{
		if(Name.empty())
		{
			if(!ScopeStack.empty())
			{
				LUAU_ERROR("cannot push global namespace into not empty scope stack");
				return *GlobalMetaData;
			}
			ScopeStack.push_back(GlobalMetaData);
			return *GlobalMetaData;
		}
		auto& Scope = ScopeStack.back();
		if(Scope->Type() == ScopeType::Namespace)
		{
			auto NamespaceScope = std::static_pointer_cast<NamespaceMetaData>(Scope);
			auto It = NamespaceScope->Namespaces.find(Name);
			if(It != NamespaceScope->Namespaces.end())
			{
				ScopeStack.push_back(It->second);
				return *It->second;
			}
			else
			{
				auto NewNamespace = std::make_shared<NamespaceMetaData>(Name);
				NamespaceScope->Namespaces.insert({Name, NewNamespace});
				ScopeStack.push_back(NewNamespace);
				return *NewNamespace;
			}
		}
		else
		{
			LUAU_ERROR_F("Cannot push %s name space in current scope", Name.c_str());
			TOLUAU_ASSERT(false);
			static NamespaceMetaData Def("");
			return Def;
		}
	}

	ClassMetaData& LuaMetaData::PushClass(const std::string &Name, const std::string &SuperName)
	{
		auto& Scope = ScopeStack.back();
		if (Scope->Type() == ScopeType::Namespace)
		{
			auto NamespaceScope = std::static_pointer_cast<NamespaceMetaData>(Scope);
			auto It = NamespaceScope->Classes.find(Name);
			if(It != NamespaceScope->Classes.end())
			{
				ScopeStack.push_back(It->second);
				return *It->second;
			}
			else
			{
				auto NewClass = std::make_shared<ClassMetaData>(Name, SuperName);
				NamespaceScope->Classes.insert({Name, NewClass});
				ScopeStack.push_back(NewClass);
				return *NewClass;
			}
		}
		else
		{
			LUAU_ERROR_F("Cannot push %s name space in current scope", Name.c_str());
			TOLUAU_ASSERT(false);
			static ClassMetaData Def("", "");
			return Def;
		}
	}

	EnumMetaData& LuaMetaData::PushEnum(const std::string &Name)
	{
		auto& Scope = ScopeStack.back();
		if (Scope->Type() == ScopeType::Namespace)
		{
			auto NamespaceScope = std::static_pointer_cast<NamespaceMetaData>(Scope);
			auto It = NamespaceScope->Enums.find(Name);
			if(It != NamespaceScope->Enums.end())
			{
				ScopeStack.push_back(It->second);
				return *It->second;
			}
			else
			{
				auto NewEnum = std::make_shared<EnumMetaData>(Name);
				NamespaceScope->Enums.insert({Name, NewEnum});
				ScopeStack.push_back(NewEnum);
				return *NewEnum;
			}
		}
		else
		{
			LUAU_ERROR_F("Cannot push %s name space in current scope", Name.c_str());
			TOLUAU_ASSERT(false);
			static EnumMetaData Def("");
			return Def;
		}
	}

	void LuaMetaData::RegVar(bool IsStatic, const std::string &TypeName, const std::string &VarName)
	{
		auto& Scope = ScopeStack.back();

		std::shared_ptr<FieldMetaData> Field = std::make_shared<FieldMetaData>();
		Field->bIsStatic = IsStatic;
		Field->Name = VarName;
		Field->Type = TypeName;

		Scope->RegVar(Field);
	}

    void LuaMetaData::RegFunction(bool IsStatic, const std::string& FunctionName, const FunctionGroupMetaData& FunctionInfo)
	{
		auto& Scope = ScopeStack.back();

		std::shared_ptr<FunctionGroupMetaData> Function = std::make_shared<FunctionGroupMetaData>();
		Function->bIsStatic = IsStatic;
		Function->Name = FunctionName;
		Function->Group.assign(FunctionInfo.Group.begin(), FunctionInfo.Group.end());
		
		Scope->RegFunction(Function);
    }

    void LuaMetaData::RegFunction(std::shared_ptr<FunctionGroupMetaData> MetaData)
    {
		auto& Scope = ScopeStack.back();
		Scope->RegFunction(MetaData);
    }

	const NamespaceMetaData* LuaMetaData::GetGlobalMetaData() const
	{
		return GlobalMetaData.get();
	}

#ifdef TOLUAUUNREAL_API

	void LuaMetaData::RegAllUEClass()
	{
		TArray<UObject*> Structs;
		GetObjectsOfClass(UStruct::StaticClass(), Structs);
		
		for(auto Object : Structs)
		{
			auto Struct = Cast<UStruct>(Object);

			if(Struct->GetClass() == UFunction::StaticClass())
			{
				continue;
			}
			
			if(auto Class = Cast<UClass>(Object))
			{
				auto SuperClass = Class->GetSuperClass();

				if(!SuperClass)
				{
					PushClass(StringEx::FStringToStdString(
						Class->IsNative() ? Class->GetPrefixCPP() + Class->GetName() : Class->GetName()), "");
				}
				else
				{
					PushClass(
					StringEx::FStringToStdString(
						Class->IsNative() ? Class->GetPrefixCPP() + Class->GetName() : Class->GetName()),
				StringEx::FStringToStdString(
						SuperClass->IsNative() ? SuperClass->GetPrefixCPP() + SuperClass->GetName() : SuperClass->GetName())
					);
				}
		
				ExportClassProperties(Class);
				ExportClassFunctions(Class);

				Pop();
			}
			else
			{
				auto SuperStruct = Struct->GetSuperStruct();

				if(!SuperStruct)
				{
					PushClass(StringEx::FStringToStdString(
						Struct->IsNative() ? Struct->GetPrefixCPP() + Struct->GetName() : Struct->GetName()), "");
				}
				else
				{
					PushClass(
					StringEx::FStringToStdString(
						Struct->IsNative() ? Struct->GetPrefixCPP() + Struct->GetName() : Struct->GetName()),
				StringEx::FStringToStdString(
						SuperStruct->IsNative() ? SuperStruct->GetPrefixCPP() + SuperStruct->GetName() : SuperStruct->GetName())
					);
				}
		
				ExportClassProperties(Struct);
				ExportClassFunctions(Struct);

				Pop();
			}
		}
	}

	ClassMetaData& LuaMetaData::PushUEClass(UClass* Class)
	{
		UClass* SuperClass = Class->GetSuperClass();
		if(SuperClass) 
		{
			PushUEClass(SuperClass);
			ExportClassProperties(SuperClass);
			ExportClassFunctions(SuperClass);
			Pop();
			return PushClass(
				StringEx::FStringToStdString(Class->GetPrefixCPP() + Class->GetName()),
				StringEx::FStringToStdString(Class->GetPrefixCPP() + SuperClass->GetName())); 
		}
		else
		{
			return PushClass(StringEx::FStringToStdString(Class->GetPrefixCPP() + Class->GetName()), ""); 
		}
	}

	ClassMetaData& LuaMetaData::PushUEStruct(UScriptStruct* Struct)
	{
		UScriptStruct* SuperStruct = Cast<UScriptStruct>(Struct->GetSuperStruct());
		if(SuperStruct) 
		{
			PushUEStruct(SuperStruct);
			ExportClassProperties(SuperStruct);
			ExportClassFunctions(SuperStruct);
			Pop();
			return PushClass(StringEx::FStringToStdString(Struct->GetStructCPPName()), StringEx::FStringToStdString(SuperStruct->GetStructCPPName())); 
		}
		else
		{
			return PushClass(StringEx::FStringToStdString(Struct->GetStructCPPName()), ""); 
		}
	}

	void LuaMetaData::ExportClassProperties(UStruct* Class)
	{
		for (TFieldIterator<FProperty> Iterator(Class); Iterator; ++Iterator)
		{
			if(Iterator->GetOwnerStruct() != Class)
			{
				continue;
			}
			
			auto Name = Iterator->GetNameCPP();
			auto PropertyClass = Iterator->GetCPPType();
			RegVar(false, StringEx::FStringToStdString(PropertyClass), StringEx::FStringToStdString(Name));
		}
	}

	void LuaMetaData::ExportClassFunctions(UStruct* Class)
	{
		TMap<FString, std::shared_ptr<FunctionGroupMetaData>> FunctionGroupMap;
		for (TFieldIterator<UFunction> FunctionIt(Class); FunctionIt; ++FunctionIt)
		{
			if(FunctionIt->GetOwnerStruct() != Class && !FunctionIt->HasAllFunctionFlags(EFunctionFlags::FUNC_Static))
			{
				continue;
			}
			
			auto FunctionName = FunctionIt->GetName();
			auto FunctionGroupPtr = FunctionGroupMap.Find(FunctionName);
			auto IsStatic = FunctionIt->HasAllFunctionFlags(EFunctionFlags::FUNC_Static);

			if(FunctionGroupPtr == nullptr)
			{
				FunctionGroupMap.Add(FunctionName, std::make_shared<FunctionGroupMetaData>());
				FunctionGroupPtr = FunctionGroupMap.Find(FunctionName);
			}

			auto FunctionGroup = *FunctionGroupPtr;
			
			FunctionGroup->Name = StringEx::FStringToStdString(FunctionName);
			FunctionGroup->bIsStatic = IsStatic;
			std::shared_ptr<FunctionMetaData> FunctionInfo = std::make_shared<FunctionMetaData>();
			for(TFieldIterator<FProperty> ParamIt(*FunctionIt); ParamIt; ++ParamIt)
			{
				auto ParamName = ParamIt->GetNameCPP();
				auto ParamType = ParamIt->GetCPPType();
				if(!ParamIt->HasAnyPropertyFlags(CPF_ReturnParm))
				{
					FunctionInfo->ParamTypes.push_back({StringEx::FStringToStdString(ParamName), StringEx::FStringToStdString(ParamType)});
				}else
				{
					FunctionInfo->ReturnType = StringEx::FStringToStdString(ParamType);
				}
			}

			FunctionGroup->Group.push_back(FunctionInfo);
		}

		if(FunctionGroupMap.Num() == 0)
		{
			return;
		}

		for (auto& Pair : FunctionGroupMap)
		{
			auto Name = Pair.Key;
			auto Group = Pair.Value;
			RegFunction(Group->bIsStatic, Group->Name, *Group);
		}
	}

	void LuaMetaData::RegAllUEEnum()
	{
		TArray<UObject*> Enums;
		GetObjectsOfClass(UEnum::StaticClass(), Enums);

		for(auto Object : Enums)
		{
			auto Enum = Cast<UEnum>(Object);
			PushEnum(StringEx::FStringToStdString(Enum->GetName()));
		
			auto EnumNum = Enum->NumEnums();
			for(int i = 0; i < EnumNum; i ++)
			{
				FString Name = Enum->GetNameByIndex(i).ToString();
				auto Value = Enum->GetValueByIndex(i);

				if(Name.Contains(TEXT("::")))
				{
					int32 Index;
					Name.FindLastChar(':', Index);
					auto Offset = Name.Len() - Index - 1;
					Name = Name.Right(Offset);
				}
				
				RegVar(true, "int64", StringEx::FStringToStdString(Name));
			}

			Pop();
		}
	}

#endif
	

    void LuaMetaData::Pop()
	{
		TOLUAU_ASSERT(!ScopeStack.empty());
		ScopeStack.pop_back();
	}

	void ClassMetaData::RegVar(const std::shared_ptr<FieldMetaData>& InField)
	{
		auto It = Fields.find(InField->Name);
		if(It == Fields.end())
		{
			Fields.insert({InField->Name, InField});
		}
		// else
		// {
		// 	LUAU_ERROR_F("%s has registered field %s", Name.c_str(), InField->Name.c_str());
		// }
	}

	void ClassMetaData::RegFunction(const std::shared_ptr<FunctionGroupMetaData>& InFunction)
	{
		auto It = Functions.find(InFunction->Name);
		if(It == Functions.end())
		{
			Functions.insert({ InFunction->Name, InFunction});
		}
		// else
		// {
		// 	LUAU_ERROR_F("%s has registered field %s", Name.c_str(), InFunction->Name.c_str());
		// }
	}

	void NamespaceMetaData::RegVar(const std::shared_ptr<FieldMetaData>& InField)
	{
		auto It = Fields.find(InField->Name);
		if(It == Fields.end())
		{
			Fields.insert({InField->Name, InField});
		}
		// else
		// {
		// 	LUAU_ERROR_F("%s has registered field %s", Name.c_str(), InField->Name.c_str());
		// }
	}
	void NamespaceMetaData::RegFunction(const std::shared_ptr<FunctionGroupMetaData>& InFunction)
	{
		auto It = Functions.find(InFunction->Name);
		if(It == Functions.end())
		{
			Functions.insert({ InFunction->Name, InFunction});
		}
		// else
		// {
		// 	LUAU_ERROR_F("%s has registered field %s", Name.c_str(), InFunction->Name.c_str());
		// }
	}

	std::shared_ptr<ILuaMetaData> ILuaMetaData::CreateMetaData()
	{
#ifdef TOLUAUUNREAL_API
#if UE_EDITOR
		if(GIsPlayInEditorWorld)
		{
			return std::make_shared<DummyMetaData>();
		}
		else
		{
			return std::make_shared<LuaMetaData>();
		}
#else
		return std::make_shared<DummyMetaData>();
#endif
#else
		return std::make_shared<DummyMetaData>();
#endif
	}

	void EnumMetaData::RegVar(const std::shared_ptr<FieldMetaData> &InField)
	{
		auto It = Fields.find(InField->Name);
		if(It == Fields.end())
		{
			Fields.insert({InField->Name, InField});
		}
		// else
		// {
		// 	LUAU_ERROR_F("%s has registered field %s", Name.c_str(), InField->Name.c_str());
		// }
	}
}
