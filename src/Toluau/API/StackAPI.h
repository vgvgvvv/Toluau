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
#include "Toluau/Util/Util.h"

#ifdef TOLUAUUNREAL_API
#include "Toluau/ToLuau_API.h"
#else
#include "ToLuau_API.h"
#endif


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
		using LuaUESharedPtr = TSharedPtr<T, ESPMode::ThreadSafe>;

		template<typename T>
		using LuaUEWeakPtr = TWeakPtr<T, ESPMode::ThreadSafe>;

		template<typename T>
		using LuaUEObjStrongPtr = TStrongObjectPtr<T>;
		
		template<typename T>
		using LuaUEObjWeakPtr = TWeakObjectPtr<T>;
#endif

		void InitStackAPI();

		ToLuau_API int32_t SetClassMetaTable(lua_State* L, std::string& ClassName);

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
			
			template<typename T, typename = void>
			struct GetClassNameWrapper
			{
				static std::string GetName()
				{
					assert(false);
					return "";
				}
			};

			template<typename T>
			struct GetClassNameWrapper<T, typename std::enable_if<HasStaticLuaClass<T>::Value>::type>
			{
				static std::string GetName()
				{
					return T::StaticLuaClass()->Name();
				}
			};

#ifdef TOLUAUUNREAL_API
			template<typename T>
			struct GetClassNameWrapper<T, typename std::enable_if<HasStaticClass<T>::Value>::type>
			{
				static std::string GetName()
				{
					return StringEx::FStringToStdString(T::StaticClass()->GetName());
				}
			};
			
			template<typename T>
			struct GetClassNameWrapper<T, typename std::enable_if<HasStaticStruct<T>::Value>::type>
			{
				static std::string GetName()
				{
					return StringEx::FStringToStdString(T::StaticStruct()->GetName());
				}
			};
#endif

			template<typename T>
			static std::string GetClassName()
			{
				return GetClassNameWrapper<T>::GetName();
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
					for (uint32_t i = 0; i < Value.size(); ++i)
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
            struct StackOperatorWrapper<T*, typename std::enable_if<HasStaticLuaClass<T>::Value>::type>
            {
                static int32_t Push(lua_State* L, T* Value)
                {
                    auto ClassName = GetClassName<T>();
                    UserData<T>* NewUserData = static_cast<UserData<T>*>(lua_newuserdatatagged(L, sizeof(UserData<T>), UD_TAG(RawPtr)));

                    NewUserData->RawPtr = Value;
                	NewUserData->DebugName = ClassName.c_str();
                	
                	SetClassMetaTable(L, ClassName);
                    return 1;
                }

                static T* Check(lua_State* L, int32_t Pos)
                {
                	void* p = nullptr;
                	if ((p = lua_touserdatatagged(L, Pos, UD_TAG(RawPtr))))
                	{
                		return static_cast<UserData<T>*>(p)->GetValue();
                	}
                	else if ((p = lua_touserdatatagged(L, Pos, UD_TAG(StdSharedPtr))))
                	{
                		return static_cast<std::shared_ptr<T>*>(p)->get();
                	}
                	else if ((p = lua_touserdatatagged(L, Pos, UD_TAG(StdWeakPtr))))
                	{
                		return (*static_cast<std::weak_ptr<T>*>(p)).lock().get();
                	}
#ifdef TOLUAUUNREAL_API
                	else if ((p = lua_touserdatatagged(L, Pos, UD_TAG(UESharedPtr))))
                	{
                		return static_cast<TSharedPtr<T>*>(p)->Get();
                	}
                	else if ((p = lua_touserdatatagged(L, Pos, UD_TAG(UEWeakPtr))))
                	{
                		TWeakPtr<T> WeakPtr = *static_cast<TWeakPtr<T>*>(p);
                		if(!WeakPtr.IsValid())
                		{
                			return nullptr;
                		}
                		return WeakPtr.Pin().Get();
                	}
                	else if ((p = lua_touserdatatagged(L, Pos, UD_TAG(UEObjStrongPtr))))
                	{
                		return static_cast<TStrongObjectPtr<T>*>(p)->Get();
                	}
                	else if ((p = lua_touserdatatagged(L, Pos, UD_TAG(UEObjWeakPtr))))
                	{
                		return static_cast<TWeakObjectPtr<T>*>(p)->Get();
                	}
                	else if (void* p = lua_touserdatatagged(L, Pos, UD_TAG(UEStruct)))
                	{
                		return *static_cast<T*>(p); 
                	}
#endif
                	return nullptr;
                }

            };

#ifdef TOLUAUUNREAL_API

			template<typename T>
			struct StackOperatorWrapper<T*, typename std::enable_if<HasStaticStruct<T>::Value>::type>
			{
				static int32_t Push(lua_State* L, T* Value)
				{
					UScriptStruct* ScriptStruct = T::StaticStruct();
					void* p = lua_newuserdatatagged(L, sizeof(T), UD_TAG(UEStruct));
					ScriptStruct->InitializeStruct(p);

					auto ClassName = GetClassName<T>();
					// set metatable
					SetClassMetaTable(L, ClassName);

					*static_cast<T*>(p) = *Value;
					
					return 1;
				}
				
				static T* Check(lua_State* L, int32_t Pos)
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
					return StackOperatorWrapper<TStrongObjectPtr<T>>::Push(L, TStrongObjectPtr<T>(Value));
				}
				
				static T* Check(lua_State* L, int32_t Pos)
				{
					return static_cast<T*>(StackOperatorWrapper<TStrongObjectPtr<T>>::Check(L, Pos).Get());
				}
			};
#endif
			
			template<typename T>
			struct StackOperatorWrapper<std::shared_ptr<T>>
			{
				static int32_t Push(lua_State* L, std::shared_ptr<T> Value)
				{
					void* p = lua_newuserdatatagged(L, sizeof(std::shared_ptr<T>), UD_TAG(StdSharedPtr));

					auto ClassName = GetClassName<T>();

					// set metatable
					SetClassMetaTable(L, ClassName);
					
					*static_cast<std::shared_ptr<T>*>(p) = Value;
					return 1;
				}

				static std::shared_ptr<T> Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(StdSharedPtr)))
					{
						return *static_cast<std::shared_ptr<T>*>(p);
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(StdWeakPtr)))
					{
						auto Weak = *static_cast<std::weak_ptr<T>*>(p);
						if(Weak.expired())
						{
							return nullptr;
						}
						return Weak.lock();
					}
					luaL_typeerror(L, 1, "std::shared_ptr");
				}
			};

			template<typename T>
			struct StackOperatorWrapper<std::weak_ptr<T>>
			{
				static int32_t Push(lua_State* L, std::weak_ptr<T> Value)
				{
					void* p = lua_newuserdatatagged(L, sizeof(std::weak_ptr<T>), UD_TAG(StdWeakPtr));

					auto ClassName = GetClassName<T>();
					// set metatable
					SetClassMetaTable(L, ClassName);
					
					*static_cast<std::weak_ptr<T>*>(p) = Value;
					return 1;
				}

				static std::weak_ptr<T> Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(StdWeakPtr)))
					{
						return *static_cast<std::weak_ptr<T>*>(p);
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(StdSharedPtr)))
					{
						auto Shared = *static_cast<std::weak_ptr<T>*>(p);
						return Shared;
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
					void* p = lua_newuserdatatagged(L, sizeof(LuaUESharedPtr<T>), UD_TAG(UESharedPtr));

					auto ClassName = GetClassName<T>();
					// set metatable
					SetClassMetaTable(L, ClassName);

					LuaUESharedPtr<T>* SharedPtr = reinterpret_cast<LuaUESharedPtr<T>*>(p);
					*SharedPtr = Value;
					return 1;
				}

				static LuaUESharedPtr<T> Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UESharedPtr)))
					{
						return *static_cast<LuaUESharedPtr<T>*>(p);
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEWeakPtr)))
					{
						auto Weak = *static_cast<LuaUEWeakPtr<T>*>(p);
						if(!Weak.IsValid())
						{
							return nullptr;
						}
						return Weak.Pin();
					}
					luaL_typeerror(L, 1, "TSharedPtr");
				}
			};

			template<typename T>
			struct StackOperatorWrapper<LuaUEWeakPtr<T>>
			{
				static int32_t Push(lua_State* L, LuaUEWeakPtr<T> Value)
				{
					void* p = lua_newuserdatatagged(L, sizeof(LuaUEWeakPtr<T>), UD_TAG(UEWeakPtr));

					auto ClassName = GetClassName<T>();
					// set metatable
					SetClassMetaTable(L, ClassName);
					
					*static_cast<LuaUEWeakPtr<T>*>(p) = Value;
					return 1;
				}

				static LuaUEWeakPtr<T> Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEWeakPtr)))
					{
						return *static_cast<LuaUEWeakPtr<T>*>(p);
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UESharedPtr)))
					{
						auto Shared = *static_cast<LuaUESharedPtr<T>*>(p);
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
					void* p = lua_newuserdatatagged(L, sizeof(LuaUEObjStrongPtr<T>), UD_TAG(UEObjStrongPtr));

					// set metatable
					auto ClassName = GetClassName<T>();;
					SetClassMetaTable(L, ClassName);
					
					*static_cast<LuaUEObjStrongPtr<T>*>(p) = Value;
					return 1;
				}

				static LuaUEObjStrongPtr<T> Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEObjStrongPtr)))
					{
						return *static_cast<LuaUEObjStrongPtr<T>*>(p);
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEObjWeakPtr)))
					{
						auto Weak = *static_cast<LuaUEObjWeakPtr<T>*>(p);
						if(!Weak.IsValid())
						{
							return nullptr;
						}
						return LuaUEObjStrongPtr<T>(Weak.Get());
					}
					luaL_typeerror(L, 1, "LuaUEObjStrongPtr");
				}
			};

			template<typename T>
			struct StackOperatorWrapper<LuaUEObjWeakPtr<T>>
			{
				static int32_t Push(lua_State* L, LuaUEObjWeakPtr<T> Value)
				{
					void* p = lua_newuserdatatagged(L, sizeof(LuaUEObjWeakPtr<T>), UD_TAG(UEObjWeakPtr));

					auto ClassName = GetClassName<T>();;
					// set metatable
					SetClassMetaTable(L, ClassName);
					
					*static_cast<LuaUEObjWeakPtr<T>*>(p) = Value;
					return 1;
				}

				static LuaUEObjWeakPtr<T> Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEObjWeakPtr)))
					{
						return *static_cast<LuaUEObjWeakPtr<T>*>(p);
					}
					if (void* p = lua_touserdatatagged(L, pos, UD_TAG(UEObjStrongPtr)))
					{
						auto Strong = *static_cast<LuaUEObjStrongPtr<T>*>(p);
						if(!Strong.IsValid())
						{
							return nullptr;
						}
						return LuaUEObjWeakPtr<T>(Strong.Get());
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
		int32_t PushArray(lua_State* L, T* Array, size_t Size, uint32_t Start = 0)
		{
			if(Array == nullptr)
			{
				lua_pushnil(L);
				return 1;
			}

			lua_newtable(L);
			uint32_t End = Start + Size;
			for (uint32_t i = Start; i < End; ++i)
			{
				lua_pushinteger(L, static_cast<int32_t>(i));
				Push<T>(L, Array[i]);
				lua_settable(L, -3);
			}

			return 1;
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