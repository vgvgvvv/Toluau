#pragma once


#ifdef TOLUAUUNREAL_API

#include "lua.h"
#include "lualib.h"

#include "Toluau/ToLuauDefine.h"

#include "Toluau/Class/ClassInfo.h"
#include "Toluau/Containers/PropertyUtils.h"

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
		static int32 Add(lua_State* L);
		static int32 Remove(lua_State* L);
		static int32 Clear(lua_State* L);
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
	
}

#endif