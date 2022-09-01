#pragma once


#ifdef TOLUAUUNREAL_API

#include "lua.h"
#include "lualib.h"
#include "Toluau/Containers/ToLuauVar.h"
#include "Toluau/API/StackAPI.h"
#include "Toluau/Class/ClassInfo.h"
#include "Toluau/ToLuauDefine.h"

#ifndef SafeDelete
#define SafeDelete(ptr) if(ptr) { delete ptr;ptr=nullptr; }
#endif

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
		static int32 Add(lua_State* L);
		static int32 Insert(lua_State* L);
		static int32 RemoveAt(lua_State* L);
		static int32 Empty(lua_State* L);

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

	template<typename T>
	class ToLuauRawArray final
	{
	public:
		
		ToLuauRawArray(TArray<T>* Value)
		{
			RawArray = Value;
		}
		
		ToLuauRawArray(TArray<T> Value)
		{
			SharedArray = MakeShared<TArray<T>>(Value);
		}

		TArray<T>* GetRawArrayPointer()
		{
			if(SharedArray)
			{
				return SharedArray.Get();
			}
			else if(RawArray)
			{
				return RawArray;
			}
			return nullptr;
		}
		
		TArray<T>& GetRawArrayRef()
		{
			return *GetRawArrayPointer();
		}

		bool IsValid() const
		{
			return GetRawArrayPointer() != nullptr;
		}
		
		static int32 Pairs(lua_State* L);
		static int32 Enumerable(lua_State* L);
		static int32 Num(lua_State* L);
		static int32 Get(lua_State* L);
		static int32 Set(lua_State* L);
		static int32 Add(lua_State* L);
		static int32 Insert(lua_State* L);
		static int32 RemoveAt(lua_State* L);
		static int32 Empty(lua_State* L);
		static int32_t SetupMetaTable(lua_State* L);

	private:
		TArray<T>* RawArray = nullptr;
		TSharedPtr<TArray<T>> SharedArray = nullptr;
	};

	DEFINE_LUA_TEMPLATE_CLASS_NAME_OneParam(ToLuauRawArray)
	
	template<typename T>
	struct ToLuauRawArrayEnumerator
	{
	public:
		TSharedPtr<ToLuauRawArray<T>> Owner = nullptr;
		// hold referrence of LuaArray, avoid gc
		class ToLuauVar* Holder = nullptr;
		int32 Index = 0;
		virtual ~ToLuauRawArrayEnumerator()
		{
			SafeDelete(Holder);
		};
		static int32_t SetupMetaTable(lua_State* L){ return 0; }
	};

	DEFINE_LUA_TEMPLATE_CLASS_NAME_OneParam(ToLuauRawArrayEnumerator)

	template <typename T>
	int32 ToLuauRawArray<T>::Pairs(lua_State* L)
	{
		auto Array = StackAPI::Check<TSharedPtr<ToLuauRawArray<T>>>(L, 1);
		auto Iter = MakeShared<ToLuauRawArrayEnumerator<T>>();
		Iter->Holder = new ToLuauVar(L, 1);
		Iter->Owner = Array;
		Iter->Index = 0;
		lua_pushcclosurek(L, &ToLuauRawArray<T>::Enumerable, "ToLuauRawArray<T>::Enumerable", 0, nullptr);
		StackAPI::Push<TSharedPtr<ToLuauRawArrayEnumerator<T>>>(L, Iter, &ToLuauRawArrayEnumerator<T>::SetupMetaTable);
		lua_pushnil(L);
		return 3;
	}

	template <typename T>
	int32 ToLuauRawArray<T>::Enumerable(lua_State* L)
	{
		auto Iter = StackAPI::Check<TSharedPtr<ToLuauRawArrayEnumerator<T>>>(L, 1);
		TSharedPtr<ToLuauRawArray<T>> Array = Iter->Owner;
		TArray<T>* RawArray = Array->GetRawArrayPointer();
		if(RawArray->IsValidIndex(Iter->Index))
		{
			StackAPI::Push(L, Iter->Index);
			StackAPI::Push<T>(L, (*RawArray)[Iter->Index]);
			Iter->Index += 1;
			return 2;
		}
		return 0;
	}

	template <typename T>
	int32 ToLuauRawArray<T>::Num(lua_State* L)
	{
		TSharedPtr<ToLuauRawArray<T>> UD = StackAPI::Check<TSharedPtr<ToLuauRawArray<T>>>(L, 1);
		if(!UD)
		{
			lua_pushinteger(L, 0);
			return 1;
		}
		auto RawArray = UD->GetRawArrayPointer();
		if(!RawArray)
		{
			lua_pushinteger(L, 0);
			return 1;
		}
		lua_pushinteger(L, RawArray->Num());
		return 1;
	}

	template <typename T>
	int32 ToLuauRawArray<T>::Get(lua_State* L)
	{
		TSharedPtr<ToLuauRawArray<T>> UD = StackAPI::Check<TSharedPtr<ToLuauRawArray<T>>>(L, 1);
		if(!UD)
		{
			lua_pushnil(L);
			return 1;
		}
		TArray<T>* RawArray = UD->GetRawArrayPointer();
		if(!RawArray)
		{
			lua_pushnil(L);
			return 1;
		}
		auto Index = StackAPI::Check<int32>(L, 2);
		if(Index < 0 || Index > RawArray->Num() - 1)
		{
			lua_pushnil(L);
			return 1;
		}
		StackAPI::Push<T>(L, (*RawArray)[Index]);
		return 1;
	}

	template <typename T>
	int32 ToLuauRawArray<T>::Set(lua_State* L)
	{
		TSharedPtr<ToLuauRawArray<T>> UD = StackAPI::Check<TSharedPtr<ToLuauRawArray<T>>>(L, 1);
		if(!UD)
		{
			return 0;
		}
		TArray<T>* RawArray = UD->GetRawArrayPointer();
		if(!RawArray)
		{
			return 0;
		}
		auto Index = StackAPI::Check<int32>(L, 2);
		if(Index < 0 || Index > RawArray->Num() - 1)
		{
			return 0;
		}
		T Value = StackAPI::Check<T>(L, 3);
		(*RawArray)[Index] = Value;
		return 0;
	}

	template <typename T>
	int32 ToLuauRawArray<T>::Add(lua_State* L)
	{
		TSharedPtr<ToLuauRawArray<T>> UD = StackAPI::Check<TSharedPtr<ToLuauRawArray<T>>>(L, 1);
		if(!UD)
		{
			return 0;
		}
		auto RawArray = UD->GetRawArrayPointer();
		if(!RawArray)
		{
			return 0;
		}
		auto Value = StackAPI::Check<T>(L, 2);
		RawArray->Add(Value);
		return 0;
	}

	template <typename T>
	int32 ToLuauRawArray<T>::Insert(lua_State* L)
	{
		TSharedPtr<ToLuauRawArray<T>> UD = StackAPI::Check<TSharedPtr<ToLuauRawArray<T>>>(L, 1);
		if(!UD)
		{
			return 0;
		}
		auto RawArray = UD->GetRawArrayPointer();
		if(!RawArray)
		{
			return 0;
		}
		auto Index = StackAPI::Check<int32>(L, 2);
		Index = FMath::Clamp(Index, 0, RawArray->Num() - 1);
		auto Value = StackAPI::Check<T>(L, 2);
		RawArray->Insert(Value, Index);
		return 0;
	}

	template <typename T>
	int32 ToLuauRawArray<T>::RemoveAt(lua_State* L)
	{
		TSharedPtr<ToLuauRawArray<T>> UD = StackAPI::Check<TSharedPtr<ToLuauRawArray<T>>>(L, 1);
		if(!UD)
		{
			return 0;
		}
		auto RawArray = UD->GetRawArrayPointer();
		if(!RawArray)
		{
			return 0;
		}
		auto Index = StackAPI::Check<int32>(L, 2);
		if(Index < 0 || Index > RawArray->Num() - 1)
		{
			return 0;
		}
		RawArray->RemoveAt(Index);
		return 0;
	}

	template <typename T>
	int32 ToLuauRawArray<T>::Empty(lua_State* L)
	{
		TSharedPtr<ToLuauRawArray<T>> UD = StackAPI::Check<TSharedPtr<ToLuauRawArray<T>>>(L, 1);
		if(!UD)
		{
			return 0;
		}
		auto RawArray = UD->GetRawArrayPointer();
		if(!RawArray)
		{
			return 0;
		}
		RawArray->Empty();
		return 0;
	}

	template <typename T>
	int32_t ToLuauRawArray<T>::SetupMetaTable(lua_State* L)
	{
		StackAPI::SetupMetaTableCommon(L);
			
		// ud meta
		lua_pushcfunction(L, &ToLuauRawArray::Pairs, "ToLuauRawArray::Pairs");
		lua_setfield(L, -2, "Pairs");
		
		lua_pushcfunction(L, &ToLuauRawArray::Pairs, "__pairs");
		lua_setfield(L, -2, "__pairs");

		lua_pushcfunction(L, &ToLuauRawArray::Enumerable, "ToLuauRawArray::Enumerable");
		lua_setfield(L, -2, "Enumerable");
		
		lua_pushcfunction(L, &ToLuauRawArray::Num, "ToLuauRawArray::Num");
		lua_setfield(L, -2, "Num");
		
		lua_pushcfunction(L, &ToLuauRawArray::Get, "ToLuauRawArray::Get");
		lua_setfield(L, -2, "Get");
		
		lua_pushcfunction(L, &ToLuauRawArray::Set, "ToLuauRawArray::Set");
		lua_setfield(L, -2, "Set");

		lua_pushcfunction(L, &ToLuauRawArray::Add, "ToLuauRawArray::Add");
		lua_setfield(L, -2, "Add");

		lua_pushcfunction(L, &ToLuauRawArray::Insert, "ToLuauRawArray::Insert");
		lua_setfield(L, -2, "Insert");

		lua_pushcfunction(L, &ToLuauRawArray::RemoveAt, "ToLuauRawArray::RemoveAt");
		lua_setfield(L, -2, "RemoveAt");
		
		lua_pushcfunction(L, &ToLuauRawArray::Empty, "ToLuauRawArray::Empty");
		lua_setfield(L, -2, "Empty");
		
		return 0;
	}

	
	
	namespace StackAPI
	{
		namespace __DETAIL__
		{
			template<typename T>
			struct StackOperatorWrapper<TArray<T>*>
			{
				static int32_t Push(lua_State* L, TArray<T>* Value)
				{
					auto ArrayWrapper = MakeShared<ToLuauRawArray<T>>(Value);
					auto ClassName = GetClassName<ToLuauRawArray<T>>();

					StackOperatorWrapper<TSharedPtr<ToLuauRawArray<T>>>::Push(L, ArrayWrapper, &ToLuauRawArray<T>::SetupMetaTable);
					
					return 1;
				}

				static TArray<T>* Check(lua_State* L, int32_t Pos)
				{
					if(lua_isnil(L, Pos))
					{
						return nullptr;
					}
					auto Result = StackOperatorWrapper<TSharedPtr<ToLuauRawArray<T>>>::Check(L, Pos);
					return Result->GetRawArrayPointer();
				}
			};
			
			template<typename T>
			struct StackOperatorWrapper<TArray<T>&>
			{
				static int32_t Push(lua_State* L, TArray<T>& Value)
				{
					return StackOperatorWrapper<TArray<T>*>::Push(L, &Value);
				}

				static TArray<T>& Check(lua_State* L, int32_t Pos)
				{
					return StackOperatorWrapper<TArray<T>*>::Check(L, Pos);
				}
			};
			
			template<typename T>
			struct StackOperatorWrapper<TArray<T>>
			{
				static int32_t Push(lua_State* L, TArray<T> Value)
				{
					auto ArrayWrapper = MakeShared<ToLuauRawArray<T>>(Value);
					auto ClassName = GetClassName<ToLuauRawArray<T>>();
			
					StackOperatorWrapper<TSharedPtr<ToLuauRawArray<T>>>::Push(L, ArrayWrapper, &ToLuauRawArray<T>::SetupMetaTable);
					
					return 1;
				}
			
				static TArray<T> Check(lua_State* L, int32_t Pos)
				{
					if(lua_isnil(L, Pos))
					{
						return nullptr;
					}
					auto Result = StackOperatorWrapper<TSharedPtr<ToLuauRawArray<T>>>::Check(L, Pos);
					return *Result->GetRawArrayPointer();
				}
			};
			
			template<typename T> struct BuildInPushValue<TArray<T>> { static constexpr bool Value = true; };
		}

		template<typename T>
		int32_t Push(lua_State* L, TArray<T> Value)
		{
			return __DETAIL__::StackOperatorWrapper<TArray<T>>::Push(L, Value);
		}

		template<typename T>
		int32_t Push(lua_State* L, TArray<T>* Value)
		{
			return __DETAIL__::StackOperatorWrapper<TArray<T>*>::Push(L, Value);
		}
		
	}
	
}

#ifdef SafeDelete
#undef SafeDelete
#endif

#endif
