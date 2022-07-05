//
// Created by 35207 on 2022/6/26 0026.
//

#pragma once


#include <cstdint>
#include <cassert>
#include <vector>
#include <map>

#include "lua.h"
#include "lualib.h"
#include "Util/Util.h"
#include "UserData.h"
#include "Class/Class.h"

namespace ToLuau
{
	namespace StackAPI
	{

        ToLuau_API int32_t PushRefObj(lua_State* L, BaseUserData* Instance, const Class* LuaClass);
        ToLuau_API void* CheckRefObj(lua_State* L, int32_t Pos, const Class* LuaClass);

		#pragma region push & check

		namespace __DETAIL__
		{
			template<typename T, typename = void>
			struct StackOperatorWrapper;

			const int kInt64Tag = 1;
			static int gInt64MT = -1;

			template<>
			struct StackOperatorWrapper<int64_t, void>
			{

				static int32_t Push(lua_State* L, int64_t Value)
				{
					void* p = lua_newuserdatatagged(L, sizeof(int64_t), kInt64Tag);

					lua_getref(L, gInt64MT);
					lua_setmetatable(L, -2);

					*static_cast<int64_t*>(p) = Value;
					return 1;
				}

				static int64_t Check(lua_State* L, int32_t pos)
				{
					if (void* p = lua_touserdatatagged(L, pos, kInt64Tag))
						return *static_cast<int64_t*>(p);

					if (lua_isnumber(L, pos))
						return lua_tointeger(L, pos);

					luaL_typeerror(L, 1, "int64");
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
						Push<T>(L, Value[i]);
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
                        T Value = Check(L, -2);
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
                        Push<K>(L, Item.first);
                        Push<V>(L, Item.second);
                        lua_settable(L, -3);
                    }
                }

                struct std::map<K, V> Check(lua_State* L, int32_t Pos)
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
                        K Key = Check(L, -1);
                        V Value = Check(L, -2);
                        Result.insert(std::make_pair(Key, Value));
                        lua_pop(L, 2); // table key
                    }
                    lua_pop(L, 1); // table
                    return Result;
                }

            };

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

            template<typename T>
            struct StackOperatorWrapper<T*, typename std::enable_if<!HasStaticLuaClass<T>::Value>::type>
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
                    auto ClassName = T::StaticLuaClass()->Name();
                    UserData<T>* NewUserData = reinterpret_cast<UserData<T>*>(lua_newuserdatadtor(L, sizeof(UserData<T>), [](void* UD){
                        UserData<T>* RealUD = reinterpret_cast<UserData<T>*>(UD);
	                    RealUD->Release();
                    }));
                    NewUserData->RawPtr = Value;
                    return PushRefObj(L, NewUserData, T::StaticLuaClass());
                }

                static T* Check(lua_State* L, int32_t Pos)
                {
                    auto ClassName = T::StaticLuaClass()->Name();
                    return reinterpret_cast<T*>(CheckRefObj(L, Pos, T::StaticLuaClass()));
                }

            };

		}

		template<typename T>
		int32_t Push(lua_State* L, const T& Value)
		{
			return __DETAIL__::StackOperatorWrapper<T>::Push(L, Value);
		}

		template<typename T>
		T Check(lua_State* L, int32_t Pos)
		{
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
		}

		template<typename T>
		int32_t PushTableItem(lua_State*  L, int32_t Index, T Value)
		{
			lua_pushinteger(L, Index);
			Push(L, Value);
			lua_settable(L, -3);
		}

		#pragma endregion

	};
}

