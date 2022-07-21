#pragma once

#include "StackAPI.h"
#include "Toluau/Util/Template.h"

// reference the way of function binding from slua unreal
// https://github.com/Tencent/sluaunreal/blob/master/Plugins/slua_unreal/Source/slua_unreal/Public/LuaCppBinding.h

namespace ToLuau
{

	struct ArgOperator
	{
		template<typename T>
		static T ReadArg(lua_State* L, int Pos)
		{
			return StackAPI::Check<T>(L, Pos);
		}
	};


	#pragma region FunctionBind

	template<typename T, T, int Offset>
	struct FunctionBind;

	template<lua_CFunction TargetFunc, int Offset>
	struct FunctionBind<lua_CFunction, TargetFunc, Offset>
	{
		static int32_t Invoke(lua_State* L)
		{
			return TargetFunc(L);
		}
	};

	// invoke with args and return
	template<typename TRet, typename... Args,
			TRet (*TargetFunc)(lua_State* L, void*, Args...), int Offset>
	struct FunctionBind<TRet(*)(lua_State* L, void*, Args...), TargetFunc, Offset>
	{
		template<class X>
		struct Functor;

		template<int32_t... Index>
		struct Functor<IntList<Index...>>
		{
			// index is int-list based 0, so should plus Offset to get first arg
			// (not include obj ptr if it's a member function)
			static TRet Invoke(lua_State* L, void* Ptr)
			{
				return TargetFunc(L, Ptr, ArgOperator::ReadArg<typename remove_cr<Args>::type>(L, Index + Offset)...);
			}
		};

		template<typename VT, bool Value = std::is_pointer<TRet>::value>
		struct ReturnPointer
		{
			constexpr static void* GetValue(VT& t)
			{
				return (void*)t;
			}
		};

		template<typename VT>
		struct ReturnPointer<VT, false>
		{
			constexpr static void* GetValue(VT& t)
			{
				return (void*)(&t);
			}
		};

		static int Invoke(lua_State* L, void* Ptr)
		{
			using I = MakeIntList<sizeof...(Args)>;
			TRet Ret = Functor<I>::Invoke(L, Ptr);
			void* V = ReturnPointer<TRet>::GetValue(Ret);
			if(V == nullptr)
			{
				return StackAPI::Push(L, nullptr);
			}
			return StackAPI::Push(L, Ret);
		}

	};

	// invoke with args without return value
	template<typename... Args, void(*TargetFunc)(lua_State* L, void*, Args...), int Offset>
	struct FunctionBind<void(*)(lua_State* L, void*, Args...), TargetFunc, Offset>
	{
		template<class X>
		struct Functor;

		template<int32_t... Index>
		struct Functor<IntList<Index...>>
		{
			// index is int-list based 0, so should plus Offset to get first arg
			// (not include obj ptr if it's a member function)
			static void Invoke(lua_State* L, void* Ptr)
			{
				TargetFunc(L, Ptr, ArgOperator::ReadArg<typename remove_cr<Args>::type>(L, Index + Offset)...);
			}
		};

		static int32_t Invoke(lua_State* L, void* Ptr)
		{
			using I = MakeIntList<sizeof...(Args)>;
			Functor<I>::Invoke(L, Ptr);
			return 0;
		}
	};

	#pragma endregion

	#pragma region LuaFunctionBind

	template<typename T, T, int32_t Offset = 1>
	struct LuaCppBinding;

	// non member function invoke
	template<typename TRet, typename... TArgs, TRet(*TargetFunc)(TArgs...), int Offset>
	struct LuaCppBinding<TRet(*)(TArgs...), TargetFunc, Offset>
	{
		static constexpr bool IsStatic = !std::is_member_function_pointer<decltype(TargetFunc)>::value;

		static TRet Invoke(lua_State* L, void* Ptr, TArgs&&... InArgs)
		{
			return TargetFunc(std::forward<TArgs>(InArgs)...);
		}

		static int32_t LuaCFunction(lua_State* L)
		{
			using F = FunctionBind<decltype(&Invoke), Invoke, Offset>;
			return F::Invoke(L, nullptr);
		}
	};

	// const member function invoke
	template<typename TOwner, typename TRet, typename... TArgs, TRet (TOwner::*TargetFunc)(TArgs...) const>
	struct LuaCppBinding<TRet (TOwner::*)(TArgs...) const, TargetFunc>
	{
		static constexpr bool IsStatic = !std::is_member_function_pointer<decltype(TargetFunc)>::value;

		static TRet Invoke(lua_State* L, void* Ptr, TArgs&&... Args)
		{
			TOwner* ThisPtr = (TOwner*)Ptr;
			return (ThisPtr->*TargetFunc)(std::forward<TArgs>(Args)...);
		}

		static int32_t LuaCFunction(lua_State* L)
		{
			void* P = StackAPI::Check<TOwner*>(L, 1);
			using F = FunctionBind<decltype(&Invoke), Invoke, 2>;
			return F::Invoke(L, P);
		}
	};


	template<typename TOwner, typename  TRet, typename... TArgs, TRet (TOwner::*TargetFunc)(TArgs...)>
	struct LuaCppBinding<TRet(TOwner::*)(TArgs...), TargetFunc>
	{
		static constexpr bool IsStatic = !std::is_member_function_pointer<decltype(TargetFunc)>::value;

		static TRet Invoke(lua_State* L, void* Ptr, TArgs&&... Args)
		{
			TOwner* ThisPtr = (TOwner*)Ptr;
			return (ThisPtr->*TargetFunc)(std::forward<TArgs>(Args)...);
		}

		static int32_t LuaCFunction(lua_State* L)
		{
			void* P = StackAPI::Check<TOwner*>(L, 1);
			using F = FunctionBind<decltype(&Invoke), Invoke, 2>;
			return F::Invoke(L, P);
		}
	};

	template<typename TOwner, typename... TArgs, void (TOwner::*TargetFunc)(TArgs...) const>
	struct LuaCppBinding<void(TOwner::*)(TArgs...), TargetFunc>
	{
		static constexpr bool IsStatic = !std::is_member_function_pointer<decltype(TargetFunc)>::value;

		static void Invoke(lua_State* L, void* Ptr, TArgs&&... Args)
		{
			TOwner* ThisPtr = (TOwner*)Ptr;
			return (ThisPtr->*TargetFunc)(std::forward<TArgs>(Args)...);
		}

		static int32_t LuaCFunction(lua_State* L)
		{
			void* P = StackAPI::Check<TOwner*>(L, 1);
			using F = FunctionBind<decltype(&Invoke), Invoke, 2>;
			return F::Invoke(L, P);
		}
	};

	template<typename TOwner, typename... TArgs, void (TOwner::*TargetFunc)(TArgs...)>
	struct LuaCppBinding<void(TOwner::*)(TArgs...), TargetFunc>
	{
		static constexpr bool IsStatic = !std::is_member_function_pointer<decltype(TargetFunc)>::value;

		static void Invoke(lua_State* L, void* Ptr, TArgs&&... Args)
		{
			TOwner* ThisPtr = (TOwner*)Ptr;
			return (ThisPtr->*TargetFunc)(std::forward<TArgs>(Args)...);
		}

		static int32_t LuaCFunction(lua_State* L)
		{
			void* P = StackAPI::Check<TOwner*>(L, 1);
			using F = FunctionBind<decltype(&Invoke), Invoke, 2>;
			return F::Invoke(L, P);
		}
	};

	template<int32_t(*TargetFunc)(lua_State* L), int32_t Offset>
	struct LuaCppBinding<int32_t(*)(lua_State*L), TargetFunc, Offset>
	{
		static int32_t LuaCFunction(lua_State* L)
		{
			return TargetFunc(L);
		}
	};

	template<int32_t Offset>
	struct LuaCppBinding<decltype(nullptr), nullptr, Offset>
	{
		static int32_t LuaCFunction(lua_State* L)
		{
			luaL_error(L, "Cannot be accessed");
			return 0;
		}
	};


	#pragma endregion

	template<typename CallableType, typename ReturnType, typename ... ArgTypes>
	struct CallableExpand
	{
		typedef CallableType* FuncType;
		typedef std::function<ReturnType(ArgTypes...)> StdFunctionType;
#ifdef TOLUAUUNREAL_API
		typedef TFunction<ReturnType(ArgTypes...)> TFunctionType;
#endif
		inline static ReturnType invoke(lua_State* L, void* ptr, ArgTypes&& ... args)
		{
			return Func != nullptr ? (*Func)(std::forward<ArgTypes>(args)...) : ReturnType();
		}

		static int LuaCFunction(lua_State* L);

		static StdFunctionType MakeStdFunction(lua_State* L, int32_t Pos)
		{
			luaL_checktype(L, Pos, LUA_TFUNCTION);
			ToLuauVar Func(L, Pos);
			if(Func.IsValid() && Func.IsFunction())
			{
				return [=](ArgTypes&& ... Args) -> ReturnType
				{
					ToLuauVar Result = Func.Call(std::forward<ArgTypes>(Args)...);
					return Result.Get<ReturnType>();
				};
			}
			else
			{
				return nullptr;
			}
		}

#ifdef TOLUAUUNREAL_API
		static TFunctionType MakeTFunction(lua_State* L, int32_t Pos)
		{
			luaL_checktype(L, Pos, LUA_TFUNCTION);
			ToLuauVar Func(L, Pos);
			if(Func.IsValid() && Func.IsFunction())
			{
				return [=](ArgTypes&& ... Args) -> ReturnType
				{
					ToLuauVar Result = Func.Call(std::forward<ArgTypes>(Args)...);
					return Result.Get<ReturnType>();
				};
			}
			else
			{
				return nullptr;
			}
		}
#endif

		static FuncType Func;
	};
	
	template<typename CallableType, typename ReturnType, typename ... ArgTypes>
	typename CallableExpand<CallableType, ReturnType, ArgTypes ...>::FuncType CallableExpand<CallableType, ReturnType, ArgTypes ...>::Func = nullptr;
	
	template<typename CallableType, typename ReturnType, typename ... ArgTypes>
	int CallableExpand<CallableType, ReturnType, ArgTypes...>::LuaCFunction(lua_State* L)
	{
		return FunctionBind<decltype(&invoke), invoke, 1>::invoke(L, nullptr);
	}

	template<typename CallableType>
	struct LuaCallableBinding
	{
		template<typename ClassType, typename ReturnType, typename ... ArgType>
		static auto DeducePrototype(ReturnType(ClassType::*)(ArgType ...) const)
		{
			return CallableExpand<CallableType, ReturnType, ArgType ...>();
		}

		template<typename ClassType, typename ReturnType, typename ... ArgType>
		static auto DeducePrototype(ReturnType(ClassType::*)(ArgType ...))
		{
			return CallableExpand<CallableType, ReturnType, ArgType ...>();
		}

		typedef decltype(DeducePrototype(&CallableType::operator())) Prototype;
	};

	namespace StackAPI
	{
		namespace __DETAIL__
		{
			template<typename T>
			struct StackOperatorWrapper<T, typename std::enable_if<std::is_function<T>::value>::type>
			{
				static int32_t Push(lua_State* L, const T& Value)
				{
					using BindType = typename LuaCallableBinding<T>::Prototype;
					BindType::Func = &Value;
					lua_pushcfunction(L, BindType::Func, "");
					return 1;
				}
				
				static T Check(lua_State* L, int32_t Pos)
				{
					using BindType = typename LuaCallableBinding<T>::Prototype;
					return BindType::MakeStdFunction(L, Pos);
				}
			};

#ifdef TOLUAUUNREAL_API
			template<typename T>
			struct StackOperatorWrapper<T, typename std::enable_if<TIsFunction<T>::Value>::type>
			{
				static int32_t Push(lua_State* L, const T& Value)
				{
					using BindType = typename LuaCallableBinding<T>::Prototype;
					BindType::Func = &Value;
					lua_pushcfunction(L, BindType::Func, "");
					return 1;
				}
				
				static T Check(lua_State* L, int32_t Pos)
				{
					using BindType = typename LuaCallableBinding<T>::Prototype;
					return BindType::MakeTFunction(L, Pos);
				}
				
			};
#endif

			template<typename Callable>
			struct StackOperatorWrapper<Callable, typename std::enable_if<is_invocable<Callable>::value>::type>
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
		}
	}
	
}
