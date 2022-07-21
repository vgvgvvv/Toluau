#include "ToLuauMap.h"



#ifdef TOLUAUUNREAL_API

#include <cassert>
#include "ToLuauVar.h"
#include "ToLuau.h"
#include "API/RegisterMacro.h"

#ifndef SafeDelete
#define SafeDelete(ptr) if(ptr) { delete ptr;ptr=nullptr; }
#endif

namespace ToLuau
{
	int ToLuauMap::Push(lua_State* L, FProperty* keyProp, FProperty* valueProp, const FScriptMap* buf, bool frombp)
	{
		TSharedPtr<ToLuauMap> luaMap = MakeShared<ToLuauMap>(keyProp, valueProp, buf, frombp);
		return StackAPI::Push(L, luaMap);
	}

	int ToLuauMap::Push(lua_State* L, FMapProperty* InProp, UObject* InObj)
	{
		TSharedPtr<ToLuauMap> NewMap = MakeShared<ToLuauMap>(InProp,InObj);
		return StackAPI::Push(L, NewMap);
	}

	void ToLuauMap::Clone(FScriptMap* Dest, FProperty* KeyProp, FProperty* ValueProp, const FScriptMap* Src)
	{
		if(!Src || Src->Num() == 0)
		{
			return;
		}

		FScriptMapHelper DstHelper = FScriptMapHelper::CreateHelperFormInnerProperties(KeyProp, ValueProp, Dest);
		FScriptMapHelper SrcHelper = FScriptMapHelper::CreateHelperFormInnerProperties(KeyProp, ValueProp, Src);
		for(int32 i = 0; i < SrcHelper.GetMaxIndex(); i ++)
		{
			if(SrcHelper.IsValidIndex(i))
			{
				auto KeyPtr = SrcHelper.GetKeyPtr(i);
				auto ValuePtr = SrcHelper.GetValuePtr(i);
				DstHelper.AddPair(KeyPtr, ValuePtr);
			}
		}
		
	}

	ToLuauMap::ToLuauMap(FProperty* keyProp, FProperty* valueProp, const FScriptMap* Buf, bool FromBP)
		: Map(new FScriptMap)
		, KeyProp(keyProp)
		, ValueProp(valueProp)
		, Prop(nullptr)
		, PropObj(nullptr)
		, Helper(FScriptMapHelper::CreateHelperFormInnerProperties(keyProp, valueProp, Map))
	{
		if(Buf)
		{
			Clone(Map, keyProp, valueProp, Buf);
			CreatedByBp = FromBP;
		}
		else
		{
			CreatedByBp = false;
		}
		ShouldFree = true;
	}

	ToLuauMap::ToLuauMap(FMapProperty* Prop, UObject* Obj)
		: Map(Prop->ContainerPtrToValuePtr<FScriptMap>(Obj))
		, KeyProp(Prop->KeyProp)
		, ValueProp(Prop->ValueProp)
		, Prop(Prop)
		, PropObj(Obj)
		, Helper(Prop, Map)
		, CreatedByBp(false)
		, ShouldFree(false)
	{
	}

	ToLuauMap::~ToLuauMap()
	{
		if(ShouldFree)
		{
			Clear();
			assert(Map);
			SafeDelete(Map);
		}
		KeyProp = ValueProp = nullptr;
		Prop = nullptr;
		PropObj = nullptr;
	}

	void ToLuauMap::AddReferencedObjects(FReferenceCollector& Collector)
	{
		// if(KeyProp)
		// {
		// 	Collector.AddReferencedObject(KeyProp);
		// }
		// if(ValueProp)
		// {
		// 	Collector.AddReferencedObject(ValueProp);
		// }
		// if(Prop)
		// {
		// 	Collector.AddReferencedObject(Prop);
		// }
		// if(PropObj)
		// {
		// 	Collector.AddReferencedObject(PropObj);
		// }
	}


	int32 ToLuauMap::SetupMetaTable(lua_State* L)
	{
		lua_pushcfunction(L, &ToLuauArray::Pairs, "__pairs");
		lua_setfield(L, -2, "__pairs");
		
		return 0;
	}

	int32 ToLuauMap::Ctor(lua_State* L)
	{
		auto KeyType = StackAPI::Check<PropertyType>(L, 1);
		auto ValueType = StackAPI::Check<PropertyType>(L, 2);
		auto KeyClass = StackAPI::CheckOptional<UClass*>(L, 3, nullptr);
		auto ValueClass =StackAPI::CheckOptional<UClass*>(L, 4, nullptr);
		if (KeyType == PropertyType::Object && !KeyClass)
			luaL_error(L, "UObject key should have 3rd parameter is UClass");
		if (ValueType == PropertyType::Object && !ValueClass)
			luaL_error(L, "UObject value should have 4th parameter is UClass");

		auto keyProp = PropertyProto::CreateProperty({ KeyType,KeyClass });
		auto valueProp = PropertyProto::CreateProperty({ ValueType,ValueClass });
		return Push(L, keyProp, valueProp, nullptr);
	}

	int32 ToLuauMap::Num(lua_State* L)
	{
		auto UD = StackAPI::Check<TSharedPtr<ToLuauMap>>(L, 1);
		check(UD);
		return StackAPI::Push(L, UD->Num());
	}

	int32 ToLuauMap::Get(lua_State* L)
	{
		auto UD = StackAPI::Check<TSharedPtr<ToLuauMap>>(L, 1);
		check(UD);
		FDefaultConstructedPropertyElement TempKey(UD->KeyProp);
		auto KeyPtr = TempKey.GetObjAddress();
		StackAPI::UE::CheckProperty(L, UD->KeyProp, (uint8*)KeyPtr, 2);

		auto ValuePtr = UD->Helper.FindValueFromHash(KeyPtr);
		if(ValuePtr)
		{
			StackAPI::UE::PushProperty(L, UD->ValueProp, ValuePtr);
		}
		else
		{
			lua_pushnil(L);
		}
		return 1;
	}

	int32 ToLuauMap::Add(lua_State* L)
	{
		auto UD = StackAPI::Check<TSharedPtr<ToLuauMap>>(L, 1);
		check(UD);
		FDefaultConstructedPropertyElement TempKey(UD->KeyProp);
		FDefaultConstructedPropertyElement TempValue(UD->ValueProp);
		auto KeyPtr = TempKey.GetObjAddress();
		auto ValuePtr = TempValue.GetObjAddress();
		StackAPI::UE::CheckProperty(L, UD->KeyProp, (uint8*)KeyPtr, 2);
		StackAPI::UE::CheckProperty(L, UD->ValueProp, (uint8*)ValuePtr, 3);
		UD->Helper.AddPair(KeyPtr, ValuePtr);
		return 0;
	}

	int32 ToLuauMap::Remove(lua_State* L)
	{
		auto UD = StackAPI::Check<TSharedPtr<ToLuauMap>>(L, 1);
		check(UD);
		FDefaultConstructedPropertyElement TempKey(UD->KeyProp);
		auto KeyPtr = TempKey.GetObjAddress();
		StackAPI::UE::CheckProperty(L, UD->KeyProp, (uint8*)KeyPtr, 2);
		return StackAPI::Push(L, UD->RemovePair(KeyPtr));
	}

	int32 ToLuauMap::Clear(lua_State* L)
	{
		auto UD = StackAPI::Check<TSharedPtr<ToLuauMap>>(L, 1);
		check(UD);
		UD->Clear();
		return 0;
	}

	int32 ToLuauMap::Pairs(lua_State* L)
	{
		auto UD = StackAPI::Check<TSharedPtr<ToLuauMap>>(L, 1);
		check(UD);
		TSharedPtr<ToLuauMapEnumerator> Iter = MakeShared<ToLuauMapEnumerator>();
		Iter->Holder = new ToLuauVar(L, 1);
		Iter->Owner = UD;
		Iter->Index = 0;
		Iter->Num = UD->Helper.Num();
		lua_pushcfunction(L, &ToLuauMap::Enumerable, "ToLuauMap::Enumerable");
		StackAPI::Push(L, Iter);
		StackAPI::Push(L, nullptr);
		return 3;
	}

	int32 ToLuauMap::Enumerable(lua_State* L)
	{
		auto Iter = StackAPI::Check<TSharedPtr<ToLuauMapEnumerator>>(L, 1);
		auto Map = Iter->Owner;
		auto& Helper = Map->Helper;
		do
		{
			if(Iter->Num <= 0)
			{
				return 0;
			}
			else if(Helper.IsValidIndex(Iter->Index))
			{
				auto PairPtr = Helper.GetPairPtr(Iter->Index);
				auto KeyPtr = Map->GetKeyPtr(PairPtr);
				auto ValuePtr = Map->GetValuePtr(PairPtr);
				StackAPI::UE::PushProperty(L, Map->KeyProp, KeyPtr);
				StackAPI::UE::PushProperty(L, Map->ValueProp, ValuePtr);
				Iter->Index += 1;
				Iter->Num -= 1;
				return 2;
			}
			else
			{
				Iter->Index += 1;
			}
		}
		while (true);
	}

	uint8* ToLuauMap::GetKeyPtr(uint8* PairPtr)
	{
		return PairPtr;
	}

	uint8* ToLuauMap::GetValuePtr(uint8* PairPtr)
	{
		return PairPtr + Helper.MapLayout.ValueOffset;
	}

	void ToLuauMap::Clear()
	{
		if(!KeyProp || !ValueProp)
			return;
		EmptyValues();
	}

	int32 ToLuauMap::Num() const
	{
		return Helper.Num();
	}

	void ToLuauMap::EmptyValues(int32 Slack)
	{
		checkSlow(Slack >= 0)
		int32 OldNum = Num();
		if(ShouldFree)
		{
			if(OldNum)
			{
				DestructItems(0, OldNum);
			}
		}
		if(OldNum || Slack)
		{
			Map->Empty(Slack, Helper.MapLayout);
		}
	}

	void ToLuauMap::DestructItems(int32 Index, int32 Count)
	{
		check(Index >= 0);
		check(Count >= 0);

		if(Count == 0)
		{
			return;
		}

		bool bDestroyKeys = !(KeyProp->PropertyFlags & (CPF_IsPlainOldData | CPF_NoDestructor));
		bool bDestroyValues = !(ValueProp->PropertyFlags & (CPF_IsPlainOldData | CPF_NoDestructor));
		
		if (bDestroyKeys || bDestroyValues) {
			uint32 Stride = Helper.MapLayout.SetLayout.Size;
			uint8* PairPtr = (uint8*)Map->GetData(Index, Helper.MapLayout);
			DestructItems(PairPtr, Stride, Index, Count, bDestroyKeys, bDestroyValues);
		}
	}

	void ToLuauMap::DestructItems(uint8* PairPtr, uint32 Stride, int32 Index, int32 Count, bool bDestroyKeys,
		bool bDestroyValues)
	{
		if(!ShouldFree)
		{
			return;
		}
		auto ValueOffset = CreatedByBp ? 0 : Helper.MapLayout.ValueOffset;
		for(;Count; ++Index)
		{
			if (Helper.IsValidIndex(Index)) {
				if (bDestroyKeys) {
					KeyProp->DestroyValue_InContainer(PairPtr);
				}
				if (bDestroyValues) {
					ValueProp->DestroyValue_InContainer(PairPtr + ValueOffset);
				}
				--Count;
			}
			PairPtr += Stride;
		}
	}

	bool ToLuauMap::RemovePair(const void* KeyPtr)
	{
		FProperty* LocalKeyPropForCapture = KeyProp;
		auto GetKeyHash = [LocalKeyPropForCapture](const void* ElementKey) { return LocalKeyPropForCapture->GetValueTypeHash(ElementKey); };
		auto KeyEqualityFn = [LocalKeyPropForCapture](const void* A, const void* B) { return LocalKeyPropForCapture->Identical(A, B); };
		if(uint8* Entry = Map->FindValue(KeyPtr, Helper.MapLayout, GetKeyHash, KeyEqualityFn))
		{
			int32 Idx = (Entry - (uint8*)Map->GetData(0, Helper.MapLayout)) / Helper.MapLayout.SetLayout.Size;
			RemoveAt(Idx);
			return true;
		}
		else
		{
			return false;
		}
	}

	void ToLuauMap::RemoveAt(int32 Index, int32 Count)
	{
		check(Helper.IsValidIndex(Index));
		DestructItems(Index, Count);
		for (; Count; ++Index)
		{
			if (Helper.IsValidIndex(Index))
			{
				Map->RemoveAt(Index, Helper.MapLayout);
				--Count;
			}
		}
	}

	ToLuauMapEnumerator::~ToLuauMapEnumerator()
	{
		SafeDelete(Holder);
	}


	LUAU_BEGIN_CLASS(ToLuauMap)
	LUAU_CUSTOM_REG(ToLuauMap, SetupMetaTable);
	LUAU_REG_LUA_NEW_FUNC(ToLuauMap, Ctor);
	LUAU_REG_LUA_FUNC(ToLuauMap, Num);
	LUAU_REG_LUA_FUNC(ToLuauMap, Get);
	LUAU_REG_LUA_FUNC(ToLuauMap, Add);
	LUAU_REG_LUA_FUNC(ToLuauMap, Remove);
	LUAU_REG_LUA_FUNC(ToLuauMap, Clear);
	LUAU_REG_LUA_FUNC(ToLuauMap, Pairs);
	LUAU_END_CLASS(ToLuauMap)

	LUAU_BEGIN_CLASS(ToLuauMapEnumerator)
	LUAU_END_CLASS(ToLuauMapEnumerator)
}

#undef SafeDelete

#endif