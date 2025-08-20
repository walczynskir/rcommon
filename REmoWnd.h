// REmoWnd.h: interface for the REmoWnd class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include <rcommon/rcommon.h>

#define REmoWnd_ClassName _T("REMOWND")

extern RCOMMON_API ATOM REmoWnd_RegisterClass();
extern RCOMMON_API HWND REmoWnd_Create(DWORD a_iStyle, int a_x, int a_y, int a_iWidth,
	int a_iHeight, HWND	a_hWndParent, LPVOID	a_lpParam);
extern RCOMMON_API LRESULT CALLBACK REmoWnd_WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);

typedef struct S_SELECT
{
	UINT iRow;
	UINT iCol;
} TSelect, *LPTSelect;

typedef struct S_EMOSTRUCT
{
	HBITMAP hBitmap;
	COLORREF clrBackBitmap;
	UINT     iRows;
	UINT     iCols;
} TEmoStruct, *LPTEmoStruct;

// sets selected emo WPARAM = notify parent, LPARAM = selection (LPTSelect)
#define REM_SETSEL (WM_USER + 1001)
#define REmoWnd_SetSel(a_hWnd, a_sel, a_bNotify) (::SendMessage(a_hWnd, REM_SETSEL, a_bNotify, reinterpret_cast<LPARAM>(a_sel)))

// gets selected bitmap of control WPARAM = unused, LPARAM = HBITMAP*
// bitmap cannot be deleted with DeletObject
#define REM_GETEMO (WM_USER + 1002)
#define REmoWnd_GetEmo(a_hWnd, a_pBmp) (::SendMessage(a_hWnd, REM_GETEMO, 0, reinterpret_cast<LPARAM>(a_pBmp)))

// gets selected bitmap of emoticon with given background color 
// WPARAM = RGB, LPARAM = HBITMAP*
// bitmap must be deleted with DeletObject
#define REM_GETTRANSPEMO (WM_USER + 1003)
#define REmoWnd_GetTranspEmo(a_hWnd, a_rgbBack, a_pBmp) (::SendMessage(a_hWnd, REM_GETTRANSPEMO, static_cast<COLORREF>(a_rgbBack), reinterpret_cast<LPARAM>(a_pBmp)))

// sets tooltip for control WPARAM = unused, LPARAM = LPCTSTR tooltip text
#define REM_SETTOOLTIP (WM_USER + 1004)
#define REmoWnd_SetTooltip(a_hWnd, a_psTooltip) (::SendMessage(a_hWnd, REM_SETTOOLTIP, 0, reinterpret_cast<LPARAM>(a_psTooltip)))

// sets drop down bitmap and size WPARAM = unused, LPARAM = LPTEmoStruct
// returns FALSE if failure, TRUE if succeded
#define REM_SETSTRUCT (WM_USER + 1005)
#define REmoWnd_SetStruct(a_hWnd, a_pEmoStruct) (::SendMessage(a_hWnd, REM_SETSTRUCT, 0, reinterpret_cast<LPARAM>(a_pEmoStruct)))

#define RECN_FIRST			WM_USER + 0x4000
#define RECN_CHANGED		(RECN_FIRST + 0)
#define RECN_PUSHED			(RECN_FIRST + 1)


typedef struct S_NMECHG
{
	NMHDR nmhdr;
	TSelect selOld;
	TSelect selNew;
} RENMCHG, *LPRENMCHG;

// styles
#define RECS_TBSTYLE			(0x0001L)	// for toolbars
#define RECS_CHANGEOPENPLACE	(0x0002L)	// allowed to open above the button or to the left
											// if not enough space
