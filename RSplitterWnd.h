#pragma once
#include <rcommon/rcommon.h>

#define RSplitterWnd_ClassName _T("RSPLITTERWND")

extern RCOMMON_API ATOM RSplitterWnd_RegisterClass();
extern RCOMMON_API LRESULT CALLBACK RSplitterWnd_WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);

extern RCOMMON_API HWND RSplitterWnd_Create(DWORD a_iStyle, LPCTSTR a_sTitle, int a_x, int a_y, int a_iWidth,
	int a_iHeight, HWND	a_hWndParent, LPVOID a_lpParam);

// control messages
// setting WND WPARAM = index of window (0 or 1) LPARAM = HWND to set
#define RSCM_SETWND (WM_USER + 1001)
// getting WND WPARAM = index of window (0 or 1) LPARAM = pointer to HWND 
#define RSCM_GETWND (WM_USER + 1002)

#define RSplitterWnd_SetWnd(a_hWnd, a_btIdx, a_hInnerWnd) ::SendMessage(a_hWnd, RSCM_SETWND, (WPARAM)a_btIdx, (LPARAM)a_hInnerWnd)
#define RSplitterWnd_GetWnd(a_hWnd, a_btIdx, a_phInnerWnd) ::SendMessage(a_hWnd, RSCM_GETWND, (WPARAM)a_btIdx, (LPARAM)a_phInnerWnd)

// setting size of windows WPARAM = index of window (0 or 1) LPARAM = size (long)
// works only for not RSCS_PERCENTSIZE mode
#define RSCM_SETSIZE (WM_USER + 1003)

// setting size of windows WPARAM = index of window (0 or 1) LPARAM = percent (*double)
// works only for RSCS_PERCENTSIZE mode
#define RSCM_SETPSIZE (WM_USER + 1004)

// getting size of windows WPARAM = index of window (0 or 1) returns size
#define RSCM_GETSIZE (WM_USER + 1005)

#define RSplitterWnd_SetSize(a_hWnd, a_btIdx, a_iSize) ::SendMessage(a_hWnd, RSCM_SETSIZE, (WPARAM)a_btIdx, (LPARAM)a_iSize)
#define RSplitterWnd_SetPSize(a_hWnd, a_btIdx, a_fPerc) ::SendMessage(a_hWnd, RSCM_SETPSIZE, (WPARAM)a_btIdx, (LPARAM)&a_fPerc)
#define RSplitterWnd_GetSize(a_hWnd, a_btIdx) ::SendMessage(a_hWnd, RSCM_GETSIZE, (WPARAM)a_btIdx, (LPARAM)0)

// splits into two equal panes
#define RSCM_SPLITEQUAL (WM_USER + 1006)
#define RSplitterWnd_SplitEqual(a_hWnd) ::SendMessage(a_hWnd, RSCM_SPLITEQUAL, (WPARAM)0, (LPARAM)0)

// refreshes (recalculates layout) of panes. Use for example after changing style
#define RSCM_REFRESH (WM_USER + 1007)
#define RSplitterWnd_Refresh(a_hWnd) ::SendMessage(a_hWnd, RSCM_REFRESH, (WPARAM)0, (LPARAM)0)

typedef struct S_RSSETBITMAP
{
	HINSTANCE hInst;
	union
	{
		UINT	 nID;
		HBITMAP  hBmp; 
	} bmp;
} RSSETBITMAP, *LPRSSETBITMAP;

// Sets bitmap for splitter bar: LPARAM - pointer to RSSETBITMAP
#define RSCM_SETBITMAP (WM_USER + 1008)
#define RSplitterWnd_SetBitmap(a_hWnd, a_pSetBitmap) ::SendMessage(a_hWnd, RSCM_SETBITMAP, (WPARAM)0, (LPARAM)a_pSetBitmap)

// Sets splitter bar thickness: WPARAM - thickness
#define RSCM_SETTHICKNESS (WM_USER + 1009)
#define RSplitterWnd_SetThickness(a_hWnd, a_iThickness) ::SendMessage(a_hWnd, RSCM_SETTHICKNESS, (WPARAM)a_iThickness, 0)

// styles
#define RSCS_HORIZONTAL		 (0x0000L)
#define RSCS_VERTICAL		 (0x0001L)
#define RSCS_REROUTEMSG		 (0x0002L)
#define RSCS_PERCENTSIZE	 (0x0004L)	// size of pane in percents
#define RSCS_NOTIP			 (0x0008L)	// no tips 
#define RSCS_BLACKTRACKER	 (0x0010L)	// black tracker instead of MS like (halftone) tracker
#define RSCS_NOTRACKERBORDER (0x0020L)	// tracker without std borders

#define RSplitterWnd_IsVertical(a_hWnd) 	(::GetWindowLong(a_hWnd, GWL_STYLE) & RSCS_VERTICAL)

