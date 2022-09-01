#pragma once


#ifdef TOLUAUUNREAL_API

#include "lua.h"
#include "lualib.h"

#include "Toluau/ToLuauDefine.h"
#include "Toluau/Containers/ToLuauVar.h"
#include "Toluau/Class/ClassName.h"
#include "Toluau/Class/ClassInfo.h"
#include "Toluau/API/StackAPI.h"
#include "Toluau/Containers/PropertyUtils.h"

#ifndef SafeDelete
#define SafeDelete(ptr) if(ptr) { delete ptr;ptr=nullptr; }
#endif

namespace ToLuau
{
	class ToLuau_API ToLuauMap : public FGCObject
	{
		DEFINE_LUA_CLASS(ToLuauMap)
	public:

		static int Push(lua_State* L, FProperty* keyProp, FProperty* valueProp, const FScriptMap* buf, bool frombp=true);
		static int Push(lua_State* L, FMapProperty* InProp, UObject* InObj);
		
		template<typename K,typename V>
		static int Push(lua_State* L, const TMap<K, V>& Value) {
			FProperty* KeyProperty = PropertyProto::CreateProperty(PropertyProto::get<K>());
			FProperty* ValueProperty = PropertyProto::CreateProperty(PropertyProto::get<V>());
			return Push(L, KeyProperty, ValueProperty, reinterpret_cast<const FScriptMap*>(&Value),false);
		}
		
		static void Clone(FScriptMap* Dest, FProperty* KeyProp, FProperty* ValueProp,const FScriptMap* Src);
		
		ToLuauMap(FProperty* keyProp, FProperty* valueProp, const FScriptMap* Buf, bool FromBP);
		ToLuauMap(FMapProperty* Prop, UObject* Obj);
		~ToLuauMap();

		const FScriptMap* Get() {
			return Map;
		}

		// Cast FScriptMap to TMap<TKey, TValue> if ElementSize matched
		template<typename TKey, typename TValue>
		const TMap<TKey, TValue>& ToTMap(lua_State* L) const {
			if (sizeof(TKey) != KeyProp->ElementSize)
				luaL_error(L, "Cast to TMap error, key element size isn't mathed(%d,%d)", sizeof(TKey), KeyProp->ElementSize);
			if (sizeof(TValue) != ValueProp->ElementSize)
				luaL_error(L, "Cast to TMap error, value element size isn't mathed(%d,%d)", sizeof(TValue), ValueProp->ElementSize);

			// modified FScriptMap::CheckConstraints function to check type constraints
			typedef FScriptMap ScriptType;
			typedef TMap<TKey, TValue> RealType;

			// Check that the class footprint is the same
			static_assert(sizeof(ScriptType) == sizeof(RealType), "FScriptMap's size doesn't match TMap");
			static_assert(alignof(ScriptType) == alignof(RealType), "FScriptMap's alignment doesn't match TMap");

			// Check member sizes
			typedef FScriptSet ScriptPairsType;
			typedef TSet<typename RealType::ElementType> RealPairsType;
			static_assert(sizeof(FScriptSet) == sizeof(RealPairsType), "FScriptMap's Pairs member size does not match TMap's");

			return *(reinterpret_cast<const TMap<TKey, TValue>*>(Map));
		}
		
		virtual void AddReferencedObjects( FReferenceCollector& Collector ) override;

		virtual FString GetReferencerName() const override
		{
			return "ToLuauMap";
		}

		static int32 SetupMetaTable(lua_State* L);
		static int32 Ctor(lua_State* L);
		static int32 Num(lua_State* L);
		static int32 Get(lua_State* L);
		static int32 Set(lua_State* L);
		static int32 RemoveAt(lua_State* L);
		static int32 Empty(lua_State* L);
		static int32 Pairs(lua_State* L);
		static int32 Enumerable(lua_State* L);
		
	private:

		uint8* GetKeyPtr(uint8* PairPtr);
		uint8* GetValuePtr(uint8* PairPtr);
		void Clear();
		int32 Num() const;
		void EmptyValues(int32 Slack = 0);
		void DestructItems(int32 Index, int32 Count);
		void DestructItems(uint8* PairPtr, uint32 Stride, int32 Index, int32 Count, bool bDestroyKeys, bool bDestroyValues);
		bool RemovePair(const void* KeyPtr);
		void RemoveAt(int32 Index, int32 Count = 1);
		
		FScriptMap* Map;
		FProperty* KeyProp;
		FProperty* ValueProp;
		FMapProperty* Prop;
		UObject* PropObj;
		FScriptMapHelper Helper;
		bool CreatedByBp;
		bool ShouldFree;
	};

	struct ToLuauMapEnumerator
	{
		DEFINE_LUA_CLASS(ToLuauMapEnumerator)
	public:
		TSharedPtr<ToLuauMap> Owner = nullptr;
		// hold referrence of LuaMap, avoid gc
		class ToLuauVar* Holder = nullptr;
		int32 Index = 0;
		int32 Num = 0;

		virtual ~ToLuauMapEnumerator();
	};

	template<typename K, typename V>
	class ToLuauRawMap
	{
		ToLuauRawMap(TMap<K, V>* Value)
		{
			RawMap = Value;
		}
		
		ToLuauRawMap(const TMap<K, V>& Value)
		{
			SharedMap = MakeShared<TMap<K, V>>();
			SharedMap->Append(Value);
		}

		TMap<K, V>* GetRawMapPointer()
		{
			if(SharedMap)
			{
				return SharedMap.Get();
			}
			else if(RawMap)
			{
				return RawMap;
			}
			return nullptr;
		}

		TMap<K, V>& GetRawMapRef()
		{
			return *GetRawMapPointer();
		}

		bool IsValid() const
		{
			return GetRawMapPointer() != nullptr;
		}

		static int32 Pairs(lua_State* L);
		static int32 Enumerable(lua_State* L);
		static int32 Num(lua_State* L);
		static int32 Contain(lua_State* L);
		static int32 Get(lua_State* L);
		static int32 Set(lua_State* L);
		static int32 RemoveAt(lua_State* L);
		static int32 Empty(lua_State* L);
		static int32_t SetupMetaTable(lua_State* L);
		
	private:
		TMap<K, V>* RawMap = nullptr;
		TSharedPtr<TMap<K, V>> SharedMap = nullptr;
	};

	DEFINE_LUA_TEMPLATE_CLASS_NAME_TwoParam(ToLuauRawMap)

	template<typename K, typename V>
	struct ToLuauRawMapEnumerator
	{
	public:
		TSharedPtr<ToLuauRawMap<K, V>> Owner = nullptr;
		// hold referrence of LuaArray, avoid gc
		class ToLuauVar* Holder = nullptr;
		typename TMap<K, V>::TKeyIterator Iter;
		virtual ~ToLuauRawMapEnumerator()
		{
			SafeDelete(Holder);
		};
		static int32_t SetupMetaTable(lua_State* L){ return 0; }
	};

	DEFINE_LUA_TEMPLATE_CLASS_NAME_TwoParam(ToLuauRawMapEnumerator)


	template <typename K, typename V>
	int32 ToLuauRawMap<K, V>::Pairs(lua_State* L)
	{
		auto Map = StackAPI::Check<TSharedPtr<ToLuauRawMap<K, V>>>(L, 1);
		if(!Map || !Map->IsValid())
		{
			return 0;
		}
		auto Iter = MakeShared<ToLuauRawMapEnumerator<K, V>>();
		Iter->Holder = new ToLuauVar(L, 1);
		Iter->Owner = Map;
		Iter->Iter = Map->GetRawMapPointer()->CreateKeyIterator();
		lua_pushcclosurek(L, &ToLuauRawMap<K, V>::Enumerable, "ToLuauRawMap<K, V>::Enumerable", 0, nullptr);
		StackAPI::Push<TSharedPtr<ToLuauRawMapEnumerator<K, V>>>(L, Iter, &ToLuauRawMapEnumerator<K, V>::SetupMetaTable);
		lua_pushnil(L);
		return 3;
	}

	template <typename K, typename V>
	int32 ToLuauRawMap<K, V>::Enumerable(lua_State* L)
	{
		auto Iter = StackAPI::Check<TSharedPtr<ToLuauRawMapEnumerator<K, V>>>(L, 1);
		auto& It = Iter->Iter;
		if(It)
		{
			StackAPI::Push<K>(L, It.Key());
			StackAPI::Push<V>(L, It.Value());
			++It;
		}
		return 0;
	}

	template <typename K, typename V>
	int32 ToLuauRawMap<K, V>::Num(lua_State* L)
	{
		TSharedPtr<ToLuauRawMap<K, V>> UD = StackAPI::Check<TSharedPtr<ToLuauRawMap<K, V>>>(L, 1);
		if(!UD)
		{
			lua_pushinteger(L, 0);
			return 1;
		}
		auto RawMap = UD->GetRawMapPointer();
		if(!RawMap)
		{
			lua_pushinteger(L, 0);
			return 1;
		}
		lua_pushinteger(L, RawMap->Num());
		return 1;
	}

	template <typename K, typename V>
	int32 ToLuauRawMap<K, V>::Contain(lua_State* L)
	{
		TSharedPtr<ToLuauRawMap<K, V>> UD = StackAPI::Check<TSharedPtr<ToLuauRawMap<K, V>>>(L, 1);
		if(!UD)
		{
			lua_pushboolean(L, false);
			return 1;
		}
		auto RawMap = UD->GetRawMapPointer();
		if(!RawMap)
		{
			lua_pushboolean(L, false);
			return 1;
		}
		if(lua_isnil(L, 2))
		{
			lua_pushboolean(L, false);
			return 1;
		}
		auto Key = StackAPI::Check<K>(L, 2);
		auto Contain = RawMap->Contains(Key);
		lua_pushboolean(L, Contain);
		return 1;
	}

	template <typename K, typename V>
	int32 ToLuauRawMap<K, V>::Get(lua_State* L)
	{
		TSharedPtr<ToLuauRawMap<K, V>> UD = StackAPI::Check<TSharedPtr<ToLuauRawMap<K, V>>>(L, 1);
		if(!UD)
		{
			lua_pushnil(L);
			return 1;
		}
		auto RawMap = UD->GetRawMapPointer();
		if(!RawMap)
		{
			lua_pushnil(L);
			return 1;
		}
		auto Key = StackAPI::Check<K>(L, 2);
		auto ValuePtr = RawMap->Find(Key);
		if(!ValuePtr)
		{
			lua_pushnil(L);
			return 1;
		}
		StackAPI::Push<V>(L, *ValuePtr);
		return 1;
	}

	template <typename K, typename V>
	int32 ToLuauRawMap<K, V>::Set(lua_State* L)
	{
		TSharedPtr<ToLuauRawMap<K, V>> UD = StackAPI::Check<TSharedPtr<ToLuauRawMap<K, V>>>(L, 1);
		if(!UD)
		{
			return 0;
		}
		auto RawMap = UD->GetRawMapPointer();
		if(!RawMap)
		{
			return 0;
		}
		if(lua_isnil(L, 2))
		{
			return 0;
		}
		auto Key = StackAPI::Check<K>(L, 2);
		auto Value = StackAPI::Check<V>(L, 3);
		auto& Target = RawMap->FindOrAdd(Key);
		Target = Value;
		return 0;
	}

	template <typename K, typename V>
	int32 ToLuauRawMap<K, V>::RemoveAt(lua_State* L)
	{
		TSharedPtr<ToLuauRawMap<K, V>> UD = StackAPI::Check<TSharedPtr<ToLuauRawMap<K, V>>>(L, 1);
		if(!UD)
		{
			return 0;
		}
		auto RawMap = UD->GetRawMapPointer();
		if(!RawMap)
		{
			return 0;
		}
		if(lua_isnil(L, 2))
		{
			return 0;
		}
		auto Key = StackAPI::Check<K>(L, 2);
		RawMap->Remove(Key);
		return 0;
	}

	template <typename K, typename V>
	int32 ToLuauRawMap<K, V>::Empty(lua_State* L)
	{
		TSharedPtr<ToLuauRawMap<K, V>> UD = StackAPI::Check<TSharedPtr<ToLuauRawMap<K, V>>>(L, 1);
		if(!UD)
		{
			return 0;
		}
		auto RawMap = UD->GetRawMapPointer();
		if(!RawMap)
		{
			return 0;
		}
		RawMap->Empty();
		return 0;
	}

	template <typename K, typename V>
	int32_t ToLuauRawMap<K, V>::SetupMetaTable(lua_State* L)
	{
		StackAPI::SetupMetaTableCommon(L);
		
		// ud meta
		lua_pushcfunction(L, &ToLuauRawMap::Pairs, "ToLuauRawMap::Pairs");
		lua_setfield(L, -2, "Pairs");
		
		lua_pushcfunction(L, &ToLuauRawMap::Pairs, "__pairs");
		lua_setfield(L, -2, "__pairs");

		lua_pushcfunction(L, &ToLuauRawMap::Enumerable, "ToLuauRawMap::Enumerable");
		lua_setfield(L, -2, "Enumerable");
		
		lua_pushcfunction(L, &ToLuauRawMap::Num, "ToLuauRawMap::Num");
		lua_setfield(L, -2, "Num");
		
		lua_pushcfunction(L, &ToLuauRawMap::Get, "ToLuauRawMap::Get");
		lua_setfield(L, -2, "Get");

		lua_pushcfunction(L, &ToLuauRawMap::Set, "ToLuauRawMap::Set");
		lua_setfield(L, -2, "Set");
		
		lua_pushcfunction(L, &ToLuauRawMap::RemoveAt, "ToLuauRawMap::RemoveAt");
		lua_setfield(L, -2, "RemoveAt");
		
		lua_pushcfunction(L, &ToLuauRawMap::Empty, "ToLuauRawMap::Empty");
		lua_setfield(L, -2, "Empty");
		
		return 0;
	}

	namespace StackAPI
	{
		namespace __DETAIL__
		{
			template<typename K, typename V>
			struct StackOperatorWrapper<TMap<K, V>*>
			{
				static int32_t Push(lua_State* L, TMap<K, V>* Value)
				{
					auto MapWrapper = MakeShared<ToLuauRawMap<K, V>>(Value);
					auto ClassName = GetClassName<ToLuauRawMap<K, V>>();

					StackOperatorWrapper<TSharedPtr<ToLuauRawMap<K, V>>>::Push(L, MapWrapper, &ToLuauRawMap<K, V>::SetupMetaTable);
					
					return 1;
				}

				static TMap<K, V>* Check(lua_State* L, int32_t Pos)
				{
					if(lua_isnil(L, Pos))
					{
						return nullptr;
					}
					auto Result = StackOperatorWrapper<TSharedPtr<ToLuauRawMap<K, V>>>::Check(L, Pos);
					return Result->GetRawArrayPointer();
				}
			};

			template<typename K, typename V>
			struct StackOperatorWrapper<TMap<K, V>&>
			{
				static int32_t Push(lua_State* L, TMap<K, V>& Value)
				{
					return StackOperatorWrapper<TMap<K, V>*>::Push(L, &Value);
				}

				static TMap<K, V>& Check(lua_State* L, int32_t Pos)
				{
					return StackOperatorWrapper<TMap<K, V>*>::Check(L, Pos);
				}
			};
			
			
			template<typename K, typename V>
			struct StackOperatorWrapper<TMap<K, V>>
			{
				static int32_t Push(lua_State* L, TMap<K, V> Value)
				{
					auto MapWrapper = MakeShared<ToLuauRawMap<K, V>>(Value);
					auto ClassName = GetClassName<ToLuauRawMap<K, V>>();
			
					StackOperatorWrapper<TSharedPtr<ToLuauRawMap<K, V>>>::Push(L, MapWrapper, &ToLuauRawMap<K, V>::SetupMetaTable);
					
					return 1;
				}
			
				static TMap<K, V> Check(lua_State* L, int32_t Pos)
				{
					if(lua_isnil(L, Pos))
					{
						return nullptr;
					}
					auto Result = StackOperatorWrapper<TSharedPtr<ToLuauRawMap<K, V>>>::Check(L, Pos);
					return *Result->GetRawArrayPointer();
				}
			};
			
			template<typename K, typename V> struct BuildInPushValue<TMap<K, V>> { static constexpr bool Value = true; };

			
		}

		template<typename K, typename V>
		int32_t Push(lua_State* L, TMap<K, V> Value)
		{
			return __DETAIL__::StackOperatorWrapper<TMap<K, V>>::Push(L, Value);
		}
		
		template<typename K, typename V>
		int32_t Push(lua_State* L, TMap<K, V>* Value)
		{
			return __DETAIL__::StackOperatorWrapper<TMap<K, V>*>::Push(L, Value);
		}
		
	}
}

#ifdef SafeDelete
#undef SafeDelete
#endif

#endif