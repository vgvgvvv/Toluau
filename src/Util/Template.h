#pragma once

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
}