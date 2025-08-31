// GetWinVer.cpp  Version 1.1
//
// Copyright (C) 2001-2003 Hans Dietrich
//
// This software is released into the public domain.  
// You are free to use it in any way you like, except
// that you may not sell this source code.
//
// This software is provided "as is" with no expressed 
// or implied warranty.  I accept no liability for any 
// damage or loss of business that this software may cause. 
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tchar.h"
#include "GetWinVer.h"
#include <Windows.h>



// from winbase.h
#ifndef VER_PLATFORM_WIN32s
#define VER_PLATFORM_WIN32s             0
#endif
#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS      1
#endif
#ifndef VER_PLATFORM_WIN32_NT
#define VER_PLATFORM_WIN32_NT           2
#endif
#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE           3
#endif


/*
    This table has been assembled from Usenet postings, personal
    observations, and reading other people's code.  Please feel
    free to add to it or correct it.


         dwPlatFormID  dwMajorVersion  dwMinorVersion  dwBuildNumber
95             1              4               0             950
95 SP1         1              4               0        >950 && <=1080
95 OSR2        1              4             <10           >1080
98             1              4              10            1998
98 SP1         1              4              10       >1998 && <2183
98 SE          1              4              10          >=2183
ME             1              4              90            3000

NT 3.51        2              3              51
NT 4           2              4               0            1381
2000           2              5               0            2195
XP             2              5               1            2600
2003 Server    2              5               2            3790

CE             3

*/



typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
#define WINVER_WINDOW7	_T("Windows 7")
#define WINVER_WINDOW8	_T("Windows 8")
#define WINVER_WINDOW81	_T("Windows 8.1")
#define WINVER_WINDOW10	_T("Windows 10")
#define WINVER_WINDOW11	_T("Windows 11")
#define WINVER_UNKNOWN	_T("Unknown Windows version")



/*
Operating System	Major	Minor	Build
Windows 7			6		1		7601
Windows 8			6		2		9200
Windows 8.1			6		3		9600
Windows 10			10		0		10240 +
Windows 11			10		0		21996 +
*/

// simplified and more up to date version of GetWinVer
BOOL GetWinVer_s(LPTSTR a_sVersion, size_t a_iVerMax, DWORD* a_pBuild)
{
	RTL_OSVERSIONINFOW rovi = { 0 };
	rovi.dwOSVersionInfoSize = sizeof(rovi);

	HMODULE l_hModule = ::GetModuleHandle(L"ntdll.dll");
	if (l_hModule == NULL)
		return FALSE;

	RtlGetVersionPtr l_fnRtlGetVersion = (RtlGetVersionPtr)GetProcAddress(l_hModule, "RtlGetVersion"); // there is no UNICODE version of GetProcAddress

	if (l_fnRtlGetVersion == NULL)
		return FALSE;


	if (l_fnRtlGetVersion(&rovi) != 0)
		return FALSE;

	if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 1)
		_tcsncpy_s(a_sVersion, a_iVerMax, WINVER_WINDOW7, _TRUNCATE);
	else if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 2)
		_tcsncpy_s(a_sVersion, a_iVerMax, WINVER_WINDOW8, _TRUNCATE);
	else if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 3)
		_tcsncpy_s(a_sVersion, a_iVerMax, WINVER_WINDOW81, _TRUNCATE);
	else if (rovi.dwMajorVersion == 10 && rovi.dwMinorVersion == 0 && rovi.dwBuildNumber < 21996)
		_tcsncpy_s(a_sVersion, a_iVerMax, WINVER_WINDOW10, _TRUNCATE);
	else if (rovi.dwMajorVersion == 10 && rovi.dwMinorVersion == 0 && rovi.dwBuildNumber >= 21996)
		_tcsncpy_s(a_sVersion, a_iVerMax, WINVER_WINDOW11, _TRUNCATE);
	else
		_tcsncpy_s(a_sVersion, a_iVerMax, WINVER_UNKNOWN, _TRUNCATE);
	
	*a_pBuild = rovi.dwBuildNumber;


	return TRUE;
}


/* original version of GetWinVer

///////////////////////////////////////////////////////////////////////////////
// GetWinVer
BOOL GetWinVer(LPTSTR pszVersion, int *nVersion, LPTSTR pszMajorMinorBuild)
{
	if (!pszVersion || !nVersion || !pszMajorMinorBuild)
		return FALSE;
	lstrcpy(pszVersion, WUNKNOWNSTR);
	*nVersion = WUNKNOWN;

	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (!GetVersionEx(&osinfo))
		return FALSE;

	DWORD dwPlatformId   = osinfo.dwPlatformId;
	DWORD dwMinorVersion = osinfo.dwMinorVersion;
	DWORD dwMajorVersion = osinfo.dwMajorVersion;
	DWORD dwBuildNumber  = osinfo.dwBuildNumber & 0xFFFF;	// Win 95 needs this

	wsprintf(pszMajorMinorBuild, _T("%u.%u.%u"), dwMajorVersion, dwMinorVersion, dwBuildNumber);

	if ((dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && (dwMajorVersion == 4))
	{
		if ((dwMinorVersion < 10) && (dwBuildNumber == 950))
		{
			lstrcpy(pszVersion, W95STR);
			*nVersion = W95;
		}
		else if ((dwMinorVersion < 10) && 
				((dwBuildNumber > 950) && (dwBuildNumber <= 1080)))
		{
			lstrcpy(pszVersion, W95SP1STR);
			*nVersion = W95SP1;
		}
		else if ((dwMinorVersion < 10) && (dwBuildNumber > 1080))
		{
			lstrcpy(pszVersion, W95OSR2STR);
			*nVersion = W95OSR2;
		}
		else if ((dwMinorVersion == 10) && (dwBuildNumber == 1998))
		{
			lstrcpy(pszVersion, W98STR);
			*nVersion = W98;
		}
		else if ((dwMinorVersion == 10) && 
				((dwBuildNumber > 1998) && (dwBuildNumber < 2183)))
		{
			lstrcpy(pszVersion, W98SP1STR);
			*nVersion = W98SP1;
		}
		else if ((dwMinorVersion == 10) && (dwBuildNumber >= 2183))
		{
			lstrcpy(pszVersion, W98SESTR);
			*nVersion = W98SE;
		}
		else if (dwMinorVersion == 90)
		{
			lstrcpy(pszVersion, WMESTR);
			*nVersion = WME;
		}
	}
	else if (dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if ((dwMajorVersion == 3) && (dwMinorVersion == 51))
		{
			lstrcpy(pszVersion, WNT351STR);
			*nVersion = WNT351;
		}
		else if ((dwMajorVersion == 4) && (dwMinorVersion == 0))
		{
			lstrcpy(pszVersion, WNT4STR);
			*nVersion = WNT4;
		}
		else if ((dwMajorVersion == 5) && (dwMinorVersion == 0))
		{
			lstrcpy(pszVersion, W2KSTR);
			*nVersion = W2K;
		}
		else if ((dwMajorVersion == 5) && (dwMinorVersion == 1))
		{
			lstrcpy(pszVersion, WXPSTR);
			*nVersion = WXP;
		}
		else if ((dwMajorVersion == 5) && (dwMinorVersion == 2))
		{
			lstrcpy(pszVersion, W2003SERVERSTR);
			*nVersion = W2003SERVER;
		}
	}
	else if (dwPlatformId == VER_PLATFORM_WIN32_CE)
	{
		lstrcpy(pszVersion, WCESTR);
		*nVersion = WCE;
	}
	return TRUE;
}
*/