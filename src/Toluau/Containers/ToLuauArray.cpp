#include "ToLuauArray.h"


#ifdef TOLUAUUNREAL_API

#include <cassert>
#include "API/RegisterMacro.h"
#include "PropertyUtils.h"
#include "ToLuau.h"
#include "ToLuauVar.h"

#ifndef SafeDelete
#define SafeDelete(ptr) if(ptr) { delete ptr;ptr=nullptr; }
#endif

namespace ToLuau
{
	void ToLuauArray::Clone(FScriptArray* DestArray, FProperty* P, const FScriptArray* SrcArray)
	{
		if(!SrcArray || SrcArray->Num() == 0)
		{
			return;
		}

		FScriptArrayHelper Helper = FScriptArrayHelper::CreateHelperFormInnerProperty(P, DestArray);
		Helper.AddValues(SrcArray->Num());
		uint8* Dest = Helper.GetRawPtr();
		uint8* Src = (uint8*)SrcArray->GetData();
		for(int i = 0; i < SrcArray->Num(); i++)
		{
			P->CopySingleValue(Dest, Src);
			Dest += P->ElementSize;
			Src += P->ElementSize;
		}
	}

	int ToLuauArray::Push(lua_State* L, FProperty* InProp, FScriptArray* InArray)
	{
		TSharedPtr<ToLuauArray> LuaArray = MakeShared<ToLuauArray>(InProp, InArray);
		return StackAPI::Push<TSharedPtr<ToLuauArray>>(L, LuaArray);
	}

	int ToLuauArray::Push(lua_State* L, FArrayProperty* InProp, UObject* InObj)
	{
		TSharedPtr<ToLuauArray> LuaArray = MakeShared<ToLuauArray>(InProp, InObj);
		return StackAPI::Push<TSharedPtr<ToLuauArray>>(L, LuaArray);
	}

	ToLuauArray::ToLuauArray(FProperty* InProp, FScriptArray* InBuf)
		: Inner(InProp)
		, Prop(nullptr)
		, OwnerObj(nullptr)
	{
		Array = new FScriptArray();
		Clone(Array, InProp, InBuf);
		ShouldFree = true;
	}

	ToLuauArray::ToLuauArray(FArrayProperty* InProp, UObject* Obj)
		: Inner(InProp->Inner)
		, Prop(InProp)
		, OwnerObj(Obj)
	{
		Array = Prop->ContainerPtrToValuePtr<FScriptArray>(Obj);
		ShouldFree = false;
	}

	ToLuauArray::~ToLuauArray()
	{
		if(ShouldFree)
		{
			Clear();
			assert(Array);
			SafeDelete(Array);
		}

		Inner = nullptr;
		OwnerObj = nullptr;
	}

	void ToLuauArray::AddReferencedObjects(FReferenceCollector& Collector)
	{
		// if(Inner)
		// {
		// 	Collector.AddReferencedObject(Inner);
		// }
		// if(Prop)
		// {
		// 	Collector.AddReferencedObject(Prop);
		// }
		// if(OwnerObj)
		// {
		// 	Collector.AddReferencedObject(OwnerObj);
		// }
	}

	int32 ToLuauArray::SetupMetaTable(lua_State* L)
	{
		
		lua_pushcfunction(L, &ToLuauArray::Pairs, "__pairs");
		lua_setfield(L, -2, "__pairs");
		
		return 0;
	}

	int ToLuauArray::Ctor(lua_State* L)
	{
		auto Type = StackAPI::Check<PropertyType>(L, 1);
		auto Class = StackAPI::CheckOptional<UClass*>(L, 2, nullptr);
		if(Type == PropertyType::Object && !Class)
		{
			luaL_errorL(L, "Array of object should have second parameter is UClass");
		}
		auto NewArray = FScriptArray();
		PropertyProto Proto(Type, Class);
		return Push(L, PropertyProto::CreateProperty(Proto), &NewArray);
	}

	int32 ToLuauArray::Pairs(lua_State* L)
	{
		auto Array = StackAPI::Check<TSharedPtr<ToLuauArray>>(L, 1);
		auto Iter = MakeShared<ToLuauArrayEnumerator>();
		Iter->Holder = new ToLuauVar(L, 1);
		Iter->Owner = Array;
		Iter->Index = 0;
		lua_pushcfunction(L, &ToLuauArray::Enumerable, "ToLuauArray::Enumerable");
		StackAPI::Push<TSharedPtr<ToLuauArrayEnumerator>>(L, Iter);
		lua_pushnil(L);
		return 3;
	}

	int32 ToLuauArray::Enumerable(lua_State* L)
	{
		auto Iter = StackAPI::Check<TSharedPtr<ToLuauArrayEnumerator>>(L, 1);
		auto Array = Iter->Owner;
		if(Array->IsValidIndex(Iter->Index))
		{
			auto Ele = Array->Inner;
			auto EleSize = Ele->ElementSize;
			auto Params = ((uint8*)Array->Array->GetData()) + Iter->Index * EleSize;
			StackAPI::Push(L, Iter->Index);
			StackAPI::UE::PushProperty(L, Ele, Params);
			Iter->Index += 1;
			return 2;
		}
		return 0;
	}

	int32 ToLuauArray::Num(lua_State* L)
	{
		auto UD = StackAPI::Check<TSharedPtr<ToLuauArray>>(L, 1);
		return StackAPI::Push<int32>(L, UD->Num());
	}

	int32 ToLuauArray::Get(lua_State* L)
	{
		auto UD = StackAPI::Check<TSharedPtr<ToLuauArray>>(L, 1);
		int32 Index = StackAPI::Check<int32>(L, 2);
		FProperty* Ele = UD->Inner;
		if(!UD->IsValidIndex(Index))
		{
			luaL_errorL(L, "array get %d, out of range", Index);
			return 0;
		}
		return StackAPI::UE::PushProperty(L, Ele, UD->GetRawPtr(Index));
	}

	int32 ToLuauArray::Set(lua_State* L)
	{
		auto UD = StackAPI::Check<TSharedPtr<ToLuauArray>>(L, 1);
		int32 Index = StackAPI::Check<int32>(L, 2);
		FProperty* Ele = UD->Inner;
		if(!UD->IsValidIndex(Index))
		{
			luaL_errorL(L, "array set %d, out of range", Index);
			return 0;
		}
		StackAPI::UE::CheckProperty(L, Ele, UD->GetRawPtr(Index), 3);
		return 0;
	}

	int32 ToLuauArray::Insert(lua_State* L)
	{
		auto UD = StackAPI::Check<TSharedPtr<ToLuauArray>>(L, 1);
		int32 Index = StackAPI::Check<int32>(L, 2);
		FProperty* Ele = UD->Inner;
		if(!UD->IsValidIndex(Index) && Index != UD->Num())
		{
			luaL_errorL(L, "array insert %d, out of range", Index);
			return 0;
		}
		StackAPI::UE::CheckProperty(L, Ele, UD->Insert(Index), 3);
		return StackAPI::Push(L, UD->Num());
	}

	int32 ToLuauArray::Remove(lua_State* L)
	{
		auto UD = StackAPI::Check<TSharedPtr<ToLuauArray>>(L, 1);
		int32 Index = StackAPI::Check<int32>(L, 2);
		if(!UD->IsValidIndex(Index))
		{
			luaL_errorL(L, "array remove %d, out of range", Index);
			return 0;
		}
		UD->Remove(Index);
		return 0;
	}

	int32 ToLuauArray::Clear(lua_State* L)
	{
		auto UD = StackAPI::Check<ToLuauArray*>(L, 1);
		UD->Clear();
		return 0;
	}

	void ToLuauArray::Clear()
	{
		if(!Inner)
		{
			return;
		}
		if(ShouldFree)
		{
			uint8* Dest = GetRawPtr(0);
			for(int32 i = 0; i < Array->Num(); i++, Dest += Inner->ElementSize)
			{
				Inner->DestroyValue(Dest);
			}
		}
		Array->Empty(0, Inner->ElementSize);
	}

	uint8* ToLuauArray::GetRawPtr(int32 Index) const
	{
		return (uint8*)Array->GetData() + Index * Inner->ElementSize;
	}

	bool ToLuauArray::IsValidIndex(int32 Index) const
	{
		return Index >= 0 && Index < Num();
	}

	uint8* ToLuauArray::Insert(int32 Index)
	{
		Array->Insert(Index, 1, Inner->ElementSize);
		ConstructItems(Index, 1);
		return GetRawPtr(Index);
	}

	uint8* ToLuauArray::Add()
	{
		auto Index = Array->Add(1, Inner->ElementSize);
		ConstructItems(Index, 1);
		return GetRawPtr(Index);
	}

	void ToLuauArray::Remove(int32 Index)
	{
		DestructItems(Index, 1);
		Array->Remove(Index, 1, Inner->ElementSize);
	}

	int32 ToLuauArray::Num() const
	{
		return Array->Num();
	}

	void ToLuauArray::ConstructItems(int32 Index, int32 Count)
	{
		uint8* Dest = GetRawPtr(Index);
		if(Inner->PropertyFlags | CPF_ZeroConstructor)
		{
			FMemory::Memzero(Dest, Count * Inner->ElementSize);
		}
		else
		{
			for(int32 i = 0; i < Count; i ++, Dest += Inner->ElementSize)
			{
				Inner->InitializeValue(Dest);
			}
		}
	}

	void ToLuauArray::DestructItems(int32 Index, int32 Count)
	{
		if(!ShouldFree)
		{
			return;
		}
		if(!(Inner->PropertyFlags & (CPF_IsPlainOldData | CPF_NoDestructor)))
		{
			uint8* Dest = GetRawPtr(Index);
			for(int32 i = 0; i < Count; i ++, Dest += Inner->ElementSize)
			{
				Inner->DestroyValue(Dest);
			}
		}
	}

	ToLuauArrayEnumerator::~ToLuauArrayEnumerator()
	{
		SafeDelete(Holder);
	}

	LUAU_BEGIN_CLASS(ToLuauArray)
	LUAU_CUSTOM_REG(ToLuauArray, SetupMetaTable)
	LUAU_REG_LUA_NEW_FUNC(ToLuauArray, Ctor);
	LUAU_REG_LUA_FUNC(ToLuauArray, Pairs);
	LUAU_REG_LUA_FUNC(ToLuauArray, Num);
	LUAU_REG_LUA_FUNC(ToLuauArray, Get);
	LUAU_REG_LUA_FUNC(ToLuauArray, Set);
	LUAU_REG_LUA_FUNC(ToLuauArray, Insert);
	LUAU_REG_LUA_FUNC(ToLuauArray, Remove);
	LUAU_REG_LUA_FUNC(ToLuauArray, Clear);
	LUAU_END_CLASS(ToLuauArray)

	LUAU_BEGIN_CLASS(ToLuauArrayEnumerator)
	LUAU_END_CLASS(ToLuauArrayEnumerator)
	
}

#undef SafeDelete

#endif