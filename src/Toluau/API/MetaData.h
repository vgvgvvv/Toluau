#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cassert>

#include "Toluau/Util/Util.h"

namespace ToLuau
{
    enum class ScopeType
    {
        Namespace,
        Class,
		Enum
    };

    struct FunctionMetaData
    {
        std::map<std::string, std::string> ParamTypes;
        std::string ReturnType;
    };

    struct FunctionGroupMetaData
    {
        bool bIsStatic;
        std::string Name;
        std::vector<std::shared_ptr<FunctionMetaData>> Group;
    };

    struct FieldMetaData
    {
        bool bIsStatic;
        std::string Name;
        std::string Type;
    };

	class IMetaDataScope
	{
	public:
		virtual ScopeType Type() const = 0;
		virtual void RegVar(const std::shared_ptr<FieldMetaData>& InField) { LUAU_ERROR("Cannot reg var in current scope !!") }
		virtual void RegFunction(const std::shared_ptr<FunctionGroupMetaData>& InField) { LUAU_ERROR("Cannot reg var in current scope !!") }
	};

	class EnumMetaData : public IMetaDataScope
	{
		friend class LuaMetaData;
	public:
		explicit EnumMetaData(const std::string& InName)
		{
			Name = InName;
		}
		virtual ScopeType Type() const override { return ScopeType::Enum; }

        void RegVar(const std::shared_ptr<FieldMetaData> &InField) override;

	private:
		std::string Name;
		std::map<std::string, std::shared_ptr<FieldMetaData>> Fields;
	};

    class  ClassMetaData : public IMetaDataScope
    {
	    friend class LuaMetaData;
    public:
	    ClassMetaData(const std::string& InName, const std::string& InSuperClass)
	    {
			Name = InName;
		}
        ScopeType Type() const override { return ScopeType::Class; }

		void RegVar(const std::shared_ptr<FieldMetaData> &InField) override;
		void RegFunction(const std::shared_ptr<FunctionGroupMetaData> &InFunction) override;

    private:
        std::string Name;
		std::string SuperClass;
        std::map<std::string, std::shared_ptr<FunctionGroupMetaData>> Functions;
        std::map<std::string, std::shared_ptr<FieldMetaData>> Fields;
    };

    class  NamespaceMetaData : public IMetaDataScope
    {
		friend class LuaMetaData;
    public:
	    explicit NamespaceMetaData(const std::string& InName)
	    {
			Name = InName;
		}
        ScopeType Type() const override { return ScopeType::Namespace; }

		void RegVar(const std::shared_ptr<FieldMetaData> &InField) override;
		void RegFunction(const std::shared_ptr<FunctionGroupMetaData> &InFunction) override;

    private:
        std::string Name;
        std::map<std::string, std::shared_ptr<ClassMetaData>> Classes;
	    std::map<std::string, std::shared_ptr<EnumMetaData>> Enums;
	    std::map<std::string, std::shared_ptr<NamespaceMetaData>> Namespaces;
	    std::map<std::string, std::shared_ptr<FunctionGroupMetaData>> Functions;
	    std::map<std::string, std::shared_ptr<FieldMetaData>> Fields;
    };

    class ToLuau_API LuaMetaData
    {
    public:
	    LuaMetaData()
	    {
			GlobalMetaData = std::make_shared<NamespaceMetaData>("");
		}

        NamespaceMetaData& PushNamespace(const std::string& Name);
        ClassMetaData& PushClass(const std::string& Name, const std::string& SuperName);
	    EnumMetaData& PushEnum(const std::string& Name);

        void RegVar(bool IsStatic, const std::string& TypeName, const std::string& VarName);
        void RegFunction(bool IsStatic, const std::string& FunctionName, const FunctionGroupMetaData& FunctionInfo);

        void Pop();
    private:
	    std::shared_ptr<NamespaceMetaData> GlobalMetaData;
        std::vector<std::shared_ptr<IMetaDataScope>> ScopeStack;
    };
}
