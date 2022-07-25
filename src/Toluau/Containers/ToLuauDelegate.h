#pragma once

#include "Toluau/ToLuauDefine.h"

#ifdef TOLUAUUNREAL_API
#include "lua.h"
#include "Toluau/API/StackAPI.h"
#include "ToLuauDelegate.generated.h"

namespace ToLuau
{
	class ToLuauVar;
}

UCLASS()
class ToLuau_API UToLuauDelegate : public UObject
{
	GENERATED_BODY()
public:
	virtual ~UToLuauDelegate() override;
	
	UFUNCTION(BlueprintCallable, Category="Lua|Delegate")
	void EventTrigger();
	
	virtual void ProcessEvent(UFunction* Function, void* Parms) override;

	void BindFunction(lua_State* L, int32 Pos, UFunction* Func);
	void BindFunction(lua_State* L, int32 Pos);
	void BindFunction(UFunction* Func);
	void Dispose();
	
#if ToLuauDebug
	FString DebugName;
#endif
	
private:
	
	ToLuau::ToLuauVar* LuaFunc;
	UFunction* UFunc;
};
#endif

#ifdef TOLUAUUNREAL_API

namespace ToLuau
{

	struct ToLuauMultiDelegate final
	{
		DEFINE_LUA_CLASS(ToLuauMultiDelegate)
	public:
		ToLuauMultiDelegate(FMulticastScriptDelegate* InDelegate, UFunction* InUFunc,
									const FString& InDebugName);

		FMulticastScriptDelegate* Delegate = nullptr;
		UFunction* UFunc = nullptr;

#if ToLuauDebug
		FString DebugName;
#endif

		static int32 Push(lua_State* L, FMulticastScriptDelegate* InDelegate, UFunction* InUFunc, const FString& InDebugName = "");

		static int32 SetupMetatable(lua_State* L);
		static int32 Add(lua_State* L);
		static int32 Remove(lua_State* L);
		static int32 Clear(lua_State* L);

		void AddRef(UToLuauDelegate* LuauDelegate);

		void RemoveRef(UToLuauDelegate* LuauDelegate);
		
	private:
		TArray<TStrongObjectPtr<UToLuauDelegate>> LuauDelegateHolder;
		
	};

	struct ToLuauDelegate final
	{
		DEFINE_LUA_CLASS(ToLuauDelegate)
	public:
		ToLuauDelegate(FScriptDelegate* InDelegate, UFunction* InUFunc,
							   const FString& InDebugName);
		
		~ToLuauDelegate() = default;
		FScriptDelegate* Delegate = nullptr;
		UFunction* UFunc = nullptr;

#if ToLuauDebug
		FString DebugName;
#endif
		
		static int32 Push(lua_State* L, FScriptDelegate* InDelegate, UFunction* InUFunc, const FString& InDebugName = "");

		static int32 SetupMetatable(lua_State* L);
		static int32 Bind(lua_State* L);
		static int32 Clear(lua_State* L);

		void AddRef(UToLuauDelegate* LuauDelegate);

		void RemoveRef();
		
	private:
		TStrongObjectPtr<UToLuauDelegate> LuauDelegateHolder;

	};
	
	template<typename R, typename ...ARGS>
	struct ToLuauBaseDelegate
	{
		using RawDelegateType = TDelegate<R, ARGS...>;
		using DelegateType = ToLuauBaseDelegate<R, ARGS...>;
		
		ToLuauBaseDelegate(TDelegate<R, ARGS...>& d)
			: RawDelegate(d)
		{
		}
		TDelegate<R, ARGS...>& RawDelegate;

		static int32_t Bind(lua_State* L)
		{
			ToLuauBaseDelegate<R, ARGS...>* Delegate = StackAPI::Check<ToLuauBaseDelegate<R, ARGS...>*>(L, 1);
			luaL_checktype(L, 2, LUA_TFUNCTION);
			ToLuauVar Func(L, 2);
			if (Func.IsValid() && Func.IsFunction())
			{
				Delegate->RawDelegate.BindLambda([=](ARGS ...args) {
					ToLuauVar Result = Func.Call(std::forward<ARGS>(args) ...);
					return Result.Get<R>();
				});
			}
			return 0;
		}

		static int32_t Clear(lua_State* L)
		{
			ToLuauBaseDelegate<R, ARGS...>* Delegate = StackAPI::Check<ToLuauBaseDelegate<R, ARGS...>*>(L, 1);
			if(Delegate)
			{
				Delegate->RawDelegate.Unbind();
			}
			return 0;
		}
		
		static int32_t SetupMetaTable(lua_State* L)
		{
			StackAPI::SetupMetaTableCommon(L);
			
			// ud meta
			lua_pushcfunction(L, &ToLuauBaseDelegate::Bind, "ToLuauBaseDelegate::Bind");
			lua_setfield(L, -2, "Bind");

			lua_pushcfunction(L, &ToLuauBaseDelegate::Clear, "ToLuauBaseDelegate::Clear");
			lua_setfield(L, -2, "Clear");
			
			return 0;
		}
	};
	
	namespace StackAPI
	{
		namespace __DETAIL__
		{
			
			template<typename R, typename ...ARGS>
			struct StackOperatorWrapper<TDelegate<R, ARGS...>>
			{
				using RawDelegateType = TDelegate<R, ARGS...>;
				using DelegateType = ToLuauBaseDelegate<R, ARGS...>;
				
				static int32_t Push(lua_State* L, const RawDelegateType& Value)
				{
					auto BaseDelegate = MakeShared<DelegateType>(Value);
					auto ClassName = GetClassName<RawDelegateType>();

					StackOperatorWrapper<TSharedPtr<DelegateType>>::template Push(L, BaseDelegate);
					SetClassMetaTable(L, ClassName, &DelegateType::SetupMetaTable);
					
					return 1;
				}

				static RawDelegateType& Check(lua_State* L, int32_t Pos)
				{
					if(lua_isnil(L, Pos))
					{
						static RawDelegateType Default;
						return Default;
					}
					auto Result = StackOperatorWrapper<TSharedPtr<DelegateType>>::template Check(L, Pos);
					return *Result;
				}
			};
		}
	}
	
	
}


#endif