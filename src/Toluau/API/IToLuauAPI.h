#pragma once

#include <string>
#include <memory>
#include <functional>
#include <stack>

#include "lua.h"
#include "Arg.h"
#include "Toluau/API/ToLuauLib.h"
#include "Toluau/API/StackAPI.h"
#include "Toluau/API/ScopeHelper.h"
#include "Toluau/ToLuau.h"

#include "Toluau/ToLuauDefine.h"

namespace ToLuau
{
	class ILuauState;

	enum class MetatableEvent
	{
		Index = 0,
		NewIndex,
		Mode,
		NameCall,
		Call,
		Iter,

		Eq,

		Add,
		Sub,
		Mul,
		Div,
		Mod,
		Pow,
		Unm,

		Len,
		Lt,
		Le,
		Concat,
		Type
	};

	const char* const MetatableEventName[] = {
		/* ORDER TM */

		"__index",
		"__newindex",
		"__mode",
		"__namecall",
		"__call",
		"__iter",

		"__eq",


		"__add",
		"__sub",
		"__mul",
		"__div",
		"__mod",
		"__pow",
		"__unm",


		"__len",
		"__lt",
		"__le",
		"__concat",
		"__type",
	};

	class ToLuau_API IToLuauAPI
	{
	protected:
		explicit IToLuauAPI(ILuauState* InOwner) : Owner(InOwner) {}
	public:
		virtual ~IToLuauAPI() = default;

        const ILuauState* GetOwner() const { return Owner; }

		using LuaFunc = std::function<int32_t(lua_State*)>;

		static std::shared_ptr<IToLuauAPI> Create(ILuauState* InOwner);

#pragma region Function

	public:
		
		virtual void CallFunc(const std::string& FuncName, int32_t RetNum = 0) = 0;

		template<typename T>
		T CallFuncR(const std::string& FuncName)
		{
			auto L = GetOwner()->GetState();
			CallFunc(FuncName, 1);
			return StackAPI::Check<T>(L, -1);
		}

		virtual void CallFuncWithArg(const std::string& FuncName, const IArg& Arg, int32_t RetNum = 0) = 0;

		template<typename T>
		T CallFuncWithArgR(const std::string& FuncName, const IArg& Arg)
		{
			auto L = GetOwner()->GetState();
			CallFuncWithArg(FuncName, Arg, 1);
			return StackAPI::Check<T>(L, -1);
		}

		template<typename TArg1>
		void CallFunc(const std::string& FuncName, TArg1 InArg1)
		{
			Arg1<TArg1> Args(InArg1);
			CallFuncWithArg(FuncName, Args);
		}

		template<typename TArg1,
				typename TArg2>
		void CallFunc(const std::string& FuncName
				, TArg1 InArg1
				, TArg2 InArg2)
		{
			Arg2<TArg1,
					TArg2> Args(InArg1,
			                    InArg2);
			CallFuncWithArg(FuncName, Args);
		}

		template<typename TArg1,
				typename TArg2,
				typename TArg3>
		void CallFunc(const std::string& FuncName
				, TArg1 InArg1
				, TArg2 InArg2
				, TArg3 InArg3)
		{
			Arg3<TArg1,
					TArg2,
					TArg3> Args(InArg1,
			                    InArg2,
			                    InArg3);
			CallFuncWithArg(FuncName, Args);
		}

		template<typename TArg1,
				typename TArg2,
				typename TArg3,
				typename TArg4>
		void CallFunc(const std::string& FuncName
				, TArg1 InArg1
				, TArg2 InArg2
				, TArg3 InArg3
				, TArg4 InArg4)
		{
			Arg4<TArg1,
					TArg2,
					TArg3,
					TArg4> Args(InArg1,
			                    InArg2,
			                    InArg3,
			                    InArg4);
			CallFuncWithArg(FuncName, Args);
		}

		template<typename TArg1,
				typename TArg2,
				typename TArg3,
				typename TArg4,
				typename TArg5>
		void CallFunc(const std::string& FuncName
				, TArg1 InArg1
				, TArg2 InArg2
				, TArg3 InArg3
				, TArg4 InArg4
				, TArg5 InArg5)
		{
			Arg5<TArg1,
					TArg2,
					TArg3,
					TArg4,
					TArg5> Args(InArg1,
			                    InArg2,
			                    InArg3,
			                    InArg4,
			                    InArg5);
			CallFuncWithArg(FuncName, Args);
		}

		template<typename TArg1,
				typename TArg2,
				typename TArg3,
				typename TArg4,
				typename TArg5,
				typename TArg6>
		void CallFunc(const std::string& FuncName
				, TArg1 InArg1
				, TArg2 InArg2
				, TArg3 InArg3
				, TArg4 InArg4
				, TArg5 InArg5
				, TArg6 InArg6)
		{
			Arg6<TArg1,
					TArg2,
					TArg3,
					TArg4,
					TArg5,
					TArg6> Args(InArg1,
			                    InArg2,
			                    InArg3,
			                    InArg4,
			                    InArg5,
			                    InArg6);
			CallFuncWithArg(FuncName, Args);
		}

		template<typename TArg1,
				typename TArg2,
				typename TArg3,
				typename TArg4,
				typename TArg5,
				typename TArg6,
				typename TArg7>
		void CallFunc(const std::string& FuncName
				, TArg1 InArg1
				, TArg2 InArg2
				, TArg3 InArg3
				, TArg4 InArg4
				, TArg5 InArg5
				, TArg6 InArg6
				, TArg7 InArg7)
		{
			Arg7<TArg1,
					TArg2,
					TArg3,
					TArg4,
					TArg5,
					TArg6,
					TArg7> Args(InArg1,
			                    InArg2,
			                    InArg3,
			                    InArg4,
			                    InArg5,
			                    InArg6,
			                    InArg7);
			CallFuncWithArg(FuncName, Args);
		}

		template<typename TArg1,
				typename TArg2,
				typename TArg3,
				typename TArg4,
				typename TArg5,
				typename TArg6,
				typename TArg7,
				typename TArg8>
		void CallFunc(const std::string& FuncName
				, TArg1 InArg1
				, TArg2 InArg2
				, TArg3 InArg3
				, TArg4 InArg4
				, TArg5 InArg5
				, TArg6 InArg6
				, TArg7 InArg7
				, TArg8 InArg8)
		{
			Arg8<TArg1,
					TArg2,
					TArg3,
					TArg4,
					TArg5,
					TArg6,
					TArg7,
					TArg8> Args(InArg1,
			                    InArg2,
			                    InArg3,
			                    InArg4,
			                    InArg5,
			                    InArg6,
			                    InArg7,
			                    InArg8);
			CallFuncWithArg(FuncName, Args);
		}


		template<typename TArg1,
				typename TArg2,
				typename TArg3,
				typename TArg4,
				typename TArg5,
				typename TArg6,
				typename TArg7,
				typename TArg8,
				typename TArg9>
		void CallFunc(const std::string& FuncName
					    , TArg1 InArg1
						, TArg2 InArg2
						, TArg3 InArg3
						, TArg4 InArg4
						, TArg5 InArg5
						, TArg6 InArg6
						, TArg7 InArg7
						, TArg8 InArg8
						, TArg9 InArg9)
		{
			Arg9<TArg1,
				TArg2,
				TArg3,
				TArg4,
				TArg5,
				TArg6,
				TArg7,
				TArg8,
				TArg9> Args(InArg1,
			                InArg2,
			                InArg3,
			                InArg4,
			                InArg5,
			                InArg6,
			                InArg7,
			                InArg8,
			                InArg9);
			CallFuncWithArg(FuncName, Args);
		}
	protected:

		virtual bool GetFuncGlobal(const std::string& LuaFunc, bool* bIsClassFunc) = 0;

		virtual int32_t DoPCall(int32_t ArgNum, int32_t RetNum) = 0;

#pragma endregion

	public:

		template<typename T>
		bool GetAnyFromState(const std::string& FullName, T& Result, bool bAllowNotFound = false)
		{
			auto L = GetOwner()->GetState();
			StackAPI::AutoStack AutoStack(L);

			auto Pos = FullName.find_last_of('.');
			std::string TableName;
			std::string TargetName;
			if(Pos == std::string::npos)
			{
				TargetName = FullName;
				lua_getglobal(L, TargetName.c_str()); // globalvalue

				if(lua_isnil(L, -1)) // nil
				{
					lua_pop(L, 1);
    			
					lua_getref(L, TOLUAU_LOADED_REF); // ref
					lua_getfield(L, -1, TargetName.c_str()); // ref field
					if(lua_isnil(L, -1)) // ref nil
					{
						lua_pop(L, 2);
						if(!bAllowNotFound)
						{
							LUAU_ERROR_F("cannot find %s in gobal and loaded", TargetName.c_str())
						}
						return false;
					}
					else
					{
						lua_remove(L, -2); // field
						Result = StackAPI::Check<T>(L, -1);
						return true;
					}
				}
				else
				{
					Result = StackAPI::Check<T>(L, -1);
					return true;
				}
			}
			else
			{
				TableName = FullName.substr(0, Pos);
				TargetName = FullName.substr(Pos + 1, FullName.size() - Pos - 1);

				StackAPI::FindTable(L, TableName, false); // table

				if(!lua_istable(L, -1)) // nil
				{
					lua_pop(L, 1); //
					if(!bAllowNotFound)
					{
						LUAU_ERROR_F("%s is not a table", TableName.c_str())
					}
					return false;
				}
			
				lua_getfield(L, -1, TargetName.c_str()); // table value

				if(lua_isnil(L, -1))  // table nil
				{
					lua_pop(L, 2);
					if(!bAllowNotFound)
					{
						LUAU_ERROR_F("cannot find %s in %s", TargetName.c_str(), TableName.c_str())
					}
					return false;
				}

				lua_remove(L, -2); // value
				Result = StackAPI::Check<T>(L, -1);
				return true;

			}
		}

		template<typename T>
		bool SetAnyToState(const std::string& FullName, const T& Value)
		{
			auto L = GetOwner()->GetState();
			StackAPI::AutoStack AutoStack(L);

			auto Pos = FullName.find_last_of('.');
			std::string TableName;
			std::string TargetName;
			
			if(Pos == std::string::npos)
			{
				TargetName = FullName;
				StackAPI::Push<T>(L, Value); // value
				lua_setglobal(L, TableName.c_str());
				return true;
			}
			else
			{
				TableName = FullName.substr(0, Pos);
				TargetName = FullName.substr(Pos + 1, FullName.size() - Pos - 1);

				StackAPI::FindTable(L, TableName, true); // table

				if(!lua_istable(L, -1)) // nil
				{
					lua_pop(L, 1); //
					LUAU_ERROR_F("%s is not a table", TableName.c_str())
					return false;
				}

				StackAPI::Push<T>(L, Value); // table value
				lua_setfield(L, -2, TargetName.c_str()); // table

				lua_pop(L, 1);
			
				return true;
			}
		}

#ifdef TOLUAUUNREAL_API

#define TO_STD(Str) StringEx::FStringToStdString(Str)
		
		template<typename T>
		bool GetAnyFromStateF(const FString& FullName, T& Result)
		{
			return GetAnyFromState<T>(TO_STD(FullName), Result);
		}

		template<typename T>
		bool SetAnyToStateF(const FString& FullName, const T& Value)
		{
			return SetAnyToState<T>(TO_STD(FullName), Value);
		}

#undef TO_STD

#endif
		
		virtual bool CheckNum(int32_t Count) = 0;

		virtual bool CheckRange(int32_t Min, int32_t Max) = 0;

		virtual void ResetStack() = 0;
	public:

		static const char* GetMtName(MetatableEvent Type);

	protected:
		ILuauState* Owner = nullptr;
	};
}
