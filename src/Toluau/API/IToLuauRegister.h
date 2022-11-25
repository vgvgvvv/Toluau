//
// Created by 35207 on 2022/6/26 0026.
//

#pragma once

#include <string>
#include <functional>

#include "lua.h"
#include "MetaData.h"
#if LUAU_SUPPORT_HOYO_CLASS
#include "FlyweightClass/Class.h"
#endif
#include "Luau/Common.h"
#include "Toluau/Class/Class.h"
#include "Toluau/ToLuauDefine.h"


struct lua_State;

namespace ToLuau
{

	struct ToLuau_API LuaClassInfo
	{
		std::string ClassName;
		uint64_t ParentClassId;
		std::vector<uint64_t> DerivedClassIds;
	};
	
	class ILuauState;
	enum class MetatableEvent;

	class ToLuau_API IToLuauRegister
	{
	protected:
		explicit IToLuauRegister(ILuauState* InOwner) : Owner(InOwner) {}
	public:

		virtual ~IToLuauRegister() = default;

		const ILuauState* GetOwner() const { return Owner; }

		static std::shared_ptr<IToLuauRegister> Create(ILuauState* InOwner);

		using LuaFunc = lua_CFunction;

		virtual void RegisterAll() = 0;

		virtual void BeginModule(const std::string& ModuleName) = 0;
		virtual void EndModule() = 0;

		void BeginClass(const std::string& ClassName) { BeginClass(ClassName, ""); }
		virtual void BeginClass(const std::string& ClassName, const std::string& SuperClassName, bool Append = true) = 0;

		virtual void RegClass(const Class* LuaClass) = 0;
		virtual void BeginClass(const Class* LuaClass) = 0; 
		virtual void BeginClass(const Class* LuaClass, const Class* SuperLuaClass, bool Append = true) = 0;

#if LUAU_SUPPORT_HOYO_CLASS
		virtual void RegClass(const FHoYoClass* LuaClass) = 0;
		virtual void BeginClass(const FHoYoClass* LuaClass) = 0; 
		virtual void BeginClass(const FHoYoClass* LuaClass, const FHoYoClass* SuperLuaClass, bool Append = true) = 0;
#endif
		
		virtual void EndClass() = 0;

		

		template<typename TClass>
		void BeginClass(bool Append = true)
		{
			static_assert(!TIsEnum<TClass>::Value, "please use begin enum !!");
			
#if LUAU_SUPPORT_HOYO_CLASS
			if constexpr (HasStaticHoYoClass<TClass>::Value)
			{
				const FHoYoClass* Class = &TClass::StaticHoYoClass();
				const FHoYoClass* SuperClass = Class->GetBaseClass();
				RegClass(SuperClass);
				BeginClass(Class);
				return;
			}
#endif
#ifdef TOLUAUUNREAL_API
			if constexpr (HasStaticClass<TClass>::Value)
			{
				UClass* Class = TClass::StaticClass();
				RegUClass(Class->GetSuperClass());
				BeginUEClass(Class, Append);
				return;
			}
			if constexpr (HasStaticStruct<TClass>::Value)
			{
				UStruct* Class = TClass::StaticStruct();
				RegUStruct(Class->GetSuperStruct());
				BeginUEStruct(Class, Append);
				return;
			}
#endif
			if constexpr (HasStaticLuaClass<TClass>::Value)
			{
				const ToLuau::Class* Class = TClass::StaticLuaClass();
				const  ToLuau::Class* SuperClass = Class->GetBaseClass();
				RegClass(SuperClass);
				BeginClass(Class);
				return;
			}

			auto ClassName = GetClassName<TClass>();
			BeginClass(ClassName, "", Append);
		}
		
		template<typename TClass, typename TBaseClass>
		void BeginClass(bool Append = true)
		{
			static_assert(!TIsEnum<TClass>::Value, "please use begin enum !!");
			
#if LUAU_SUPPORT_HOYO_CLASS
			if constexpr (HasStaticHoYoClass<TClass>::Value)
			{
				const FHoYoClass* Class = &TClass::StaticHoYoClass();
				const FHoYoClass* SuperClass = Class->GetBaseClass();
				RegClass(SuperClass);
				TOLUAU_ASSERT(SuperClass != nullptr && *SuperClass == TBaseClass::StaticHoYoClass());
				BeginClass(Class);
				return;
			}
#endif
#ifdef TOLUAUUNREAL_API
			if constexpr (HasStaticClass<TClass>::Value)
			{
				UClass* Class = TClass::StaticClass();
				UClass* SuperClass = Class->GetSuperClass();
				RegUClass(SuperClass);
				TOLUAU_ASSERT(SuperClass != nullptr && TBaseClass::StaticClass() != SuperClass);
				BeginUEClass(Class, Append);
				return;
			}
			if constexpr (HasStaticStruct<TClass>::Value)
			{
				UStruct* Class = TClass::StaticStruct();
				UStruct* SuperClass = Class->GetSuperStruct();
				RegUStruct(SuperClass);
				TOLUAU_ASSERT(SuperClass != nullptr && TBaseClass::StaticStruct() != SuperClass);
				BeginUEStruct(Class, Append);
				return;
			}
#endif
			if constexpr (HasStaticLuaClass<TClass>::Value)
			{
				const ToLuau::Class* Class = TClass::StaticLuaClass();
				const  ToLuau::Class* SuperClass = Class->GetBaseClass();
				RegClass(SuperClass);
				TOLUAU_ASSERT(SuperClass != nullptr && *SuperClass == *TBaseClass::StaticLuaClass());
				BeginClass(Class);
				return;
			}

			auto ClassName = GetClassName<TClass>();
			auto BaseClassName = GetClassName<TBaseClass>();
			BeginClass(ClassName, BaseClassName, Append);
		}

		virtual void BeginEnum(const std::string& EnumName) = 0;
		virtual void EndEnum() = 0;

		virtual void BeginStaticLib(const std::string& StaticLibName) = 0;
		virtual void EndStaticLib() = 0;

		virtual void RegFunction(const std::string& FuncName, LuaFunc Func) = 0;
		virtual void RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter) = 0;
		virtual void RegMetaFunction(const MetatableEvent& FuncName, LuaFunc Func) = 0;

		virtual const LuaClassInfo* GetClassInfo(const std::string& Name) = 0;

		virtual ILuaMetaData& GetMetaData() = 0;

#ifdef TOLUAUUNREAL_API
		virtual void RegUClass(UClass* Class, bool Append = true) = 0;
		virtual void BeginUEClass(UClass* Class, bool Append = true) = 0;
		virtual void EndUEClass() = 0;
		
		virtual void RegUEnum(UEnum* Enum) = 0;
		virtual void RegUStruct(UStruct* Struct, bool Append = true) = 0;
		virtual void BeginUEStruct(UStruct* Struct, bool Append = true) = 0;
		virtual void EndUEStruct() = 0;
	
#endif

		virtual int32_t GetEnumRef(const std::string& EnumName) const = 0;
		virtual void SetEnumRef(const std::string& EnumName, int32_t Ref) = 0;
		
		virtual int32_t GetStaticLibRef(const std::string& StaticLibName) const = 0;
		virtual void SetStaticLibRef(const std::string& StaticLibName, int32_t Ref) = 0;
		
		virtual int32_t GetClassMetaRef(const std::string& ClassName) const = 0;
		virtual void SetClassMetaRef(const std::string& ClassName, int32_t Ref) = 0;

#ifdef TOLUAUUNREAL_API
		virtual int32_t GetClassMetaRefByUStruct(const UStruct* Struct) const = 0;
		virtual int32_t GetClassMetaRefByUClass(const UClass* Class) const = 0;
#endif

		virtual int32_t GetClassMetaRefByLuaClass(const ToLuau::Class* Class) const = 0;

#if LUAU_SUPPORT_HOYO_CLASS
		virtual int32_t GetClassMetaRefByHoYoClass(const FHoYoClass* Class) const = 0;
#endif
		
		template<typename T>
		int32_t GetClassMetaRefByClass(const void* Obj)
		{
#if LUAU_SUPPORT_HOYO_CLASS
			if constexpr (HasStaticHoYoClass<T>::Value)
			{
				T* RealObj = static_cast<T*>(Obj);
				FHoYoClass* CurrentClass = RealObj->GetHoYoClassPtr();
				auto Result = GetClassMetaRefByHoYoClass(CurrentClass);
				if(Result > 0)
				{
					return Result;
				}
			}
#endif
#ifdef TOLUAUUNREAL_API
			if constexpr (HasStaticClass<T>::Value)
			{
				T* RealObj = static_cast<T*>(Obj);
				UClass* CurrentClass = RealObj->GetClass();
				auto Result = GetClassMetaRefByUClass(CurrentClass);
				if(Result > 0)
				{
					return Result;
				}
			}
			if constexpr (HasStaticStruct<T>::Value)
			{
				UStruct* CurrentClass = T::StaticStruct();
				auto Result = GetClassMetaRefByUStruct(CurrentClass);
				if(Result > 0)
				{
					return Result;
				}
			}
#endif
			if constexpr (HasStaticLuaClass<T>::Value)
			{
				T* RealObj = static_cast<T*>(Obj);
				Class* CurrentClass = RealObj->GetLuaClass();
				auto Result = GetClassMetaRefByLuaClass(CurrentClass);
				if(Result > 0)
				{
					return Result;
				}
			}

			{
				auto Ref = GetClassMetaRef(GetClassName<T>());
				if(Ref > 0)
				{
					return Ref;
				}
			}

			return -1;
		}
		
	protected:
		ILuauState* Owner = nullptr;
	};

	class ToLuau_API ILuauStaticRegister
	{
	public:
		ILuauStaticRegister();
		virtual ~ILuauStaticRegister();
		virtual void LuaRegister(IToLuauRegister* Register) = 0;
	};

}

