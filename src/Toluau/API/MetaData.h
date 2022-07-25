#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace ToLuau
{
    enum class ScopeType
    {
        Namespace,
        Class
    };

    class IMetaDataScope
    {
    public:
        virtual ScopeType Type() const = 0;
    };

    class  FunctionMetaData
    {
        std::map<std::string, std::string> ParamTypes;
        std::string ReturnType;
    };

    class  FunctionGroupMetaData
    {
        bool bIsStatic;
        std::string Name;
        std::vector<std::shared_ptr<FunctionMetaData>> Group;
    };

    class  FieldMetaData
    {
        bool bIsStatic;
        std::string Name;
        std::string Type;
    };

    class  ClassMetaData : public IMetaDataScope
    {
    public:
        virtual ScopeType Type() const override { return ScopeType::Class; }

    private:
        std::string Name;
        std::map<std::string, std::shared_ptr<FunctionGroupMetaData>> Functions;
        std::map<std::string, std::shared_ptr<FieldMetaData>> Fields;
    };

    class  NamespaceMetaData : public IMetaDataScope
    {
    public:
        virtual ScopeType Type() const override { return ScopeType::Namespace; }

    private:
        std::string Name;
        std::map<std::string, std::shared_ptr<ClassMetaData>> Classes;
    };

    class LuaMetaData
    {
    public:
        NamespaceMetaData& PushNamespace(const std::string& Name);
        ClassMetaData& PushClass(const std::string& Name, const std::string& SuperName);

        void RegVar(const std::string& TypeName, const std::string& VarName);

        void Pop();
    private:
        std::map<std::string, std::shared_ptr<NamespaceMetaData>> ClassMetaData;
        std::vector<std::shared_ptr<IMetaDataScope>> ScopeStack;
    };
}
