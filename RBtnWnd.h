 #pragma once
// RBtnWnd.h: interface for the RBtnWnd control
//
//////////////////////////////////////////////////////////////////////
#include <rcommon/rcommon.h>

#define RBtnWnd_ClassName _T("RBTNWND")

extern RCOMMON_API ATOM RBtnWnd_RegisterClass();
extern RCOMMON_API HWND RBtnWnd_CreateEx(DWORD a_iStyle, DWORD a_iStyleEx, LPCTSTR a_sCaption, int a_x, int a_y, int a_iWidth,
	int a_iHeight, HWND	a_hWndParent, UINT a_idCtrl);
extern RCOMMON_API LRESULT CALLBACK RBtnWnd_WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);


typedef struct S_SETBITMAP
{
	HINSTANCE hInst;  
	union
	{
		UINT	idBmp;
		HBITMAP hBmp;
	};
	COLORREF  clrMask;
	int       iCount;
} TSetBitmap, *LPTSetBitmap;


// sets bitmap info WPARAM = unused, LPARAM = bitmap info (LPTSetBitmap), LRESULT = success (BOOL)
#define RBWM_SETBITMAP (WM_USER + 1001)
#define RBtnWnd_SetBitmap(a_hWnd, a_pSetBmp) (::SendMessage(a_hWnd, RBWM_SETBITMAP, 0, reinterpret_cast<LPARAM>(a_pSetBmp)))

// gets bitmap info WPARAM = unused, LPARAM = a_pSetBmp (LPTSetBitmap) 
#define RBWM_GETBITMAP (WM_USER + 1002)
#define RBtnWnd_GetBitmap(a_hWnd, a_pSetBmp) (::SendMessage(a_hWnd, RBWM_GETBITMAP, 0, reinterpret_cast<LPARAM>(a_pSetBmp)))

typedef struct S_BTNTIPDELAY
{
	UINT iShow; // in miliseconds (show tip after iShow miliseconds)
	UINT iHide; // in miliseconds (hide tip after iHide miliseconds)
} RBTNTIPDELAY, *LPRBTNTIPDELAY;

#define RBWM_SETTIP (WM_USER + 1003) // WPARAM = LPRBTNTIPDELAY, LPARAM = a_sTip (LPTSTR) 
#define RBtnWnd_SetTip(a_hWnd, a_sTip, a_pTipDelay) (::SendMessage(a_hWnd, RBWM_SETTIP, reinterpret_cast<WPARAM>(a_pTipDelay), reinterpret_cast<LPARAM>(a_sTip)))

// sets text color WPARAM = unused, LPARAM = a_clrText (COLORREF) 
#define RBWM_SETTEXTCOLOR (WM_USER + 1005)
#define RBtnWnd_SetTextColor(a_hWnd, a_clrText) (::SendMessage(a_hWnd, RBWM_SETTEXTCOLOR, 0, static_cast<LPARAM>(a_clrText)))

// gets text color WPARAM = unused, LPARAM = UNUSED, LRESULT = COLORREF
#define RBWM_GETTEXTCOLOR (WM_USER + 1006)
#define RBtnWnd_GetTextColor(a_hWnd) (static_cast<COLORREF>(::SendMessage(a_hWnd, RBWM_GETTEXTCOLOR, 0, 0L)))

#define RBCS_TBSTYLE		(0x0001L)	// for toolbars
#define RBCS_RGN			(0x0002L)	// sets button rgn from bitmap using transpraent color
