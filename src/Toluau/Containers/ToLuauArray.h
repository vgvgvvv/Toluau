#pragma once
#include "lua.h"
#include "lualib.h"

#include "Toluau/ToLuauDefine.h"


#include "Toluau/Class/ClassInfo.h"

#ifdef TOLUAUUNREAL_API

namespace ToLuau
{
	class ToLuau_API ToLuauArray : public FGCObject
	{
		DEFINE_LUA_CLASS(ToLuauArray)
	public:

		static void Clone(FScriptArray* DestArray, FProperty* P, const FScriptArray* SrcArray);
		
		static int Push(lua_State* L, FProperty* InProp, FScriptArray* InArray);
		static int Push(lua_State* L, FArrayProperty* InProp, UObject* InObj);
		
		ToLuauArray(FProperty* InProp, FScriptArray* InBuf);
		ToLuauArray(FArrayProperty* Prop, UObject* Obj);
		~ToLuauArray();

		const FScriptArray* Get() {
			return Array;
		}
		
		// Cast FScriptArray to TArray<T> if ElementSize matched
		template<typename T>
		const TArray<T>& ToTArray(lua_State* L) const {
			if(sizeof(T)!=Inner->ElementSize)
				luaL_error(L,"Cast to TArray error, element size isn't mathed(%d,%d)",sizeof(T),Inner->ElementSize);
			return *(reinterpret_cast<const TArray<T>*>( Array ));
		}
		
		virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

		virtual FString GetReferencerName() const override
		{
			return "ToLuauArray";
		}

		static int32 SetupMetaTable(lua_State* L);
		static int Ctor(lua_State* L);
		static int32 Pairs(lua_State* L);
		static int32 Enumerable(lua_State* L);
		static int32 Num(lua_State* L);
		static int32 Get(lua_State* L);
		static int32 Set(lua_State* L);
		static int32 Insert(lua_State* L);
		static int32 Remove(lua_State* L);
		static int32 Clear(lua_State* L);

	private:
		int32 Num() const;
		void Clear();
		uint8* GetRawPtr(int32 Index) const;
		bool IsValidIndex(int32 Index) const;
		uint8* Insert(int32 Index);
		uint8* Add();
		void Remove(int32 Index);
		void ConstructItems(int32 Index, int32 Count);
		void DestructItems(int32 Index, int32 Count);
		
		FProperty* Inner = nullptr;
		FScriptArray* Array = nullptr;
		FArrayProperty* Prop = nullptr;
		UObject* OwnerObj = nullptr;
		bool ShouldFree = false;
	};

	struct ToLuauArrayEnumerator {
		DEFINE_LUA_CLASS(ToLuauArrayEnumerator)
	public:
		TSharedPtr<ToLuauArray> Owner = nullptr;
		// hold referrence of LuaArray, avoid gc
		class ToLuauVar* Holder = nullptr;
		int32 Index = 0;
		virtual ~ToLuauArrayEnumerator();
	};

}

#endif
