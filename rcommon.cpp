// rcommon.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "rcommon.h"

#ifdef RCOMMON_DLL //DLL

HINSTANCE g_hModule = NULL;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hModule = static_cast<HINSTANCE>(hModule);
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

HINSTANCE RCommon_GetInstance(void)
{
	ASSERT(g_hModule != NULL);
	return g_hModule;
}


#else // STATIC LIBRARY
HINSTANCE RCommon_GetInstance(void)
{
	return ::GetModuleHandle(NULL);
}

#endif
