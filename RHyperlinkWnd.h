#pragma once
// RHyperlinkWnd.h: interface for the RHyperlinkWnd control
//
//////////////////////////////////////////////////////////////////////
#include "rcommon.h"

#define RHyperlinkWnd_ClassName _T("RHYPERLINKWND")

typedef struct S_HYPERLINKCOLORS
{
	COLORREF clrLink;
	COLORREF clrActive;
	COLORREF clrVisited;
	COLORREF clrHover;
} RHYPERLINKCOLORS, *LPRHYPERLINKCOLORS;


#define HS_LEFT		0x0000
#define HS_CENTER	0x0001
#define HS_RIGHT	0x0002
#define HS_NOTIP	0x0004

extern RCOMMON_API ATOM RHyperlinkWnd_RegisterClass(void);
extern RCOMMON_API HWND RHyperlinkWnd_CreateEx(DWORD a_iStyle, DWORD a_iStyleEx, LPCTSTR a_sText, int a_x, int a_y, int a_iWidth,
	int a_iHeight, HWND	a_hWndParent, LPVOID a_lpParam);
extern RCOMMON_API LRESULT CALLBACK RHyperlinkWnd_WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);

// sets hyperlink colors WPARAM = redraw (BOOL), LPARAM = colors (LPRHYPERLINKCOLORS)
#define RHM_SETCOLORS (WM_USER + 1001)
#define RHyperlinkWnd_SetColors(a_hWnd, a_pColors, a_bRedraw) (::SendMessage(a_hWnd, RHM_SETCOLORS, a_bRedraw, reinterpret_cast<LPARAM>(a_pColors)))

// gets hyperlink colors WPARAM = unused, LPARAM = colors (LPRHYPERLINKCOLORS)
#define RHM_GETCOLORS (WM_USER + 1002)
#define RHyperlinkWnd_GetColors(a_hWnd, a_pColors) (::SendMessage(a_hWnd, RHM_GETCOLORS, 0, reinterpret_cast<LPARAM>(a_pColors)))

// sets hyperlink followed WPARAM = redraw (BOOL), LPARAM = followed (BOOL)
#define RHM_SETFOLLOWED (WM_USER + 1003)
#define RHyperlinkWnd_SetFollowed(a_hWnd, a_bFollowed, a_bRedraw) (::SendMessage(a_hWnd, RHM_SETFOLLOWED, a_bRedraw, a_bFollowed))

// gets hyperlink followed WPARAM = unused, LPARAM = unused, returns followed (BOOL)
#define RHM_GETFOLLOWED (WM_USER + 1004)
#define RHyperlinkWnd_GetFollowed(a_hWnd) (::SendMessage(a_hWnd, RHM_GETFOLLOWED, 0, OL))

// sets hyperlink url WPARAM = unused, LPARAM = url (LPCTSTR)
#define RHM_SETURL (WM_USER + 1005)
#define RHyperlinkWnd_SetUrl(a_hWnd, a_sUrl) (::SendMessage(a_hWnd, RHM_SETURL, 0, reinterpret_cast<LPARAM>(a_sUrl)))

// gets hyperlink url WPARAM = buffer size (int), LPARAM = buffer for url (LPTSTR) returns TCHAR copied
#define RHM_GETURL (WM_USER + 1006)
#define RHyperlinkWnd_GetUrl(a_hWnd, a_iSize, a_psUrl) (::SendMessage(a_hWnd, RHM_GETURL, a_iSize, reinterpret_cast<LPARAM>(a_sUrl)))

// gets hyperlink url length WPARAM = unused, LPARAM = unused returns length in TCHAR without trailing zero
#define RHM_GETURLLENGTH (WM_USER + 1007)
#define RHyperlinkWnd_GetUrlLength(a_hWnd) (::SendMessage(a_hWnd, RHM_GETURLLENGTH, a_iSize, reinterpret_cast<LPARAM>(a_sUrl)))

// sets cursor WPARAM = unused, LPARAM = cursor (HCURSOR)
#define RHM_SETCURSOR (WM_USER + 1008)
#define RHyperlinkWnd_SetCursor(a_hWnd, a_hCursor) (::SendMessage(a_hWnd, RHM_SETCURSOR, 0, reinterpret_cast<LPARAM>(a_hCursor)))

// gets cursor WPARAM = unused, LPARAM = unused, returns cursor (HCURSOR)
#define RHM_GETCURSOR (WM_USER + 1009)
#define RHyperlinkWnd_GetCursor(a_hWnd) (::SendMessage(a_hWnd, RHM_GETCURSOR, 0, OL))

#define RKN_FIRST			(WM_USER + 0x4020)
