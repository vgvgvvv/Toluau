//
// Created by 35207 on 2022/6/26 0026.
//

#pragma once


#include <cstdint>
#include "lua.h"
#include "lualib.h"
#include "Util/Util.h"

namespace ToLuau
{
	namespace StackAPI
	{
		namespace __DETAIL__
		{
			template<typename T, typename = void>
			struct StackOperatorWrapper
			{
				static int32_t Push(lua_State* L, T Value)
				{
					Lua::Error("not support current type !!");
					return 0;
				}

				static T Check(lua_State* L, int32_t pos)
				{
					Lua::Error("not support current type !!");
					return T{};
				}
			};

			const int kInt64Tag = 1;
			static int gInt64MT = -1;

			template<typename T>
			struct StackOperatorWrapper<typename std::is_same<int64_t, T>>
			{

				static int32_t Push(lua_State* L, T Value)
				{
					void* p = lua_newuserdatatagged(L, sizeof(int64_t), kInt64Tag);

					lua_getref(L, gInt64MT);
					lua_setmetatable(L, -2);

					*static_cast<int64_t*>(p) = Value;
					return 1;
				}

				static T Check(lua_State* L, int32_t pos)
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
			struct StackOperatorWrapper<nullptr_t, void>
			{
				static int32_t Push(lua_State* L, nullptr_t Value)
				{
					lua_pushnil(L);
					return 1;
				}

				static nullptr_t Check(lua_State* L, int32_t Pos)
				{
					return nullptr;
				}
			};

			template<>
			struct StackOperatorWrapper<void*, void>
			{
				static int32_t Push(lua_State* L, void* Value)
				{
					lua_pushlightuserdata(L, Value);
					return 1;
				}

				static void* Check(lua_State* L, int32_t Pos)
				{
					return static_cast<void*>(lua_tolightuserdata(L, Pos));
				}
			};

			template<>
			struct StackOperatorWrapper<const std::string&, void>
			{
				static int32_t Push(lua_State* L, const std::string& Value)
				{
					lua_pushstring(L, Value.c_str());
					return 1;
				}

			};

			template<>
			struct StackOperatorWrapper<std::string, void>
			{
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
			struct StackOperatorWrapper<std::is_enum<T>>
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

		}

		template<typename T>
		int32_t Push(lua_State* L, T Value)
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

	};
}

