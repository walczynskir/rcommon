#pragma once
// RColorWnd.h: interface for the RColorWnd class.
//
//////////////////////////////////////////////////////////////////////
#include <rcommon/rcommon.h>

#define RColorWnd_ClassName _T("RCOLORWND")


extern RCOMMON_API ATOM RColorWnd_RegisterClass();
extern RCOMMON_API HWND RColorWnd_Create(DWORD a_iStyle, int a_x, int a_y, int a_iWidth,
	int a_iHeight, HWND	a_hWndParent, LPVOID	a_lpParam);
extern RCOMMON_API LRESULT CALLBACK RColorWnd_WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);

typedef struct S_NMCCCHG
{
	NMHDR nmhdr;
	COLORREF clrOld;
	COLORREF clrNew;
} RCCNMCHG, *LPRCCNMCHG;

// styles
#define RCCS_TBSTYLE		(0x0001L)	// for toolbars
#define RCCS_WORDSTYLE		(0x0002L)	// looks like word 2003


// defines set of colors
typedef struct S_COLORSDEF
{
	UINT      iCount;
	COLORREF* pRGB;
	LPCTSTR*  pColorName;
} RCCCOLORSDEF, *LPRCCCOLORSDEF;


// sets color of control WPARAM = unused, LPARAM = color (RGB value)
#define RCM_SETCOLOR (WM_USER + 1001)
#define RColorWnd_SetColor(a_hWnd, a_clr, a_bNotify) (::SendMessage(a_hWnd, RCM_SETCOLOR, a_bNotify, a_clr))

// gets color of control WPARAM = unused, LPARAM = unused, LRESULT: color (RGB value)
#define RCM_GETCOLOR (WM_USER + 1002)
#define RColorWnd_GetColor(a_hWnd) (static_cast<COLORREF>(::SendMessage(a_hWnd, RCM_GETCOLOR, 0, 0)))

// sets tooltip for control WPARAM = unused, LPARAM = LPCTSTR tooltip text
#define RCM_SETTOOLTIP (WM_USER + 1003)
#define RColorWnd_SetTooltip(a_hWnd, a_psTooltip) (::SendMessage(a_hWnd, RCM_SETTOOLTIP, 0, reinterpret_cast<LPARAM>(a_psTooltip)))

// sets BITMAP for control WPARAM = unused, LPARAM = HBITMAP bitmap to show
#define RCM_SETBITMAP (WM_USER + 1004)
#define RColorWnd_SetBitmap(a_hWnd, a_hBitmap) (::SendMessage(a_hWnd, RCM_SETBITMAP, 0, reinterpret_cast<LPARAM>(a_hBitmap)))

// sets columns count in dropdown WPARAM = unused, LPARAM = UINT columns count, LRESULT = previous columns count
#define RCM_SETCOLUMNS (WM_USER + 1005)
#define RColorWnd_SetColumns(a_hWnd, a_iCols) (static_cast<UINT>(::SendMessage(a_hWnd, RCM_SETCOLUMNS, 0, a_iCols)))

// sets colors in dropdown WPARAM = unused, LPARAM = LPRCCCOLORSDEF colors
#define RCM_SETCOLORS (WM_USER + 1006)
#define RColorWnd_SetColors(a_hWnd, a_pColors) (::SendMessage(a_hWnd, RCM_SETCOLORS, 0, a_pColors))


#define RCCN_FIRST		WM_USER + 0x4000
#define RCCN_CHANGED	(RCCN_FIRST + 0)
#define RCCN_PUSHED		(RCCN_FIRST + 0)

