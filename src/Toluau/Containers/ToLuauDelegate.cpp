#include "ToLuauDelegate.h"
#include "Toluau/API/StackAPI.h"
#include "Toluau/API/RegisterMacro.h"
#include "Toluau/ToLuau.h"

#ifndef SafeDelete
#define SafeDelete(ptr) if(ptr) { delete ptr;ptr=nullptr; }
#endif

#ifdef TOLUAUUNREAL_API

UToLuauDelegate::~UToLuauDelegate()
{
	SafeDelete(LuaFunc);
}

void UToLuauDelegate::EventTrigger()
{
	check(false);
}

void UToLuauDelegate::ProcessEvent(UFunction* Function, void* Parms)
{
	ensure(LuaFunc != nullptr && UFunc != nullptr);
	LuaFunc->CallByUFunction(UFunc, static_cast<uint8*>(Parms));
}

void UToLuauDelegate::BindFunction(lua_State* L, int32 Pos, UFunction* Func)
{
	luaL_checktype(L, Pos, LUA_TFUNCTION);
	ensure(Func);
	LuaFunc = new ToLuau::ToLuauVar(L, Pos, ToLuau::ToLuauVar::Type::Function);
	UFunc = Func;
}

void UToLuauDelegate::BindFunction(lua_State* L, int32 Pos)
{
	luaL_checktype(L, Pos, LUA_TFUNCTION);
	LuaFunc = new ToLuau::ToLuauVar(L, Pos, ToLuau::ToLuauVar::Type::Function);
}

void UToLuauDelegate::BindFunction(UFunction* Func)
{
	ensure(Func);
	UFunc = Func;
}

void UToLuauDelegate::Dispose()
{
	SafeDelete(LuaFunc);
	UFunc = nullptr;
}

#ifdef TOLUAUUNREAL_API



namespace ToLuau
{
	ToLuauMultiDelegate::ToLuauMultiDelegate(FMulticastScriptDelegate* InDelegate, UFunction* InUFunc, const FString& InDebugName)
		: Delegate(InDelegate)
		, UFunc(InUFunc)
#if ToLuauDebug
		, DebugName(InDebugName)
#endif
	{
	}

	int32 ToLuauMultiDelegate::Push(lua_State* L, FMulticastScriptDelegate* InDelegate, UFunction* InUFunc, const FString& InDebugName)
	{
		TSharedPtr<ToLuauMultiDelegate> MultiDelegate = MakeShared<ToLuauMultiDelegate>(InDelegate, InUFunc, InDebugName);
		return StackAPI::Push(L, MultiDelegate);
	}

	int32 ToLuauMultiDelegate::SetupMetatable(lua_State* L)
	{
		return 0;
	}

	int32 ToLuauMultiDelegate::Add(lua_State* L)
	{
		TSharedPtr<ToLuauMultiDelegate> ScriptDelegate = StackAPI::Check<TSharedPtr<ToLuauMultiDelegate>>(L, 1);

		auto LuaDelegate = NewObject<UToLuauDelegate>(GetTransientPackage(), UToLuauDelegate::StaticClass());

#if ToLuauDebug
		LuaDelegate->DebugName = ScriptDelegate->DebugName;
#endif
		LuaDelegate->BindFunction(L, 2, ScriptDelegate->UFunc);

		FScriptDelegate Delegate;
		Delegate.BindUFunction(LuaDelegate, TEXT("EventTrigger"));
		ScriptDelegate->Delegate->AddUnique(Delegate);

		ScriptDelegate->AddRef(LuaDelegate);

		lua_pushlightuserdata(L, LuaDelegate);
		return 1;
	}

	int32 ToLuauMultiDelegate::Remove(lua_State* L)
	{
		TSharedPtr<ToLuauMultiDelegate> ScriptDelegate = StackAPI::Check<TSharedPtr<ToLuauMultiDelegate>>(L, 1);
		if(!lua_islightuserdata(L,2))
			luaL_error(L,"arg 2 expect ULuaDelegate");
		
		auto LuaDelegate =  reinterpret_cast<UToLuauDelegate*>(lua_tolightuserdata(L,2));

		if(!IsValid(LuaDelegate))
		{
			luaL_error(L, "Invalid UToLuauDelegate!");
			return 0;
		}
		
		FScriptDelegate Delegate;
		Delegate.BindUFunction(LuaDelegate, TEXT("EventTrigger"));
		ScriptDelegate->Delegate->Remove(Delegate);

		ScriptDelegate->RemoveRef(LuaDelegate);

		LuaDelegate->Dispose();
		return 0;
	}

	int32 ToLuauMultiDelegate::Clear(lua_State* L)
	{
		TSharedPtr<ToLuauMultiDelegate> ScriptDelegate = StackAPI::Check<TSharedPtr<ToLuauMultiDelegate>>(L, 1);

		auto AllLuaDelegate = ScriptDelegate->Delegate->GetAllObjects();
		for (auto Delegate : AllLuaDelegate)
		{
			UToLuauDelegate* LuaDelegate = Cast<UToLuauDelegate>(Delegate);
			if(LuaDelegate)
			{
				LuaDelegate->Dispose();
				ScriptDelegate->RemoveRef(LuaDelegate);
			}
		}

		ScriptDelegate->Delegate->Clear();
		return 0;
	}

	void ToLuauMultiDelegate::AddRef(UToLuauDelegate* LuauDelegate)
	{
		LuauDelegateHolder.AddUnique(TStrongObjectPtr(LuauDelegate));
	}

	void ToLuauMultiDelegate::RemoveRef(UToLuauDelegate* LuauDelegate)
	{
		LuauDelegateHolder.RemoveAll([LuauDelegate](TStrongObjectPtr<UToLuauDelegate> InDelegate)
		{
			return InDelegate.Get() == LuauDelegate;
		});
	}

	LUAU_BEGIN_CLASS(ToLuauMultiDelegate)
	LUAU_CUSTOM_REG(ToLuauMultiDelegate, SetupMetatable)
	LUAU_REG_LUA_FUNC(ToLuauMultiDelegate, Add)
	LUAU_REG_LUA_FUNC(ToLuauMultiDelegate, Remove)
	LUAU_REG_LUA_FUNC(ToLuauMultiDelegate, Clear)
	LUAU_END_CLASS(ToLuauMultiDelegate)

	ToLuauDelegate::ToLuauDelegate(FScriptDelegate* InDelegate, UFunction* InUFunc, const FString& InDebugName)
		: Delegate(InDelegate)
		, UFunc(InUFunc)
#if ToLuauDebug
		, DebugName(InDebugName)
#endif
	{
	}

	int32 ToLuauDelegate::Push(lua_State* L, FScriptDelegate* InDelegate, UFunction* InUFunc, const FString& InDebugName)
	{
		TSharedPtr<ToLuauDelegate> ScriptDelegate = MakeShared<ToLuauDelegate>(InDelegate, InUFunc, InDebugName);
		return StackAPI::Push(L, ScriptDelegate);
	}

	int32 ToLuauDelegate::SetupMetatable(lua_State* L)
	{
		return 0;
	}

	int32 ToLuauDelegate::Bind(lua_State* L)
	{
		TSharedPtr<ToLuauDelegate> ScriptDelegate = StackAPI::Check<TSharedPtr<ToLuauDelegate>>(L, 1);
		if(ScriptDelegate)
		{
			auto Object = ScriptDelegate->Delegate->GetUObject();
			if(Object)
			{
				UToLuauDelegate* LuauDelegate = Cast<UToLuauDelegate>(Object);
				LuauDelegate->Dispose();
				ScriptDelegate->RemoveRef();
			}
		}

		{
			
			auto LuauDelegate = NewObject<UToLuauDelegate>(GetTransientPackage(), UToLuauDelegate::StaticClass());
#if ToLuauDebug
			LuauDelegate->DebugName = ScriptDelegate->DebugName;
#endif
			LuauDelegate->BindFunction(L, 2, ScriptDelegate->UFunc);

			ScriptDelegate->Delegate->BindUFunction(LuauDelegate, TEXT("EventTrigger"));

			ScriptDelegate->AddRef(LuauDelegate);

			lua_pushlightuserdata(L, LuauDelegate);
			return 1;
			
		}
		
	}

	int32 ToLuauDelegate::Clear(lua_State* L)
	{
		TSharedPtr<ToLuauDelegate> ScriptDelegate = StackAPI::Check<TSharedPtr<ToLuauDelegate>>(L, 1);
		if(ScriptDelegate)
		{
			auto Object = ScriptDelegate->Delegate->GetUObject();
			if(Object)
			{
				UToLuauDelegate* LuauDelegate = Cast<UToLuauDelegate>(Object);
				LuauDelegate->Dispose();
				ScriptDelegate->RemoveRef();
			}
		}
		ScriptDelegate->Delegate->Clear();
		return 0;
	}

	void ToLuauDelegate::AddRef(UToLuauDelegate* LuauDelegate)
	{
		LuauDelegateHolder.Reset(LuauDelegate);
	}

	void ToLuauDelegate::RemoveRef()
	{
		LuauDelegateHolder.Reset();
	}

	LUAU_BEGIN_CLASS(ToLuauDelegate)
	LUAU_CUSTOM_REG(ToLuauDelegate, SetupMetatable)
	LUAU_REG_LUA_FUNC(ToLuauDelegate, Bind)
	LUAU_REG_LUA_FUNC(ToLuauDelegate, Clear)
	LUAU_END_CLASS(ToLuauDelegate)

	LUAU_BEGIN_CLASS(FDelegateHandle)
	LUAU_REG_FUNC(FDelegateHandle, IsValid)
	LUAU_REG_FUNC(FDelegateHandle, Reset)
	LUAU_END_CLASS(FDelegateHandle)
	
}

#endif

#endif

#undef SafeDelete