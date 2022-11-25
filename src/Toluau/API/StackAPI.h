//
// Created by 35207 on 2022/6/26 0026.
//

#pragma once


#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cmath>

#include "lua.h"
#include "lualib.h"
#include "UserData.h"
#include "Toluau/Util/Template.h"
#include "Toluau/Util/Util.h"
#include "Toluau/Class/ClassName.h"

#include "Toluau/ToLuauDefine.h"

#ifdef TOLUAUUNREAL_API
#include "UObject/StrongObjectPtr.h"
#include "UObject/Object.h"
#endif

#if LUAU_SUPPORT_HOYO_CLASS
#include "FlyweightClass/Class.h"
#endif

#define UD_TAG(Tag) static_cast<int32_t>(UserDataType::Tag)

namespace ToLuau
{
	class Class;
	
	namespace StackAPI
	{
		struct AutoStack {
			AutoStack(lua_State* L) {
				this->L = L;
				this->top = lua_gettop(L);
			}
			~AutoStack() {
				lua_settop(this->L,this->top);
			}
			lua_State* L;
			int top;
		};

		struct StackChecker {
			StackChecker(lua_State* L, int32_t TargetOffset = 0)
			{
				this->L = L;
				this->TargetTop = lua_gettop(L) + TargetOffset;
				TOLUAU_ASSERT(TargetTop > 0)
			}
			
			StackChecker(lua_State* L, int32_t PopNum, int32_t PushNum)
				: StackChecker(L, -PopNum + PushNum)
			{
			}
			
			~StackChecker() {
				TOLUAU_ASSERT(lua_gettop(L) == this->TargetTop)
			}
			lua_State* L;
			int TargetTop;
		};

		enum class UserDataType
		{
			None = 0,
			Int64,
			UInt64,
			RawPtr,
			RawValue,
			StdSharedPtr,
			StdWeakPtr,

#ifdef TOLUAUUNREAL_API
			UEStruct,
			UESharedPtr,
			UEWeakPtr,
			UESharedThreadSafePtr,
			UEWeakThreadSafePtr,
			UEObjStrongPtr,
			UEObjWeakPtr,
			UObjectRawPtr,
#endif
			Max
		};

		ToLuau_API extern std::string UserDataTypeNames[];

#ifdef TOLUAUUNREAL_API
		template<typename T>
		using LuaUESharedPtr = TSharedPtr<T>;

		template<typename T>
		using LuaUEWeakPtr = TWeakPtr<T>;

		template<typename T>
		using LuaUESharedThreadSafePtr = TSharedPtr<T, ESPMode::ThreadSafe>;

		template<typename T>
		using LuaUEWeakThreadSafePtr = TWeakPtr<T, ESPMode::ThreadSafe>;

		template<typename T>
		using LuaUEObjStrongPtr = TStrongObjectPtr<T>;
		
		template<typename T>
		using LuaUEObjWeakPtr = TWeakObjectPtr<T>;

#ifdef TOLUAUUNREAL_API
		namespace UE
		{
			ToLuau_API int32_t PushUEStruct(lua_State* L, void* Value, UScriptStruct* StructType);

			ToLuau_API int32_t PushNewUEStruct(lua_State* L, UScriptStruct* StructType);
			
			ToLuau_API void* CheckUEStruct(lua_State* L, int32_t Pos, UScriptStruct* StructType = nullptr);

			template<typename T>
			T* CheckUEStruct(lua_State* L, int32_t Pos)
			{
				if constexpr (HasStaticStruct<T>::Value)
				{
					return static_cast<T*>(CheckUEStruct(L, Pos, T::StaticStruct()));
				}
				return nullptr;
			}
		}
#endif

#endif
		
#if LUAU_SUPPORT_HOYO_CLASS
		namespace HoYoClass
		{
			using PushFunc = TFunction<int32_t(lua_State*, void*)>;
			using CheckFunc = TFunction<void*(lua_State*, int32_t)>;
			
			extern ToLuau_API TMap<FName, PushFunc> HoYoClassRegisterPushFuncMap;
			extern ToLuau_API TMap<FName, CheckFunc> HoYoClassRegisterCheckFuncMap;

			template<typename T>
			struct HoYoClassStackAPIRegister
			{
				HoYoClassStackAPIRegister(PushFunc CustomPushFunc, CheckFunc CustomCheckFunc)
				{
					HoYoClassRegisterPushFuncMap.Add(T::StaticHoYoClass().GetName(), CustomPushFunc);
					HoYoClassRegisterCheckFuncMap.Add(T::StaticHoYoClass().GetName(), CustomCheckFunc);
				}
			};

#define LUAU_REG_CUSTOM_HOYO_CLASS_PUSHER_CHECKER(TypeName, Pusher, Checker) \
		namespace \
		{\
			static ToLuau::StackAPI::HoYoClass::PushFunc TypeName##PusherFunc = Pusher; \
			static ToLuau::StackAPI::HoYoClass::CheckFunc TypeName##CheckerFunc = Checker; \
			static ToLuau::StackAPI::HoYoClass::HoYoClassStackAPIRegister<TypeName> TypeName##Register(TypeName##PusherFunc, TypeName##CheckerFunc); \
		}
			
		}
#endif

		void InitStackAPI();

		ToLuau_API int32_t SetupMetaTableCommon(lua_State*L);
		
#ifdef TOLUAUUNREAL_API
		ToLuau_API int32_t GetClassMetaRef(lua_State* L, const UStruct* Struct);
		ToLuau_API int32_t GetClassMetaRef(lua_State* L, const UClass* Class);
#endif
		ToLuau_API int32_t GetClassMetaRef(lua_State* L, const ToLuau::Class* Class);
		ToLuau_API int32_t GetClassMetaRef(lua_State* L, const std::string& ClassName);
#if LUAU_SUPPORT_HOYO_CLASS
		ToLuau_API int32_t GetClassMetaRef(lua_State* L, const FHoYoClass* Class);
#endif

		template<typename T>
		int32_t GetClassMetaRefByClass(lua_State* L, const void* Obj)
		{
#if LUAU_SUPPORT_HOYO_CLASS
			if constexpr (HasStaticHoYoClass<T>::Value)
			{
				const T* RealObj = static_cast<const T*>(Obj);
				const FHoYoClass& CurrentClass = RealObj->GetHoYoClass();
				auto Result = GetClassMetaRef(L, &CurrentClass);
				if(Result > 0)
				{
					return Result;
				}
			}
#endif
#ifdef TOLUAUUNREAL_API
			if constexpr (HasStaticClass<T>::Value)
			{
				const T* RealObj = static_cast<const T*>(Obj);
				const UClass* CurrentClass = RealObj->GetClass();
				auto Result = GetClassMetaRef(L, CurrentClass);
				if(Result > 0)
				{
					return Result;
				}
			}
			if constexpr (HasStaticStruct<T>::Value)
			{
				const UStruct* CurrentClass = T::StaticStruct();
				auto Result = GetClassMetaRef(L, CurrentClass);
				if(Result > 0)
				{
					return Result;
				}
			}
#endif
			if constexpr (HasStaticLuaClass<T>::Value)
			{
				const T* RealObj = static_cast<const T*>(Obj);
				const Class* CurrentClass = RealObj->GetLuaClass();
				auto Result = GetClassMetaRef(L, CurrentClass);
				if(Result > 0)
				{
					return Result;
				}
			}

			{
				auto Ref = GetClassMetaRef(L, GetClassName<T>());
				if(Ref > 0)
				{
					return Ref;
				}
			}

			return -1;
		}

		ToLuau_API int32_t SetExternalClassMetaTableInternal(lua_State* L, std::string& ClassName, int32_t ClassRef, lua_CFunction SetupMetaTable, bool bDontRegisterMetatable);

		ToLuau_API int32_t SetClassMetaTable(lua_State* L, std::string& ClassName, lua_CFunction SetupMetaTable = nullptr, bool bDontRegisterMetatable = false);
		
		template<typename T>
		int32_t SetClassMetaTable(lua_State* L, const void* Obj, lua_CFunction SetupMetaTable = nullptr, bool bDontRegisterMetatable = false)
		{
			auto ClassRef = GetClassMetaRefByClass<T>(L, Obj);
			auto ClassName = GetClassName<T>(Obj);
			
			if(SetupMetaTable != nullptr)
			{
				return SetExternalClassMetaTableInternal(L, ClassName, ClassRef, SetupMetaTable, bDontRegisterMetatable);
			}
			
			if(ClassRef > 0)
			{
				lua_getref(L, ClassRef); // ud meta
				lua_setmetatable(L, -2); // ud
				return 1;
			}
			
			luaL_errorL(L, "cannot find class meta ref %s", ClassName.c_str());
		}

		ToLuau_API bool HasMetaTable(lua_State* L, const std::string& ClassName);
#ifdef TOLUAUUNREAL_API
		ToLuau_API bool HasMetaTable(lua_State* L, const UStruct* Struct);
		ToLuau_API bool HasMetaTable(lua_State* L, const UClass* Class);
#endif
#if LUAU_SUPPORT_HOYO_CLASS
		ToLuau_API bool HasMetaTable(lua_State* L, const FHoYoClass* Class);
#endif
		ToLuau_API bool HasMetaTable(lua_State* L, const ToLuau::Class* Class);

		template<typename T>
		bool HasMetaTable(lua_State* L, const void* Obj)
		{
#if LUAU_SUPPORT_HOYO_CLASS
			if constexpr (HasStaticHoYoClass<T>::Value)
			{
				const T* RealObj = static_cast<const T*>(Obj);
				const FHoYoClass& CurrentClass = RealObj->GetHoYoClass();
				return HasMetaTable(L, &CurrentClass);
			}
#endif
#ifdef TOLUAUUNREAL_API
			if constexpr (HasStaticClass<T>::Value)
			{
				const T* RealObj = static_cast<const T*>(Obj);
				const UClass* CurrentClass = RealObj->GetClass();
				return HasMetaTable(L, CurrentClass);
			}
			if constexpr (HasStaticStruct<T>::Value)
			{
				const UStruct* CurrentClass = T::StaticStruct();
				return HasMetaTable(L, CurrentClass);
			}
#endif
			if constexpr (HasStaticLuaClass<T>::Value)
			{
				const T* RealObj = static_cast<const T*>(Obj);
				const Class* CurrentClass = RealObj->GetLuaClass();
				return HasMetaTable(L, CurrentClass);
			}

			{
				auto ClassName = GetClassName<T>();
				return HasMetaTable(L, ClassName);
			}

		}

#ifdef TOLUAUUNREAL_API
		ToLuau_API bool RegClass(lua_State* L, UClass* Class);
		ToLuau_API bool RegStruct(lua_State* L, UScriptStruct* Struct);
#endif
		
		namespace Int64Detail
		{
			extern ToLuau_API int gInt64MT;
		}

		namespace UInt64Detail
		{
			extern ToLuau_API int gUInt64MT; 
		}
		
		ToLuau_API int32_t RegisterInt64(lua_State* L);
		ToLuau_API int32_t RegisterUInt64(lua_State* L);

		#pragma region push & check

		namespace __DETAIL__
		{
			
			template<typename T, typename = void>
			struct StackOperatorWrapper;

			template<typename T, typename = void> struct BuildInPushValue { static constexpr bool Value = false; };

			template<>
			struct StackOperatorWrapper<luau_int64, void>
			{
				static int32_t Push(lua_State* L, luau_int64& Value)
				{
					void* p = lua_newuserdatatagged(L, sizeof(luau_int64), UD_TAG(Int64));
			
					lua_getref(L, Int64Detail::gInt64MT);
					if(lua_isnil(L, -1))
					{
						lua_pop(L, 1);
						RegisterInt64(L);
						
						if(lua_isnil(L, -1))
						{
							luaL_errorL(L, "cannot register int64 meta table !!");
						}
					}
					
					lua_setmetatable(L, -2);
			
					*static_cast<luau_int64*>(p) = Value;
					return 1;
				}
			
				static luau_int64 Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(Int64)))
						return *static_cast<luau_int64*>(p);
			
					if (lua_isnumber(L, pos))
						return lua_tointeger(L, pos);

					Lua::DumpStack(L, "type error");
					luaL_typeerror(L, pos, "int64");
				}
			};
			template<> struct BuildInPushValue<luau_int64> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<luau_uint64, void>
			{
				static int32_t Push(lua_State* L, luau_uint64& Value)
				{
					void* p = lua_newuserdatatagged(L, sizeof(luau_uint64), UD_TAG(UInt64));
			
					lua_getref(L, UInt64Detail::gUInt64MT); // ud mt
					if(lua_isnil(L, -1)) // ud nil
					{
						lua_pop(L, 1); // ud 
						RegisterUInt64(L);  // ud mt
						
						if(lua_isnil(L, -1))
						{
							luaL_errorL(L, "cannot register int64 meta table !!");
						}
					}
					
					lua_setmetatable(L, -2); // ud
			
					*static_cast<luau_uint64*>(p) = Value;
					return 1;
				}
			
				static luau_uint64 Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UInt64)))
						return *static_cast<luau_uint64*>(p);
			
					if (lua_isnumber(L, pos))
						return lua_tointeger(L, pos);

					Lua::DumpStack(L, "type error");
					luaL_typeerror(L, pos, "uint64");
				}
			};
			template<> struct BuildInPushValue<luau_uint64> { static constexpr bool Value = true; };
			
			template<>
			struct StackOperatorWrapper<int32_t, void>
			{
				static int32_t Push(lua_State* L, int32_t& Value)
				{
					lua_pushinteger(L, static_cast<int32_t>(Value));
					return 1;
				}

				static int32_t Check(lua_State* L, int32_t pos)
				{
					return static_cast<int32_t>(luaL_checkinteger(L, pos));
				}
			};
			template<> struct BuildInPushValue<int32_t> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<int16_t , void>
			{
				static int32_t Push(lua_State* L, int16_t& Value)
				{
					lua_pushnumber(L, static_cast<int32_t>(Value));
					return 1;
				}

				static int16_t Check(lua_State* L, int32_t pos)
				{
					return static_cast<int16_t>(luaL_checkinteger(L, pos));
				}
			};
			template<> struct BuildInPushValue<int16_t> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<int8_t, void>
			{
				static int32_t Push(lua_State* L, int8_t& Value)
				{
					lua_pushnumber(L, static_cast<int32_t>(Value));
					return 1;
				}

				static int8_t Check(lua_State* L, int32_t pos)
				{
					return static_cast<int8_t>(luaL_checkinteger(L, pos));
				}
			};
			template<> struct BuildInPushValue<int8_t> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<uint32_t, void>
			{
				static int32_t Push(lua_State* L, uint32_t& Value)
				{
					lua_pushunsigned(L, static_cast<uint32_t>(Value));
					return 1;
				}

				static uint32_t Check(lua_State* L, int32_t Pos)
				{
					return static_cast<uint32_t>(luaL_checkunsigned(L, Pos));
				}
			};
			template<> struct BuildInPushValue<uint32_t> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<uint16_t, void>
			{
				static int32_t Push(lua_State* L, uint16_t& Value)
				{
					lua_pushunsigned(L, static_cast<uint32_t>(Value));
					return 1;
				}

				static uint16_t Check(lua_State* L, int32_t Pos)
				{
					return static_cast<uint16_t>(luaL_checkunsigned(L, Pos));
				}
			};
			template<> struct BuildInPushValue<uint16_t> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<uint8_t, void>
			{
				static int32_t Push(lua_State* L, uint8_t& Value)
				{
					lua_pushunsigned(L, static_cast<uint32_t>(Value));
					return 1;
				}

				static uint8_t Check(lua_State* L, int32_t Pos)
				{
					return static_cast<uint8_t>(luaL_checkunsigned(L, Pos));
				}
			};
			template<> struct BuildInPushValue<uint8_t> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<bool, void>
			{
				static int32_t Push(lua_State* L, bool& Value)
				{
					lua_pushboolean(L, static_cast<bool>(Value));
					return 1;
				}

				static bool Check(lua_State* L, int32_t Pos)
				{
					return static_cast<bool>(luaL_checkboolean(L, Pos));
				}
			};
			template<> struct BuildInPushValue<bool> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<float, void>
			{
				static int32_t Push(lua_State* L, float& Value)
				{
					lua_pushnumber(L, static_cast<float>(Value));
					return 1;
				}

				static float Check(lua_State* L, int32_t Pos)
				{
					return static_cast<float>(luaL_checknumber(L, Pos));
				}
			};
			template<> struct BuildInPushValue<float> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<double, void>
			{
				static int32_t Push(lua_State* L, double& Value)
				{
					lua_pushnumber(L, static_cast<double>(Value));
					return 1;
				}

				static double Check(lua_State* L, int32_t Pos)
				{
					return static_cast<double>(luaL_checknumber(L, Pos));
				}
			};
			template<> struct BuildInPushValue<double> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<std::nullptr_t, void>
			{
				static int32_t Push(lua_State* L, std::nullptr_t Value)
				{
					lua_pushnil(L);
					return 1;
				}

				static std::nullptr_t Check(lua_State* L, int32_t Pos)
				{
					return nullptr;
				}
			};
			template<> struct BuildInPushValue<std::nullptr_t> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<std::string, void>
			{
				static int32_t Push(lua_State* L, const std::string& Value)
				{
					lua_pushstring(L, Value.c_str());
					return 1;
				}

				static std::string Check(lua_State* L, int32_t Pos)
				{
					size_t len;
					const char* str = luaL_checklstring(L, Pos, &len);
					return std::string{str, len};
				}
			};
			template<> struct BuildInPushValue<std::string> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<const char*, void>
			{
				static int32_t Push(lua_State* L, const char* Value)
				{
					lua_pushstring(L, Value);
					return 1;
				}

				static std::string Check(lua_State* L, int32_t Pos)
				{
					const char* str = luaL_checkstring(L, Pos);
					return str;
				}
			};

			template<typename T>
			struct StackOperatorWrapper<T, typename std::enable_if<std::is_enum_v<T>>::type>
			{
				static int32_t Push(lua_State* L, T Value)
				{
					lua_pushinteger(L, static_cast<int32_t>(Value));
					return 1;
				}

				static T Check(lua_State* L, int32_t Pos)
				{
					return static_cast<T>(luaL_checkinteger(L, Pos));
				}
			};
			template<typename T> struct BuildInPushValue<T, typename std::enable_if<std::is_enum_v<T>>::type> { static constexpr bool Value = true; };

			template<typename T>
			struct StackOperatorWrapper<std::vector<T>>
			{
				static int32_t Push(lua_State* L, const std::vector<T>& Value)
				{
					lua_newtable(L);
					for (uint32_t i = 0; i < Value.size(); ++i)
					{
						lua_pushinteger(L, static_cast<int32_t>(i));
						StackOperatorWrapper<T>::Push(L, Value[i]);
						lua_settable(L, -3);
					}
					return 1;
				}

				static std::vector<T> Check(lua_State* L, int32_t Pos)
				{
					std::vector<T> Result;

					if(!lua_istable(L, Pos))
					{
						Lua::DumpStack(L, "type error");
						luaL_typeerrorL(L, Pos, "table");
						return Result;
					}

					lua_pushvalue(L, Pos); // table

					lua_pushnil(L); // table nil
					while(lua_next(L, -2) != 0) // table key value
					{
						lua_pushvalue(L, -2); // table key value key
						T Value = StackOperatorWrapper<T>::Check(L, -2);
						Result.insert(Value);
						lua_pop(L, 2); // table key
					}
					lua_pop(L, 2); // table
					return Result;
				}
			};
			template<typename T> struct BuildInPushValue<std::vector<T>> { static constexpr bool Value = true; };

			template<typename K, typename V>
			struct StackOperatorWrapper<std::map<K, V>>
			{
				static int32_t Push(lua_State* L, const std::map<K, V>& Value)
				{
					lua_newtable(L);
					for (const auto &Item: Value)
					{
						StackOperatorWrapper<K>::Push(L, Item.first);
						StackOperatorWrapper<V>::Push(L, Item.second);
						lua_settable(L, -3);
					}
					return 1;
				}

				static std::map<K, V> Check(lua_State* L, int32_t Pos)
				{
					std::map<K, V> Result;
					if(!lua_istable(L, Pos))
					{
						Lua::DumpStack(L, "type error");
						luaL_typeerror(L, Pos, "table");
						return Result;
					}

					lua_pushvalue(L, Pos);

					lua_pushnil(L); // table nil
					while(lua_next(L, -2) != 0) // table key value
					{
						lua_pushvalue(L, -2); // table key value key
						K Key = StackOperatorWrapper<K>::Check(L, -1);
						V Value = StackOperatorWrapper<V>::Check(L, -2);
						Result.insert(std::make_pair(Key, Value));
						lua_pop(L, 2); // table key
					}
					lua_pop(L, 1); // table
					return Result;
				}

			};
			template<typename K, typename V> struct BuildInPushValue<std::map<K, V>> { static constexpr bool Value = true; };

			

#ifdef TOLUAUUNREAL_API

			template<>
			struct StackOperatorWrapper<FString>
			{
				static int32_t Push(lua_State* L, const FString& Value)
				{
					return StackOperatorWrapper<std::string>::Push(L, StringEx::FStringToStdString(Value));
				}
				
				static FString Check(lua_State* L, int32_t Pos)
				{
					return StringEx::StdStringToFString(StackOperatorWrapper<std::string>::Check(L, Pos));
				}
			};
			template<> struct BuildInPushValue<FString> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<FName>
			{
				static int32_t Push(lua_State* L, const FName& Value)
				{
					return StackOperatorWrapper<std::string>::Push(L, StringEx::FStringToStdString(Value.ToString()));
				}
				
				static FName Check(lua_State* L, int32_t Pos)
				{
					auto Str = StackOperatorWrapper<std::string>::Check(L, Pos);
					return FName(Str.c_str());
				}
			};
			template<> struct BuildInPushValue<FName> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<FText>
			{
				static int32_t Push(lua_State* L, const FText& Value)
				{
					return StackOperatorWrapper<std::string>::Push(L, StringEx::FStringToStdString(Value.ToString()));
				}
				
				static FText Check(lua_State* L, int32_t Pos)
				{
					auto Str = StackOperatorWrapper<std::string>::Check(L, Pos);
					return FText::FromString(FString(Str.c_str()));
				}
			};
			template<> struct BuildInPushValue<FText> { static constexpr bool Value = true; };

#endif

			template<typename T>
			struct StackOperatorWrapper<T*,
				typename std::enable_if<
					!HasStaticLuaClass<T>::Value
					&& !HasGetClassName<T>::Value
					&& !IsDefineClassNameByMacro<T>::Value
#if LUAU_SUPPORT_HOYO_CLASS
					&& !HasStaticHoYoClass<T>::Value
#endif
#ifdef TOLUAUUNREAL_API
					&& !HasStaticStruct<T>::Value
					&& !TIsDerivedFrom<T, UObject>::Value
					&& !TIsDerivedFrom<T, FProperty>::Value
#endif
					>::type>
			{
				static int32_t Push(lua_State* L, T* Value)
				{
					if(!Value)
					{
						lua_pushnil(L);
						return 1;
					}
					lua_pushlightuserdata(L, (void*)Value);
					return 1;
				}

				static T* Check(lua_State* L, int32_t Pos)
				{
					if(!lua_islightuserdata(L, Pos))
					{
						Lua::DumpStack(L, "type error");
						luaL_typeerror(L, Pos, "lightuserdata");
						return nullptr;
					}
					return static_cast<T*>(lua_tolightuserdata(L, Pos));
				}
			};

			template<typename T>
			struct StackOperatorWrapper<T&, typename std::enable_if<
				(HasStaticLuaClass<T>::Value
				|| IsDefineClassNameByMacro<T>::Value
#ifdef TOLUAUUNREAL_API
				|| HasStaticStruct<T>::Value // Struct Pointer Use Normal RawPtr
#endif
				)
#if LUAU_SUPPORT_HOYO_CLASS
				&& !HasStaticHoYoClass<T>::Value
#endif
#ifdef TOLUAUUNREAL_API
				&& !TIsDerivedFrom<T, UObject>::Value
#endif
			>::type>
			{
				static int32_t Push(lua_State* L, T& Value)
				{
					return StackOperatorWrapper<T*>::Push(L, &Value);
				}

				static T& Check(lua_State* L, int32_t Pos)
				{
					return *StackOperatorWrapper<T*>::Check(L, Pos);
				}
			};

			template<typename T>
			int32_t PushRawPtr(lua_State* L, T* Value, lua_CFunction SetupMetatable = nullptr, bool bDontRegisterMetatable = false)
			{
				if(!Value)
				{
					lua_pushnil(L);
				}
				auto ClassName = ToLuau::GetClassName<T>(Value);

				UserData Temp;
				void* p = static_cast<UserData*>(lua_newuserdatatagged(L, sizeof(Temp), UD_TAG(RawPtr)));
				memcpy(p, &Temp, sizeof(Temp));
				auto NewUserData = static_cast<UserData*>(p);
				NewUserData->SetValue<T>(Value);
#if ToLuauDebug
				NewUserData->SetDebugName(ClassName.c_str());
#endif
				if(HasMetaTable<T>(L, Value) || SetupMetatable)
				{
					SetClassMetaTable<T>(L, Value, SetupMetatable, bDontRegisterMetatable);
				}
				return 1;
			}
			
			template<typename T>
			T* CheckRawPtr(lua_State* L, int32_t Pos)
			{
				if (void* RawPtr = lua_touserdatatagged(L, Pos, UD_TAG(RawPtr)))
				{
					return static_cast<UserData*>(RawPtr)->GetValue<T>();
				}
				else if (void* RawValue = lua_touserdatatagged(L, Pos, UD_TAG(RawValue)))
				{
					return static_cast<UserData*>(RawValue)->GetValue<T>();
				}
				else if (void* StdSharedPtr = lua_touserdatatagged(L, Pos, UD_TAG(StdSharedPtr)))
				{
					return static_cast<std::shared_ptr<T>*>(StdSharedPtr)->get();
				}
				else if (void* StdWeakPtr = lua_touserdatatagged(L, Pos, UD_TAG(StdWeakPtr)))
				{
					return (*static_cast<std::weak_ptr<T>*>(StdWeakPtr)).lock().get();
				}
#ifdef TOLUAUUNREAL_API
				else if (void* UESharedPtr = lua_touserdatatagged(L, Pos, UD_TAG(UESharedPtr)))
				{
					return static_cast<LuaUESharedPtr<T>*>(UESharedPtr)->Get();
				}
				else if (void* UEWeakPtr = lua_touserdatatagged(L, Pos, UD_TAG(UEWeakPtr)))
				{
					LuaUEWeakPtr<T> WeakPtr = *static_cast<LuaUEWeakPtr<T>*>(UEWeakPtr);
					if(!WeakPtr.IsValid())
					{
						return nullptr;
					}
					return WeakPtr.Pin().Get();
				}
				else if (void* UESharedThreadSafePtr = lua_touserdatatagged(L, Pos, UD_TAG(UESharedThreadSafePtr)))
				{
					return static_cast<LuaUESharedThreadSafePtr<T>*>(UESharedThreadSafePtr)->Get();
				}
				else if (void* UEWeakThreadSafePtr = lua_touserdatatagged(L, Pos, UD_TAG(UEWeakThreadSafePtr)))
				{
					LuaUEWeakThreadSafePtr<T> WeakPtr = *static_cast<LuaUEWeakThreadSafePtr<T>*>(UEWeakThreadSafePtr);
					if(!WeakPtr.IsValid())
					{
						return nullptr;
					}
					return WeakPtr.Pin().Get();
				}
				else if (void* UEObjStrongPtr = lua_touserdatatagged(L, Pos, UD_TAG(UEObjStrongPtr)))
				{
					return static_cast<TStrongObjectPtr<T>*>(UEObjStrongPtr)->Get();
				}
				else if (void* UEObjWeakPtr = lua_touserdatatagged(L, Pos, UD_TAG(UEObjWeakPtr)))
				{
					return static_cast<TWeakObjectPtr<T>*>(UEObjWeakPtr)->Get();
				}
				else if (lua_userdatatag(L, Pos) == UD_TAG(UEStruct))
				{
					return static_cast<T*>(StackAPI::UE::CheckUEStruct(L, Pos));
				}
#endif
				return nullptr;
			}
			
			template<typename T>
			struct StackOperatorWrapper<T*, typename std::enable_if<
				(HasStaticLuaClass<T>::Value
				|| IsDefineClassNameByMacro<T>::Value
#ifdef TOLUAUUNREAL_API
				|| HasStaticStruct<T>::Value // Struct Pointer Use Normal RawPtr
#endif
				)
#if LUAU_SUPPORT_HOYO_CLASS
				&& !HasStaticHoYoClass<T>::Value
#endif
#ifdef TOLUAUUNREAL_API
				&& !TIsDerivedFrom<T, UObject>::Value
#endif
			>::type>
			{
				static int32_t Push(lua_State* L, T* Value)
				{
					return PushRawPtr<T>(L, Value);
				}

				static T* Check(lua_State* L, int32_t Pos)
				{
					return CheckRawPtr<T>(L, Pos);
				}

			};

#ifdef TOLUAUUNREAL_API

			template<typename T>
			struct StackOperatorWrapper<T, typename std::enable_if<HasStaticStruct<T>::Value>::type>
			{
				static int32_t Push(lua_State* L, T& Value)
				{
					return UE::PushUEStruct(L, &Value, T::StaticStruct());
				}
				
				static T Check(lua_State* L, int32_t Pos)
				{
					if(void* Result = UE::CheckUEStruct(L, Pos))
					{
						return *static_cast<T*>(Result);
					}
					Lua::DumpStack(L, "type error");
					luaL_typeerror(L, Pos, GetClassName<T>().c_str());
				}
			};
			template<typename T> struct BuildInPushValue<T, typename std::enable_if<HasStaticStruct<T>::Value>::type> { static constexpr bool Value = true; };

			template<typename T>
			struct StackOperatorWrapper<T*, typename std::enable_if<TIsDerivedFrom<T, UObject>::Value || HasStaticClass<T>::Value>::type>
			{
				static int32_t Push(lua_State* L, T* Value, lua_CFunction SetupMetatable = nullptr, bool bDontRegisterMetatable = false)
				{
					if(!Value || !IsValid(Value) ||!GUObjectArray.IsValid(Value) || Value->IsPendingKill())
					{
						lua_pushnil(L);
						return 1;
					}

					if(false)
					{
						void* p = lua_newuserdatatagged(L, sizeof(UObject*), UD_TAG(UObjectRawPtr));
						auto ClassName = GetClassName<T>(Value);
						if(!SetupMetatable && !HasMetaTable<T>(L, Value))
						{
							RegClass(L, Value->GetClass());
						}
						if(SetupMetatable || HasMetaTable<T>(L, Value))
						{
							SetClassMetaTable(L, ClassName, SetupMetatable);
						}
						UObject*& Ptr = *static_cast<UObject**>(p);
						Ptr = Value;
						return 1;
					}
					
					return StackOperatorWrapper<TStrongObjectPtr<T>>::Push(L, TStrongObjectPtr<T>(Value), SetupMetatable, bDontRegisterMetatable);
				}
				
				static T* Check(lua_State* L, int32_t Pos)
				{
					if(lua_isnil(L, Pos))
					{
						return nullptr;
					}
					if(false)
					{
						if (void* p = lua_touserdatatagged(L, Pos, UD_TAG(UObjectRawPtr)))
						{
							UObject*& Ptr = *static_cast<UObject**>(p);
							return Cast<T>(Ptr);
						}
					}
					return static_cast<T*>(StackOperatorWrapper<TStrongObjectPtr<T>>::Check(L, Pos).Get());
				}
			};
#endif
			
			template<typename T>
			struct StackOperatorWrapper<std::shared_ptr<T>>
			{
				static int32_t Push(lua_State* L, std::shared_ptr<T> Value, lua_CFunction SetupMetatable = nullptr, bool bDontRegisterMetatable = false)
				{
					if(Value == nullptr)
					{
						lua_pushnil(L);
						return 1;
					}
					
					std::shared_ptr<void> Temp;
					void* p = lua_newuserdatatagged(L, sizeof(Temp), UD_TAG(StdSharedPtr));
					memcpy(p, &Temp, sizeof(Temp));

					auto RawObj = Value.get();
					auto ClassName = GetClassName<T>(RawObj);

#ifdef TOLUAUUNREAL_API
					if(!SetupMetatable)
					{
						if constexpr (HasStaticStruct<T>::Value && HasStaticHoYoClass<T>::Value)
						{
							if(!HasMetaTable<T>(L, RawObj))
							{
								RegStruct(L, const_cast<UScriptStruct*>(Cast<const UScriptStruct>(Value->GetHoYoClass().GetUStruct())));
							}
						}
					}
#endif
					// set metatable
					if(SetupMetatable || HasMetaTable<T>(L, RawObj))
					{
						SetClassMetaTable<T>(L, RawObj, SetupMetatable, bDontRegisterMetatable);
					}

					auto& Ptr = *static_cast<std::shared_ptr<T>*>(p);
					Ptr.reset();
					Ptr = Value;
					return 1;
				}

				static std::shared_ptr<T> Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(StdSharedPtr)))
					{
						return std::static_pointer_cast<T>(*static_cast<std::shared_ptr<void>*>(p));
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(StdWeakPtr)))
					{
						auto Weak = *static_cast<std::weak_ptr<void>*>(p);
						if(Weak.expired())
						{
							return nullptr;
						}
						return std::static_pointer_cast<T>(Weak.lock());
					}
					Lua::DumpStack(L, "type error");
					luaL_typeerror(L, pos, GetClassName<T>().c_str());
				}
			};
			template<typename T> struct BuildInPushValue<std::shared_ptr<T>> { static constexpr bool Value = true; };

			
			template<typename T>
			struct StackOperatorWrapper<std::weak_ptr<T>>
			{
				static int32_t Push(lua_State* L, std::weak_ptr<T> Value, lua_CFunction SetupMetatable = nullptr, bool bDontRegisterMetatable = false)
				{
					if(Value.expired())
					{
						lua_pushnil(L);
						return 1;
					}
					std::weak_ptr<T> Temp;
					void* p = lua_newuserdatatagged(L, sizeof(Temp), UD_TAG(StdWeakPtr));
					memcpy(p, &Temp, sizeof(Temp));

					auto RawObj = Value.lock().get();
					auto ClassName = GetClassName<T>(Value.lock().get());
					
#ifdef TOLUAUUNREAL_API
					if constexpr (HasStaticStruct<T>::Value && HasStaticHoYoClass<T>::Value)
					{
						if(!HasMetaTable<T>(L, RawObj))
						{
							RegStruct(L, const_cast<UScriptStruct*>(Cast<const UScriptStruct>(Value->GetHoYoClass().GetUStruct())));
						}
					}
#endif
					// set metatable
					if(HasMetaTable<T>(L, RawObj) || SetupMetatable)
					{
						SetClassMetaTable<T>(L, RawObj, SetupMetatable, bDontRegisterMetatable);
					}

					auto& Ptr = *static_cast<std::weak_ptr<T>*>(p);
					Ptr.reset();
					Ptr = Value;
					return 1;
				}

				static std::weak_ptr<T> Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(StdWeakPtr)))
					{
						return std::static_pointer_cast<T>(static_cast<std::weak_ptr<void>*>(p)->lock());
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(StdSharedPtr)))
					{
						auto Shared = *static_cast<std::shared_ptr<void>*>(p);
						return std::static_pointer_cast<T>(Shared);
					}
					Lua::DumpStack(L, "type error");
					luaL_typeerror(L, pos, GetClassName<T>().c_str());
				}
			};
			template<typename T> struct BuildInPushValue<std::weak_ptr<T>> { static constexpr bool Value = true; };
			
#ifdef TOLUAUUNREAL_API
#if LUAU_SUPPORT_HOYO_CLASS
			
			template<typename T>
			struct StackOperatorWrapper<T*, typename std::enable_if<
				HasStaticHoYoClass<T>::Value
#ifdef TOLUAUUNREAL_API
				&& !TIsDerivedFrom<T, UObject>::Value
#endif
			>::type>
			{
				static int32_t Push(lua_State* L, T* Value)
				{
					if(!Value)
					{
						lua_pushnil(L);
					}
					
					const FHoYoClass& HoYoClass = Value->GetHoYoClass();
					auto* CurrentClass = &HoYoClass;
					do
					{
						auto FuncPtr = HoYoClass::HoYoClassRegisterPushFuncMap.Find(CurrentClass->GetName());
						if(FuncPtr)
						{
							return (*FuncPtr)(L, Value);
						}
						CurrentClass = CurrentClass->GetBaseClass();
					}while(CurrentClass);

					if(HoYoClass.GetUClass())
					{
						LUAU_ERROR_F("please register custom hoyo class (%s) pusher and checker with LUAU_REG_CUSTOM_HOYO_CLASS_PUSHER_CHECKER, it may cause crash",
								StringEx::FStringToStdString(HoYoClass.GetName().ToString()).c_str());
					}
					
					return PushRawPtr<T>(L, Value);
				}

				static T* Check(lua_State* L, int32_t Pos)
				{
					if(lua_isnil(L, Pos))
					{
						return nullptr;
					}
					
					const FHoYoClass& HoYoClass = T::StaticHoYoClass();
					auto* CurrentClass = &HoYoClass;
					do
					{
						auto FuncPtr = HoYoClass::HoYoClassRegisterCheckFuncMap.Find(CurrentClass->GetName());
						if(FuncPtr)
						{
							return static_cast<T*>((*FuncPtr)(L, Pos));
						}
						CurrentClass = CurrentClass->GetBaseClass();
					} while (CurrentClass);
					
					if(HoYoClass.GetUClass())
					{
						LUAU_ERROR_F("please register custom hoyo class (%s) pusher and checker with LUAU_REG_CUSTOM_HOYO_CLASS_PUSHER_CHECKER, it may cause crash",
								StringEx::FStringToStdString(HoYoClass.GetName().ToString()).c_str());
					}
					
					return CheckRawPtr<T>(L, Pos);
				}
			};
#endif
#endif

#ifdef TOLUAUUNREAL_API

#pragma region ue smart ptr

			template<typename T, ESPMode SPMode>
			struct StackOperatorWrapper<TSharedPtr<const T, SPMode>>
			{
				static int32_t Push(lua_State* L, TSharedPtr<const T, SPMode> Value, lua_CFunction SetupMetatable = nullptr, bool bDontRegisterMetatable = false)
				{
					return StackOperatorWrapper<TSharedPtr<T, SPMode>>::Push(L, ConstCastSharedPtr<T>(Value), SetupMetatable, bDontRegisterMetatable);
				}

				static TSharedPtr<const T, SPMode> Check(lua_State* L, int32_t pos)
				{
					TSharedPtr<T, SPMode> Result = StackOperatorWrapper<TSharedPtr<T, SPMode>>::Check(L, pos);
					return ConstCastSharedPtr<const T>(Result);
				}
			};
			
			template<typename T, ESPMode SPMode>
			struct StackOperatorWrapper<TSharedPtr<T, SPMode>, typename std::enable_if<!TIsConst<T>::Value>::type>
			{
				static int32_t Push(lua_State* L, TSharedPtr<T, SPMode> Value, lua_CFunction SetupMetatable = nullptr, bool bDontRegisterMetatable = false)
				{
					if(!Value.IsValid())
					{
						lua_pushnil(L);
						return 1;
					}

					auto Tag = SPMode == ESPMode::ThreadSafe ? UserDataType::UESharedThreadSafePtr : UserDataType::UESharedPtr;
					
					TSharedPtr<void, SPMode> Temp;
					void* p = lua_newuserdatatagged(L, sizeof(Temp), static_cast<int32_t>(Tag));
					memcpy(p, &Temp, sizeof(TSharedPtr<void, SPMode>));

					auto RawObj = Value.Get();
					auto ClassName = GetClassName<T>(RawObj);
#ifdef TOLUAUUNREAL_API
					if constexpr (HasStaticStruct<T>::Value && HasStaticHoYoClass<T>::Value)
					{
						if(!HasMetaTable<T>(L, RawObj))
						{
							RegStruct(L, const_cast<UScriptStruct*>(Cast<const UScriptStruct>(Value->GetHoYoClass().GetUStruct())));
						}
					}
#endif
					// set metatable
					if(SetupMetatable || HasMetaTable<T>(L, RawObj))
					{
						SetClassMetaTable<T>(L, RawObj, SetupMetatable, bDontRegisterMetatable);
					}

					*static_cast<TSharedPtr<void, SPMode>*>(p) = Value;
					return 1;
				}

				static TSharedPtr<T, SPMode> Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UESharedPtr)))
					{
						TOLUAU_ASSERT(SPMode == ESPMode::NotThreadSafe)
						return StaticCastSharedPtr<T>(*static_cast<TSharedPtr<void, SPMode>*>(p));
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UESharedThreadSafePtr)))
					{
						TOLUAU_ASSERT(SPMode == ESPMode::ThreadSafe)
						return StaticCastSharedPtr<T>(*static_cast<TSharedPtr<void, SPMode>*>(p));
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEWeakPtr)))
					{
						TOLUAU_ASSERT(SPMode == ESPMode::NotThreadSafe)
						auto Weak = *static_cast<TWeakPtr<void, SPMode>*>(p);
						if(!Weak.IsValid())
						{
							return nullptr;
						}
						return StaticCastSharedPtr<T>(Weak.Pin());
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEWeakThreadSafePtr)))
					{
						TOLUAU_ASSERT(SPMode == ESPMode::ThreadSafe)
						auto Weak = *static_cast<TWeakPtr<void, SPMode>*>(p);
						if(!Weak.IsValid())
						{
							return nullptr;
						}
						return StaticCastSharedPtr<T>(Weak.Pin());
					}
					Lua::DumpStack(L, "type error");
					luaL_typeerror(L, pos, GetClassName<T>().c_str());
				}
			};
			template<typename T, ESPMode SPMode> struct BuildInPushValue<TSharedPtr<T, SPMode>> { static constexpr bool Value = true; };

			template<typename T, ESPMode SPMode>
			struct StackOperatorWrapper<TWeakPtr<const T, SPMode>>
			{
				static int32_t Push(lua_State* L, TWeakPtr<const T, SPMode> Value, lua_CFunction SetupMetatable = nullptr, bool bDontRegisterMetatable = false)
				{
					return StackOperatorWrapper<TWeakPtr<T, SPMode>>::Push(L, ConstCastSharedPtr<T>(Value.Pin()), SetupMetatable, bDontRegisterMetatable);
				}

				static TWeakPtr<const T, SPMode> Check(lua_State* L, int32_t pos)
				{
					TWeakPtr<T, SPMode> Result = StackOperatorWrapper<TWeakPtr<T, SPMode>>::Check(L, pos);
					return ConstCastSharedPtr<const T>(Result.Pin());
				}
			};
			
			template<typename T, ESPMode SPMode>
			struct StackOperatorWrapper<TWeakPtr<T, SPMode>, typename std::enable_if<!TIsConst<T>::Value>::type>
			{
				static int32_t Push(lua_State* L, TWeakPtr<T, SPMode> Value, lua_CFunction SetupMetatable = nullptr, bool bDontRegisterMetatable = false)
				{
					if(!Value.IsValid())
					{
						lua_pushnil(L);
						return 1;
					}
					
					auto Tag = SPMode == ESPMode::ThreadSafe ? UserDataType::UEWeakThreadSafePtr : UserDataType::UEWeakPtr;
					
					void* p = lua_newuserdatatagged(L, sizeof(TWeakPtr<void, SPMode>), static_cast<int32_t>(Tag));

					auto RawObj = Value.Pin().Get();
					auto ClassName = GetClassName<T>(RawObj);
#ifdef TOLUAUUNREAL_API
					if constexpr (HasStaticStruct<T>::Value && HasStaticHoYoClass<T>::Value)
					{
						if(!HasMetaTable<T>(L, RawObj))
						{
							RegStruct(L, const_cast<UScriptStruct*>(Cast<const UScriptStruct>(Value->GetHoYoClass().GetUStruct())));
						}
					}
#endif
					// set metatable
					if(SetupMetatable || HasMetaTable<T>(L, RawObj))
					{
						SetClassMetaTable<T>(L, RawObj, SetupMetatable, bDontRegisterMetatable);
					}
					
					*static_cast<TWeakPtr<void, SPMode>*>(p) = Value;
					return 1;
				}

				static TWeakPtr<T, SPMode> Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEWeakPtr)))
					{
						TOLUAU_ASSERT(SPMode == ESPMode::NotThreadSafe)
						return *static_cast<TWeakPtr<void, SPMode>*>(p);
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEWeakThreadSafePtr)))
					{
						TOLUAU_ASSERT(SPMode == ESPMode::ThreadSafe)
						return *static_cast<TWeakPtr<void, SPMode>*>(p);
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UESharedPtr)))
					{
						TOLUAU_ASSERT(SPMode == ESPMode::NotThreadSafe)
						auto Shared = *static_cast<TSharedPtr<void, SPMode>*>(p);
						if(!Shared.IsValid())
						{
							return nullptr;
						}
						return Shared;
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UESharedThreadSafePtr)))
					{
						TOLUAU_ASSERT(SPMode == ESPMode::ThreadSafe)
						auto Shared = *static_cast<TSharedPtr<void, SPMode>*>(p);
						if(!Shared.IsValid())
						{
							return nullptr;
						}
						return Shared;
					}
					Lua::DumpStack(L, "type error");
					luaL_typeerror(L, pos, GetClassName<T>().c_str());
				}
			};
			template<typename T, ESPMode SPMode> struct BuildInPushValue<TWeakPtr<T, SPMode>> { static constexpr bool Value = true; };

			template<typename T>
			struct StackOperatorWrapper<LuaUEObjStrongPtr<T>>
			{
				static int32_t Push(lua_State* L, LuaUEObjStrongPtr<T> Value, lua_CFunction SetupMetatable = nullptr, bool bDontRegisterMetatable = false)
				{
					if(!Value.IsValid())
					{
						lua_pushnil(L);
						return 1;
					}
					
					LuaUEObjStrongPtr<UObject> Temp;
					void* p = lua_newuserdatatagged(L, sizeof(Temp), UD_TAG(UEObjStrongPtr));
					memcpy(p, &Temp, sizeof(Temp));

					// set metatable
					auto RawObj = Value.Get();
					auto ClassName = GetClassName<T>(RawObj);
					if(!HasMetaTable<T>(L, RawObj))
					{
						RegClass(L, Value->GetClass());
					}
					if(SetupMetatable || HasMetaTable<T>(L, RawObj))
					{
						SetClassMetaTable<T>(L, RawObj, SetupMetatable, bDontRegisterMetatable);
					}

					LuaUEObjStrongPtr<UObject>& StrongObjectPtr = *static_cast<LuaUEObjStrongPtr<UObject>*>(p);
					StrongObjectPtr.Reset( (UObject*)(Value.Get()));
					return 1;
				}

				static LuaUEObjStrongPtr<T> Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEObjStrongPtr)))
					{
						LuaUEObjStrongPtr<UObject>& Ptr = *static_cast<LuaUEObjStrongPtr<UObject>*>(p);
						return LuaUEObjStrongPtr<T>(Cast<T>(Ptr.Get()));
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEObjWeakPtr)))
					{
						auto& Weak = *static_cast<LuaUEObjWeakPtr<UObject>*>(p);
						if(!Weak.IsValid())
						{
							return nullptr;
						}
						return LuaUEObjStrongPtr<T>(Cast<T>(Weak.Get()));
					}
					Lua::DumpStack(L, "type error");
					luaL_typeerror(L, pos, GetClassName<T>().c_str());
				}
			};
			template<typename T> struct BuildInPushValue<LuaUEObjStrongPtr<T>> { static constexpr bool Value = true; };
			
			template<>
			struct StackOperatorWrapper<FWeakObjectPtr>
			{
				static int32_t Push(lua_State* L, FWeakObjectPtr Value, lua_CFunction SetupMetatable = nullptr, bool bDontRegisterMetatable = false)
				{
					if(!Value.IsValid())
					{
						lua_pushnil(L);
						return 1;
					}
					FWeakObjectPtr Temp;
					void* p = lua_newuserdatatagged(L, sizeof(Temp), UD_TAG(UEObjWeakPtr));
					memcpy(p, &Temp, sizeof(Temp));

					// set metatable
					auto Obj = Value.Get();
					std::string ClassName = GetClassName<UObject>(Obj);
					if(!HasMetaTable<UObject>(L, Obj))
					{
						RegClass(L, Value.Get()->GetClass());
					}
					if(SetupMetatable || HasMetaTable<UObject>(L, Obj))
					{
						SetClassMetaTable<UObject>(L, Obj, SetupMetatable, bDontRegisterMetatable);
					}

					*static_cast<FWeakObjectPtr*>(p) = Value;
					return 1;
				}

				static FWeakObjectPtr Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEObjWeakPtr)))
					{
						return *static_cast<FWeakObjectPtr*>(p);
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEObjStrongPtr)))
					{
						auto& Strong = *static_cast<LuaUEObjStrongPtr<UObject>*>(p);
						if(!Strong.IsValid())
						{
							return nullptr;
						}
						return FWeakObjectPtr(Strong.Get());
					}
					Lua::DumpStack(L, "type error");
					luaL_typeerror(L, pos, "LuaUEObjWeakPtr");
				}
			};
			template<> struct BuildInPushValue<FWeakObjectPtr> { static constexpr bool Value = true; };

			template<>
			struct StackOperatorWrapper<FSoftObjectPtr>
			{
				static int32_t Push(lua_State* L, FSoftObjectPtr Value)
				{
					if(!Value.IsValid())
					{
						lua_pushnil(L);
						return 1;
					}

					lua_pushstring(L, StringEx::FStringToStdString(Value.ToSoftObjectPath().ToString()).c_str());
					
					return 1;
				}

				static FSoftObjectPtr Check(lua_State* L, int32_t Pos)
				{
					if(!lua_isstring(L, Pos))
					{
						Lua::DumpStack(L, "type error");
						luaL_typeerrorL(L, Pos, "string");
					}
					auto Path = StackOperatorWrapper<FString>::Check(L, Pos);
					return FSoftObjectPtr(FSoftObjectPath(Path));
				}
			};
			template<> struct BuildInPushValue<FSoftObjectPtr> { static constexpr bool Value = true; };
			
			template<typename T>
			struct StackOperatorWrapper<LuaUEObjWeakPtr<T>>
			{
				static int32_t Push(lua_State* L, LuaUEObjWeakPtr<T> Value, lua_CFunction SetupMetatable = nullptr, bool bDontRegisterMetatable = false)
				{
					if(!Value.IsValid())
					{
						lua_pushnil(L);
						return 1;
					}
					
					LuaUEObjWeakPtr<UObject> Temp;
					void* p = lua_newuserdatatagged(L, sizeof(Temp), UD_TAG(UEObjWeakPtr));
					memcpy(p, &Temp, sizeof(Temp));

					auto RawObj = Value.Get();
					auto ClassName = GetClassName<T>(RawObj);
					// set metatable
					if(!HasMetaTable<T>(L, RawObj))
					{
						RegClass(L, Value->GetClass());
					}
					if(SetupMetatable || HasMetaTable<T>(L, RawObj))
					{
						SetClassMetaTable<T>(L, RawObj, SetupMetatable, bDontRegisterMetatable);
					}

					LuaUEObjWeakPtr<UObject>& WeakPtr = *static_cast<LuaUEObjWeakPtr<UObject>*>(p);
					WeakPtr.Reset();
					WeakPtr = (UObject*)(Value.Get());
					return 1;
				}

				static LuaUEObjWeakPtr<T> Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEObjWeakPtr)))
					{
						return *static_cast<LuaUEObjWeakPtr<UObject>*>(p);
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEObjStrongPtr)))
					{
						auto& Strong = *static_cast<LuaUEObjStrongPtr<UObject>*>(p);
						if(!Strong.IsValid())
						{
							return nullptr;
						}
						return LuaUEObjWeakPtr<UObject>(Strong.Get());
					}
					Lua::DumpStack(L, "type error");
					luaL_typeerror(L, pos, GetClassName<T>().c_str());
				}
			};
			template<typename T> struct BuildInPushValue<LuaUEObjWeakPtr<T>> { static constexpr bool Value = true; };
#pragma endregion

#endif

			template<typename T>
			int32_t PushRawValue(lua_State* L, T Value, lua_CFunction SetupMetatable, bool bDontRegisterMetatable = false)
			{
				auto ClassName = ToLuau::GetClassName<T>(nullptr);

				UserData Temp;
				void* p = static_cast<UserData*>(lua_newuserdatatagged(L, sizeof(Temp), UD_TAG(RawValue)));
				memcpy(p, &Temp, sizeof(Temp));
				auto NewUserData = static_cast<UserData*>(p);
				NewUserData->SetValue<T>(Value);
#if ToLuauDebug
				NewUserData->SetDebugName(ClassName.c_str());
#endif
#ifdef TOLUAUUNREAL_API
				if constexpr (HasStaticStruct<T>::Value)
				{
					if(!HasMetaTable<T>(L,  &Value))
					{
						RegStruct(L, T::StaticStruct());
					}
				}
#endif
				if(SetupMetatable || HasMetaTable<T>(L, &Value))
				{
					SetClassMetaTable<T>(L, &Value, SetupMetatable, bDontRegisterMetatable);
				}
				return 1;
			}

			template<typename T>
			T CheckRawValue(lua_State* L, int32_t Pos)
			{
				if (void* RawPtr = lua_touserdatatagged(L, Pos, UD_TAG(RawPtr)))
				{
					return *static_cast<UserData*>(RawPtr)->GetValue<T>();
				}
				if (void* RawValue = lua_touserdatatagged(L, Pos, UD_TAG(RawValue)))
				{
					return *static_cast<UserData*>(RawValue)->GetValue<T>();
				}
				else if (void* StdSharedPtr = lua_touserdatatagged(L, Pos, UD_TAG(StdSharedPtr)))
				{
					return *static_cast<std::shared_ptr<T>*>(StdSharedPtr)->get();
				}
				else if (void* StdWeakPtr = lua_touserdatatagged(L, Pos, UD_TAG(StdWeakPtr)))
				{
					return *(*static_cast<std::weak_ptr<T>*>(StdWeakPtr)).lock().get();
				}
#ifdef TOLUAUUNREAL_API
				else if (void* UESharedPtr = lua_touserdatatagged(L, Pos, UD_TAG(UESharedPtr)))
				{
					return *static_cast<LuaUESharedPtr<T>*>(UESharedPtr)->Get();
				}
				else if (void* UEWeakPtr = lua_touserdatatagged(L, Pos, UD_TAG(UEWeakPtr)))
				{
					LuaUEWeakPtr<T> WeakPtr = *static_cast<LuaUEWeakPtr<T>*>(UEWeakPtr);
					if(!WeakPtr.IsValid())
					{
						if constexpr (std::is_constructible_v<T>)
						{
							return {};
						}
						TOLUAU_ASSERT(false);
					}
					return *WeakPtr.Pin().Get();
				}
				else if (void* UESharedThreadSafePtr = lua_touserdatatagged(L, Pos, UD_TAG(UESharedThreadSafePtr)))
				{
					return *static_cast<LuaUESharedThreadSafePtr<T>*>(UESharedThreadSafePtr)->Get();
				}
				else if (void* UEWeakThreadSafePtr = lua_touserdatatagged(L, Pos, UD_TAG(UEWeakThreadSafePtr)))
				{
					LuaUEWeakThreadSafePtr<T> WeakPtr = *static_cast<LuaUEWeakThreadSafePtr<T>*>(UEWeakThreadSafePtr);
					if(!WeakPtr.IsValid())
					{
						if constexpr (std::is_constructible_v<T>)
						{
							return {};
						}
						TOLUAU_ASSERT(false);
					}
					return *WeakPtr.Pin().Get();
				}
#endif
				return {};
			}
			
			template<typename T>
			struct StackOperatorWrapper<T, typename std::enable_if<
					(HasStaticLuaClass<T>::Value
					|| HasGetClassName<T>::Value
					|| IsDefineClassNameByMacro<T>::Value
#if LUAU_SUPPORT_HOYO_CLASS
					|| HasStaticHoYoClass<T>::Value
#endif
					)
#ifdef TOLUAUUNREAL_API
					&& !HasStaticStruct<T>::Value
#endif
					&& std::is_same<typename RawClass<T>::Type, T>::value
					&& !BuildInPushValue<T>::Value
				>::type>
			{
				static int32_t Push(lua_State* L, T Value, lua_CFunction SetupMetatable = nullptr, bool bDontRegisterMetatable = false)
				{
					return PushRawValue<T>(L, Value, SetupMetatable, bDontRegisterMetatable);
				}

				static T Check(lua_State* L, int32_t Pos)
				{
					return CheckRawValue<T>(L, Pos);
				}
			};
			
		}

		template<typename T>
		int32_t Push(lua_State* L, const T& Value)
		{
			return __DETAIL__::StackOperatorWrapper<T>::Push(L, const_cast<T&>(Value));
		}
		
		template<typename T>
		int32_t Push(lua_State* L, const T& Value, lua_CFunction SetupMetatable)
		{
			return __DETAIL__::StackOperatorWrapper<T>::Push(L, const_cast<T&>(Value), SetupMetatable);
		}

		template<typename T>
		int32_t Push(lua_State* L, const T* Value)
		{
			return __DETAIL__::StackOperatorWrapper<T*>::Push(L, const_cast<T*>(Value));
		}
		
		template<typename T>
		int32_t Push(lua_State* L, const T* Value, lua_CFunction SetupMetatable)
		{
			return __DETAIL__::StackOperatorWrapper<T>::Push(L, const_cast<T*>(Value), SetupMetatable);
		}
		
		template<typename T>
		int32_t PushRawPtr(lua_State* L, const T* Value)
		{
			return __DETAIL__::PushRawPtr<T>(L, const_cast<T*>(Value));
		}
		
		template<typename T>
		T* CheckRawPtr(lua_State* L, int32_t Pos)
		{
			return __DETAIL__::CheckRawPtr<T>(L, Pos);
		}

		template<typename T>
		int32_t PushValue(lua_State* L, T Value)
		{
			return __DETAIL__::StackOperatorWrapper<T>::Push(L, Value);
		}

		template<class TFirst,class ...TArgs>
		int PushArgs(lua_State* L, TFirst f, TArgs&& ...args);

		inline int32_t PushArgs(lua_State* L);
		
		template<class TFirst,class ...TArgs>
		int PushArgs(lua_State* L, TFirst f, TArgs&& ...args) {
			Push<TFirst>(L,f);
			return 1+PushArgs(L, std::forward<TArgs>(args)...);
		}

		template<class TFirst,class ...TArgs>
		int PushArgsWithRef(lua_State* L, TFirst f, const TArgs& ...args);
		
		inline int32_t PushArgsWithRef(lua_State* L);
		
		template<class TFirst,class ...TArgs>
		int PushArgsWithRef(lua_State* L, TFirst f, const TArgs& ...args) {
			Push<TFirst>(L,f);
			return 1+PushArgsWithRef(L, args...);
		}

		inline int32_t PushArgs(lua_State* L)
		{
			return 0;
		}
		
		inline int32_t PushArgsWithRef(lua_State* L)
		{
			return 0;
		}

		template<typename T>
		T Check(lua_State* L, int32_t Pos)
		{
			return __DETAIL__::StackOperatorWrapper<T>::Check(L, Pos);
		}
		
		template<typename T>
		T CheckOptional(lua_State* L, int32_t Pos, T DefaultValue = T())
		{
			auto Top = lua_gettop(L);
			if((Pos > 0 && Pos > Top) || (Pos < 0 && Pos + Top < 0))
			{
				return DefaultValue;
			}
			return __DETAIL__::StackOperatorWrapper<T>::Check(L, Pos);
		}

		template<typename T>
		int32_t PushTableItem(lua_State*  L, int32_t Index, T Value)
		{
			lua_pushinteger(L, Index);
			Push(L, Value);
			lua_settable(L, -3);
			return 0;
		}

		ToLuau_API int32_t FindTable(lua_State* L, const std::string& FullName, bool bCreateIfNil);

#pragma region



#ifdef TOLUAUUNREAL_API

		namespace UE
		{
			void InitProperty();
		
			int32_t PushProperty(lua_State* L, FProperty* Up, uint8* Params);

			int32_t PushProperty(lua_State* L, FProperty* Up, UObject* Obj);
			
			int32_t CheckProperty(lua_State* L, FProperty* Prop, uint8* Params, int32_t Pos);
			
			int32_t CheckProperty(lua_State* L, FProperty* Prop, UObject* Obj, int32_t Pos);

			int32_t PushUFunction(lua_State* L, UFunction* Function, UClass* Class = nullptr);

		}

#endif

#pragma endregion

		#pragma endregion

	};
}

#undef UD_TAG
