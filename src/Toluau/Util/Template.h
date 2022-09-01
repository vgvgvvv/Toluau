#pragma once
#ifdef TOLUAUUNREAL_API
#include "UObject/StrongObjectPtr.h"
#endif

namespace ToLuau
{
	template<int ...>
	struct IntList
	{
	};

	template<typename L, typename R>
	struct Concat;

	template<int... TL, int... TR>
	struct Concat<IntList<TL...>, IntList<TR...>>
	{
		typedef IntList<TL..., TR...> type;
	};

	template<int n>
	struct MakeIntList_t
	{
		typedef typename Concat<typename MakeIntList_t<n - 1>::type, IntList<n - 1>>::type type;
	};

	template<>
	struct MakeIntList_t<0>
	{
		typedef IntList<> type;
	};

	template<int n>
	using MakeIntList = typename MakeIntList_t<n>::type;

	template<typename T>
	struct remove_cr
	{
		typedef T type;
	};

	template<typename T>
	struct remove_cr<const T&>
	{
		typedef typename remove_cr<T>::type type;
	};

	template<typename T>
	struct remove_cr<T&>
	{
		typedef typename remove_cr<T>::type type;
	};

	template<typename T>
	struct remove_cr<T&&>
	{
		typedef typename remove_cr<T>::type type;
	};

	template<class T>
	struct remove_ptr_const
	{
		typedef T type;
	};

	template<class T>
	struct remove_ptr_const<const T*>
	{
		typedef T* type;
	};
	
	template <typename F, typename... Args>
	struct is_invocable :
		std::is_constructible<
			std::function<void(Args ...)>,
			std::reference_wrapper<typename std::remove_reference<F>::type>
		>
	{
	};

	template <typename R, typename F, typename... Args>
	struct is_invocable_r :
		std::is_constructible<
			std::function<R(Args ...)>,
			std::reference_wrapper<typename std::remove_reference<F>::type>
		>
	{
	};

// SFINAE test
	template <typename T>
	class HasStaticLuaClass
	{
		typedef char one;
		struct two { char x[2]; };

		template <typename C>
		static one test( decltype(&C::StaticLuaClass) ) ;
		template <typename C>
		static two test(...);

	public:
		enum { Value = sizeof(test<T>(0)) == sizeof(char) };
	};

#if LUAU_SUPPORT_HOYO_CLASS
	template <typename T>
	class HasStaticHoYoClass
	{
		typedef char one;
		struct two { char x[2]; };

		template <typename C>
		static one test( decltype(&C::StaticHoYoClass) ) ;
		template <typename C>
		static two test(...);

	public:
		enum { Value = sizeof(test<T>(0)) == sizeof(char) };
	};
#endif

#ifdef TOLUAUUNREAL_API
	// SFINAE test
	template <typename T>
	class HasStaticClass
	{
		typedef char one;
		struct two { char x[2]; };

		template <typename C>
		static one test( decltype(&C::StaticClass) ) ;
		template <typename C>
		static two test(...);

	public:
		enum { Value = sizeof(test<T>(0)) == sizeof(char) };
	};

	// SFINAE test
	template <typename T>
	class HasStaticStruct
	{
		typedef char one;
		struct two { char x[2]; };

		template <typename C>
		static one test( decltype(&C::StaticStruct) ) ;
		template <typename C>
		static two test(...);

	public:
		enum { Value = sizeof(test<T>(0)) == sizeof(char) };
	};
#endif

	template<size_t I = 0, typename... Tp, typename F>
	void ForEachApply(std::tuple<Tp...>& t, F &&f) {
		f(std::get<I>(t));
		if constexpr(I+1 != sizeof...(Tp)) {
			ForEachApply<I+1>(t, std::forward<F>(f));
		}
	}
	
	template<typename T>
	struct RawClass
	{
		using Type = T;
	};

	template<typename T>
	struct RawClass<const T>;

	template<typename T>
	struct RawClass<T&>;

	template<typename T>
	struct RawClass<T*>;
	
	template<typename T>
	struct RawClass<const T>
	{
		using Type = typename RawClass<T>::Type;
	};
	
	template<typename T>
	struct RawClass<T&>
	{
		using Type = typename RawClass<T>::Type;
	};

	template<typename T>
	struct RawClass<T*>
	{
		using Type = typename RawClass<T>::Type;
	};

	template<typename T>
	using RawClass_T = typename RawClass<T>::Type;
	
	
}
