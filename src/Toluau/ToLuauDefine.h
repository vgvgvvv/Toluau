#pragma once

#ifdef TOLUAUUNREAL_API

#include "Toluau/ToLuauUnreal_API.h"
#define TOLUAU_ASSERT(expr) \
	check(expr)

#else

#include <cassert>
#include "ToLuau_API.h"
#define TOLUAU_ASSERT(expr) \
	assert(expr)
#endif

#ifndef USE_TOLUAU
	#define USE_TOLUAU 0
#endif

#ifndef LUAU_SUPPORT_HOYO_CLASS
	#define LUAU_SUPPORT_HOYO_CLASS 0
#endif

#ifndef ToLuauDebug
	#define ToLuauDebug 1
#endif

namespace ToLuau
{
	using luau_int64 = long long;
	using luau_uint64 = unsigned long long;
}