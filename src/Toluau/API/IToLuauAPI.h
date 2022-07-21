#pragma once

#include <string>
#include <memory>
#include <functional>

#include "lua.h"
#include "Arg.h"

#ifdef TOLUAUUNREAL_API
#include "Toluau/ToLuau_API.h"
#else
#include "ToLuau_API.h"
#endif

namespace ToLuau
{
	class ILuauState;

	enum class MetatableEvent : uint8_t
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
		virtual void CallFunc(const std::string& FuncName) = 0;

		virtual void CallFuncWithArg(const std::string& FuncName, const IArg& Arg) = 0;

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

		static const char* GetMtName(MetatableEvent Type);

	protected:
		ILuauState* Owner = nullptr;
	};
}
