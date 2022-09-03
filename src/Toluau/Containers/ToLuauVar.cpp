#include "ToLuauVar.h"
#include <cassert>
#include <cmath>
#include "Toluau/Util/Util.h"
#include "Toluau/API/ToLuauLib.h"



namespace ToLuau
{
	ToLuauVar::ToLuauVar()
	{
	}

	ToLuauVar::ToLuauVar(lua_Integer Value)
	{
		Set(Value);
	}

	ToLuauVar::ToLuauVar(size_t Value)
	{
		Set(static_cast<lua_Integer>(Value));
	}

	ToLuauVar::ToLuauVar(lua_Number Value)
	{
		Set(Value);
	}

	ToLuauVar::ToLuauVar(const char* Value, size_t Len)
	{
		Set(Value, Len);
	}

#ifdef TOLUAUUNREAL_API
	ToLuauVar::ToLuauVar(const FString& Value)
	{
		Set(Value);
	}
#endif

	ToLuauVar::ToLuauVar(const std::string& Value)
	{
		Set(Value);
	}

	ToLuauVar::ToLuauVar(bool Value)
	{
		Set(Value);
	}

	ToLuauVar::ToLuauVar(lua_State* L, int32_t Pos)
	{
		Set(L, Pos);
	}

	ToLuauVar::ToLuauVar(lua_State* L, int32_t Pos, Type T)
	{
		Init(L, Pos, T);
	}

	void ToLuauVar::Init(lua_State* L, int32_t Pos, Type T)
	{
		OwnerState = L;
		switch (T)
		{
		case Type::None:
			{
				break;
			}
		case Type::Number:
			{
				Set(lua_tonumber(L, Pos));
				break;
			}
		case Type::String:
			{
				size_t Len;
				const char* Buf = lua_tolstring(L, Pos, &Len);
				Set(Buf, Len);
				break;
			}
		case Type::Bool:
			{
				Set(!!lua_toboolean(L, Pos));
				break;
			}
		case Type::LightUserData:
			{
				Var NewVar;
				NewVar.Ptr = lua_tolightuserdata(L, Pos);
				NewVar.LuaType = T;
				Vars.push_back(NewVar);
				break;
			}
		case Type::Function:
		case Type::Table:
		case Type::UserData:
			{
				Var NewVar;
				NewVar.Ref = std::make_shared<LuaRef>(L, Pos);
				NewVar.LuaType = T;
				Vars.push_back(NewVar);
				break;
			}
		case Type::Tuple:
			{
				TOLUAU_ASSERT(Pos > 0 && lua_gettop(L) >= Pos);
				InitTuple(L, Pos);
				break;
			}
		default:
			break;
		}
	}

	void ToLuauVar::InitTuple(lua_State* L, int32_t Num)
	{
		auto Top = lua_gettop(L);
		TOLUAU_ASSERT(Top >= Num);
		int32_t First = Top - Num + 1;
		for(size_t i = 0; i < Num; i ++)
		{
			int32_t Pos = First + i;
			int32_t CurrentType = lua_type(L, Pos);

			switch (CurrentType)
			{
			case LUA_TBOOLEAN:
				{
					Var NewVar;
					NewVar.LuaType = Type::Bool;
					NewVar.Bool = !!lua_toboolean(L, Pos);
					Vars.push_back(NewVar);
					break;
				}
			case LUA_TNUMBER:
				{
					Var NewVar;
					NewVar.LuaType = Type::Number;
					NewVar.Num = lua_tonumber(L, Pos);
					Vars.push_back(NewVar);
					break;
				}
			case LUA_TSTRING:
				{
					Var NewVar;
					NewVar.LuaType = Type::String;
					size_t Len;
					auto Buf = lua_tolstring(L, Pos, &Len);
					NewVar.Str = std::make_shared<std::string>(Buf, Len);
					Vars.push_back(NewVar);
					break;
				}
			case LUA_TFUNCTION:
				{
					Var NewVar;
					NewVar.LuaType = Type::Function;
					NewVar.Ref = std::make_shared<LuaRef>(L, Pos);
					Vars.push_back(NewVar);
					break;
				}
			case LUA_TTABLE:
				{
					Var NewVar;
					NewVar.LuaType = Type::Table;
					NewVar.Ref = std::make_shared<LuaRef>(L, Pos);
					Vars.push_back(NewVar);
					break;
				}
			case LUA_TUSERDATA:
				{
					Var NewVar;
					NewVar.LuaType = Type::UserData;
					NewVar.Ref = std::make_shared<LuaRef>(L, Pos);
					Vars.push_back(NewVar);
					break;
					break;
				}
			case LUA_TLIGHTUSERDATA:
				{
					Var NewVar;
					NewVar.LuaType = Type::LightUserData;
					NewVar.Ptr = lua_tolightuserdata(L, Pos);
					Vars.push_back(NewVar);
					break;
				}
			case LUA_TNIL:
			default:
				{
					Var NewVar;
					NewVar.LuaType = Type::None;
					Vars.push_back(NewVar);
					break;
				}
			}
		}
	}

	void ToLuauVar::Set(lua_State* L, int32_t Pos)
	{
		Clear();
		auto T = lua_type(L, Pos);
		Type SelfType = Type::None;
		switch (T)
		{
		case LUA_TNUMBER:
			SelfType = Type::Number;
			break;
		case LUA_TSTRING:
			SelfType = Type::String;
			break;;
		case LUA_TFUNCTION:
			SelfType = Type::Function;
			break;
		case LUA_TTABLE:
			SelfType = Type::Table;
			break;
		case LUA_TUSERDATA:
			SelfType = Type::UserData;
			break;
		case LUA_TLIGHTUSERDATA:
			SelfType = Type::LightUserData;
			break;
		case LUA_TBOOLEAN:
			SelfType = Type::Bool;
			break;
		case LUA_TNIL:
			SelfType = Type::None;
			break;
		}
		Init(L, Pos, SelfType);
	}

	void ToLuauVar::Set(lua_Integer Value)
	{
		Clear();
		Var NewVar;
		NewVar.Num = Value;
		NewVar.LuaType = Type::Number;
		Vars.push_back(NewVar);
	}

	void ToLuauVar::Set(lua_Number Value)
	{
		Clear();
		Var NewVar;
		NewVar.Num = Value;
		NewVar.LuaType = Type::Number;
		Vars.push_back(NewVar);
	}

	void ToLuauVar::Set(const char* Value, size_t Len)
	{
		Clear();
		Var NewVar;
		NewVar.Str = std::make_shared<std::string>(Value, Len);
		NewVar.LuaType = Type::String;
		Vars.push_back(NewVar);
	}

	void ToLuauVar::Set(const std::string& Str)
	{
		Clear();
		Var NewVar;
		NewVar.Str = std::make_shared<std::string>(Str);
		NewVar.LuaType = Type::String;
		Vars.push_back(NewVar);
	}

#ifdef TOLUAUUNREAL_API
	void ToLuauVar::Set(const FString& Value)
	{
		Clear();
		Var NewVar;
		NewVar.Str = std::make_shared<std::string>(StringEx::FStringToStdString(Value));
		NewVar.LuaType = Type::String;
		Vars.push_back(NewVar);
	}
#endif

	void ToLuauVar::Set(bool Bool)
	{
		Clear();
		Var NewVar;
		NewVar.Bool = Bool;
		NewVar.LuaType = Type::Bool;
		Vars.push_back(NewVar);
	}

	bool ToLuauVar::IsNil() const
	{
		return Vars.empty() || Vars[0].LuaType == Type::None;
	}

	bool ToLuauVar::IsFunction() const
	{
		return Vars.size() == 1 && Vars[0].LuaType == Type::Function;
	}

	bool ToLuauVar::IsTuple() const
	{
		return Vars.size() > 1;
	}

	bool ToLuauVar::IsTable() const
	{
		return Vars.size() == 1 && Vars[0].LuaType == Type::Table;
	}

	bool ToLuauVar::IsNumber() const
	{
		return Vars.size() == 1 && Vars[0].LuaType == Type::Number;
	}

	bool ToLuauVar::IsString() const
	{
		return Vars.size() == 1 && Vars[0].LuaType == Type::String;
	}

	bool ToLuauVar::IsBool() const
	{
		return Vars.size() == 1 && Vars[0].LuaType == Type::Bool;
	}

	bool ToLuauVar::IsUserData() const
	{
		if(Vars.size() == 1 && Vars[0].LuaType == Type::UserData)
		{
			// TODO UserData Cannot be nullptr
			return true;
		}
		return false;
	}

	bool ToLuauVar::IsLightUserData() const
	{
		return Vars.size() == 1 && Vars[0].LuaType == Type::LightUserData;
	}

	bool ToLuauVar::IsValid() const
	{
		return Vars.size() > 0 && OwnerState != nullptr;
	}

	enum ToLuauVar::Type ToLuauVar::GetType() const
	{
		if(Vars.size() == 0)
		{
			return Type::None;
		}
		else if(Vars.size() > 1)
		{
			return Type::Tuple;
		}
		return Vars[0].LuaType;
	}

	int32_t ToLuauVar::ToInt() const
	{
		TOLUAU_ASSERT(Vars.size() == 1);
		switch (Vars[0].LuaType)
		{
		case Type::Number:
			return Vars[0].Num;
		case Type::None:
			return 0;
		case Type::Bool:
			return Vars[0].Bool ? 1 : 0;
		default:
			return -1;
		}
	}

	float ToLuauVar::ToFloat() const
	{
		TOLUAU_ASSERT(Vars.size() == 1);
		switch (Vars[0].LuaType)
		{
		case Type::Number:
			return Vars[0].Num;
		case Type::None:
			return 0;
		case Type::Bool:
			return Vars[0].Bool ? 1 : 0;
		default:
			return NAN;
		}
	}

	double ToLuauVar::ToDouble() const
	{
		TOLUAU_ASSERT(Vars.size() == 1);
		switch (Vars[0].LuaType)
		{
		case Type::Number:
			return Vars[0].Num;
		case Type::None:
			return 0;
		case Type::Bool:
			return Vars[0].Bool ? 1 : 0;
		default:
			return NAN;
		}
		return 0;
	}

	std::string ToLuauVar::ToStdString() const
	{
		TOLUAU_ASSERT(Vars.size() == 1 && Vars[0].LuaType == Type::String);
		return *Vars[0].Str;
	}

#ifdef TOLUAUUNREAL_API
	FString ToLuauVar::ToFString() const
	{
		TOLUAU_ASSERT(Vars.size() == 1 && Vars[0].LuaType == Type::String);
		return StringEx::StdStringToFString(*Vars[0].Str);
	}
#endif

	bool ToLuauVar::ToBool() const
	{
		TOLUAU_ASSERT(Vars.size() == 1 && Vars[0].LuaType == Type::Bool);
		return Vars[0].Bool;
	}

	void* ToLuauVar::ToLightUserData() const
	{
		TOLUAU_ASSERT(Vars.size() == 1 && Vars[0].LuaType == Type::LightUserData);
		return Vars[0].Ptr;
	}

	size_t ToLuauVar::Num() const
	{
		if(IsTable())
		{
			auto R = Push();
			size_t N = lua_objlen(OwnerState, -1);
			lua_pop(OwnerState, 1);
			return N;
		}
		return Vars.size();
	}

	ToLuauVar ToLuauVar::GetAt(size_t Index) const
	{
		if(IsTable())
		{
			Push();
			lua_pushinteger(OwnerState, Index);
			lua_gettable(OwnerState, -2);
			ToLuauVar Result(OwnerState, -1);
			lua_pop(OwnerState, 2);
			return Result;
		}
		else
		{
			TOLUAU_ASSERT(Index > 0);
			TOLUAU_ASSERT(Index < Vars.size());
			ToLuauVar Result;
			Var NewVar;
			NewVar.Clone(Vars[0]);
			Result.Vars.push_back(NewVar);
			return Result;
		}
	}

	int32_t ToLuauVar::DoCall(int32_t Argn) const
	{
		if(!IsValid())
		{
			LUAU_LOG("lua function is invalid");
		}
		auto L = OwnerState;
		int32_t Buttom = lua_gettop(L) - Argn + 1; // args...

		lua_getref(L, TOLUAU_ERROR_HANDLER_REF); // args... errhandle
		lua_insert(L, Buttom); // errhandle args...
		Vars[0].Ref->Push(); // errhandle args... func
		
		{
			lua_insert(L, Buttom+1); // errhandle func args...
			if(lua_pcall(L, Argn, LUA_MULTRET, Buttom) != LUA_OK)
			{
				lua_pop(L, 1); //
			}
			// errhandle rets...
			lua_remove(L, Buttom); // rets... (remove err handle)
		}
		return lua_gettop(L) - Buttom + 1;
	}

#ifdef TOLUAUUNREAL_API
	int ToLuauVar::PushArgByParams(FProperty* Prop, uint8* Parms) const
	{
		if(StackAPI::UE::PushProperty(OwnerState, Prop, Parms))
		{
			return Prop->ElementSize;
		}
		return 0;
	}

	bool ToLuauVar::CallByUFunction(UFunction* UFunc, uint8* Params, FOutParmRec* OutParams,
	                                void* const Z_Param__Result, bool bNeedSelf)
	{
		auto Self = bNeedSelf ? const_cast<ToLuauVar*>(this) : nullptr;
		if(!UFunc)
		{
			return false;
		}

		if(!IsValid())
		{
			LUAU_ERROR("calling invalid ufunction");
			return false;
		}

		const bool bHasReturnParam = UFunc->ReturnValueOffset != MAX_uint16;
		if(UFunc->ParmsSize == 0 && !bHasReturnParam)
		{
			int32 NArg = 0;
			if(Self)
			{
				Push();
				NArg++;
			}

			int32 N = DoCall(NArg);
			lua_pop(OwnerState, N);
		}

		int32 N = 0;
		if(Self)
		{
			Push();
			N++;
		}

		static auto IsRealOutParam = [](int64 PropFlag)
		{
			return ((PropFlag & CPF_OutParm) && !(PropFlag & CPF_ConstParm) && !(PropFlag & CPF_BlueprintReadOnly));
		};
		
		// push arguments to lua state
		for(TFieldIterator<FProperty> It(UFunc); It && (It->PropertyFlags&CPF_Parm);++It)
		{
			FProperty* Prop = *It;
			uint64 propflag = Prop->GetPropertyFlags();
			if (UFunc->HasAnyFunctionFlags(FUNC_Native))
			{
				if ((propflag & CPF_ReturnParm))
				{
					continue;
				}
			}
			else if (IsRealOutParam(propflag))
			{
				continue;
			}

			PushArgByParams(Prop, Params+Prop->GetOffset_ForInternal());
			N++;
		}

		int RetCount = DoCall(N);
		int Remain = RetCount;
		// if lua return value
		// we only handle first lua return value
		if(Remain > 0 && bHasReturnParam)
		{
			auto Prop = UFunc->GetReturnProperty();
			StackAPI::UE::CheckProperty(OwnerState, Prop, Params + Prop->GetOffset_ForInternal(), lua_absindex(OwnerState, -Remain));
			if (RESULT_PARAM)
			{
				Prop->CopyCompleteValue(RESULT_PARAM, Prop->ContainerPtrToValuePtr<uint8>(Params));
			}
			Remain--;
		}

		// fill lua return value to blueprint stack if argument is out param
		for (TFieldIterator<FProperty> It(UFunc); Remain > 0 && It && (It->PropertyFlags & CPF_Parm); ++It)
		{
			FProperty* Prop = *It;
			uint64 PropFlag = Prop->GetPropertyFlags();
			if (IsRealOutParam(PropFlag))
			{
				FOutParmRec* Out = OutParams;
				while (Out && Out->Property != Prop)
				{
					Out = Out->NextOutParm;
				}
				uint8* outParam = Out ? Out->PropAddr : Params + Prop->GetOffset_ForInternal();
				StackAPI::UE::CheckProperty(OwnerState, Prop, outParam, lua_absindex(OwnerState, -Remain));
				Remain--;
			}
		}
		// pop returned value
		lua_pop(OwnerState, RetCount);
		return true;
	}
#endif

	void ToLuauVar::Var::Clone(const Var& Other)
	{
		switch ( Other.LuaType)
		{
		case Type::Number:
			Num = Other.Num;
			break;
		case Type::Bool:
			Bool = Other.Bool;
			break;
		case Type::String:
			Str = Other.Str;
			break;
		case Type::Function:
		case Type::UserData:
		case Type::Table:
			Ref = Other.Ref;
			break;
		case Type::LightUserData:
			Ptr = Other.Ptr;
			break;
		case Type::None:
		case Type::Tuple:
			break;
		default: ;
		}
		LuaType = Other.LuaType;
	}

	void ToLuauVar::Var::Push(lua_State* L) const
	{
		switch (LuaType)
		{
		case Type::Number:
			lua_pushnumber(L, Num);
			break;
		case Type::Bool:
			lua_pushboolean(L, Bool);
			break;
		case Type::String:
			lua_pushlstring(L, Str->c_str(), Str->size());
			break;
		case Type::Function:
		case Type::UserData:
		case Type::Table:
			Ref->Push();
			break;
		case Type::LightUserData:
			lua_pushlightuserdata(L, Ptr);
			break;
		default:
			lua_pushnil(L);
			break;
		}
	}

	int32_t ToLuauVar::Push() const
	{
		if(OwnerState == nullptr)
		{
			return 0;
		}
		auto Size = Vars.size();
		if(Size == 0)
		{
			lua_pushnil(OwnerState);
			return 1;
		}

		if(Size == 1)
		{
			Vars[0].Push(OwnerState);
			return 1;
		}

		for(size_t i = 0; i < Vars.size(); i ++)
		{
			Vars[i].Push(OwnerState);
		}

		return Size;
		
	}

	void ToLuauVar::Clear()
	{
		Vars.clear();
	}

	void ToLuauVar::Clone(const ToLuauVar& Other)
	{
		Clear();
		OwnerState = Other.OwnerState;
		for (auto& Value : Other.Vars)
		{
			Var NewVar;
			NewVar.Clone(Value);
			Vars.push_back(NewVar);
		}
	}

	void ToLuauVar::Move(ToLuauVar&& Other)
	{
		OwnerState = Other.OwnerState;
		Vars = std::move(Other.Vars);
		
		Other.OwnerState = nullptr;
	}
}

