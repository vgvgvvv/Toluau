#pragma once

#include "Toluau/ToLuauDefine.h"

#include "lua.h"
#include "Toluau/API/StackAPI.h"
#include "Toluau/Class/ClassName.h"
#include "Toluau/Containers/ToLuauVar.h"
#if TOLUAUUNREAL_API
#include "ToLuauDelegate.generated.h"

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
	
	::ToLuau::ToLuauVar* LuaFunc;
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
		using RawDelegateType = TDelegate<R(ARGS...)>;
		using DelegateType = ToLuauBaseDelegate<R, ARGS...>;
		
		ToLuauBaseDelegate(TDelegate<R(ARGS...)>* d);
		
		ToLuauBaseDelegate(TDelegate<R(ARGS...)> d);

		TDelegate<R(ARGS...)>* GetRawDelegatePtr() const;

		TDelegate<R(ARGS...)>* RawDelegate = nullptr;
		TSharedPtr<TDelegate<R(ARGS...)>> SharedDelegate;

		static int32_t Bind(lua_State* L);

		static int32_t Clear(lua_State* L);

		static int32_t SetupMetaTable(lua_State* L);
	};

	template <typename R, typename ... ARGS>
	ToLuauBaseDelegate<R, ARGS...>::ToLuauBaseDelegate(TDelegate<R(ARGS...)>* d)
		: RawDelegate(d)
		, SharedDelegate(nullptr)
	{
	}

	template <typename R, typename ... ARGS>
	ToLuauBaseDelegate<R, ARGS...>::ToLuauBaseDelegate(TDelegate<R(ARGS...)> d)
		: RawDelegate(nullptr)
	{
		SharedDelegate = MakeShared<TDelegate<R(ARGS...)>>(d);
	}

	template <typename R, typename ... ARGS>
	TDelegate<R(ARGS...)>* ToLuauBaseDelegate<R, ARGS...>::GetRawDelegatePtr() const
	{
		if(SharedDelegate)
		{
			return SharedDelegate.Get();
		}
		else if(RawDelegate)
		{
			return RawDelegate;
		}
		return nullptr;
	}

	template <typename R, typename ... ARGS>
	int32_t ToLuauBaseDelegate<R, ARGS...>::Bind(lua_State* L)
	{
		auto Delegate = StackAPI::Check<TSharedPtr<ToLuauBaseDelegate<R, ARGS...>>>(L, 1);
		luaL_checktype(L, 2, LUA_TFUNCTION);
		::ToLuau::ToLuauVar Func(L, 2);
		if (Func.IsValid() && Func.IsFunction())
		{
			if constexpr (!TIsSame<R, void>::Value)
			{
				Delegate->GetRawDelegatePtr()->BindLambda([=](ARGS ...args) {
					::ToLuau::ToLuauVar Result = Func.Call(std::forward<ARGS>(args) ...);
					return Result.Get<R>();
				});
			}
			else
			{
				Delegate->GetRawDelegatePtr()->BindLambda([=](ARGS ...args) {
					Func.Call(std::forward<ARGS>(args) ...);
				});
			}
			
		}
		return 0;
	}

	template <typename R, typename ... ARGS>
	int32_t ToLuauBaseDelegate<R, ARGS...>::Clear(lua_State* L)
	{
		auto Delegate = StackAPI::Check<TSharedPtr<ToLuauBaseDelegate<R, ARGS...>>>(L, 1);
		if(Delegate)
		{
			Delegate->GetRawDelegatePtr()->Unbind();
		}
		return 0;
	}

	template <typename R, typename ... ARGS>
	int32_t ToLuauBaseDelegate<R, ARGS...>::SetupMetaTable(lua_State* L)
	{
		StackAPI::SetupMetaTableCommon(L);
			
		// ud meta
		lua_pushcfunction(L, &ToLuauBaseDelegate::Bind, "ToLuauBaseDelegate::Bind");
		lua_setfield(L, -2, "Bind");

		lua_pushcfunction(L, &ToLuauBaseDelegate::Clear, "ToLuauBaseDelegate::Clear");
		lua_setfield(L, -2, "Clear");
			
		return 0;
	}

	template<typename R, typename ...ARGS>
	struct GetClassNameWrapper<ToLuauBaseDelegate<R, ARGS...>>
	{
		static constexpr bool IsValid = true;
		static constexpr bool DefineByMacro = false;
		using Type = ToLuauBaseDelegate<R, ARGS...>;
		static std::string GetName(const void* Obj)
		{
			return GetClassNameWrapper<typename ToLuauBaseDelegate<R, ARGS...>::RawDelegateType>::GetName(Obj);
		}
	};
	
	namespace StackAPI
	{
		namespace __DETAIL__
		{
			template<typename R, typename ...ARGS>
			struct StackOperatorWrapper<TDelegate<R(ARGS...)>*>
			{
				using RawDelegateType = TDelegate<R(ARGS...)>;
				using DelegateType = ToLuauBaseDelegate<R, ARGS...>;
				
				static int32_t Push(lua_State* L, RawDelegateType* Value)
				{
					auto BaseDelegate = MakeShared<DelegateType>(Value);
					auto ClassName = GetClassName<RawDelegateType>();

					StackOperatorWrapper<TSharedPtr<DelegateType>>::Push(L, BaseDelegate, &DelegateType::SetupMetaTable);
					
					return 1;
				}

				static RawDelegateType* Check(lua_State* L, int32_t Pos)
				{
					if(lua_isnil(L, Pos))
					{
						static RawDelegateType Default;
						return Default;
					}
					auto& Result = StackOperatorWrapper<TSharedPtr<DelegateType>>::Check(L, Pos);
					return (Result->GetRawDelegatePtr());
				}
			};
			
			template<typename R, typename ...ARGS>
			struct StackOperatorWrapper<TDelegate<R(ARGS...)>&>
			{
				using RawDelegateType = TDelegate<R(ARGS...)>;
				using DelegateType = ToLuauBaseDelegate<R, ARGS...>;
				
				static int32_t Push(lua_State* L, RawDelegateType& Value)
				{
					return StackOperatorWrapper<TDelegate<R(ARGS...)>*>::Push(L, &Value);
				}

				static RawDelegateType& Check(lua_State* L, int32_t Pos)
				{
					return *StackOperatorWrapper<TDelegate<R(ARGS...)>*>::Check(L, Pos);
				}
			};
			
			template<typename R, typename ...ARGS>
			struct StackOperatorWrapper<TDelegate<R(ARGS...)>>
			{
				using RawDelegateType = TDelegate<R(ARGS...)>;
				using DelegateType = ToLuauBaseDelegate<R, ARGS...>;

				static int32_t Push(lua_State* L, RawDelegateType& Value)
				{
					auto BaseDelegate = MakeShared<DelegateType>(&Value);
					auto ClassName = GetClassName<RawDelegateType>();

					StackOperatorWrapper<TSharedPtr<DelegateType>>::Push(L, BaseDelegate, &DelegateType::SetupMetaTable);
					
					return 1;
				}
				
				static RawDelegateType Check(lua_State* L, int32_t Pos)
				{
					if(lua_isnil(L, Pos))
					{
						return {};
					}
					if(lua_isfunction(L, Pos))
					{
						RawDelegateType NewDelegate;
						ToLuauVar Func(L, Pos);
						if constexpr (!TIsSame<R, void>::Value)
						{
							NewDelegate.BindLambda([Func](ARGS ...args) {
								ToLuauVar Result = Func.Call(std::forward<ARGS>(args) ...);
								return Result.Get<R>();
							});
						}
						else
						{
							NewDelegate.BindLambda([Func](ARGS ...args) {
								Func.Call(std::forward<ARGS>(args) ...);
							});
						}
						
						return NewDelegate;
					}
					auto Result = StackOperatorWrapper<TSharedPtr<DelegateType>>::Check(L, Pos);
					return *(Result->GetRawDelegatePtr());
				}
			};

			template<typename R, typename ...ARGS> struct BuildInPushValue<TDelegate<R(ARGS...)>> { static constexpr bool Value = true; };

		}
	}
	
	
}


#endif