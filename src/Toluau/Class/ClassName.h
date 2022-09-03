#pragma once
#include <cassert>

#include "Toluau/Util/Util.h"
#include "Toluau/Util/Template.h"
#include "Toluau/ToLuauDefine.h"

#define DEFINE_LUA_CLASS_NAME(ClassName) \
template<> \
struct GetClassNameWrapper<ClassName> \
{ \
	using Type = ClassName;\
	static constexpr bool IsValid = true; \
	static constexpr bool DefineByMacro = true;\
	static std::string GetName(const void* Obj) \
	{ \
		return #ClassName; \
	} \
};

#define DEFINE_LUA_CLASS_NAME_WITH_ALIAS(ClassName, Alias) \
template<> \
struct GetClassNameWrapper<ClassName> \
{ \
	using Type = ClassName;\
	static constexpr bool IsValid = true; \
	static constexpr bool DefineByMacro = true; \
	static std::string GetName(const void* Obj) \
	{ \
		return #Alias; \
	} \
};

#define DEFINE_LUA_TEMPLATE_CLASS_NAME_OneParam(ClassName) \
template<typename T> \
struct GetClassNameWrapper<ClassName<T>> \
{ \
	static constexpr bool IsValid = true; \
	static constexpr bool DefineByMacro = false; \
	using Type = ClassName<T>; \
	static std::string GetName(const void* Obj) \
	{ \
		std::string Result; \
		Result += #ClassName "<"; \
		Result += GetClassName<T>(); \
		Result += ">"; \
		return Result; \
	} \
};

#define DEFINE_LUA_TEMPLATE_CLASS_NAME_TwoParam(ClassName) \
template<typename T1, typename T2> \
struct GetClassNameWrapper<ClassName<T1, T2>> \
{ \
	static constexpr bool IsValid = true; \
	static constexpr bool DefineByMacro = false; \
	using Type = ClassName<T1, T2>; \
	static std::string GetName(const void* Obj) \
	{ \
		std::string Result; \
		Result += #ClassName "<"; \
		Result += GetClassName<T1>(); \
		Result += ", "; \
		Result += GetClassName<T2>(); \
		Result += ">"; \
		return Result; \
	} \
};

namespace ToLuau
{
	
	template<typename T, typename = void>
	struct HasGetClassName;
	
	template<typename T, typename = void>
	struct IsDefineClassNameByMacro;

	template<typename T>
	static std::string GetClassName(const void* Obj = nullptr);
	
	template<typename T, typename = void>
	struct GetClassNameWrapper;

	template<typename T>
	struct GetClassNameWrapper<T, typename std::enable_if<
		HasStaticLuaClass<T>::Value
#if LUAU_SUPPORT_HOYO_CLASS
		&& !HasStaticHoYoClass<T>::Value
#endif
#ifdef TOLUAUUNREAL_API
		&& !HasStaticStruct<T>::Value
		&& !TIsDerivedFrom<T, UObject>::Value
#endif
	>::type>;

#if LUAU_SUPPORT_HOYO_CLASS
	template<typename T>
	struct GetClassNameWrapper<T, typename std::enable_if<
		HasStaticHoYoClass<T>::Value
		&& !HasStaticLuaClass<T>::Value
		&& !HasStaticStruct<T>::Value
		&& !TIsDerivedFrom<T, UObject>::Value>::type>;
#endif

#ifdef TOLUAUUNREAL_API
	
	template<typename T>
	struct GetClassNameWrapper<T, typename std::enable_if<HasStaticClass<T>::Value>::type>;

	template<typename T>
	struct GetClassNameWrapper<T, typename std::enable_if<HasStaticStruct<T>::Value && !HasStaticLuaClass<T>::Value>::type>;

	template<typename T, ESPMode Mode>
	struct GetClassNameWrapper<TSharedPtr<T, Mode>>;

	template<typename T, ESPMode Mode>
	struct GetClassNameWrapper<TWeakPtr<T, Mode>>;
	
	template<typename T>
	struct GetClassNameWrapper<TStrongObjectPtr<T>>;
	
	template<typename T>
	struct GetClassNameWrapper<TWeakObjectPtr<T>>;
	
	template<typename R, typename ...ARGS>
	struct GetClassNameWrapper<TDelegate<R(ARGS...)>>;

#endif

	DEFINE_LUA_CLASS_NAME(void)
	DEFINE_LUA_CLASS_NAME(int8_t)
	DEFINE_LUA_CLASS_NAME(int16_t)
	DEFINE_LUA_CLASS_NAME(int32_t)
	DEFINE_LUA_CLASS_NAME(luau_int64)
	DEFINE_LUA_CLASS_NAME(uint8_t)
	DEFINE_LUA_CLASS_NAME(uint16_t)
	DEFINE_LUA_CLASS_NAME(uint32_t)
	DEFINE_LUA_CLASS_NAME(luau_uint64)
	DEFINE_LUA_CLASS_NAME(bool)
	DEFINE_LUA_CLASS_NAME(float)
	DEFINE_LUA_CLASS_NAME(double)
	DEFINE_LUA_CLASS_NAME_WITH_ALIAS(std::string, string)

#ifdef TOLUAUUNREAL_API
	DEFINE_LUA_CLASS_NAME(FString)
	DEFINE_LUA_CLASS_NAME(FName)
	DEFINE_LUA_CLASS_NAME(FText)
#endif

	template<typename T>
	struct GetClassNameWrapper<T, typename std::enable_if<
		HasStaticLuaClass<T>::Value
#if LUAU_SUPPORT_HOYO_CLASS
		&& !HasStaticHoYoClass<T>::Value
#endif
#ifdef TOLUAUUNREAL_API
		&& !HasStaticStruct<T>::Value
		&& !TIsDerivedFrom<T, UObject>::Value
#endif
	>::type>
	{
		static constexpr bool IsValid = true;
		static constexpr bool DefineByMacro = false;
		using Type = T;
		static std::string GetName(const void* Obj)
		{
			if(!Obj)
			{
				return T::StaticLuaClass()->Name();
			}
			return static_cast<const T*>(Obj)->GetLuaClass()->Name();
		}
	};

#if LUAU_SUPPORT_HOYO_CLASS
	template<typename T>
	struct GetClassNameWrapper<T, typename std::enable_if<
		HasStaticHoYoClass<T>::Value
		&& !HasStaticLuaClass<T>::Value
		&& !HasStaticStruct<T>::Value
		&& !TIsDerivedFrom<T, UObject>::Value>::type>
	{
		using Type = T;
		static constexpr bool IsValid = true;
		static constexpr bool DefineByMacro = false;
		static std::string GetName(const void* Obj)
		{
			if(!Obj)
			{
				return StringEx::FStringToStdString(T::StaticHoYoClass().GetName().ToString());
			}
			return StringEx::FStringToStdString(static_cast<const T*>(Obj)->GetHoYoClass().GetName().ToString());
		}
	};
#endif

#ifdef TOLUAUUNREAL_API
	template<typename T>
	struct GetClassNameWrapper<T, typename std::enable_if<HasStaticClass<T>::Value>::type>
	{
		static constexpr bool IsValid = true;
		static constexpr bool DefineByMacro = false;
		using Type = T;
		static std::string GetName(const void* Obj)
		{
			UClass* C = T::StaticClass();
			if(C->IsNative())
			{
				if(!Obj)
				{
					return StringEx::FStringToStdString(C->GetPrefixCPP() + C->GetName());
				}
				return StringEx::FStringToStdString(C->GetPrefixCPP() + static_cast<const T*>(Obj)->GetClass()->GetName());
			}
			else
			{
				if(!Obj)
				{
					return StringEx::FStringToStdString(C->GetName());
				}
				return StringEx::FStringToStdString(static_cast<const T*>(Obj)->GetClass()->GetName());
			}
		}
	};
	
	template<typename T>
	struct GetClassNameWrapper<T, typename std::enable_if<HasStaticStruct<T>::Value && !HasStaticLuaClass<T>::Value>::type>
	{
		static constexpr bool IsValid = true;
		static constexpr bool DefineByMacro = false;
		using Type = T;
		static std::string GetName(const void* Obj)
		{
			UScriptStruct* Script = T::StaticStruct();
			return StringEx::FStringToStdString(T::StaticStruct()->GetStructCPPName());
		}
	};

	template<typename T>
	struct GetClassNameWrapper<TArray<T>>
	{
		static constexpr bool IsValid = true;
		static constexpr bool DefineByMacro = false;
		using Type = TArray<T>;
		static std::string GetName(const void* Obj)
		{
			return "ToLuauArray";
		}
	};
	
	template<typename K, typename V>
	struct GetClassNameWrapper<TMap<K, V>>
	{
		static constexpr bool IsValid = true;
		static constexpr bool DefineByMacro = false;
		using Type = TMap<K, V>;
		static std::string GetName(const void* Obj)
		{
			return "ToLuauMap";
		}
	};
	
	template<typename T, ESPMode Mode>
	struct GetClassNameWrapper<TSharedPtr<T, Mode>>
	{
		static constexpr bool IsValid = true;
		static constexpr bool DefineByMacro = IsDefineClassNameByMacro<T>::Value;
		using Type = TSharedPtr<T, Mode>;
		static std::string GetName(const void* Obj)
		{
			return  GetClassName<T>(Obj);
		}
	};
	
	template<typename T, ESPMode Mode>
	struct GetClassNameWrapper<TWeakPtr<T, Mode>>
	{
		static constexpr bool IsValid = true;
		static constexpr bool DefineByMacro = IsDefineClassNameByMacro<T>::Value;
		using Type = TWeakPtr<T, Mode>;
		static std::string GetName(const void* Obj)
		{
			return GetClassName<T>(Obj);
		}
	};

	template<typename T>
	struct GetClassNameWrapper<TStrongObjectPtr<T>>
	{
		static constexpr bool IsValid = true;
		static constexpr bool DefineByMacro = IsDefineClassNameByMacro<T>::Value;
		using Type = TStrongObjectPtr<T>;
		static std::string GetName(const void* Obj)
		{
			return GetClassName<T>(Obj);
		}
	};

	template<typename T>
	struct GetClassNameWrapper<TWeakObjectPtr<T>>
	{
		static constexpr bool IsValid = true;
		static constexpr bool DefineByMacro = IsDefineClassNameByMacro<T>::Value;
		using Type = TWeakObjectPtr<T>;
		static std::string GetName(const void* Obj)
		{
			return GetClassName<T>(Obj);
		}
	};


	// make generic
	template<class R, class ...ARGS>
	struct MakeGenericTypeName
	{
		static void Value(std::string& output, const char* delimiter) {
			MakeGenericTypeName<R>::Value(output, delimiter);
			if(std::tuple_size<std::tuple<ARGS...>>::value <= 1)
			{
				MakeGenericTypeName<ARGS...>::Value(output, "");
			}
			else
			{
				output += delimiter;
				MakeGenericTypeName<ARGS...>::Value(output, delimiter);
			}
		}
	};

	template<class R>
	struct MakeGenericTypeName<R>
	{
		static void Value(std::string& output, const char* delimiter)
		{
			output += (GetClassName<typename RawClass<R>::Type>(nullptr));
		}
	};
	
	template<typename R, typename ...ARGS>
	struct GetClassNameWrapper<TDelegate<R(ARGS...)>>
	{
		static constexpr bool IsValid = true;
		static constexpr bool DefineByMacro = false;
		using Type = TDelegate<R(ARGS...)>;
		static std::string GetName(const void* Obj)
		{
			std::string Params;
			if constexpr (std::tuple_size<std::tuple<ARGS...>>::value != 0)
			{
				MakeGenericTypeName<ARGS...>::Value(Params, ",");
			}
			
			std::string Return = GetClassName<typename RawClass<R>::Type>();
			if(Return == "void")
			{
				Return = "";
			}
			return "(" + Params +") -> (" + Return + ")";
		}
	};

#endif

	template<typename T, typename>
	struct HasGetClassName { static constexpr bool Value = false; };
	
	template<typename T>
	struct HasGetClassName<T, std::enable_if_t<GetClassNameWrapper<typename RawClass<T>::Type>::IsValid>> { static constexpr bool Value = true;};

	template<typename T, typename>
	struct IsDefineClassNameByMacro { static constexpr bool Value = false; };
	
	template<typename T>
	struct IsDefineClassNameByMacro<T, std::enable_if_t<GetClassNameWrapper<typename RawClass<T>::Type>::DefineByMacro>> { static constexpr bool Value = true;};

	template<typename T>
	static std::string GetClassName(const void* Obj)
	{
		return GetClassNameWrapper<typename RawClass<T>::Type>::GetName(Obj);
	}

#ifdef TOLUAUUNREAL_API
	static std::string GetUStructName(UStruct* Struct)
	{
		std::string ClassName;
		if(Struct->IsNative())
		{
			ClassName = StringEx::FStringToStdString(Struct->GetPrefixCPP() + Struct->GetName());
		}
		else
		{
			ClassName = StringEx::FStringToStdString(Struct->GetName());
		}
		return ClassName;
	}

	static std::string GetUClassName(UClass* Class)
	{
		std::string ClassName;
		if(Class->IsNative())
		{
			ClassName = StringEx::FStringToStdString(Class->GetPrefixCPP() + Class->GetName());
		}
		else
		{
			ClassName = StringEx::FStringToStdString(Class->GetName());
		}
		return ClassName;
	}
#endif
	
}

