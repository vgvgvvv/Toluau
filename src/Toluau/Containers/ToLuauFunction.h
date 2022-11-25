#pragma once

#include "lua.h"
#include "Toluau/Class/ClassName.h"
#include "Toluau/API/StackAPI.h"
#include "Toluau/API/CppBinding.h"

namespace ToLuau
{

	namespace StackAPI
	{
		namespace __DETAIL__
		{
			template<typename R, typename ... ARGS>
			struct StackOperatorWrapper<std::function<R(ARGS...)>>;

#ifdef TOLUAUUNREAL_API
			template<typename R, typename ... ARGS>
			struct StackOperatorWrapper<TFunction<R(ARGS...)>>;

			template<typename R, typename ... ARGS>
			struct StackOperatorWrapper<TFunction<R(ARGS...)>&>;

			template<typename R, typename ... ARGS>
			struct StackOperatorWrapper<TFunction<R(ARGS...)>*>;
#endif
		}
	}
	
	template<typename R, typename ...ARGS>
	struct ToLuauStdFunction
	{
		using RawDelegateType = std::function<R(ARGS...)>;
		using DelegateType = ToLuauStdFunction<R, ARGS...>;
		
		ToLuauStdFunction(std::function<R(ARGS...)>* d);
		
		ToLuauStdFunction(std::function<R(ARGS...)> d);

		std::function<R(ARGS...)>* GetRawDelegatePtr() const;

		std::function<R(ARGS...)>* RawDelegate = nullptr;
		std::shared_ptr<std::function<R(ARGS...)>> SharedDelegate;

		
		template<class X>
		struct Functor;

		template<int32_t... Index>
		struct Functor<IntList<Index...>>
		{
			// index is int-list based 0, so should plus Offset to get first arg
			// (not include obj ptr if it's a member function)
			static R Invoke(lua_State* L, std::function<R(ARGS...)>* FuncPtr)
			{
				if(FuncPtr)
				{
					auto Func = *FuncPtr;
					return Func(ArgOperator<typename remove_cr<ARGS>::type>::ReadArg(L, Index + 2)...);
				}
				return {};
			}
		};

		static int32_t Call(lua_State* L)
		{
			auto FuncPtr = StackAPI::Check<std::function<R(ARGS...)>*>(L, 1);
			using I = MakeIntList<sizeof...(ARGS)>;
			if constexpr (std::is_same<R, void>::value)
			{
				Functor<I>::Invoke(L, FuncPtr);
				return 0;
			}
			else
			{
				R Ret = Functor<I>::Invoke(L, FuncPtr);
				return StackAPI::Push(L, Ret);
			}
		}

		static int32_t SetupMetaTable(lua_State* L);
	};

	template <typename R, typename ... ARGS>
	ToLuauStdFunction<R, ARGS...>::ToLuauStdFunction(std::function<R(ARGS...)>* d)
		: RawDelegate(d)
		, SharedDelegate(nullptr)
	{
	}

	template <typename R, typename ... ARGS>
	ToLuauStdFunction<R, ARGS...>::ToLuauStdFunction(std::function<R(ARGS...)> d)
		: RawDelegate(nullptr)
	{
		SharedDelegate = MakeShared<std::function<R(ARGS...)>>(d);
	}

	template <typename R, typename ... ARGS>
	std::function<R(ARGS...)>* ToLuauStdFunction<R, ARGS...>::GetRawDelegatePtr() const
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
	int32_t ToLuauStdFunction<R, ARGS...>::SetupMetaTable(lua_State* L)
	{
		StackAPI::SetupMetaTableCommon(L);
			
		// ud meta
		lua_pushcfunction(L, &ToLuauStdFunction::Call, "ToLuauBaseDelegate::Call");
		lua_setfield(L, -2, "__call");

		return 0;
	}

	template<typename R, typename ...ARGS>
	struct GetClassNameWrapper<ToLuauStdFunction<R, ARGS...>>
	{
		static constexpr bool IsValid = true;
		static constexpr bool DefineByMacro = false;
		using Type = ToLuauStdFunction<R, ARGS...>;
		static std::string GetName(const void* Obj)
		{
			return GetClassNameWrapper<typename ToLuauStdFunction<R, ARGS...>::RawDelegateType>::GetName(Obj);
		}
	};

	
	namespace StackAPI
	{
		namespace __DETAIL__
		{
			template<typename R, typename ... ARGS>
			struct StackOperatorWrapper<std::function<R(ARGS...)>>
			{

				using RawDelegateType = std::function<R(ARGS...)>;
				using DelegateType = ToLuauStdFunction<R, ARGS...>;
				
				static int32_t Push(lua_State* L, const RawDelegateType& Value)
				{
					using BindType = typename LuaCallableBinding<RawDelegateType>::Prototype;
					BindType::Func = &Value;
					lua_pushcfunction(L, BindType::Func, "");
					return 1;
				}

				static int32_t Push(lua_State* L, const RawDelegateType Value)
				{
					std::shared_ptr<DelegateType> StdFuncPtr = std::make_shared<DelegateType>(Value);
					return StackOperatorWrapper<std::shared_ptr<DelegateType>>::Push(L, StdFuncPtr, &DelegateType::SetupMetaTable, true);
				}
				
				static RawDelegateType Check(lua_State* L, int32_t Pos)
				{
					if(lua_isnil(L, Pos))
					{
						return {};
					}
					else if(lua_isfunction(L, Pos))
					{
						using BindType = typename LuaCallableBinding<RawDelegateType>::Prototype;
						return BindType::MakeStdFunction(L, Pos);
					}
					else
					{
						std::shared_ptr<DelegateType> Result = StackOperatorWrapper<std::shared_ptr<DelegateType>>::Check(L, Pos);
						return *(Result->GetRawDelegatePtr());
					}
				}
				
			};
			template<typename R, typename ...ARGS> struct BuildInPushValue<std::function<R(ARGS...)>> { static constexpr bool Value = true; };
		}
	}

#ifdef TOLUAUUNREAL_API
	template<typename R, typename ...ARGS>
	struct ToLuauTFunction
	{
		using RawDelegateType = TFunction<R(ARGS...)>;
		using DelegateType = ToLuauTFunction<R, ARGS...>;
		
		ToLuauTFunction(TFunction<R(ARGS...)>* d);
		
		ToLuauTFunction(TFunction<R(ARGS...)> d);

		TFunction<R(ARGS...)>* GetRawDelegatePtr() const;

		TFunction<R(ARGS...)>* RawDelegate = nullptr;
		TSharedPtr<TFunction<R(ARGS...)>> SharedDelegate;



		template<class X>
		struct Functor;

		template<int32_t... Index>
		struct Functor<IntList<Index...>>
		{
			// index is int-list based 0, so should plus Offset to get first arg
			// (not include obj ptr if it's a member function)
			static R Invoke(lua_State* L, TFunction<R(ARGS...)>* FuncPtr)
			{
				if(FuncPtr)
				{
					auto Func = *FuncPtr;
					return Func(ArgOperator<typename remove_cr<ARGS>::type>::ReadArg(L, Index + 2)...);
				}
				return {};
			}
		};

		static int32_t Call(lua_State* L)
		{
			auto FuncPtr = StackAPI::Check<TFunction<R(ARGS...)>*>(L, 1);
			using I = MakeIntList<sizeof...(ARGS)>;
			if constexpr (TIsSame<R, void>::Value)
			{
				Functor<I>::Invoke(L, FuncPtr);
				return 0;
			}
			else
			{
				R Ret = Functor<I>::Invoke(L, FuncPtr);
				return StackAPI::Push(L, Ret);
			}
			
		}
		
		static int32_t SetupMetaTable(lua_State* L);
	};

	template <typename R, typename ... ARGS>
	ToLuauTFunction<R, ARGS...>::ToLuauTFunction(TFunction<R(ARGS...)>* d)
		: RawDelegate(d)
		, SharedDelegate(nullptr)
	{
	}

	template <typename R, typename ... ARGS>
	ToLuauTFunction<R, ARGS...>::ToLuauTFunction(TFunction<R(ARGS...)> d)
		: RawDelegate(nullptr)
	{
		SharedDelegate = MakeShared<TFunction<R(ARGS...)>>(d);
	}

	template <typename R, typename ... ARGS>
	TFunction<R(ARGS...)>* ToLuauTFunction<R, ARGS...>::GetRawDelegatePtr() const
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
	int32_t ToLuauTFunction<R, ARGS...>::SetupMetaTable(lua_State* L)
	{
		StackAPI::SetupMetaTableCommon(L);
			
		// ud meta
		lua_pushcfunction(L, &ToLuauTFunction::Call, "ToLuauBaseDelegate::Call");
		lua_setfield(L, -2, "__call");

		return 0;
	}

	template<typename R, typename ...ARGS>
	struct GetClassNameWrapper<ToLuauTFunction<R, ARGS...>>
	{
		static constexpr bool IsValid = true;
		static constexpr bool DefineByMacro = false;
		using Type = ToLuauTFunction<R, ARGS...>;
		static std::string GetName(const void* Obj)
		{
			return GetClassNameWrapper<typename ToLuauTFunction<R, ARGS...>::RawDelegateType>::GetName(Obj);
		}
	};


#endif
	
	namespace StackAPI
	{
		namespace __DETAIL__
		{

#ifdef TOLUAUUNREAL_API
			template<typename R, typename ... ARGS>
			struct StackOperatorWrapper<TFunction<R(ARGS...)>>
			{
				using RawDelegateType = TFunction<R(ARGS...)>;
				using DelegateType = ToLuauTFunction<R, ARGS...>;

				static int32_t Push(lua_State* L, RawDelegateType Value)
				{
					TSharedPtr<DelegateType> StdFuncPtr = MakeShared<DelegateType>(Value);
					return StackOperatorWrapper<TSharedPtr<DelegateType>>::Push(L, StdFuncPtr, &DelegateType::SetupMetaTable, true);
				}
				
				static RawDelegateType Check(lua_State* L, int32_t Pos)
				{
					if(lua_isnil(L, Pos))
					{
						static RawDelegateType Default;
						return Default;
					}
					else if(lua_isfunction(L, Pos))
					{
						using BindType = typename LuaCallableBinding<RawDelegateType>::Prototype;
						return BindType::MakeStdFunction(L, Pos);
					}
					else
					{
						TSharedPtr<DelegateType> Result = StackOperatorWrapper<TSharedPtr<DelegateType>>::Check(L, Pos);
						return *(Result->GetRawDelegatePtr());
					}
				}
				
			};
			template<typename R, typename ... ARGS> struct BuildInPushValue<TFunction<R(ARGS...)>> { static constexpr bool Value = true; };

			template<typename R, typename ... ARGS>
			struct StackOperatorWrapper<TFunction<R(ARGS...)>&>
			{
				using RawDelegateType = TFunction<R(ARGS...)>;
				using DelegateType = ToLuauTFunction<R, ARGS...>;
				
				static int32_t Push(lua_State* L, const RawDelegateType& Value)
				{
					using BindType = typename LuaCallableBinding<RawDelegateType>::Prototype;
					BindType::Func = &Value;
					lua_pushcfunction(L, BindType::Func, "");
					return 1;
				}
				
				static RawDelegateType& Check(lua_State* L, int32_t Pos)
				{
					static RawDelegateType Default;
					if(lua_isnil(L, Pos))
					{
						return Default;
					}
					else if(lua_isfunction(L, Pos))
					{
						TOLUAU_ASSERT(false);
						return Default;
					}
					
					TSharedPtr<DelegateType> Result = StackOperatorWrapper<TSharedPtr<DelegateType>>::Check(L, Pos);
					return *(Result->GetRawDelegatePtr());
				}
			};
			template<typename R, typename ... ARGS> struct BuildInPushValue<TFunction<R(ARGS...)>&> { static constexpr bool Value = true; };

			template<typename R, typename ... ARGS>
			struct StackOperatorWrapper<TFunction<R(ARGS...)>*>
			{
				using RawDelegateType = TFunction<R(ARGS...)>;
				using DelegateType = ToLuauTFunction<R, ARGS...>;
				
				static int32_t Push(lua_State* L, const RawDelegateType* Value)
				{
					using BindType = typename LuaCallableBinding<RawDelegateType>::Prototype;
					BindType::Func = Value;
					lua_pushcfunction(L, BindType::Func, "");
					return 1;
				}
				
				static RawDelegateType* Check(lua_State* L, int32_t Pos)
				{
					if(lua_isnil(L, Pos))
					{
						return nullptr;
					}
					else if(lua_isfunction(L, Pos))
					{
						TOLUAU_ASSERT(false);
						return nullptr;
					}
					
					TSharedPtr<DelegateType> Result = StackOperatorWrapper<TSharedPtr<DelegateType>>::Check(L, Pos);
					return Result->GetRawDelegatePtr();
				}
			};
			template<typename R, typename ... ARGS> struct BuildInPushValue<TFunction<R(ARGS...)>*> { static constexpr bool Value = true; };

#endif
			
		}
	}

	namespace StackAPI
	{
		namespace __DETAIL__
		{
			template<typename Callable>
			struct StackOperatorWrapper<Callable, typename std::enable_if<std::is_invocable<Callable>::value>::type>
			{
				static int32_t Push(lua_State* L, const Callable& Value)
				{
					using BindType = typename LuaCallableBinding<Callable>::Prototype;
					BindType::Func = &Value;
					lua_pushcfunction(L, BindType::Func, "");
					return 1;
				}
				
				static Callable Check(lua_State* L, int32_t Pos)
				{
					using BindType = typename LuaCallableBinding<Callable>::Prototype;
					return BindType::MakeStdFunction(L, Pos);
				}
			};
			template<typename Callable> struct BuildInPushValue<Callable, typename std::enable_if<std::is_invocable<Callable>::value>::type> { static constexpr bool Value = true; };
		}
	}

}
