//
// Created by 35207 on 2022/6/26 0026.
//

#pragma once


#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cassert>

#include "lua.h"
#include "lualib.h"
#include "UserData.h"
#include "Toluau/Util/Template.h"
#include "Toluau/Util/Util.h"

#include "Toluau/ToLuauDefine.h"


#ifdef TOLUAUUNREAL_API
#include "UObject/StrongObjectPtr.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
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

		enum class UserDataType
		{
			None = 0,
			Int64 = 1,
			UInt64,
			RawPtr,
			StdSharedPtr,
			StdWeakPtr,

#ifdef TOLUAUUNREAL_API
			UEStruct,
			UESharedPtr,
			UEWeakPtr,
			UEObjStrongPtr,
			UEObjWeakPtr,
#endif
			Max
		};

#ifdef TOLUAUUNREAL_API
		template<typename T>
		using LuaUESharedPtr = TSharedPtr<T>;

		template<typename T>
		using LuaUEWeakPtr = TWeakPtr<T>;

		template<typename T>
		using LuaUEObjStrongPtr = TStrongObjectPtr<T>;
		
		template<typename T>
		using LuaUEObjWeakPtr = TWeakObjectPtr<T>;
#endif

		void InitStackAPI();

		ToLuau_API int32_t SetupMetaTableCommon(lua_State*L);
		
		ToLuau_API int32_t SetClassMetaTable(lua_State* L, std::string& ClassName, lua_CFunction SetupMetaTable = nullptr);

		namespace Int64Detail
		{
			extern int gInt64MT;
		}

		namespace UInt64Detail
		{
			extern int gUInt64MT; 
		}
		
		int32_t RegisterInt64(lua_State* L);
		int32_t RegisterUInt64(lua_State* L);

		#pragma region push & check

		namespace __DETAIL__
		{
			// SFINAE test
			template <typename T>
			class HasStaticLuaClass
			{
				typedef char one;
				struct two { char x[2]; };

				template <typename C>
				static one test( decltype(&C::StaticLuaClass) ) ;
				template <typename C>
				static two test(...);

			public:
				enum { Value = sizeof(test<T>(0)) == sizeof(char) };
			};

#ifdef TOLUAUUNREAL_API
			// SFINAE test
			template <typename T>
			class HasStaticClass
			{
				typedef char one;
				struct two { char x[2]; };

				template <typename C>
				static one test( decltype(&C::StaticClass) ) ;
				template <typename C>
				static two test(...);

			public:
				enum { Value = sizeof(test<T>(0)) == sizeof(char) };
			};

			// SFINAE test
			template <typename T>
			class HasStaticStruct
			{
				typedef char one;
				struct two { char x[2]; };

				template <typename C>
				static one test( decltype(&C::StaticStruct) ) ;
				template <typename C>
				static two test(...);

			public:
				enum { Value = sizeof(test<T>(0)) == sizeof(char) };
			};
#endif
			template<typename T>
			static std::string GetClassName(void* Obj);
			
			template<typename T, typename = void>
			struct GetClassNameWrapper
			{
				static std::string GetName(void* Obj)
				{
					assert(false);
					return "";
				}
			};

			template<typename T>
			struct GetClassNameWrapper<T, typename std::enable_if<HasStaticLuaClass<T>::Value>::type>
			{
				static std::string GetName(void* Obj)
				{
					return static_cast<T*>(Obj)->GetLuaClass()->Name();
				}
			};

#ifdef TOLUAUUNREAL_API
			template<typename T>
			struct GetClassNameWrapper<T, typename std::enable_if<HasStaticClass<T>::Value>::type>
			{
				static std::string GetName(void* Obj)
				{
					return StringEx::FStringToStdString(static_cast<T*>(Obj)->GetClass()->GetName());
				}
			};
			
			template<typename T>
			struct GetClassNameWrapper<T, typename std::enable_if<HasStaticStruct<T>::Value && !HasStaticLuaClass<T>::Value>::type>
			{
				static std::string GetName(void* Obj)
				{
					return StringEx::FStringToStdString(T::StaticStruct()->GetName());
				}
			};

			// make generic
			template<class R, class ...ARGS>
				struct MakeGenericTypeName {
				static void Value(std::string& output, const char* delimiter) {
					MakeGenericTypeName<R>::Value(output, delimiter);
					MakeGenericTypeName<ARGS...>::Value(output, delimiter);
				}
			};

			template<class R>
			struct MakeGenericTypeName<R>
			{
				static void Value(std::string& output, const char* delimiter)
				{
					output += (GetClassName<typename remove_cr<R>::type>());
					output += delimiter;
				}
			};
			
			template<typename R, typename ...ARGS>
			struct GetClassNameWrapper<TDelegate<R, ARGS...>>
			{
				static std::string GetName(void* Obj)
				{
					std::string Result;
					Result += "TBaseDelegate<";
					Result += MakeGenericTypeName<R, ARGS...>(Result, ",");
					Result += ">";
					return Result;
				}
			};

#endif

			template<typename T>
			static std::string GetClassName(void* Obj)
			{
				return GetClassNameWrapper<T>::GetName(Obj);
			}
			
			template<typename T, typename = void>
			struct StackOperatorWrapper;

			template<>
			struct StackOperatorWrapper<int64_t, void>
			{
			
				static int32_t Push(lua_State* L, int64_t Value)
				{
					void* p = lua_newuserdatatagged(L, sizeof(int64_t), UD_TAG(Int64));
			
					lua_getref(L, Int64Detail::gInt64MT);
					if(lua_isnil(L, -1))
					{
						lua_pop(L, 1);
						RegisterInt64(L);
						
						lua_getref(L, Int64Detail::gInt64MT);
						if(lua_isnil(L, -1))
						{
							luaL_errorL(L, "cannot register int64 meta table !!");
						}
					}
					
					lua_setmetatable(L, -2);
			
					*static_cast<int64_t*>(p) = Value;
					return 1;
				}
			
				static int64_t Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(Int64)))
						return *static_cast<int64_t*>(p);
			
					if (lua_isnumber(L, pos))
						return lua_tointeger(L, pos);
			
					luaL_typeerror(L, 1, "int64");
				}
			};

			template<>
			struct StackOperatorWrapper<uint64_t, void>
			{
				static int32_t Push(lua_State* L, uint64_t Value)
				{
					void* p = lua_newuserdatatagged(L, sizeof(uint64_t), UD_TAG(UInt64));
			
					lua_getref(L, UInt64Detail::gUInt64MT);
					if(lua_isnil(L, -1))
					{
						lua_pop(L, 1);
						RegisterUInt64(L);
						
						lua_getref(L, UInt64Detail::gUInt64MT);
						if(lua_isnil(L, -1))
						{
							luaL_errorL(L, "cannot register int64 meta table !!");
						}
					}
					
					lua_setmetatable(L, -2);
			
					*static_cast<uint64_t*>(p) = Value;
					return 1;
				}
			
				static uint64_t Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UInt64)))
						return *static_cast<uint64_t*>(p);
			
					if (lua_isnumber(L, pos))
						return lua_tointeger(L, pos);
			
					luaL_typeerror(L, 1, "uint64");
				}
			};
			
			template<>
			struct StackOperatorWrapper<int32_t, void>
			{
				static int32_t Push(lua_State* L, int32_t Value)
				{
					lua_pushinteger(L, static_cast<int32_t>(Value));
					return 1;
				}

				static int32_t Check(lua_State* L, int32_t pos)
				{
					return static_cast<int32_t>(luaL_checkinteger(L, pos));
				}
			};

			template<>
			struct StackOperatorWrapper<int16_t , void>
			{
				static int32_t Push(lua_State* L, int16_t Value)
				{
					lua_pushnumber(L, static_cast<int32_t>(Value));
					return 1;
				}

				static int16_t Check(lua_State* L, int32_t pos)
				{
					return static_cast<int16_t>(luaL_checkinteger(L, pos));
				}
			};

			template<>
			struct StackOperatorWrapper<int8_t, void>
			{
				static int32_t Push(lua_State* L, int8_t Value)
				{
					lua_pushnumber(L, static_cast<int32_t>(Value));
					return 1;
				}

				static int8_t Check(lua_State* L, int32_t pos)
				{
					return static_cast<int8_t>(luaL_checkinteger(L, pos));
				}
			};

			template<>
			struct StackOperatorWrapper<uint32_t, void>
			{
				static int32_t Push(lua_State* L, uint32_t Value)
				{
					lua_pushunsigned(L, static_cast<uint32_t>(Value));
					return 1;
				}

				static uint32_t Check(lua_State* L, int32_t Pos)
				{
					return static_cast<uint32_t>(luaL_checkunsigned(L, Pos));
				}
			};

			template<>
			struct StackOperatorWrapper<uint16_t, void>
			{
				static int32_t Push(lua_State* L, uint16_t Value)
				{
					lua_pushunsigned(L, static_cast<uint32_t>(Value));
					return 1;
				}

				static uint16_t Check(lua_State* L, int32_t Pos)
				{
					return static_cast<uint16_t>(luaL_checkunsigned(L, Pos));
				}
			};

			template<>
			struct StackOperatorWrapper<uint8_t, void>
			{
				static int32_t Push(lua_State* L, uint8_t Value)
				{
					lua_pushunsigned(L, static_cast<uint32_t>(Value));
					return 1;
				}

				static uint8_t Check(lua_State* L, int32_t Pos)
				{
					return static_cast<uint8_t>(luaL_checkunsigned(L, Pos));
				}
			};

			template<>
			struct StackOperatorWrapper<bool, void>
			{
				static int32_t Push(lua_State* L, bool Value)
				{
					lua_pushboolean(L, static_cast<bool>(Value));
					return 1;
				}

				static bool Check(lua_State* L, int32_t Pos)
				{
					return static_cast<bool>(luaL_checkboolean(L, Pos));
				}
			};

			template<>
			struct StackOperatorWrapper<float, void>
			{
				static int32_t Push(lua_State* L, float Value)
				{
					lua_pushnumber(L, static_cast<float>(Value));
					return 1;
				}

				static float Check(lua_State* L, int32_t Pos)
				{
					return static_cast<float>(luaL_checknumber(L, Pos));
				}
			};

			template<>
			struct StackOperatorWrapper<double, void>
			{
				static int32_t Push(lua_State* L, double Value)
				{
					lua_pushnumber(L, static_cast<double>(Value));
					return 1;
				}

				static double Check(lua_State* L, int32_t Pos)
				{
					return static_cast<double>(luaL_checknumber(L, Pos));
				}
			};

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

//			template<>
//			struct StackOperatorWrapper<void*, void>
//			{
//				static int32_t Push(lua_State* L, void* Value)
//				{
//					lua_pushlightuserdata(L, Value);
//					return 1;
//				}
//
//				static void* Check(lua_State* L, int32_t Pos)
//				{
//					return static_cast<void*>(lua_tolightuserdata(L, Pos));
//				}
//			};

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

			

#ifdef TOLUAUUNREAL_API

			template<typename T>
			struct StackOperatorWrapper<TArray<T>>
			{
				static int32_t Push(lua_State* L, const TArray<T>& Value)
				{
					lua_newtable(L);
					for (uint32_t i = 1; i <= Value.size(); ++i)
					{
						lua_pushinteger(L, static_cast<int32_t>(i));
						StackOperatorWrapper<T>::Push(L, Value[i]);
						lua_settable(L, -3);
					}
                    return 1;
				}

				static TArray<T> Check(lua_State* L, int32_t Pos)
				{
                    TArray<T> Result;

                    if(!lua_istable(L, Pos))
                    {
                        luaL_typeerrorL(L, Pos, "table");
                        return Result;
                    }

                    lua_pushvalue(L, Pos); // table

                    lua_pushnil(L); // table nil
                    while(lua_next(L, -2) != 0) // table key value
                    {
                        lua_pushvalue(L, -2); // table key value key
                        T Value = StackOperatorWrapper<T>::Check(L, -2);
                        Result.Add(Value);
                        lua_pop(L, 2); // table key
                    }
                    lua_pop(L, 2); // table
                    return Result;
				}
			};

            template<typename K, typename V>
            struct StackOperatorWrapper<TMap<K, V>>
            {
                static int32_t Push(lua_State* L, const TMap<K, V>& Value)
                {
                    lua_newtable(L);
                    for (const auto &Item: Value)
                    {
                        StackOperatorWrapper<K>::Push(L, Item.Key);
                        StackOperatorWrapper<V>::Push(L, Item.Value);
                        lua_settable(L, -3);
                    }
                	return 1;
                }

                static TMap<K, V> Check(lua_State* L, int32_t Pos)
                {
                    TMap<K, V> Result;
                    if(!lua_istable(L, Pos))
                    {
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
                        Result.Add(Key, Value);
                        lua_pop(L, 2); // table key
                    }
                    lua_pop(L, 1); // table
                    return Result;
                }

            };

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

#endif

            template<typename T>
            struct StackOperatorWrapper<T*,
				typename std::enable_if<
					!HasStaticLuaClass<T>::Value
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
                    lua_pushlightuserdata(L, Value);
                    return 1;
                }

                static T* Check(lua_State* L, int32_t Pos)
                {
                    if(!lua_islightuserdata(L, Pos))
                    {
                        luaL_typeerror(L, Pos, "lightuserdata");
                        return nullptr;
                    }
                    return reinterpret_cast<T*>(lua_tolightuserdata(L, Pos))->RawPtr;
                }
            };

            template<typename T>
            struct StackOperatorWrapper<T*, typename std::enable_if<
            	HasStaticLuaClass<T>::Value
#ifdef TOLUAUUNREAL_API
				|| HasStaticStruct<T>::Value // Struct Pointer Use Normal RawPtr
#endif
			>::type>
            {
                static int32_t Push(lua_State* L, T* Value)
                {
                	if(!Value)
                	{
                		lua_pushnil(L);
                	}
                    auto ClassName = GetClassName<T>(Value);
                    UserData<T>* NewUserData = static_cast<UserData<T>*>(lua_newuserdatatagged(L, sizeof(UserData<T>), UD_TAG(RawPtr)));

                    NewUserData->RawPtr = Value;
#if ToLuauDebug
                	NewUserData->DebugName = ClassName.c_str();
#endif 	
                	SetClassMetaTable(L, ClassName);
                    return 1;
                }

                static T* Check(lua_State* L, int32_t Pos)
                {
                	if (void* RawPtr = lua_touserdatatagged(L, Pos, UD_TAG(RawPtr)))
                	{
                		return static_cast<UserData<T>*>(RawPtr)->GetValue();
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
                		return static_cast<TSharedPtr<T>*>(UESharedPtr)->Get();
                	}
                	else if (void* UEWeakPtr = lua_touserdatatagged(L, Pos, UD_TAG(UEWeakPtr)))
                	{
                		TWeakPtr<T> WeakPtr = *static_cast<TWeakPtr<T>*>(UEWeakPtr);
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
                	else if (void* UEStruct = lua_touserdatatagged(L, Pos, UD_TAG(UEStruct)))
                	{
                		return static_cast<T*>(UEStruct); 
                	}
#endif
                	return nullptr;
                }

            };

#ifdef TOLUAUUNREAL_API

			template<typename T>
			struct StackOperatorWrapper<T, typename std::enable_if<HasStaticStruct<T>::Value>::type>
			{
				static int32_t Push(lua_State* L, const T& Value)
				{
					if(!Value)
					{
						lua_pushnil(L);
						return 1;
					}
					UScriptStruct* ScriptStruct = T::StaticStruct();
					void* p = lua_newuserdatatagged(L, sizeof(T), UD_TAG(UEStruct));
					ScriptStruct->InitializeStruct(p);
			
					auto ClassName = GetClassName<T>(Value);
					// set metatable
					SetClassMetaTable(L, ClassName);
			
					*static_cast<T*>(p) = Value;
					
					return 1;
				}
				
				static T Check(lua_State* L, int32_t Pos)
				{
					if (void* p = lua_touserdatatagged(L, Pos, UD_TAG(UEStruct)))
					{
						return *static_cast<T*>(p);
					}
					luaL_typeerror(L, 1, "UEStruct");
				}
			};


			template<typename T>
			struct StackOperatorWrapper<T*, typename std::enable_if<TIsDerivedFrom<T, UObject>::Value>::type>
			{
				static int32_t Push(lua_State* L, T* Value)
				{
					if(!Value)
					{
						lua_pushnil(L);
						return 1;
					}
					return StackOperatorWrapper<TStrongObjectPtr<T>>::Push(L, TStrongObjectPtr<T>(Value));
				}
				
				static T* Check(lua_State* L, int32_t Pos)
				{
					auto Ptr = StackOperatorWrapper<TStrongObjectPtr<T>>::Check(L, Pos);
					T* Obj = static_cast<T*>(Ptr.Get());
					return Obj;
				}
			};
#endif
			
			template<typename T>
			struct StackOperatorWrapper<std::shared_ptr<T>>
			{
				static int32_t Push(lua_State* L, std::shared_ptr<T> Value)
				{
					if(Value == nullptr)
					{
						lua_pushnil(L);
						return 1;
					}
					
					std::shared_ptr Temp;
					void* p = lua_newuserdatatagged(L, sizeof(Temp), UD_TAG(StdSharedPtr));
					memcpy(p, &Temp, sizeof(Temp));

					auto ClassName = GetClassName<T>(Value.get());

					// set metatable
					SetClassMetaTable(L, ClassName);

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
					luaL_typeerror(L, 1, "std::shared_ptr");
				}
			};

			template<typename T>
			struct StackOperatorWrapper<std::weak_ptr<T>>
			{
				static int32_t Push(lua_State* L, std::weak_ptr<T> Value)
				{
					if(Value.expired())
					{
						lua_pushnil(L);
						return 1;
					}
					std::weak_ptr<T> Temp;
					void* p = lua_newuserdatatagged(L, sizeof(Temp), UD_TAG(StdWeakPtr));
					memcpy(p, &Temp, sizeof(Temp));

					auto ClassName = GetClassName<T>(Value.lock().get());
					// set metatable
					SetClassMetaTable(L, ClassName);

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
					luaL_typeerror(L, 1, "std::weak_ptr");
				}
			};

#ifdef TOLUAUUNREAL_API

#pragma region ue smart ptr
			
			template<typename T>
			struct StackOperatorWrapper<LuaUESharedPtr<T>>
			{
				static int32_t Push(lua_State* L, LuaUESharedPtr<T> Value)
				{
					if(!Value.IsValid())
					{
						lua_pushnil(L);
						return 1;
					}
					
					LuaUESharedPtr<void> Temp;
					void* p = lua_newuserdatatagged(L, sizeof(Temp), UD_TAG(UESharedPtr));
					memcpy(p, &Temp, sizeof(LuaUESharedPtr<void>));
					
					auto ClassName = GetClassName<T>(Value.Get());
					// set metatable
					SetClassMetaTable(L, ClassName);
					
					*static_cast<LuaUESharedPtr<void>*>(p) = Value;
					return 1;
				}

				static LuaUESharedPtr<T> Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UESharedPtr)))
					{
						return StaticCastSharedPtr<T>(*static_cast<LuaUESharedPtr<void>*>(p));
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEWeakPtr)))
					{
						auto Weak = *static_cast<LuaUEWeakPtr<void>*>(p);
						if(!Weak.IsValid())
						{
							return nullptr;
						}
						return StaticCastSharedPtr<T>(Weak.Pin());
					}
					luaL_typeerror(L, 1, "TSharedPtr");
				}
			};

			template<typename T>
			struct StackOperatorWrapper<LuaUEWeakPtr<T>>
			{
				static int32_t Push(lua_State* L, LuaUEWeakPtr<T> Value)
				{
					if(!Value.IsValid())
					{
						lua_pushnil(L);
						return 1;
					}
					void* p = lua_newuserdatatagged(L, sizeof(LuaUEWeakPtr<void>), UD_TAG(UEWeakPtr));

					auto ClassName = GetClassName<T>(Value.Pin().Get());
					// set metatable
					SetClassMetaTable(L, ClassName);
					
					*static_cast<LuaUEWeakPtr<void>*>(p) = Value;
					return 1;
				}

				static LuaUEWeakPtr<T> Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEWeakPtr)))
					{
						return *static_cast<LuaUEWeakPtr<void>*>(p);
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UESharedPtr)))
					{
						auto Shared = *static_cast<LuaUESharedPtr<void>*>(p);
						if(!Shared.IsValid())
						{
							return nullptr;
						}
						return Shared;
					}
					luaL_typeerror(L, 1, "TWeakPtr");
				}
			};

			template<typename T>
			struct StackOperatorWrapper<LuaUEObjStrongPtr<T>>
			{
				static int32_t Push(lua_State* L, LuaUEObjStrongPtr<T> Value)
				{
					if(!Value.IsValid())
					{
						lua_pushnil(L);
						return 1;
					}
					
					LuaUEObjStrongPtr<UObject> Temp;
					void* p = lua_newuserdatatagged(L, sizeof(Temp), UD_TAG(UEObjStrongPtr));\
					memcpy(p, &Temp, sizeof(Temp));

					// set metatable
					auto ClassName = GetClassName<T>(Value.Get());;
					SetClassMetaTable(L, ClassName);

					LuaUEObjStrongPtr<UObject>& StrongObjectPtr = *static_cast<LuaUEObjStrongPtr<UObject>*>(p);
					StrongObjectPtr.Reset();
					StrongObjectPtr = Value;
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
					luaL_typeerror(L, 1, "LuaUEObjStrongPtr");
				}
			};

			template<>
			struct StackOperatorWrapper<FWeakObjectPtr>
			{
				static int32_t Push(lua_State* L, FWeakObjectPtr Value)
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
					std::string ClassName = StringEx::FStringToStdString(Value.Get()->GetClass()->GetName());
					SetClassMetaTable(L, ClassName);

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
					luaL_typeerror(L, 1, "LuaUEObjWeakPtr");
				}
			};

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
						luaL_typeerrorL(L, Pos, "string");
					}
					auto Path = StackOperatorWrapper<FString>::Check(L, Pos);
					return FSoftObjectPtr(FSoftObjectPath(Path));
				}
			};
			
			template<typename T>
			struct StackOperatorWrapper<LuaUEObjWeakPtr<T>>
			{
				static int32_t Push(lua_State* L, LuaUEObjWeakPtr<T> Value)
				{
					if(!Value.IsValid())
					{
						lua_pushnil(L);
						return 1;
					}
					
					LuaUEObjWeakPtr<UObject> Temp;
					void* p = lua_newuserdatatagged(L, sizeof(Temp), UD_TAG(UEObjWeakPtr));
					memcpy(p, &Temp, sizeof(Temp));

					auto ClassName = GetClassName<T>(Value.Get());;
					// set metatable
					SetClassMetaTable(L, ClassName);
					
					*static_cast<LuaUEObjWeakPtr<UObject>*>(p) = Value;
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
					luaL_typeerror(L, 1, "LuaUEObjWeakPtr");
				}
			};
#pragma endregion

#endif

		}

		template<typename T>
		int32_t Push(lua_State* L, const T& Value)
		{
			return __DETAIL__::StackOperatorWrapper<T>::Push(L, Value);
		}

		template<typename T>
		int32_t PushValue(lua_State* L, T Value)
		{
			return __DETAIL__::StackOperatorWrapper<T>::Push(L, Value);
		}

		template<class TFirst,class ...TArgs>
		int PushArgs(lua_State* L, TFirst f, TArgs&& ...args) {
			Push<TFirst>(L,f);
			return 1+PushArgs(std::forward<TArgs>(args)...);
		}

		inline int32_t PushArgs(lua_State* L)
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

#pragma region

#ifdef TOLUAUUNREAL_API

		namespace UE
		{
			void InitProperty();
		
			int32_t PushProperty(lua_State* L, FProperty* Up, uint8* Params);

			int32_t PushProperty(lua_State* L, FProperty* Up, UObject* Obj);
			
			int32_t CheckProperty(lua_State* L, FProperty* Prop, uint8* Params, int32_t Pos);
			
			int32_t CheckProperty(lua_State* L, FProperty* Prop, UObject* Obj, int32_t Pos);

			int32_t PushUFunction(lua_State* L, UFunction* Function);

		}

#endif

#pragma endregion

		#pragma endregion

	};
}

#undef UD_TAG
#include "Toluau/Containers/ToLuauVar.h"
#include "Toluau/Containers/ToLuauDelegate.h"
#include "Toluau/Containers/ToLuauArray.h"
#include "Toluau/Containers/ToLuauMap.h"
