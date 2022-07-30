//
// Created by chen.dong on 7/25/2022.
//
#include "MetaData.h"
#include <cassert>
#include "Toluau/Util/Util.h"

namespace ToLuau
{

	NamespaceMetaData& ToLuau::LuaMetaData::PushNamespace(const std::string &Name)
	{
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
			assert(false);
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
			assert(false);
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
			assert(false);
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

    void LuaMetaData::RegFunction(bool IsStatic, const std::string& FunctionName, const FunctionGroupMetaData& FunctionInfo) {

    }

	void LuaMetaData::Pop()
	{
		assert(!ScopeStack.empty());
		ScopeStack.pop_back();
	}

	void ClassMetaData::RegVar(const std::shared_ptr<FieldMetaData>& InField)
	{
		auto It = Fields.find(InField->Name);
		if(It == Fields.end())
		{
			Fields.insert({InField->Name, InField});
		}
		else
		{
			LUAU_ERROR_F("%s has registered field %s", Name.c_str(), InField->Name.c_str());
		}
	}

	void ClassMetaData::RegFunction(const std::shared_ptr<FunctionGroupMetaData>& InField)
	{

	}

	void NamespaceMetaData::RegVar(const std::shared_ptr<FieldMetaData>& InField)
	{
		auto It = Fields.find(InField->Name);
		if(It == Fields.end())
		{
			Fields.insert({InField->Name, InField});
		}
		else
		{
			LUAU_ERROR_F("%s has registered field %s", Name.c_str(), InField->Name.c_str());
		}
	}
	void NamespaceMetaData::RegFunction(const std::shared_ptr<FunctionGroupMetaData>& InField)
	{

	}

	void EnumMetaData::RegVar(const std::shared_ptr<FieldMetaData> &InField)
	{
		auto It = Fields.find(InField->Name);
		if(It == Fields.end())
		{
			Fields.insert({InField->Name, InField});
		}
		else
		{
			LUAU_ERROR_F("%s has registered field %s", Name.c_str(), InField->Name.c_str());
		}
	}
}
