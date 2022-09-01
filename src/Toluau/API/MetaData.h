#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cassert>
#include <tuple>

#include "Toluau/Class/ClassInfo.h"
#include "Toluau/Util/Util.h"
#include "Toluau/Class/ClassName.h"

namespace ToLuau
{
	class LuaAnyType final
	{
		DEFINE_LUA_CLASS(LuaAnyType)
	};
	
    enum class ScopeType
    {
        Namespace,
        Class,
		Enum
    };

    struct FunctionMetaData
    {
        std::vector<std::tuple<std::string, std::string>> ParamTypes;
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
		virtual ~IMetaDataScope() = default;
		virtual ScopeType Type() const = 0;
		virtual void RegVar(const std::shared_ptr<FieldMetaData>& InField) { LUAU_ERROR("Cannot reg var in current scope !!") }
		virtual void RegFunction(const std::shared_ptr<FunctionGroupMetaData>& InField) { LUAU_ERROR("Cannot reg var in current scope !!") }
	};

	class EnumMetaData : public IMetaDataScope
	{
		friend class LuaMetaData;
	public:
		EnumMetaData(){}
		explicit EnumMetaData(const std::string& InName)
		{
			Name = InName;
		}
		virtual ScopeType Type() const override { return ScopeType::Enum; }

        void RegVar(const std::shared_ptr<FieldMetaData> &InField) override;

		[[nodiscard]] const std::string& GetName() const
		{
			return Name;
		}

		[[nodiscard]] const std::map<std::string, std::shared_ptr<FieldMetaData>>& GetFields() const
		{
			return Fields;
		}

	private:
		std::string Name;
		std::map<std::string, std::shared_ptr<FieldMetaData>> Fields;
	};

    class ClassMetaData : public IMetaDataScope
    {
	    friend class LuaMetaData;
    public:
    	ClassMetaData() {}
	    ClassMetaData(const std::string& InName, const std::string& InSuperClass)
	    {
			Name = InName;
	    	SuperClass = InSuperClass;
		}
        ScopeType Type() const override { return ScopeType::Class; }

		void RegVar(const std::shared_ptr<FieldMetaData> &InField) override;
		void RegFunction(const std::shared_ptr<FunctionGroupMetaData> &InFunction) override;

	    [[nodiscard]] const std::string& GetName() const
	    {
		    return Name;
	    }

	    [[nodiscard]] const std::string& GetSuperClass() const
	    {
		    return SuperClass;
	    }

	    [[nodiscard]] const std::map<std::string, std::shared_ptr<FunctionGroupMetaData>>& GetFunctions() const
	    {
		    return Functions;
	    }

	    [[nodiscard]] const std::map<std::string, std::shared_ptr<FieldMetaData>>& GetFields() const
	    {
		    return Fields;
	    }

    private:
        std::string Name;
		std::string SuperClass;
        std::map<std::string, std::shared_ptr<FunctionGroupMetaData>> Functions;
        std::map<std::string, std::shared_ptr<FieldMetaData>> Fields;
    };

    class NamespaceMetaData : public IMetaDataScope
    {
		friend class LuaMetaData;
    public:
    	NamespaceMetaData(){};
	    explicit NamespaceMetaData(const std::string& InName)
	    {
			Name = InName;
		}
        ScopeType Type() const override { return ScopeType::Namespace; }

		void RegVar(const std::shared_ptr<FieldMetaData> &InField) override;
		void RegFunction(const std::shared_ptr<FunctionGroupMetaData> &InFunction) override;

		[[nodiscard]] const std::string& GetName() const
		{
			return Name;
		}

		[[nodiscard]] const std::map<std::string, std::shared_ptr<ClassMetaData>>& GetClasses() const
		{
			return Classes;
		}

		[[nodiscard]] const std::map<std::string, std::shared_ptr<EnumMetaData>>& GetEnums() const
		{
			return Enums;
		}

		[[nodiscard]] const  std::map<std::string, std::shared_ptr<NamespaceMetaData>>& GetNamespaces() const
		{
			return Namespaces;
		}

		[[nodiscard]] const  std::map<std::string, std::shared_ptr<FunctionGroupMetaData>>& GetFunctions() const
		{
			return Functions;
		}

		[[nodiscard]] const  std::map<std::string, std::shared_ptr<FieldMetaData>>& GetFields() const
		{
			return Fields;
		}

	private:
        std::string Name;
        std::map<std::string, std::shared_ptr<ClassMetaData>> Classes;
	    std::map<std::string, std::shared_ptr<EnumMetaData>> Enums;
	    std::map<std::string, std::shared_ptr<NamespaceMetaData>> Namespaces;
	    std::map<std::string, std::shared_ptr<FunctionGroupMetaData>> Functions;
	    std::map<std::string, std::shared_ptr<FieldMetaData>> Fields;
    };

	namespace DETAIL
	{
		template<int I>
		void GetParams(std::vector<std::tuple<std::string, std::string>>& Result);
		
		template<int I, typename T, typename... Tp>
    	void GetParams(std::vector<std::tuple<std::string, std::string>>& Result);
	
		template<int I, typename T, typename... Tp>
		void GetParams(std::vector<std::tuple<std::string, std::string>>& Result)
		{
			auto Name = "Arg" + std::to_string(I);
			auto ClassName = GetClassName<T>(nullptr);
			Result.push_back({Name, ClassName});
			GetParams<I+1, Tp...>(Result);
		}
		
        		
		template<int I>
		void GetParams(std::vector<std::tuple<std::string, std::string>>& Result) { }

		template<typename T, bool IsStatic>
		struct FunctionMetaDataConvertor;
		
		template<typename TRet, typename... TArgs, bool IsStatic>
		struct FunctionMetaDataConvertor<TRet(*)(TArgs...), IsStatic>
		{
			static std::shared_ptr<FunctionMetaData> GetFunctionMetaData()
			{
				std::shared_ptr<FunctionMetaData> Result = std::make_shared<FunctionMetaData>();
				Result->ReturnType = GetClassName<TRet>(nullptr);
				std::vector<std::tuple<std::string, std::string>> Params;
				GetParams<0, TArgs...>(Params);
				Result->ParamTypes.insert(Result->ParamTypes.end(), Params.begin(), Params.end());
				return Result;
			}
			
			static std::shared_ptr<FunctionGroupMetaData> GetFunctionGroupMetaData(const std::string& FunctionName)
			{
				std::shared_ptr<FunctionGroupMetaData> Result = std::make_shared<FunctionGroupMetaData>();
				Result->bIsStatic = IsStatic;
				Result->Name = FunctionName;
				Result->Group.push_back(GetFunctionMetaData());
				return Result;
			}
		};
		
		template<typename TOwner, typename TRet, typename... TArgs, bool IsStatic>
		struct FunctionMetaDataConvertor<TRet (TOwner::*)(TArgs...) const, IsStatic>
		{
			static std::shared_ptr<FunctionMetaData> GetFunctionMetaData()
			{
				std::shared_ptr<FunctionMetaData> Result = std::make_shared<FunctionMetaData>();
				Result->ReturnType = GetClassName<TRet>(nullptr);
				std::vector<std::tuple<std::string, std::string>> Params;
				GetParams<0, TArgs...>(Params);
				Result->ParamTypes.insert(Result->ParamTypes.end(), Params.begin(), Params.end());
				return Result;
			}
			
			static std::shared_ptr<FunctionGroupMetaData> GetFunctionGroupMetaData(const std::string& FunctionName)
			{
				std::shared_ptr<FunctionGroupMetaData> Result = std::make_shared<FunctionGroupMetaData>();
				Result->bIsStatic = IsStatic;
				Result->Name = FunctionName;
				Result->Group.push_back(GetFunctionMetaData());
				return Result;
			}
		};
		
		template<typename TOwner, typename TRet, typename... TArgs, bool IsStatic>
		struct FunctionMetaDataConvertor<TRet (TOwner::*)(TArgs...), IsStatic>
		{
			static std::shared_ptr<FunctionMetaData> GetFunctionMetaData()
			{
				std::shared_ptr<FunctionMetaData> Result = std::make_shared<FunctionMetaData>();
				Result->ReturnType = GetClassName<TRet>(nullptr);
				std::vector<std::tuple<std::string, std::string>> Params;
				GetParams<0, TArgs...>(Params);
				Result->ParamTypes.insert(Result->ParamTypes.end(), Params.begin(), Params.end());
				return Result;
			}
			
			static std::shared_ptr<FunctionGroupMetaData> GetFunctionGroupMetaData(const std::string& FunctionName)
			{
				std::shared_ptr<FunctionGroupMetaData> Result = std::make_shared<FunctionGroupMetaData>();
				Result->bIsStatic = IsStatic;
				Result->Name = FunctionName;
				Result->Group.push_back(GetFunctionMetaData());
				return Result;
			}
		};
		
		template<typename TOwner,typename... TArgs, bool IsStatic>
		struct FunctionMetaDataConvertor<void (TOwner::*)(TArgs...) const, IsStatic>
		{
			static std::shared_ptr<FunctionMetaData> GetFunctionMetaData()
			{
				std::shared_ptr<FunctionMetaData> Result = std::make_shared<FunctionMetaData>();
				std::vector<std::tuple<std::string, std::string>> Params;
				GetParams<0, TArgs...>(Params);
				Result->ParamTypes.insert(Result->ParamTypes.end(), Params.begin(), Params.end());
				return Result;
			}
			
			static std::shared_ptr<FunctionGroupMetaData> GetFunctionGroupMetaData(const std::string& FunctionName)
			{
				std::shared_ptr<FunctionGroupMetaData> Result = std::make_shared<FunctionGroupMetaData>();
				Result->bIsStatic = IsStatic;
				Result->Name = FunctionName;
				Result->Group.push_back(GetFunctionMetaData());
				return Result;
			}
		};

		template<typename TOwner,typename... TArgs, bool IsStatic>
		struct FunctionMetaDataConvertor<void (TOwner::*)(TArgs...), IsStatic>
		{
			static std::shared_ptr<FunctionMetaData> GetFunctionMetaData()
			{
				std::shared_ptr<FunctionMetaData> Result = std::make_shared<FunctionMetaData>();
				std::vector<std::tuple<std::string, std::string>> Params;
				GetParams<0, TArgs...>(Params);
				Result->ParamTypes.insert(Result->ParamTypes.end(), Params.begin(), Params.end());
				return Result;
			}
			
			static std::shared_ptr<FunctionGroupMetaData> GetFunctionGroupMetaData(const std::string& FunctionName)
			{
				std::shared_ptr<FunctionGroupMetaData> Result = std::make_shared<FunctionGroupMetaData>();
				Result->bIsStatic = IsStatic;
				Result->Name = FunctionName;
				Result->Group.push_back(GetFunctionMetaData());
				return Result;
			}
		};
		
	}

	class ToLuau_API ILuaMetaData
	{
	public:
		static std::shared_ptr<ILuaMetaData> CreateMetaData();
		virtual ~ILuaMetaData() = default;
		virtual NamespaceMetaData& PushNamespace(const std::string& Name) = 0;
		virtual ClassMetaData& PushClass(const std::string& Name, const std::string& SuperName)  = 0;
		virtual EnumMetaData& PushEnum(const std::string& Name)  = 0;

		virtual void RegVar(bool IsStatic, const std::string& TypeName, const std::string& VarName)  = 0;
		virtual void RegFunction(bool IsStatic, const std::string& FunctionName, const FunctionGroupMetaData& FunctionInfo)  = 0;
		virtual void RegFunction(std::shared_ptr<FunctionGroupMetaData> MetaData) = 0;

		template<typename TF>
		void RegFunction(const std::string& FunctionName)
		{
			auto FunctionGroupMeta = DETAIL::FunctionMetaDataConvertor<TF, !std::is_member_function_pointer_v<TF>>::GetFunctionGroupMetaData(FunctionName);
			RegFunction(FunctionGroupMeta);
		}
		public:
    	
		virtual const NamespaceMetaData* GetGlobalMetaData() const = 0;

#ifdef TOLUAUUNREAL_API
		virtual void RegAllUEClass() = 0;
		virtual ClassMetaData& PushUEClass(UClass* Class)  = 0;
		virtual ClassMetaData& PushUEStruct(UScriptStruct* Struct)  = 0;
		virtual void ExportClassProperties(UStruct* Class)  = 0;
		virtual void ExportClassFunctions(UStruct* Class)  = 0;
		virtual void RegAllUEEnum()  = 0;
#endif

		virtual void Pop()  = 0;
	};
	
}
