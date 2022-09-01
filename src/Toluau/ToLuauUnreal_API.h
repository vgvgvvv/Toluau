#pragma once

#ifdef TOLUAUUNREAL_API
	#define ToLuau_API TOLUAUUNREAL_API
	#define ToLuauDebug !UE_BUILD_SHIPPING && !UE_BUILD_TEST
#endif
