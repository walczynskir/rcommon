#pragma once
// header files for impelmentation of celltips for GridRaw
#include <rcommon/rcommon.h>

#define RCellTip_ClassName _T("RCELLTIP")

extern RCOMMON_API ATOM RCellTip_Register();
extern RCOMMON_API HWND RCellTip_Create(HWND a_hWndOwner, DWORD a_dwStyle);
extern RCOMMON_API LRESULT CALLBACK RCellTip_WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);

// styles
#define RCTS_ANIMATE			(0x0001L)
#define RCTS_CONSTDX			(0x0002L)
#define RCTS_CONSTDY			(0x0004L)
#define RCTS_CONSTSIZE			(RCTS_CONSTDX | RCTS_CONSTDY)
#define RCTS_FRAME				(0x0008L)

// control messages
// setting BKCOLOR WPARAM = 0 LPARAM = RGB
#define RCTM_SETBKCOLOR (WM_USER + 1001)
// getting BKCOLOR WPARAM = 0 LPARAM = 0, LRESULT RGB 
#define RCTM_GETBKCOLOR (WM_USER + 1002)

#define RCellTip_SetBkColor(a_hWnd, a_clr)	::SendMessage(a_hWnd, RCTM_SETBKCOLOR, 0, (LPARAM)a_clr)
#define RCellTip_GetBkColor(a_hWnd)			::SendMessage(a_hWnd, RCTM_GETBKCOLOR, 0, 0)

// setting TEXTCOLOR WPARAM = 0 LPARAM = RGB
#define RCTM_SETTEXTCOLOR (WM_USER + 1003)
// getting TEXTCOLOR WPARAM = 0 LPARAM = 0, LRESULT RGB 
#define RCTM_GETTEXTCOLOR (WM_USER + 1004)

#define RCellTip_SetTextColor(a_hWnd, a_clr)	::SendMessage(a_hWnd, RCTM_SETTEXTCOLOR, 0, (LPARAM)a_clr)
#define RCellTip_GetTextColor(a_hWnd)			::SendMessage(a_hWnd, RCTM_GETTEXTCOLOR, 0, 0)

// setting size WPARAM = 0 LPARAM = LPSIZE
#define RCTM_SETSIZE (WM_USER + 1005)

// getting size WPARAM = 0 LPARAM = LPSIZE
#define RCTM_GETSIZE (WM_USER + 1006)

#define RCellTip_SetSize(a_hWnd, a_pSize)	::SendMessage(a_hWnd, RCTM_SETSIZE, 0, (LPARAM)a_pSize)
#define RCellTip_GetSize(a_hWnd, a_pSize)	::SendMessage(a_hWnd, RCTM_GETSIZE, 0, (LPARAM)a_pSize)

// shows Tip WPARAM = 0 LPARAM = LPPOINT
#define RCTM_POPUP (WM_USER + 1007)

// shows Tip WPARAM = 0 LPARAM = LPPOINT
#define RCTM_MOVE (WM_USER + 1008)

// hides Tip WPARAM = 0 LPARAM = 0, LRESULT LPSIZE 
#define RCTM_POP (WM_USER + 1009)

// calculates tip size using current text Tip WPARAM = 0 LPARAM = 0, LRESULT not used
#define RCTM_CALCULATESIZE (WM_USER + 1010)

#define RCellTip_PopUp(a_hWnd, a_pPt)	::SendMessage(a_hWnd, RCTM_POPUP, 0, (LPARAM)a_pPt)
#define RCellTip_Move(a_hWnd, a_pPt)	::SendMessage(a_hWnd, RCTM_MOVE, 0, (LPARAM)a_pPt)
#define RCellTip_Pop(a_hWnd)	        ::SendMessage(a_hWnd, RCTM_POP, 0, 0)
#define RCellTip_CalculateSize(a_hWnd)	::SendMessage(a_hWnd, RCTM_CALCULATESIZE, 0, 0L)

typedef struct S_TIPDELAY
{
	UINT iShow;
	UINT iHide;
} RTIPDELAY, *LPRTIPDELAY;

// sets delay times WPARAM = 0 LPARAM = LPRTIPDELAY, LRESULT not used
#define RCTM_SETDELAY (WM_USER + 1011)
// gets delay times WPARAM = 0 LPARAM = = LPRTIPDELAY, LRESULT not used
#define RCTM_GETDELAY (WM_USER + 1012)

#define RCellTip_SetDelay(a_hWnd, a_pDelay)	::SendMessage(a_hWnd, RCTM_SETDELAY, 0, reinterpret_cast<LPARAM>(a_pDelay))
#define RCellTip_GetDelay(a_hWnd, a_pDelay)	::SendMessage(a_hWnd, RCTM_GETDELAY, 0, reinterpret_cast<LPARAM>(a_pDelay))
