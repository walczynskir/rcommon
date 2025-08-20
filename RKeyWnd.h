#pragma once
// RKeyWnd.h: interface for the RKeyWnd class.
//
//////////////////////////////////////////////////////////////////////
#include "rcommon.h"

#define RKeyWnd_ClassName _T("RKEYWND")

typedef struct S_SHORTCUTKEY
{
	UINT iMod;
	UINT iKey;
} RSHORTCUTKEY, *LPRSHORTCUTKEY;

extern RCOMMON_API ATOM RKeyWnd_RegisterClass();
extern RCOMMON_API HWND RKeyWnd_CreateEx(DWORD a_iStyle, DWORD a_iStyleEx, int a_x, int a_y, int a_iWidth,
	int a_iHeight, HWND	a_hWndParent, LPVOID a_lpParam);
extern RCOMMON_API LRESULT CALLBACK RKeyWnd_WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);
extern RCOMMON_API int RKeyDown_GetKeyDesc(LPRSHORTCUTKEY a_pKey, LPTSTR a_sDesc, UINT a_iDescLen);

extern RCOMMON_API UINT RKeyDown_ToFModifier(BYTE a_btMod);
extern RCOMMON_API UINT RKeyDown_ToMModifier(BYTE a_btMod);



// sets text when ctrl empty WPARAM = unused, LPARAM = text (LPCTSTR)
#define RCM_SETEMPTYTEXT (WM_USER + 1001)
#define RKeyWnd_SetEmptyText(a_hWnd, a_sEmptyText) (::SendMessage(a_hWnd, RCM_SETEMPTYTEXT, 0, reinterpret_cast<LPARAM>(a_sEmptyText)))

// gets text when ctrl empty WPARAM = unused, LPARAM = text (LPTSTR)
#define RCM_GETEMPTYTEXT (WM_USER + 1002)
#define RKeyWnd_GetEmptyText(a_hWnd, a_sEmptyText, a_ilen) (::SendMessage(a_hWnd, RCM_GETEMPTYTEXT, reinterpret_cast<WPARAM>(a_iLen), reinterpret_cast<LPARAM>(a_sEmptyText)))

// sets shortcut key WPARAM = 0, LPARAM = key (LPRSHORTCUTKEY)
#define RCM_SETKEY (WM_USER + 1003)
#define RKeyWnd_SetKey(a_hWnd, a_pKey) (::SendMessage(a_hWnd, RCM_SETKEY, 0, reinterpret_cast<LPARAM>(a_pKey)))

// gets text when ctrl empty WPARAM = unused, LPARAM = key (LPRSHORTCUTKEY)
#define RCM_GETKEY (WM_USER + 1004)
#define RKeyWnd_GetKey(a_hWnd, a_pKey) (::SendMessage(a_hWnd, RCM_GETKEY, 0, reinterpret_cast<LPARAM>(a_pKey)))

// RKeyWnd styles
#define RKS_LEFT	0x0000L		// left text
#define RKS_CENTER	0x0001L		// center text
#define RKS_RIGHT	0x0002L		// text to right


#define RKN_FIRST			(WM_USER + 0x4000)
#define RKN_CHANGE			(RKN_FIRST + 0)
#define RKN_SETFOCUS		(RKN_FIRST + 1)
#define RKN_KILLFOCUS		(RKN_FIRST + 2)

typedef struct S_NMKCHG
{
	NMHDR nmhdr;
	RSHORTCUTKEY keyOld;
	RSHORTCUTKEY keyNew;
} RKNMCHG, *LPRKNMCHG;
