#pragma once
// RTheme.h: interface for the RTheme class.
//
//////////////////////////////////////////////////////////////////////
#include <rcommon/rcommon.h>
#include <uxtheme.h>
#include <Vssym32.h>


typedef HRESULT (FAR WINAPI *CTDPROC)(HTHEME);
typedef HTHEME (FAR WINAPI *OTDPROC)(HWND, LPCWSTR);
typedef HRESULT (FAR WINAPI *DTBPROC)(HTHEME, HDC, int, int, LPCRECT, LPCRECT);
typedef HRESULT (FAR WINAPI *DTBEPROC)(HTHEME, HDC, int, int, LPCRECT, const DTBGOPTS*);
typedef HRESULT (FAR WINAPI *DTPBPROC)(HWND, HDC, LPRECT);
typedef HRESULT (FAR WINAPI *DTTPROC)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, LPCRECT);
typedef HRESULT (FAR WINAPI *DTEPROC)(HTHEME, HDC, int, int, LPCRECT, UINT, UINT, LPRECT);
typedef HRESULT (FAR WINAPI *GTBCRPROC)(HTHEME, HDC, int, int, LPCRECT, LPRECT);
typedef HRESULT (FAR WINAPI *GTFPROC)(HTHEME hTheme, HDC, int, int, int, LOGFONT*);

class RCOMMON_API RTheme  
{
public:
	RTheme();
	virtual ~RTheme();

	HRESULT CloseData() 
	{ 
		ASSERT(m_hTheme != NULL);
		HRESULT l_hr = (s_pfCloseData == NULL) ? S_OK : s_pfCloseData(m_hTheme);
		l_hr == S_OK ? m_hTheme = NULL : 0;
		return l_hr; 
	};

	HTHEME OpenData(HWND a_hWnd, LPCWSTR a_wsClassList) 
	{ 
		m_hTheme = (s_pfOpenData == NULL) ? NULL : s_pfOpenData(a_hWnd, a_wsClassList); 
		return m_hTheme; 
	};

	HRESULT DrawBackground(HDC a_hDC, int a_iPartId, int a_iStateId, LPCRECT a_pRect, LPCRECT a_pClipRect = NULL)
	{
		ASSERT(m_hTheme != NULL);

		TRACE0("Needs to be updated with checking of as below, look here: https://learn.microsoft.com/en-us/windows/win32/api/uxtheme/nf-uxtheme-drawthemebackground");
		//if (_hTheme)
		//{
		//	if (IsThemeBackgroundPartiallyTransparent(_hTheme, BP_PUSHBUTTON, _iStateId))
		//	{
		//		DrawThemeParentBackground(_hwnd, hdcPaint, prcPaint);
		//	}

		//	DrawThemeBackground(_hTheme,
		//		hdcPaint,
		//		BP_PUSHBUTTON,
		//		_iStateId,
		//		&rcClient,
		//		prcPaint);
		//}


		return (s_pfDrawBackground == NULL) ? S_OK : s_pfDrawBackground(m_hTheme, a_hDC, a_iPartId, a_iStateId, a_pRect, a_pClipRect);
	};

	HRESULT DrawBackgroundEx(HDC a_hDC, int a_iPartId, int a_iStateId, LPCRECT a_pRect, const DTBGOPTS* a_pOptions = NULL)
	{
		ASSERT(m_hTheme != NULL);
		return (s_pfDrawBackgroundEx == NULL) ? S_OK : s_pfDrawBackgroundEx(m_hTheme, a_hDC, a_iPartId, a_iStateId, a_pRect, a_pOptions);
	};

	HRESULT DrawParentBackground(HWND a_hWnd, HDC a_hDC, LPRECT a_pRect)
	{
		ASSERT(m_hTheme != NULL);
		return (s_pfDrawParentBackground == NULL) ? S_OK : s_pfDrawParentBackground(a_hWnd, a_hDC, a_pRect);
	};

	HRESULT DrawEdge(HDC a_hDC, int a_iPartId, int a_iStateId, LPCRECT a_pDestRect, UINT a_iEdge, UINT a_iFlags, LPRECT a_pContentRect)
	{
		ASSERT(m_hTheme != NULL);
		return (s_pfDrawEdge == NULL) ? S_OK : s_pfDrawEdge(m_hTheme, a_hDC, a_iPartId, a_iStateId, a_pDestRect, a_iEdge, a_iFlags, a_pContentRect);
	};


	HRESULT DrawText(HDC a_hDC, int a_iPartId, int a_iStateId, LPCTSTR a_sText, int a_iCharCount, DWORD a_dwTextFlags, DWORD a_dwTextFlags2, LPCRECT a_pRect);

	HRESULT GetBackgroundContentRect(HDC a_hDC, int a_iPartId, int a_iStateId, LPCRECT a_pBoundingRect, LPRECT a_pContentRect)
	{
		ASSERT(m_hTheme != NULL);
		return (s_pfGetBackgroundContentRect == NULL) ? S_OK : s_pfGetBackgroundContentRect(m_hTheme, a_hDC, a_iPartId, a_iStateId, a_pBoundingRect, a_pContentRect); 
	};

	HRESULT GetFont(HDC a_hDC, int a_iPartId, int a_iStateId, int a_iPropId, LOGFONT* a_pLogfont)
	{
		ASSERT(m_hTheme != NULL);
		return (s_pfGetFont == NULL) ? S_OK : s_pfGetFont(m_hTheme, a_hDC, a_iPartId, a_iStateId, a_iPropId, a_pLogfont); 
	};

private:
	HTHEME m_hTheme;

	static bool Initialize();
	static bool s_bInitialized;
	static CTDPROC s_pfCloseData;
	static DTBPROC s_pfDrawBackground;
	static DTBEPROC s_pfDrawBackgroundEx;
	static DTPBPROC s_pfDrawParentBackground;
	static DTEPROC s_pfDrawEdge;
	static OTDPROC s_pfOpenData;
	static DTTPROC s_pfDrawText;
	static GTBCRPROC s_pfGetBackgroundContentRect;
	static GTFPROC s_pfGetFont;
};

