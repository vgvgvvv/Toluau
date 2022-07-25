﻿#pragma once

#include <string>

#include "Toluau/ToLuauDefine.h"

#include "lua.h"
#include "Toluau/API/UserData.h"
#include "Toluau/API/StackAPI.h"


namespace ToLuau
{
	class ToLuau_API ToLuauVar
	{
	public:
		enum class Type
		{
			Nil,
			Number,
			Bool,
			String,
			Function,
			UserData,
			LightUserData,
			Table,
			Tuple
		};

		ToLuauVar();
		ToLuauVar(lua_Integer Value);
		ToLuauVar(size_t Value);
		ToLuauVar(lua_Number Value);
		ToLuauVar(const char* Value, size_t Len);
#ifdef TOLUAUUNREAL_API
		ToLuauVar(const FString& Value);
#endif
		ToLuauVar(const std::string& Value);
		ToLuauVar(bool Value);

		ToLuauVar(lua_State* L,int32_t Pos);
		ToLuauVar(lua_State* L,int32_t Pos, Type T);

		static ToLuauVar CreateTuple(lua_State* L, size_t N)
		{
			ToLuauVar NewVar;
			NewVar.Init(L, N, Type::Tuple);
			return NewVar;
		}
		
		void Init(lua_State* L, int32_t Pos, Type T);
		void InitTuple(lua_State* L, int32_t Num);

		ToLuauVar(const ToLuauVar& Other) : ToLuauVar()
		{
			Clear();
			Clone(Other);
		}

		ToLuauVar(ToLuauVar&& Other) noexcept : ToLuauVar()
		{
			Move(std::move(Other));
		}
		
		void operator=(const ToLuauVar& Other)
		{
			Clear();
			Clone(Other);
		}
		
		void operator=(ToLuauVar&& Other) noexcept
		{
			Clear();
			Move(std::move(Other));
		}

		virtual  ~ToLuauVar()
		{
			Clear();
		}


		void Set(lua_State* L, int32_t Pos);
		void Set(lua_Integer Value);
		void Set(lua_Number Value);
		void Set(const char* Value, size_t Len);
		void Set(const std::string& Str);
#ifdef TOLUAUUNREAL_API
		void Set(const FString& Value);
#endif
		void Set(bool Bool); 
		
		bool IsNil() const;
		bool IsFunction() const;
		bool IsTuple() const;
		bool IsTable() const;
		bool IsNumber() const;
		bool IsString() const;
		bool IsBool() const;
		bool IsUserData() const;
		bool IsLightUserData() const;
		bool IsValid() const;
		Type GetType() const;

		int32_t ToInt() const;
		float ToFloat() const;
		double ToDouble() const;
		std::string ToStdString() const;
#ifdef TOLUAUUNREAL_API
		FString ToFString() const;
#endif
		bool ToBool() const;
		void* ToLightUserData() const;
		template<typename T>
		T* ToUserData() const
		{
			assert(Vars.size() == 1 && Vars[0].LuaType == Type::UserData);
			auto Ref = Vars[0].Ref;
			lua_pushvalue(Ref->OwnerState, Ref->Ref);
			auto Result = StackAPI::Check<T*>(Ref->OwnerState, -1);
			lua_pop(Ref->OwnerState, 1);
			return Result;
		}

#pragma region Table

		size_t Num() const;
		
		ToLuauVar GetAt(size_t Index) const;
		
		template<typename T>
		T GetAt(size_t Index)
		{
			return GetAt(Index).Get<T>();
		}

		template<typename T>
		void SetAt(T Value, int32_t Index = -1)
		{
            assert(IsTable());
			Push();
			if(Index < 0)
			{
				Index = lua_objlen(OwnerState, -1) + 1;
			}
			StackAPI::Push(OwnerState, Index);
			StackAPI::Push(OwnerState, Value);
			lua_settable(OwnerState, -3);
			lua_pop(OwnerState, 1);
		}

		template<typename R, typename T>
		R GetFromTable(T Key, bool bRawGet = false)
		{
            assert(IsTable());
			StackAPI::AutoStack A(OwnerState);
			Push();
			StackAPI::Push(OwnerState, Key);
			if(bRawGet)
			{
				lua_rawget(OwnerState, -2);
			}
			else
			{
				lua_gettable(OwnerState, -2);
			}
			return StackAPI::Check<R>(OwnerState, -1);
		}
		
		template<typename R, typename T>
		void GetFromTable(T Key, R& Result, bool bRawGet = false)
		{
			Result = GetFromTable(Key, bRawGet);
		}

		template<typename K, typename V>
		void SetToTable(K Key, V Value)
		{
			assert(IsTable());
			Push();
			StackAPI::Push(OwnerState, Key);
			StackAPI::Push(OwnerState, Value);
			lua_settable(OwnerState, -3);
			lua_pop(OwnerState, 1);
		}
	

#pragma endregion

#pragma region Function

		int32_t DoCall(int32_t Argn) const;

		ToLuauVar Return(int32_t RetN) const
		{
			assert(RetN >= 0);
			if(RetN == 0)
			{
				return ToLuauVar();
			}
			else if(RetN == 1)
			{
				return ToLuauVar(OwnerState, -1);
			}
			return ToLuauVar::CreateTuple(OwnerState, RetN);
		}
		
		template<typename ...ARGS>
		ToLuauVar Call(ARGS&&... args) const
		{
			if(!IsValid())
			{
				LUAU_ERROR("toluau var is not valid");
				return ToLuauVar();
			}
			if(!IsFunction())
			{
				LUAU_ERROR("toluau var is not a function, cannot call");
				return ToLuauVar();
			}

			auto N = StackAPI::PushArgs(OwnerState, std::forward<ARGS>(args)...);
			auto RetN = DoCall(N);
			auto Ret = Return(RetN);
			lua_pop(OwnerState, RetN);
			return Ret;
		}

#ifdef TOLUAUUNREAL_API

		int PushArgByParams(FProperty* Prop, uint8* Parms) const;
		
		bool CallByUFunction(UFunction* UFunc, uint8* Params, FOutParmRec *OutParams = nullptr, RESULT_DECL = nullptr, bool bNeedSelf = false);

#endif

#pragma endregion

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#endif
		struct Var
		{
			union
			{
				std::shared_ptr<LuaRef> Ref;
				lua_Number Num;
				std::shared_ptr<std::string> Str;
				void* Ptr;
				bool Bool;
			};
			Type LuaType;
			Var():
				Ptr(nullptr),
				LuaType(Type::Nil)
			{
			}

			Var(const Var& Other)
				: Ptr(Other.Ptr),
				  LuaType(Other.LuaType)
			{
			}

			Var(Var&& Other) noexcept
				: Ptr(Other.Ptr),
				  LuaType(Other.LuaType)
			{
			}

			Var& operator=(const Var& Other)
			{
				if (this == &Other)
					return *this;
				Ref = Other.Ref;
				Num = Other.Num;
				Str = Other.Str;
				Ptr = Other.Ptr;
				Bool = Other.Bool;
				LuaType = Other.LuaType;
				return *this;
			}

			Var& operator=(Var&& Other) noexcept
			{
				if (this == &Other)
					return *this;
				Ref = std::move(Other.Ref);
				Num = Other.Num;
				Str = std::move(Other.Str);
				Ptr = Other.Ptr;
				Bool = Other.Bool;
				LuaType = Other.LuaType;
				return *this;
			}

			~Var()
			{
			}

			void Clone(const Var& Other);
			void Push(lua_State* L) const;
		};

#ifdef _WIN32
#pragma warning(pop)
#endif
		
		int32_t Push() const;

		void Clear();

		void Clone(const ToLuauVar& Other);
		void Move(ToLuauVar&& Other);

		template<typename T>
		T Get()
		{
			Push();
			T Ret = StackAPI::Check<typename remove_cr<T>::type>(OwnerState, -1);
			lua_pop(OwnerState, 1);
			return std::move(Ret);
		}

		template<typename T>
		void Get(T& Result)
		{
			Result = Get<T>();
		}

		std::vector<Var> Vars;
		lua_State* OwnerState = nullptr;
	};

}

