// RTheme.cpp: implementation of the RTheme class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RTheme.h"
#include <tchar.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
bool RTheme::s_bInitialized = Initialize();
CTDPROC RTheme::s_pfCloseData = NULL;
DTBPROC RTheme::s_pfDrawBackground = NULL;
DTBEPROC RTheme::s_pfDrawBackgroundEx = NULL;
DTPBPROC RTheme::s_pfDrawParentBackground = NULL;
DTEPROC RTheme::s_pfDrawEdge = NULL;
OTDPROC RTheme::s_pfOpenData = NULL;
DTTPROC RTheme::s_pfDrawText = NULL;
GTBCRPROC RTheme::s_pfGetBackgroundContentRect = NULL;
GTFPROC RTheme::s_pfGetFont = NULL;
GTSCPROC RTheme::s_pfGetSysColor = NULL;
GTCPROC RTheme::s_pfGetThemeColor = NULL;


RTheme::RTheme()
{
	m_hTheme = NULL;
}

RTheme::~RTheme()
{
	if (m_hTheme != NULL)
	{
		CloseData();
	}
}


bool
RTheme::Initialize()
{
	HMODULE l_hModule = ::LoadLibrary(_T("uxtheme"));
	if (l_hModule == NULL)
	{
		return true;
	}

	// TODO: replace with casual calls instead of pointers to functions
	s_pfCloseData = (CTDPROC)::GetProcAddress(l_hModule, "CloseThemeData");
	s_pfDrawBackground = (DTBPROC)::GetProcAddress(l_hModule, "DrawThemeBackground");
	s_pfDrawBackgroundEx = (DTBEPROC)::GetProcAddress(l_hModule, "DrawThemeBackgroundEx");
	s_pfDrawParentBackground = (DTPBPROC)::GetProcAddress(l_hModule, "DrawThemeParentBackground");
	s_pfDrawEdge = (DTEPROC)::GetProcAddress(l_hModule, "DrawThemeEdge");
	s_pfOpenData = (OTDPROC)::GetProcAddress(l_hModule, "OpenThemeData");
	s_pfDrawText = (DTTPROC)::GetProcAddress(l_hModule, "DrawThemeText");
	s_pfGetBackgroundContentRect = (GTBCRPROC)::GetProcAddress(l_hModule, "GetThemeBackgroundContentRect");	
	s_pfGetFont = reinterpret_cast<GTFPROC>(::GetProcAddress(l_hModule, "GetThemeFont"));
	s_pfGetSysColor = reinterpret_cast<GTSCPROC>(::GetProcAddress(l_hModule, "GetThemeSysColor"));
	s_pfGetThemeColor = reinterpret_cast<GTCPROC>(::GetProcAddress(l_hModule, "GetThemeColor"));
	return true;
}


HRESULT RTheme::DrawText(HDC a_hDC, int a_iPartId, int a_iStateId, LPCTSTR a_sText, int a_iCharCount, DWORD a_dwTextFlags, DWORD a_dwTextFlags2, LPCRECT a_pRect)
{
	ASSERT(m_hTheme != NULL);
	if (s_pfDrawText == NULL) 
	{
		return S_OK;
	}

	long l_iLen = static_cast<long>(_tcslen(a_sText));
	wchar_t* l_wsText = new wchar_t[l_iLen + 1];

#ifdef _UNICODE
		wcsncpy_s(l_wsText, l_iLen + 1, a_sText, _TRUNCATE);
#else
		MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, a_sText, l_iLen + 1,
			l_wsText, l_iLen + 1);
#endif

	HRESULT l_hRes = s_pfDrawText(m_hTheme, a_hDC, a_iPartId, a_iStateId, l_wsText, a_iCharCount, a_dwTextFlags, a_dwTextFlags2, a_pRect);
	delete[] l_wsText;
	return l_hRes;
}

