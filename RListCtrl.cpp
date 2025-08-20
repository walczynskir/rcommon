// RListCtrl.cpp : Definition of RListCtrl control
//

#include "stdafx.h"
#include "RListCtrl.h"
#include "RListData.h"

#include "drawutl.h"
#ifdef _WIN32_WCE
#include <aygshell.h>
#else
#include "celltip.h"
#include "RTheme.h"
#include "RMemDC.h"
#include "RoundIter.h"
#endif

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL  0x020A
#endif

#ifndef WHEEL_DELTA
#define WHEEL_DELTA	   120
#endif 


typedef struct S_COLPOS
{
	long xLeft;
	long xRight;
} T_COLPOS;

typedef struct S_ROWPOS
{
	long yTop;
	long yBottom;
} T_ROWPOS;




static const long c_iWindowOfs = sizeof(RListData*) - 4;
static const int c_dxVScroll = ::GetSystemMetrics(SM_CYHSCROLL);
static const int c_dyHScroll = ::GetSystemMetrics(SM_CXVSCROLL);

#ifndef _WIN32_WCE
static HHOOK s_hook = NULL;
static LRESULT CALLBACK HookMessageProc(int code, WPARAM wParam, LPARAM lParam);
#endif

// message handlers
static inline BOOL	OnCreate(HWND a_hWnd, LPCREATESTRUCT a_lpStruct);
static inline void	OnShowWindow(HWND a_hWnd, BOOL a_bShow);
static inline void	OnSize(HWND a_hWnd, UINT a_iType, int a_dx, int a_dy);

#ifdef _WIN32_WCE
static inline void	OnDestroy(HWND a_hWnd);
#else _WIN32_WCE
static inline void	OnNcDestroy(HWND a_hWnd);
#endif
static inline void	OnPaint(HWND a_hWnd);
static inline void	OnSetFocus(HWND a_hWnd);
static inline void	OnKillFocus(HWND a_hWnd);
static inline void	OnKeyDown(HWND a_hWnd, UINT a_iChar, LPARAM a_lParam);
static inline void	OnKeyUp(HWND a_hWnd, UINT a_iChar, LPARAM a_lParam);
static inline void	OnChar(HWND a_hWnd, UINT a_iChar, LPARAM a_lParam);
static inline void	OnVScroll(HWND a_hWnd, UINT a_iSBCode);
static inline void	OnHScroll(HWND a_hWnd, UINT a_iSBCode);
static inline void	OnLButtonDown(HWND a_hWnd, UINT a_iFlags, const POINT& a_pt);
static inline void	OnLButtonUp(HWND a_hWnd, UINT a_iFlags, const POINT& a_pt);
static inline void	OnMouseMove(HWND a_hWnd, UINT a_iFlags, const POINT& a_pt);
static inline void  OnMouseWheel(HWND a_hWnd, short a_nKeys, short a_nDelta, const POINT& a_pt);
static inline void	OnLButtonDblClk(HWND a_hWnd, UINT a_iKeys, const POINT& a_pt);
static inline void	OnCaptureChanged(HWND a_hWnd, HWND a_hWndNew);
static inline LRESULT OnGetDlgCode(HWND a_hWnd, LPMSG a_pMsg);
static inline LRESULT OnNotify(HWND a_hWnd, int a_idCtrl, LPNMHDR a_pNmhdr);
static inline void OnNotifyTooltip(HWND a_hWnd, LPNMHDR a_pNmHdr);

static inline void	OnSetCol(HWND a_hWnd, LPRLCCOLDEF a_pColDef);
static inline void	OnSetMode(HWND a_hWnd, BOOL a_bSet, long a_iFlags);
static inline void	OnSetDataProc(HWND a_hWnd, void* a_pObj, RLISTDATAPROC a_proc);
static inline void	OnSetCountProc(HWND a_hWnd, void* a_pObj, RLISTCOUNTPROC a_proc);
static inline void	OnSetDrawProc(HWND a_hWnd, void* a_pObj, RLISTDRAWPROC a_proc);
static inline void	OnSetDrawBkProc(HWND a_hWnd, void* a_pObj, RLISTDRAWBKPROC a_proc);
static inline void	OnSetGridProc(HWND a_hWnd, void* a_pObj, RLISTGRIDPROC a_proc);
static inline COLORREF	OnSetColor(HWND a_hWnd, UINT a_iIdx, COLORREF a_cr);
static inline COLORREF	OnGetColor(HWND a_hWnd, UINT a_iIdx);
static inline void OnSetFixedRows(HWND a_hWnd, long a_iFixedRows);
static inline void OnSetFixedCols(HWND a_hWnd, long a_iFixedCols);
static inline long OnGetFixed(HWND a_hWnd, BOOL a_bCols);
static inline long OnSetRowHeight(HWND a_hWnd, long a_iRow, long a_iHeight);
static inline long OnSetColWidth(HWND a_hWnd, long a_iCol, long a_iWidth);
static inline long OnRowsCount(HWND a_hWnd);
static inline long OnColsCount(HWND a_hWnd);
static inline long OnGetRowHeight(HWND a_hWnd, long a_iRow);
static inline long OnGetColWidth(HWND a_hWnd, long a_iCol);
static inline void OnEditEnd(HWND a_hWnd, BOOL a_bCommit);
static inline void OnSetTooltip(HWND a_hWnd, LPCTSTR a_sTooltip);
static inline UINT OnGetTooltip(HWND a_hWnd, LPTSTR a_sTooltip, UINT a_iSize);
static inline void OnRefresh(HWND a_hWnd);
static inline LRESULT OnGetFont(HWND a_hWnd);
static inline void OnSetFont(HWND a_hWnd, HFONT a_hFont);
static inline LRESULT OnGetTextMargin(HWND a_hWnd);
static inline void OnSetTextMargin(HWND a_hWnd, short a_nMargin);
static inline void OnSetSelected(HWND a_hWnd, UINT a_iRow);
static inline LRESULT OnGetSelected(HWND a_hWnd);
static inline void OnInvalidateRow(HWND a_hWnd, long a_iRow);
static inline void OnInvalidateCol(HWND a_hWnd, long a_iCol);
static inline void OnInvalidateCell(HWND a_hWnd, long a_iRow, long a_iCol);
static inline void OnEditCell(HWND a_hWnd, long a_iRow, long a_iCol);
static inline void OnGetHoverCell(HWND a_hWnd, LPCELL a_pCell);

static inline void Draw(HWND a_hWnd, HDC a_hDC);
static inline void SetTooltip(HWND a_hWnd);

static inline bool NotifyStart(HWND a_hWnd, const CELL& a_cell, LPRLCNMEDIT a_pnmEdit);
static inline LRESULT Notify(HWND a_hWnd, LPNMHDR a_pNmhdr);

void EditCell(HWND a_hWnd, const CELL& a_cell, HWND a_hWndEdit, LPRECT a_pRectEdit);

static inline RListData*	GetRListCtrlData(HWND a_hWnd);
static inline void			SetRListCtrlData(HWND a_hWnd, RListData* a_pData);


// drawing - internal
static inline void DrawBk(HWND a_hWnd, HDC a_hDC, LPRECT a_pRect);
static inline void DrawTracking(HWND a_hWnd, HDC a_hdc);
static inline void DrawGrid(HWND a_hWnd, HDC a_hDC);
static BOOL DrawGrid(HWND a_hWnd, HDC a_hDC, LPRLGRID a_pGrid);

static inline void DrawRows(HWND a_hWnd, HDC a_dc);

static void	DrawRow(HWND a_hWnd, HDC a_hDC, long a_iRow);
static void	DrawCell(HWND a_hWnd, HDC a_hDC, long a_iRow, long a_iCol);
static void	DrawCell(HWND a_hWnd, HDC a_hDC, LPRLCELL a_pCell);

static void	InvalidateRow(HWND a_hWnd, long a_iRow);
static void	InvalidateCol(HWND a_hWnd, long a_iCol);
static void	InvalidateCell(HWND a_hWnd, long a_iRow, long a_iCol);

static void	InvalidateDrawRectV(HWND a_hWnd);
static void	InvalidateDrawRectH(HWND a_hWnd, long a_iRight);
static void InvalidateTrackCol(HWND a_hWnd, long a_xStart, long a_xEnd);
static void InvalidateTrackedCol(HWND a_hWnd, long a_iStartTracked);
static void InvalidateTrackRow(HWND a_hWnd, long a_xStart, long a_xEnd);
static void InvalidateTrackedRow(HWND a_hWnd, long a_iStartTracked);

static inline void	ClipCursorCol(HWND a_hWnd, long a_iCol);
static inline void	ClipCursorRow(HWND a_hWnd, long a_iRow);

// celltips
static inline void CellTip_Pop(HWND a_hWnd);

static void CellTip(HWND a_hWnd, const POINT& a_pt);
static BOOL HasToShowCellTip(HWND a_hWnd, const POINT& a_pt, LPRLCELLPOS a_pCell, LPSIZE a_pSize);


// helpers
static inline void CalculateDrawRect(HWND a_hWnd);
static long CalculateRows2Draw(HWND a_hWnd);
static long GetRightGridSize(HWND a_hWnd);
static T_COLPOS GetColPos(HWND a_hWnd, long a_iCol);
static T_ROWPOS GetRowPos(HWND a_hWnd, long a_iRow);

static inline T_ROWPOS GetLastRowPos(HWND a_hWnd);
static inline T_COLPOS GetLastColPos(HWND a_hWnd);
static inline RECT GetCellRect(HWND a_hWnd, long a_iRow, long a_iCol);

static inline long TrackedColFromPoint(HWND a_hWnd, const POINT& a_pt);
static inline long TrackedRowFromPoint(HWND a_hWnd, const POINT& a_pt);

static inline long GetLastVisibleCol(HWND a_hWnd);
static inline long GetLastVisibleRow(HWND a_hWnd);

static inline BOOL HitInRange(HWND a_hWnd, const POINT& a_pt);
static inline BOOL MouseOnHeaderGridCol(HWND a_hWnd, const POINT& a_pt);
static inline BOOL MouseOnHeaderGridRow(HWND a_hWnd, const POINT& a_pt);
static inline BOOL MouseOnGridRow(HWND a_hWnd, const POINT& a_pt);
static inline BOOL MouseOnGridCol(HWND a_hWnd, const POINT& a_pt);
static inline BOOL MouseOnGrid(HWND a_hWnd, const POINT& a_pt);
static void SetMouseCell(HWND a_hWnd, const POINT& a_pt, long a_iRow, long a_iCol);

static long GetRowFromPoint(HWND a_hWnd, const POINT& a_pt);
static long GetColFromPoint(HWND a_hWnd, const POINT& a_pt);
static RLCELLPOS GetCellFromPoint(HWND a_hWnd, const POINT& a_pt, bool a_bRect);

static inline void SetTrackedColumnWidth(HWND a_hWnd, const POINT& a_pt);
static inline void SetTrackedRowHeight(HWND a_hWnd, const POINT& a_pt);
static inline void FinishTracking(HWND a_hWnd);
static inline long GetHScrollLastCol(HWND a_hWnd, long a_dx);

static long GetRowToSeeLast(HWND a_hWnd, long a_dy);
static long GetColToSeeLast(HWND a_hWnd, long a_dx);

static SIZE GetClientSize(HWND a_hWnd);

static BOOL IsRowOnScreen(HWND a_hWnd, long a_iRow);

static bool	SelChanging(HWND a_hWnd, long a_iRow);
static void	SelChanged(HWND a_hWnd);

// moving around list
static inline void	SelectRow(HWND a_hWnd, const POINT& a_pt);
static inline void	SelectRowEx(HWND a_hWnd, long a_iRow);
static inline void	SelectRow(HWND a_hWnd, long a_iRow);
static inline void	VScrollTo(HWND a_hWnd, long a_iRow);
static inline void	VScroll(HWND a_hWnd, long a_iScroll);
static inline void	HScrollTo(HWND a_hWnd, long a_iPos);
static inline void	HScroll(HWND a_hWnd, long a_iScroll);
inline static void	First(HWND a_hWnd);
inline static void	Last(HWND a_hWnd);

static bool CanEditCell(HWND a_hWnd, const CELL& a_cell, HWND* a_phWndEdit, LPRECT a_pRect);
static void EndEdit(HWND a_hWndEdit, WNDPROC a_wndprocDef, bool a_bCommit, bool a_bContinue, bool a_bReverse);



// functional macros
#pragma todo ("smooth scroll")
#pragma todo ("registered functions for celltips - I don't know what will be drawed.")
#pragma todo ("Handle not known count of rows")


//	---------------------------------------------------------------------------------------
//	Main RListCtrl procedure
//
LRESULT CALLBACK 
RListCtrl_WndProc(
	HWND a_hWnd, 
	UINT a_iMsg, 
	WPARAM a_wParam, 
	LPARAM a_lParam
	)
{

	switch (a_iMsg) 
	{
		case WM_CREATE:
			if (!OnCreate(a_hWnd, (LPCREATESTRUCT)a_lParam))
			{
				return -1;
			}
			break;

		case WM_SHOWWINDOW:
			OnShowWindow(a_hWnd, static_cast<BOOL>(a_wParam));
			break;

		case WM_PAINT:
			OnPaint(a_hWnd);
			break;

		case WM_SIZE:
			OnSize(a_hWnd, static_cast<UINT>(a_wParam), LOWORD(a_lParam), HIWORD(a_lParam));
			break;

		case WM_SETFOCUS:
			OnSetFocus(a_hWnd);
			break;

		case WM_KILLFOCUS:
			OnKillFocus(a_hWnd);
			break;

		case WM_KEYDOWN:
			OnKeyDown(a_hWnd, static_cast<UINT>(a_wParam), a_lParam);
			break;

		case WM_KEYUP:
			OnKeyUp(a_hWnd, static_cast<UINT>(a_wParam), a_lParam);
			break;

		case WM_CHAR:
			OnChar(a_hWnd, static_cast<UINT>(a_wParam), a_lParam);
			break;

		case WM_VSCROLL:
			OnVScroll(a_hWnd, LOWORD(a_wParam));
			break;

		case WM_HSCROLL:
			OnHScroll(a_hWnd, LOWORD(a_wParam));
			break;

		case WM_MOUSEWHEEL:
			{
				POINT l_pt = {LOWORD(a_lParam), HIWORD(a_lParam)};
				OnMouseWheel(a_hWnd, LOWORD(a_wParam), HIWORD(a_wParam), l_pt);
				break;
			}

		case WM_LBUTTONDOWN:
			{
				POINT l_pt = {LOWORD(a_lParam), HIWORD(a_lParam)};
				OnLButtonDown(a_hWnd, static_cast<UINT>(a_wParam), l_pt);
				break;
			}

		case WM_LBUTTONUP:
			{
				POINT l_pt = {LOWORD(a_lParam), HIWORD(a_lParam)};
				OnLButtonUp(a_hWnd, static_cast<UINT>(a_wParam), l_pt);
				break;
			}

		case WM_MOUSEMOVE:
			{
				POINT l_pt = {LOWORD(a_lParam), HIWORD(a_lParam)};
				OnMouseMove(a_hWnd, static_cast<UINT>(a_wParam), l_pt);
				break;
			}

		case WM_LBUTTONDBLCLK:
			{
				POINT l_pt = {LOWORD(a_lParam), HIWORD(a_lParam)};
				OnLButtonDblClk(a_hWnd, static_cast<UINT>(a_wParam), l_pt);
				break;
			}

		case WM_CAPTURECHANGED:
			OnCaptureChanged(a_hWnd, (HWND)a_lParam);
			break;

		case WM_GETDLGCODE:
			return OnGetDlgCode(a_hWnd, (LPMSG)a_lParam);

		case WM_NOTIFY:
			return OnNotify(a_hWnd, static_cast<int>(a_wParam), reinterpret_cast<LPNMHDR>(a_lParam));

#ifdef _WIN32_WCE
		case WM_DESTROY:
			OnDestroy(a_hWnd);
#else
		case WM_NCDESTROY:
			OnNcDestroy(a_hWnd);
#endif
			break;

		case RLCM_SETCOL:
			OnSetCol(a_hWnd, (LPRLCCOLDEF)a_lParam);
			break;

		case RLCM_SETMODE:
			OnSetMode(a_hWnd, (BOOL)a_wParam, static_cast<long>(a_lParam));
			break;

		case RLCM_SETDATAPROC:
			OnSetDataProc(a_hWnd, (void*)a_wParam, (RLISTDATAPROC)a_lParam);
			break;

		case RLCM_SETCOUNTPROC:
			OnSetCountProc(a_hWnd, (void*)a_wParam, (RLISTCOUNTPROC)a_lParam);
			break;

		case RLCM_SETDRAWPROC:
			OnSetDrawProc(a_hWnd, (void*)a_wParam, (RLISTDRAWPROC)a_lParam);
			break;

		case RLCM_SETDRAWBKPROC:
			OnSetDrawBkProc(a_hWnd, (void*)a_wParam, (RLISTDRAWBKPROC)a_lParam);
			break;

		case RLCM_SETGRIDPROC:
			OnSetGridProc(a_hWnd, (void*)a_wParam, (RLISTGRIDPROC)a_lParam);
			break;

		case RLCM_SETCOLOR:
			return (LRESULT)OnSetColor(a_hWnd, (UINT)a_wParam, (COLORREF)a_lParam);

		case RLCM_GETCOLOR:
			return (LRESULT)OnGetColor(a_hWnd, (UINT)a_wParam);

		case RLCM_SETFIXED:
			(BOOL)(a_wParam) ? OnSetFixedRows(a_hWnd, static_cast<long>(a_lParam)) : OnSetFixedCols(a_hWnd, static_cast<long>(a_lParam));
			break;

		case RLCM_GETFIXED:
			return OnGetFixed(a_hWnd, (BOOL)a_wParam);

		case RLCM_SETROWHEIGHT:
			return OnSetRowHeight(a_hWnd, static_cast<long>(a_wParam), static_cast<long>(a_lParam));

		case RLCM_SETCOLWIDTH:
			return OnSetColWidth(a_hWnd, static_cast<long>(a_wParam), static_cast<long>(a_lParam));

		case RLCM_ROWSCOUNT:
			return OnRowsCount(a_hWnd);

		case RLCM_COLSCOUNT:
			return OnColsCount(a_hWnd);

		case RLCM_GETROWHEIGHT:
			return OnGetRowHeight(a_hWnd, static_cast<long>(a_wParam));

		case RLCM_GETCOLWIDTH:
			return OnGetColWidth(a_hWnd, static_cast<long>(a_wParam));

		case RLCM_EDITEND:
			OnEditEnd(a_hWnd, static_cast<BOOL>(a_wParam));
			break;

		case RLCM_SETTOOLTIP:
			OnSetTooltip(a_hWnd, reinterpret_cast<LPCTSTR>(a_lParam));
			break;

		case RLCM_GETTOOLTIP:
			return static_cast<LRESULT>(OnGetTooltip(a_hWnd, reinterpret_cast<LPTSTR>(a_lParam), static_cast<UINT>(a_wParam)));

		case RLCM_REFRESH:
			OnRefresh(a_hWnd);
			break;

		case RLCM_GETFONT:
			return OnGetFont(a_hWnd);

		case RLCM_SETFONT:
			OnSetFont(a_hWnd, reinterpret_cast<HFONT>(a_lParam));
			break;

		case RLCM_GETTEXTMARGIN:
			return OnGetTextMargin(a_hWnd);

		case RLCM_SETTEXTMARGIN:
			OnSetTextMargin(a_hWnd, static_cast<short>(a_lParam));
			break;

		case RLCM_SETSELECTED:
			OnSetSelected(a_hWnd, static_cast<UINT>(a_lParam));
			break;

		case RLCM_GETSELECTED:
			return OnGetSelected(a_hWnd);

		case RLCM_INVALIDATEROW:
			OnInvalidateRow(a_hWnd, static_cast<long>(a_lParam));
			break;

		case RLCM_INVALIDATECOL:
			OnInvalidateCol(a_hWnd, static_cast<long>(a_lParam));
			break;

		case RLCM_INVALIDATECELL:
			OnInvalidateCell(a_hWnd, static_cast<long>(a_wParam), static_cast<long>(a_lParam));
			break;

		case RLCM_EDITCELL:
			OnEditCell(a_hWnd, static_cast<long>(a_wParam), static_cast<long>(a_lParam));
			break;
	
		case RLCM_GETHOVERCELL:
			OnGetHoverCell(a_hWnd, reinterpret_cast<LPCELL>(a_lParam));
			break;

		default:
			return DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
   }
   return 0;
}



/*	---------------------------------------------------------------------------------------
	Registration of RListCtrl class
*/
ATOM	/*WY ATOM (id) zarejestrowanej kontrolki lub NULL w razie niepowodzenia */
RListCtrl_Register()
{
#ifdef _WIN32_WCE
	WNDCLASS l_wndClass;
#else
	WNDCLASSEX l_wndClass;
	l_wndClass.cbSize = sizeof(WNDCLASSEX); 
	l_wndClass.hIconSm	   = NULL;
#endif

	l_wndClass.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	l_wndClass.lpfnWndProc   = ::RListCtrl_WndProc;
	l_wndClass.cbClsExtra    = 0;
	l_wndClass.cbWndExtra    = sizeof(RListData*);
	l_wndClass.hInstance     = RCommon_GetInstance();
	l_wndClass.hCursor       = ::LoadCursor(NULL, IDC_ARROW);
	l_wndClass.hbrBackground = NULL;
	l_wndClass.lpszMenuName  = NULL;
	l_wndClass.lpszClassName = RListCtrl_ClassName;
	l_wndClass.hIcon         = NULL;

#ifdef _WIN32_WCE
	return ::RegisterClass(&l_wndClass);
#else
	#ifndef RLCF_NOTOOLTIP
		RCellTip_Register();
	#endif
	return ::RegisterClassEx(&l_wndClass);
#endif
}


/*	---------------------------------------------------------------------------------------
	Creation of RListCtrl window
*/
HWND	// Handle of created window or NULL if failed
RListCtrl_Create(
	LPTSTR		a_sTitle,		//WE title
	DWORD		a_iStyle,		//WE style,
	int			a_x,			// horizontal position of window
	int			a_y,			// vertical position of window
	int			a_iWidth,		// window width
	int			a_iHeight,		// window height
	HWND		a_hWndParent,   // handle to parent or owner window
	HMENU		a_hMenu,        // handle to menu or child-window identifier
	LPVOID		a_lpParam			// pointer to window-creation data
	)
{
	return ::CreateWindow(RListCtrl_ClassName, a_sTitle, a_iStyle,
      a_x, a_y, a_iWidth, a_iHeight, a_hWndParent, a_hMenu, RCommon_GetInstance(), a_lpParam);
}


/*	---------------------------------------------------------------------------------------
	Zwraca dane kontrolki (udostêpniona)
*/
const RListData*
RListCtrl_GetData(
	HWND a_hWnd	//WE uchwyt kontrolki
	)
{
	return GetRListCtrlData(a_hWnd);
}


//	---------------------------------------------------------------------------------------
//	Draws one cell
//
BOOL RListCtrl_DrawCell(HWND a_hWnd, HDC a_hDC, void* /*a_pObj*/, LPRLCELL a_pCell)
{
	DrawCell(a_hWnd, a_hDC, a_pCell);
	return TRUE;
}


//	---------------------------------------------------------------------------------------
//	Draws background
//
BOOL RListCtrl_DrawBk(HWND a_hWnd, HDC a_hDC, void* /*a_pObj*/, LPRECT a_pRectInv)
{
	DrawBk(a_hWnd, a_hDC, a_pRectInv);
	return TRUE;
}


void DrawBk(HWND a_hWnd, HDC a_hDC, LPRECT a_pRectInv)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	COLORREF l_clrBk = (IsFocused(a_hWnd) ? l_pData->GetColor(RLC_BK_ACT) : l_pData->GetColor(RLC_BK_NOACT));
	// can paint only invalidated rect because background is solid
	RDraw::FillSolidRect(a_hDC, *a_pRectInv, l_clrBk);
}


//	---------------------------------------------------------------------------------------
//	Draws one grid line
//
BOOL RListCtrl_DrawGrid(HWND a_hWnd, HDC a_hDC, void* /*a_pObj*/, LPRLGRID a_pGrid)
{
	DrawGrid(a_hWnd, a_hDC, a_pGrid);
	return TRUE;
}


LRESULT CALLBACK
RListCtrl_EditFilter(WNDPROC a_wndprocDef, HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam)
{
	if ((a_iMsg == WM_KILLFOCUS) || ((a_iMsg == WM_KEYDOWN) && (a_wParam == VK_ESCAPE)))
	{
		EndEdit(a_hWnd, a_wndprocDef, false, false, false);
	}
	else if (a_iMsg == WM_KEYDOWN && (a_wParam == VK_RETURN) && 
		!IsKeyPressed(VK_CONTROL) && !IsKeyPressed(VK_MENU) && !IsKeyPressed(VK_SHIFT))
	{
		EndEdit(a_hWnd, a_wndprocDef, true, false, false);
	}
	else if (a_iMsg == WM_KEYDOWN && (a_wParam == VK_TAB) &&
		!IsKeyPressed(VK_CONTROL) && !IsKeyPressed(VK_MENU))
	{
		EndEdit(a_hWnd, a_wndprocDef, true, (a_wParam == VK_TAB), IsKeyPressed(VK_SHIFT));
	}
	LRESULT l_res = ::CallWindowProc(a_wndprocDef, a_hWnd, a_iMsg, a_wParam, a_lParam);
	return l_res;
}


/*	---------------------------------------------------------------------------------------
	Zwraca dane kontrolki
*/
RListData*	//WY wskaŸnik na dodatkowe dane
GetRListCtrlData(
	HWND a_hWnd	//WE uchwyt kontrolki
	)
{
#ifdef _WIN32_WCE
	return reinterpret_cast<RListData*>(::GetWindowLong(a_hWnd, c_iWindowOfs));
#else
#pragma warning(disable: 4312)
	return reinterpret_cast<RListData*>(::GetWindowLongPtr(a_hWnd, c_iWindowOfs));
#pragma warning(default: 4312)
#endif

}


/*	---------------------------------------------------------------------------------------
	Ustawia dane kontrolki
*/
static void	//WY wskaŸnik na dodatkowe dane
SetRListCtrlData(
	HWND a_hWnd,	//WE uchwyt kontrolki
	RListData* a_pData	//WE dodatkowe dane
	)
{
#ifdef _WIN32_WCE
	::SetWindowLong(a_hWnd, c_iWindowOfs, reinterpret_cast<LONG_PTR>(a_pData));
#else
#pragma warning(disable: 4244)
	::SetWindowLongPtr(a_hWnd, c_iWindowOfs, reinterpret_cast<LONG_PTR>(a_pData));
#pragma warning(default: 4244)
#endif
}


/*	---------------------------------------------------------------------------------------
	Handler of WM_CREATE
*/
BOOL OnCreate(HWND a_hWnd, LPCREATESTRUCT /*a_lpStruct*/)
{
	RListData* l_pData = new RListData();
	SetRListCtrlData(a_hWnd, l_pData);
	l_pData->SetDrawBkProc(RListCtrl_DrawBk, NULL);
	l_pData->SetDrawProc(RListCtrl_DrawCell, NULL);
	l_pData->SetGridProc(RListCtrl_DrawGrid, NULL);

	return TRUE;
}


/*	---------------------------------------------------------------------------------------
	Inicjacja podczas tworzenia okna
*/
static void	
OnShowWindow(
	HWND a_hWnd,
	BOOL a_bShow
	)
{
#ifndef RLCF_NOTOOLTIP
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (l_pData->GetCellTip() == NULL)
	{
		l_pData->SetCellTip(RCellTip_Create(a_hWnd, RCTS_CONSTDY | RCTS_ANIMATE | RCTS_FRAME));
	}
#endif
	if (!a_bShow)
	{
		::ReleaseCapture();
	}
}


/*	---------------------------------------------------------------------------------------
	Message handler of WM_SIZE
*/
void OnSize(HWND a_hWnd, UINT a_iType, int /*a_dx*/, int /*a_dy*/)
{
	if (a_iType != SIZE_MINIMIZED)
	{
		RListData* l_pData = GetRListCtrlData(a_hWnd);
		if (l_pData->GetWndEdit() != NULL)
		{
			// to force end of editing
			::SetFocus(a_hWnd);
		}
		l_pData->SetRectCalculated(FALSE);
	}
}


#ifdef _WIN32_WCE
/*	---------------------------------------------------------------------------------------
	Postmortem:-)
*/
static void
OnDestroy(
	HWND a_hWnd		//WE okno
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	delete l_pData;
}
#else
/*	---------------------------------------------------------------------------------------
	Postmortem:-)
*/
static void
OnNcDestroy(
	HWND a_hWnd		//WE okno
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (l_pData->GetTooltipWnd() != NULL)
	{
		::DestroyWindow(l_pData->GetTooltipWnd());
	}

	delete l_pData;
}
#endif


//	---------------------------------------------------------------------------------------
//	Drawing - WM_PAINT message handler (all drawing here)
//
static void
OnPaint(
	HWND a_hWnd	//WE okno
	)
{
	PAINTSTRUCT l_ps;
	HDC l_hdc = ::BeginPaint(a_hWnd, &l_ps);

	RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (l_pData->GetColumnsCount() > 0 && l_pData->GetRowsCount() > 0)
	{	// this block is needed also to correctly draw (to destroy RMemDC before EndPaint)
#ifdef _WIN32_WCE
	HDC l_hMemDC = l_hdc;
#else
	#ifdef _DEBUG 
		HDC l_hMemDC = l_hdc;
	#else	
		RECT l_rectWin;
		::GetClientRect(a_hWnd, &l_rectWin);

		RMemDC l_memDC(l_hdc, &l_rectWin);
		HDC l_hMemDC = l_memDC;
	#endif
#endif

	l_pData->DrawBk(a_hWnd, l_hMemDC, &l_ps.rcPaint);

	Draw(a_hWnd, l_hMemDC);
	}
	::EndPaint(a_hWnd, &l_ps);
}

//	---------------------------------------------------------------------------
//	Really draws List control
//
void Draw(
	HWND a_hWnd, 
	HDC a_hDC
	)
{
	CalculateDrawRect(a_hWnd);
	DrawRows(a_hWnd, a_hDC);
	DrawGrid(a_hWnd, a_hDC);
	DrawTracking(a_hWnd, a_hDC);
}

//	---------------------------------------------------------------------------------------
//	Handler of WM_LBUTTONDOWN message
//
void OnLButtonDown(HWND a_hWnd, UINT /*a_iFlags*/, const POINT& a_pt) 
{
	RLCNMMOUSE l_nmMouse;
	l_nmMouse.nmhdr.code = RLCN_LBUTTONDOWN;

	RLCELLPOS l_cellpos = GetCellFromPoint(a_hWnd, a_pt, true);
	l_nmMouse.cell = l_cellpos.cell;
	l_nmMouse.pt = a_pt;

	if (Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmMouse)))
	{
		::SetFocus(a_hWnd);
		return;
	}

/*
#ifdef _DEBUG
T_CELL l_iCell = GetCellFromPoint(a_hWnd, a_pt, false);
RTrace2("Button Down, Cell(%d, %d)\n", l_iCell.iRow, l_iCell.iCol);
#endif
*/
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	long l_iCol;
	long l_iRow;

	if ((l_iCol = TrackedColFromPoint(a_hWnd, a_pt)) >= 0)
	{
		if (!l_pData->TestFlags(LMB_CONSTCOLS))
		{
			// nie wiem dlaczego ale muszê tu jeszcze raz ustawic kursor
			HCURSOR l_hCur = ::LoadCursor(NULL, IDC_SIZEWE);
			::SetCursor(l_hCur) ;
			l_pData->SetTrackPos(a_pt.x);
			l_pData->SetTrackedColRow(l_iCol);
			::SetCapture(a_hWnd);
			l_pData->SetTrackingCol(TRUE);
			ClipCursorCol(a_hWnd, l_iCol);
		}
	}
	else if ((l_iRow = TrackedRowFromPoint(a_hWnd, a_pt)) >= 0)
	{
		if (!l_pData->TestFlags(LMB_CONSTROWS))
		{
			// nie wiem dlaczego ale muszê tu jeszcze raz ustawic kursor
			HCURSOR l_hCur = ::LoadCursor(NULL, IDC_SIZENS);
			::SetCursor(l_hCur) ;
			l_pData->SetTrackPos(a_pt.y);
			l_pData->SetTrackedColRow(l_iRow);
			::SetCapture(a_hWnd);
			l_pData->SetTrackingRow(TRUE);
			ClipCursorRow(a_hWnd, l_iRow);
		}
	}
	else if (HitInRange(a_hWnd, a_pt) && (!l_pData->TestFlags(LMB_MOVEONUP)))
	{
		SelectRow(a_hWnd, a_pt);
	}
	::SetFocus(a_hWnd);
}


//	---------------------------------------------------------------------------------------
//	Handler of WM_LBUTTONUP message
//
void OnLButtonUp(HWND a_hWnd, UINT /*a_iFlags*/, const POINT& a_pt) 
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
/*
#ifdef _DEBUG
T_CELL l_iCell = GetCellFromPoint(a_hWnd, a_pt, false);
RTrace2("Button Up, Cell(%d, %d)\n", l_iCell.iRow, l_iCell.iCol);
#endif
*/
	if (l_pData->IsTrackingCol())
	{
		::ReleaseCapture();
		SetTrackedColumnWidth(a_hWnd, a_pt);
		return;
	}
	if (l_pData->IsTrackingRow())
	{
		::ReleaseCapture();
		SetTrackedRowHeight(a_hWnd, a_pt);
		return;
	}
	RLCNMMOUSE l_nmMouse;
	l_nmMouse.nmhdr.code = RLCN_LBUTTONUP;

	RLCELLPOS l_cellpos = GetCellFromPoint(a_hWnd, a_pt, true);
	l_nmMouse.cell = l_cellpos.cell;
	l_nmMouse.pt = a_pt;


	if (Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmMouse)))
	{
		return;
	}

	if (HitInRange(a_hWnd, a_pt) && (l_pData->TestFlags(LMB_MOVEONUP)))
	{
		SelectRow(a_hWnd, a_pt);
	}
}


//	---------------------------------------------------------------------------------------
//	Handler of WM_MOUSEMOVE message
//
void OnMouseMove(HWND a_hWnd, UINT /*a_iFlags*/, const POINT& a_pt) 
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (l_pData->IsTrackingCol())
	{
		InvalidateTrackCol(a_hWnd, l_pData->GetTrackPos(), a_pt.x) ;
		l_pData->SetTrackPos(a_pt.x);
	}
	else if (l_pData->IsTrackingRow())
	{
		InvalidateTrackRow(a_hWnd, l_pData->GetTrackPos(), a_pt.y) ;
		l_pData->SetTrackPos(a_pt.y);
	}
	else if (l_pData->GetWndEdit() != NULL)
	{
		// do nothing
	}
	else	// check if we are entering or leaving cell
	{
		if (!l_pData->IsTrackingMouse())
		{
			::SetCapture(a_hWnd);
			l_pData->SetTrackingMouse(true);
		}
		RECT l_rectClient;
		::GetClientRect(a_hWnd, &l_rectClient);
		if (!::PtInRect(&l_rectClient, a_pt))
		{
			CellTip_Pop(a_hWnd);
			::ReleaseCapture();
			SetMouseCell(a_hWnd, a_pt, -1, -1);
		}
		else if (MouseOnHeaderGridRow(a_hWnd, a_pt))
		{
			SetMouseCell(a_hWnd, a_pt, -1, -1);
			CellTip_Pop(a_hWnd);
			if (!l_pData->TestFlags(LMB_CONSTROWS))
			{
				HCURSOR l_hCur = ::LoadCursor(NULL, IDC_SIZENS);
				::SetCursor(l_hCur);
			}
		}
		else if (MouseOnHeaderGridCol(a_hWnd, a_pt))
		{
			SetMouseCell(a_hWnd, a_pt, -1, -1);
			CellTip_Pop(a_hWnd);
			if (!l_pData->TestFlags(LMB_CONSTCOLS))
			{
				HCURSOR l_hCur = ::LoadCursor(NULL, IDC_SIZEWE);
				::SetCursor(l_hCur);
			}
		}
		else
		{
			RLCELLPOS l_cell = GetCellFromPoint(a_hWnd, a_pt, true);
			if (!l_pData->GetMouseCell().Equal(RCell(l_cell.cell)))
			{
				SetMouseCell(a_hWnd, a_pt, l_cell.cell.iRow, l_cell.cell.iCol);
				// hide old celltip, and try to show new if needed
				CellTip_Pop(a_hWnd);
#ifndef RLCF_NOTOOLTIP
				CellTip(a_hWnd, a_pt);
#endif
			}
		}
	}
}


//	---------------------------------------------------------------------------------------
//	Handler of WM_MOUSEWHEEL message
//
void OnMouseWheel(HWND a_hWnd, short /*a_nKeys*/, short a_nDelta, const POINT& /*a_pt*/)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	// not working when in edit mode
	if (l_pData->GetWndEdit() != NULL)
	{
		return;
	}

	if (!HasVertScrollBar(a_hWnd))
	{
		return;
	}

	VScroll(a_hWnd, -a_nDelta / WHEEL_DELTA) ;
}


//	---------------------------------------------------------------------
//	ustalenie kolumny
//
static void
OnSetCol(
	HWND a_hWnd,
	LPRLCCOLDEF a_pColDef
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	l_pData->AddColumn(a_pColDef->psColName, a_pColDef->iLength);
}


//	---------------------------------------------------------------------
//	Ustalenie trybu
//
static void	
OnSetMode(
	HWND a_hWnd, 
	BOOL a_bSet, 
	long a_iFlags
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);

	if (a_bSet)
	{
		l_pData->SetFlags(a_iFlags);
	}
	else
	{
		l_pData->UnsetFlags(a_iFlags);
	}
	l_pData->SetRectCalculated(FALSE);
}


//	---------------------------------------------------------------------
//	Ustalenie funkcji zwracaj¹cej dane
//
static void	
OnSetDataProc(
	HWND a_hWnd, 
	void* a_pObj, 
	RLISTDATAPROC a_proc
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	l_pData->SetDataProc(a_proc, a_pObj);
}


//	---------------------------------------------------------------------
//	Ustalenie funkcji zwracaj¹cej licznoœæ
//
static void	
OnSetCountProc(
	HWND a_hWnd, 
	void* a_pObj, 
	RLISTCOUNTPROC a_proc
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	l_pData->SetCountProc(a_proc, a_pObj);
}


//	---------------------------------------------------------------------
//	sets drawing cell procedure
//
static void	
OnSetDrawProc(
	HWND a_hWnd, 
	void* a_pObj, 
	RLISTDRAWPROC a_proc
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	l_pData->SetDrawProc(a_proc, a_pObj);
}


//	---------------------------------------------------------------------
//	sets drawing background procedure
//
static void	
OnSetDrawBkProc(HWND a_hWnd, void* a_pObj, RLISTDRAWBKPROC a_proc)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	l_pData->SetDrawBkProc(a_proc, a_pObj);
}


//	---------------------------------------------------------------------
//	sets drawing grid procedure
//
static void	
OnSetGridProc(
	HWND a_hWnd, 
	void* a_pObj, 
	RLISTGRIDPROC a_proc
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	l_pData->SetGridProc(a_proc, a_pObj);
}


//	---------------------------------------------------------------------
//	sets color
//
static COLORREF
OnSetColor(
	HWND a_hWnd, 
	UINT a_iIdx, 
	COLORREF a_cr
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	return l_pData->SetColor(a_iIdx, a_cr);
}


//	---------------------------------------------------------------------
//	gets color
//
static COLORREF
OnGetColor(
	HWND a_hWnd, 
	UINT a_iIdx
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	return l_pData->GetColor(a_iIdx);
}


//	---------------------------------------------------------------------
//	sets fixed cols count
//
void
OnSetFixedCols(
	HWND a_hWnd, 
	long a_iFixedCols
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	l_pData->SetFixedCols(a_iFixedCols);
}


//	---------------------------------------------------------------------
//	sets fixed cols count
//
void
OnSetFixedRows(
	HWND a_hWnd, 
	long a_iFixedRows
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	l_pData->SetFixedRows(a_iFixedRows);
}


//	---------------------------------------------------------------------
//	returns number of fixed rows or cols
//
static inline long
OnGetFixed(
	HWND a_hWnd, 
	BOOL a_bCols
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	return a_bCols ? l_pData->GetFixedCols() : l_pData->GetFixedRows();
}


//	---------------------------------------------------------------------
//	sets row height
//
static inline long
OnSetRowHeight(
	HWND a_hWnd, 
	long a_iRow,
	long a_iHeight
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	long l_iHeight;
	if (a_iRow == -1)
	{
		l_iHeight = l_pData->GetDefRowHeight();
		l_pData->SetDefRowHeight(a_iHeight);
	}
	else
	{
		l_iHeight = l_pData->GetRowHeight(a_iRow);
		l_pData->SetRowHeight(a_iRow, a_iHeight);
	}
	l_pData->SetRectCalculated(FALSE);
	return l_iHeight;
}


//	---------------------------------------------------------------------
//	sets col width
//
static inline long
OnSetColWidth(
	HWND a_hWnd, 
	long a_iCol,
	long a_iWidth
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	long l_iWidth = l_pData->GetColumn(a_iCol).GetWidth();
	l_pData->SetColumnWidth(a_iCol, a_iWidth);
	l_pData->SetRectCalculated(FALSE);
	return l_iWidth;
}


//	---------------------------------------------------------------------
//	returns rows count
//
static inline long
OnRowsCount(
	HWND a_hWnd
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	return l_pData->GetRowsCount();
}


//	---------------------------------------------------------------------
//	returns cols count
//
static inline long
OnColsCount(
	HWND a_hWnd
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	return l_pData->GetColumnsCount();
}


//	---------------------------------------------------------------------
//	returns row height
//
static inline long
OnGetRowHeight(
	HWND a_hWnd,
	long a_iRow
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (a_iRow == -1)
	{
		return l_pData->GetDefRowHeight();
	}
	return l_pData->GetRowHeight(a_iRow);
}


//	---------------------------------------------------------------------
//	returns cols count
//
static inline long
OnGetColWidth(
	HWND a_hWnd,
	long a_iCol
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	return l_pData->GetColumn(a_iCol).GetWidth();
}


//	---------------------------------------------------------------------
//	Draws grid
//
static void
DrawGrid(
 	HWND a_hWnd,	//WE okno
	HDC a_hDC		//WE kontekst
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (!l_pData->TestFlags(LMB_DRAWVLINES) && !l_pData->TestFlags(LMB_DRAWHLINES))
	{
		return ;
	}

	RLGRID l_grid;
	COLORREF l_clr = (IsFocused(a_hWnd) ? l_pData->GetColor(RLC_GRID_ACT) : l_pData->GetColor(RLC_GRID_NOACT));
	l_grid.hPen = ::CreatePen(PS_SOLID, 1, l_clr);

	if (l_pData->TestFlags(LMB_DRAWHLINES))
	{
		l_grid.bVert = FALSE;
		long l_iStartRow = l_pData->GetFirstVisRow();
		long l_iLastVisRow = GetLastVisibleRow(a_hWnd);
		T_COLPOS l_colPos = GetLastColPos(a_hWnd);

		long l_iRow;  // row
		// l_rect.left = l_pData->GetFixedColsWidth(); // if only for normal cell
		l_grid.rect.left = 0;
		l_grid.rect.right = l_colPos.xRight;
		// fixed rows grid
		for (l_iRow = 0; l_iRow < l_pData->GetFixedRows(); l_iRow++)
		{
			long l_yPos = GetRowPos(a_hWnd, l_iRow).yBottom;
			l_grid.iRowCol = l_iRow;
			l_grid.rect.top = l_grid.rect.bottom = l_yPos - 1;
			l_pData->DrawGrid(a_hWnd, a_hDC, &l_grid);
		}

		// normal rows
		for (l_iRow = l_iStartRow; l_iRow <= l_iLastVisRow; l_iRow++)
		{
			long l_yPos = GetRowPos(a_hWnd, l_iRow).yBottom;
			l_grid.iRowCol = l_iRow;
			l_grid.rect.top = l_grid.rect.bottom = l_yPos - 1;
			l_pData->DrawGrid(a_hWnd, a_hDC, &l_grid);
		}
	}

	if (l_pData->TestFlags(LMB_DRAWVLINES))
	{
		l_grid.bVert = TRUE;
		long l_iHeight = GetLastRowPos(a_hWnd).yBottom;
		long l_iStartCol = l_pData->GetFirstVisCol();
		long l_iLastCol = GetLastVisibleCol(a_hWnd);

		// l_grid.rect.top = l_pData->GetFixedRowsHeight(); // when only normal cella with grid
		l_grid.rect.top = 0;
		l_grid.rect.bottom = l_iHeight;

		long l_iCol;
		// fixed cols
		for (l_iCol = 0; l_iCol < l_pData->GetFixedCols(); l_iCol++)
		{
			long l_xPos = GetColPos(a_hWnd, l_iCol).xRight;
			l_grid.iRowCol = l_iCol;
			l_grid.rect.left = l_grid.rect.right = l_xPos - 1;
			l_pData->DrawGrid(a_hWnd, a_hDC, &l_grid);
		}

		// normal cols
		for (l_iCol = l_iStartCol; l_iCol <= l_iLastCol; l_iCol++)
		{
			long l_xPos = GetColPos(a_hWnd, l_iCol).xRight;
			l_grid.iRowCol = l_iCol;
			l_grid.rect.left = l_grid.rect.right = l_xPos - 1;
			l_pData->DrawGrid(a_hWnd, a_hDC, &l_grid);
		}
	}
	::DeleteObject(l_grid.hPen);
}


//	---------------------------------------------------------------------------------------
//	Draws grid
//
static BOOL 
DrawGrid(
	HWND /*a_hWnd*/, 
	HDC a_hDC, 
	LPRLGRID a_pGrid
	)
{
	HPEN l_hPenOld = (HPEN)::SelectObject(a_hDC, a_pGrid->hPen);

	if (a_pGrid->bVert)
	{
		::MoveToEx(a_hDC, a_pGrid->rect.left, a_pGrid->rect.top, NULL);
		::LineTo(a_hDC, a_pGrid->rect.left, a_pGrid->rect.bottom);
	}
	else
	{		
		::MoveToEx(a_hDC, a_pGrid->rect.left, a_pGrid->rect.top, NULL) ;
		::LineTo(a_hDC, a_pGrid->rect.right, a_pGrid->rect.top) ;
	}

	::SelectObject(a_hDC, l_hPenOld);
	return TRUE;
}


//	---------------------------------------------------------------------------------------
//	Narysowanie wierszy
//
static void	
DrawRows(
 	HWND a_hWnd,	//WE okno
	HDC a_hDC		//WE kontekst
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);


	// const rows
	long l_iRow;
	for (l_iRow = 0; l_iRow < l_pData->GetFixedRows(); l_iRow++)
	{
		DrawRow(a_hWnd, a_hDC, l_iRow);
	}
	long l_iStart = l_pData->GetFirstVisRow();

	// non-const rows
	long l_iLastVisRow = GetLastVisibleRow(a_hWnd) ;
	for (l_iRow = l_iStart; l_iRow <= l_iLastVisRow; l_iRow++)
	{
		DrawRow(a_hWnd, a_hDC, l_iRow);
	}
}


//	---------------------------------------------------------------------------------------
//	Draws one row
//
static void 
DrawRow(
 	HWND a_hWnd,	//IN window
	HDC a_hDC,		//IN device context
	long a_iRow		//IN row to draw
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);

	long l_iStartCol = l_pData->GetFirstVisCol();
	long l_iLastCol = GetLastVisibleCol(a_hWnd);
	long l_iCol;

	// fixed columns
	for (l_iCol = 0; l_iCol < l_pData->GetFixedCols(); l_iCol++)
	{
		DrawCell(a_hWnd, a_hDC, a_iRow, l_iCol);
	}

	// not fixed columns
	for (l_iCol = l_iStartCol; l_iCol <= l_iLastCol; l_iCol++)
	{
		DrawCell(a_hWnd, a_hDC, a_iRow, l_iCol);
	}

}


//	---------------------------------------------------------------------------------------
//	Draws one cell
//
static void 
DrawCell(
	HWND a_hWnd,
	HDC a_hDC, 
	long a_iRow, 
	long a_iCol
	)
{

	RListData* l_pData = GetRListCtrlData(a_hWnd);

	// skonstruuj rectangla
	RECT l_rect = GetCellRect(a_hWnd, a_iRow, a_iCol);
	RLCELL l_cell;
	BOOL l_bSelected = (l_pData->GetSelRow() == a_iRow);
	BOOL l_bFixed = (a_iRow < l_pData->GetFixedRows()) || (a_iCol < l_pData->GetFixedCols());

	// set colors

	if (l_bFixed)
	{
		if (l_bSelected)
		{
			(IsFocused(a_hWnd)) ? 
				l_pData->GetColors(E_CT_FIXEDACTSEL, &(l_cell.clrs)) : l_pData->GetColors(E_CT_FIXEDNOACTSEL, &(l_cell.clrs));
		}
		else
		{
			(IsFocused(a_hWnd)) ? 
				l_pData->GetColors(E_CT_FIXEDACT, &(l_cell.clrs)) : l_pData->GetColors(E_CT_FIXEDNOACTNOSEL, &(l_cell.clrs));
		}
	}
	else
	{
		if (l_bSelected)
		{
			(IsFocused(a_hWnd)) ?
				l_pData->GetColors(E_CT_CELLACTSEL, &(l_cell.clrs)) : l_pData->GetColors(E_CT_CELLNOACTSEL, &(l_cell.clrs));
		}
		else
		{
			(IsFocused(a_hWnd)) ?
				l_pData->GetColors(E_CT_CELLACT, &(l_cell.clrs)) : l_pData->GetColors(E_CT_CELLNOACTNOSEL, &(l_cell.clrs));
		}
	}

	l_cell.iState = 
		(l_bSelected ? RLS_SELECTED : 0) | 
		(IsFocused(a_hWnd) ? RLS_FOCUSED : 0);
	if (
		(l_bSelected && l_cell.clrs.clrBackground != l_pData->GetColor(RLC_BK_ACT)) || 
		(!l_pData->TestFlags(LMB_DRAWTRANSP))
		)
	{
		l_cell.iState |= RLS_DRAWBK;
	}


	// is cell fixed
	if (l_bFixed)
	{
		l_cell.iState |= RLS_FIXED;
	}

	l_cell.iGridMode = l_pData->GetFlags();
	l_cell.pos.cell.iRow = a_iRow;
	l_cell.pos.cell.iCol = a_iCol;
	l_cell.hFont = l_pData->GetFont();
	l_cell.pos.rect = l_rect;
	l_cell.nMargin = l_pData->GetTextMargin();
	LPCTSTR l_psText;

	// titles from set col
	if (l_pData->GetData(a_iRow, a_iCol, &l_psText))
	{
		l_cell.sText = l_psText;
	}

	// titles from set col
	else if (l_bFixed && (a_iRow == 0))
	{
		const RColumn& l_col = l_pData->GetColumn(a_iCol);
		l_cell.sText = l_col.GetColName().c_str();
	}

	if (l_pData->TestFlags(LMB_LEFTALIGN))
	{
		l_cell.iFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;
	}
	else if (l_pData->TestFlags(LMB_RIGHTALIGN))
	{
		l_cell.iFormat = DT_RIGHT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;
	}
	else
	{
		l_cell.iFormat = 0;
	}


	l_pData->DrawCell(a_hWnd, a_hDC, &l_cell);
}


//	---------------------------------------------------------------------------------------
//	Draws one cell
//
static void 
DrawCell(
	HWND /*a_hWnd*/,
	HDC a_hDC, 
	LPRLCELL a_pCell
	)
{

	RDraw::CLRCELL l_clrs;
	l_clrs.clrBorderLeft = a_pCell->clrs.clrLBorder;
	l_clrs.clrBorderRight = a_pCell->clrs.clrRBorder;
	l_clrs.clrBorderTop = a_pCell->clrs.clrUBorder;
	l_clrs.clrBorderBottom = a_pCell->clrs.clrDBorder;
	l_clrs.clrBk = a_pCell->clrs.clrBackground;

	RDraw::TEXTCELL l_text;
	l_text.clrText = a_pCell->clrs.clrText;
	l_text.hFont = a_pCell->hFont;
	l_text.sText = a_pCell->sText;
	l_text.iFormat = a_pCell->iFormat;
	l_text.nMargin = a_pCell->nMargin;

	BOOL l_bTransparent = (!(a_pCell->iState & RLS_DRAWBK));
	BOOL l_bBorder = (a_pCell->iGridMode & LMB_DRAWCELLBORDER);
	//l_bTransparent = (!(a_pCell->iState & RLS_FIXED) && (!(a_pCell->iState & RLS_DRAWBK)));

	RDraw::DrawCell(a_hDC, a_pCell->pos.rect, l_clrs, l_text, l_bTransparent, l_bBorder);
}


//	---------------------------------------------------------------------------------------
//	Draws tracking line
//
static void
DrawTracking(
	HWND a_hWnd, 
	HDC a_hdc
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (l_pData->IsTrackingCol())
	{
		long l_iHeight = GetLastRowPos(a_hWnd).yBottom;
		if (l_pData->GetTrackLineInverted())
		{
			RECT l_rect = {l_pData->GetTrackPos(), 0, l_pData->GetTrackPos() + l_pData->GetTrackLineWidth(), l_iHeight};
			::InvertRect(a_hdc, &l_rect);

		}
		else
		{
			::MoveToEx(a_hdc, l_pData->GetTrackPos(), 0, NULL);
			HPEN l_hPen = ::CreatePen(PS_SOLID, l_pData->GetTrackLineWidth(), l_pData->GetTrackLineColor());
			HPEN l_hPenOld = (HPEN)::SelectObject(a_hdc, l_hPen);
			::LineTo(a_hdc, l_pData->GetTrackPos(), l_iHeight) ;
			::SelectObject(a_hdc, l_hPenOld);
			::DeleteObject(l_hPen);
		}
	}
	else if (l_pData->IsTrackingRow())
	{
		long l_iWidth = GetLastColPos(a_hWnd).xRight;
		if (l_pData->GetTrackLineInverted())
		{
			RECT l_rect = {0, l_pData->GetTrackPos(), l_iWidth, l_pData->GetTrackPos() + l_pData->GetTrackLineWidth()};
			::InvertRect(a_hdc, &l_rect);

		}
		else
		{
			::MoveToEx(a_hdc, 0, l_pData->GetTrackPos(), NULL);
			HPEN l_hPen = ::CreatePen(PS_SOLID, l_pData->GetTrackLineWidth(), l_pData->GetTrackLineColor());
			HPEN l_hPenOld = (HPEN)::SelectObject(a_hdc, l_hPen);
			::LineTo(a_hdc, l_iWidth, l_pData->GetTrackPos()) ;
			::SelectObject(a_hdc, l_hPenOld);
			::DeleteObject(l_hPen);
		}
	}

}


//	---------------------------------------------------------------------------------------
//	returns not fixed rows count to draw
//
static long	//OUT rows to draw count
CalculateRows2Draw(
	HWND a_hWnd		//WE window handle
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	return GetLastVisibleRow(a_hWnd) - l_pData->GetFirstVisRow() + 1;
}


//	---------------------------------------------------------------------------------------
//	Handler of WM_SETFOCUS
//
void
OnSetFocus(
	HWND a_hWnd
	)
{
	NMHDR l_nmhdr;
	l_nmhdr.code = RLCN_SETFOCUS;
	Notify(a_hWnd, &l_nmhdr);
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);
}


//	---------------------------------------------------------------------------------------
//	Handler of WM_KILLFOCUS
//
void
OnKillFocus(
	HWND a_hWnd
	)
{
	NMHDR l_nmhdr;
	l_nmhdr.code = RLCN_SETFOCUS;
	Notify(a_hWnd, &l_nmhdr);

	RListData* l_pData = GetRListCtrlData(a_hWnd);
	POINT l_pt;
	::GetCursorPos(&l_pt);
	::ScreenToClient(a_hWnd, &l_pt);

	SetMouseCell(a_hWnd, l_pt, -1, -1);
#ifndef RLCF_NOTOOLTIP
	RCellTip_Pop(l_pData->GetCellTip());
#endif
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);
}


//	---------------------------------------------------------------------------------------
//	Handler of WM_KEYDOWN
//
void OnKeyDown(HWND a_hWnd,	UINT a_iChar, LPARAM a_lParam)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	NMKEY l_nmkey;
	l_nmkey.hdr.code = NM_KEYDOWN;
	l_nmkey.nVKey = a_iChar;
	l_nmkey.uFlags = a_lParam;
	if (Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmkey)) != 0)
	{
		return;
	}
	switch(a_iChar)
	{
	case VK_DOWN:
		SelectRow(a_hWnd, l_pData->GetSelRow() + 1);
		break;

	case VK_UP:
		SelectRow(a_hWnd, l_pData->GetSelRow() - 1) ;
		break;

	case VK_LEFT:
		if (HasHorzScrollBar(a_hWnd))
		{
			HScroll(a_hWnd, -1);
		}
		break;

	case VK_RIGHT:
		if (HasHorzScrollBar(a_hWnd))
		{
			HScroll(a_hWnd, 1);
		}
		break;

	case VK_NEXT:
		SelectRowEx(a_hWnd, 
			l_pData->GetSelRow() + GetLastVisibleRow(a_hWnd) - l_pData->GetFirstVisRow() + 1);
		break;

	case VK_PRIOR:
		SelectRowEx(a_hWnd, l_pData->GetSelRow() - GetLastVisibleRow(a_hWnd) + l_pData->GetFirstVisRow() - 1);
		break;

	case VK_HOME:
		First(a_hWnd);
		break;

	case VK_END:
		Last(a_hWnd);
		break;

	case VK_RETURN:
		{
			#pragma todo ("check, if works correctly after removing declaration of l_pData var")
		const RListData* l_pData = GetRListCtrlData(a_hWnd);
		if ((l_nmkey.nVKey == VK_RETURN) && (!l_pData->IsEditCell()))
		{
			long l_iSelRow = l_pData->GetSelRow();
			if (l_iSelRow >= l_pData->GetFixedRows())
			{
				RLCNMSELECTED l_nmSelected;
				l_nmSelected.nmhdr.code = RLCN_SELECTED;
				l_nmSelected.iRow = l_iSelRow;
				Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmSelected));
			}
		}
	}
}
}


//	---------------------------------------------------------------------------------------
//	Handler of WM_CHAR
//
void OnChar(HWND a_hWnd, UINT a_iChar, LPARAM a_lParam)
{
	NMCHAR l_nmchar;
	l_nmchar.hdr.code = NM_CHAR;
	l_nmchar.ch = a_iChar;
	l_nmchar.dwItemPrev = a_lParam;
	if (Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmchar)) != 0)
	{
		return;
	}
}


//	---------------------------------------------------------------------------------------
//	Handler of WM_KEYUP
//
void OnKeyUp(HWND a_hWnd, UINT a_iChar, LPARAM a_lParam)
{
	NMKEY l_nmkey;
	l_nmkey.hdr.code = RLCN_KEYUP;
	l_nmkey.nVKey = a_iChar;
	l_nmkey.uFlags = a_lParam;
	if (Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmkey)) != 0)
	{
		return;
	}
}


//	---------------------------------------------------------------------------------------
//	Zaznaczenie wiersza ze skrollowaniem 
//	Jeœli wartoœæ wiersza jest mniejsza od zera to wybiera 1 wiersz nie fixed 
//	jeœli jest wiêksza od iloœci wierszy to wybiera ostatni
//
static void 
SelectRowEx(
	HWND a_hWnd,
	long a_iRow
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (a_iRow < l_pData->GetFixedRows())
	{
		a_iRow = l_pData->GetFixedRows();
	}
	if (a_iRow >= l_pData->GetRowsCount())
	{
		a_iRow = l_pData->GetRowsCount() - 1;
	}
	SelectRow(a_hWnd, a_iRow);
}


//	---------------------------------------------------------------------------------------
//	Zaznaczenie wiersza ze skrollowaniem
//
static void 
SelectRow(
	HWND a_hWnd,
	long a_iRow
	)
{
	// mo¿na blokowaæ
	if (SelChanging(a_hWnd, a_iRow))
		return;

	RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (
		(a_iRow < l_pData->GetFixedRows()) || 
		(a_iRow >= l_pData->GetRowsCount()) ||
		(a_iRow == l_pData->GetSelRow())
		)
	{
		return;
	}

	// do we need scroll
	short l_nMoveFirstVis = 0 ; // 0 - nie ; 1 - do góry ; -1 na dó³
	long l_iRows2Draw = GetLastVisibleRow(a_hWnd) - l_pData->GetFirstVisRow() + 1;
	if (a_iRow > l_pData->GetFirstVisRow())
	{
		if (a_iRow > GetLastVisibleRow(a_hWnd))
		{
			l_nMoveFirstVis = 1 ;
		}
	}
	else if (a_iRow < l_pData->GetFirstVisRow())
	{
		l_nMoveFirstVis = -1 ;
	}

	// trzeba przesun¹æ pierwszy - tak by wybrany by³ na œrodku
 	if (l_nMoveFirstVis != 0)
	{
		long l_iNewPos = a_iRow - l_iRows2Draw / 2 + 1 ;
		if (l_iNewPos <= 0)
			l_iNewPos = 1 ;
		if (l_iNewPos > l_pData->GetRowsCount() - l_iRows2Draw + 1)
			l_iNewPos = l_pData->GetRowsCount() - l_iRows2Draw + 1 ;
		VScrollTo(a_hWnd, l_iNewPos) ;
	}
	else
	{
		InvalidateRow(a_hWnd, l_pData->GetSelRow());
		InvalidateRow(a_hWnd, a_iRow) ;
	}

	// nowe pozycje
	l_pData->SetSelRow(a_iRow);
	SelChanged(a_hWnd);
}


//	---------------------------------------------------------------------------------------
//	Selects row from point
//
static void	
SelectRow(
	HWND a_hWnd, 
	const POINT& a_pt
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	RLCELLPOS l_cell = GetCellFromPoint(a_hWnd, a_pt, false);
	if ((l_cell.cell.iCol >= 0) && (l_cell.cell.iRow >= 0) && (l_cell.cell.iRow != l_pData->GetSelRow()))
	{
		SelectRow(a_hWnd, l_cell.cell.iRow) ;
	}
}


//	---------------------------------------------------------------------------------------
//	Checks if we can change row
//
bool	
SelChanging(
	HWND a_hWnd, 
	long a_iRow
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);

	RLCNMSEL l_nmhdr;
	l_nmhdr.nmhdr.code = RLCN_SELCHANGING;
	l_nmhdr.iCurrRow = l_pData->GetSelRow();
	l_nmhdr.iNewRow = a_iRow;

	return (Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmhdr)) != 0);
}


//	---------------------------------------------------------------------------------------
//	Message after active row has changed
//
static void	
SelChanged(
	HWND a_hWnd
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);

	RLCNMSEL l_nmhdr;
	l_nmhdr.nmhdr.code = RLCN_SELCHANGED;
	l_nmhdr.iCurrRow = l_pData->GetPrevSelRow();
	l_nmhdr.iNewRow = l_pData->GetSelRow();

	Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmhdr));
}


//	---------------------------------------------------------------------------------------
//	Moving list to given row
//
static void 
VScrollTo(
	HWND a_hWnd,
	long a_iRow
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	VScroll(a_hWnd, a_iRow - l_pData->GetFirstVisRow()) ;
}


//	---------------------------------------------------------------------------------------
//	Moves list to given row without moving current row
//
static void 
VScroll(
	HWND a_hWnd,
	long a_iScroll
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);

	// oblicz wzglêdn¹ pozycjê (od pocz¹tku rysowania) aktywnego wiersza
	long l_iFirstVisRowOld = l_pData->GetFirstVisRow();
	long l_iOnScreenSel ;	// po³o¿enie wiersza zaznaczonego wzglêdne
	long l_iSelRowDiff = l_pData->GetSelRow() - l_iFirstVisRowOld;

	if (l_iSelRowDiff < 0)
		l_iOnScreenSel = -1 ;
	else if (l_iSelRowDiff == 0)
		l_iOnScreenSel = 1 ;
	else 
	{
		if (l_iSelRowDiff < CalculateRows2Draw(a_hWnd))
			l_iOnScreenSel = l_iSelRowDiff + 1 ;
		else
			l_iOnScreenSel = -1 ;
	}

	if (l_iFirstVisRowOld + a_iScroll < l_pData->GetFixedRows())
	{
		l_pData->SetFirstVisRow(l_pData->GetFixedRows());
	}
	else
	{
		l_pData->SetFirstVisRow(l_iFirstVisRowOld + a_iScroll);
	}

	// za³atwienie by ostatni wiersz wyœwietla³ siê najni¿ej jak to tylko mo¿liwe
	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);
	long l_iLastRow = GetRowToSeeLast(a_hWnd, l_rect.bottom - l_rect.top);
	if (l_pData->GetFirstVisRow() > l_iLastRow)
	{
		l_pData->SetFirstVisRow(l_iLastRow);
	}

	if (l_iFirstVisRowOld != l_pData->GetFirstVisRow())
	{
		l_pData->SetRectCalculated(FALSE);
		InvalidateDrawRectV(a_hWnd);
	}
}


//	---------------------------------------------------------------------------------------
//	Invalidates given row
//
static void 
InvalidateRow(
	HWND a_hWnd, 
	long a_iRow
	)
{
	RECT l_rect ;	// invalidowany prostok¹t

	T_ROWPOS l_rowPos = GetRowPos(a_hWnd, a_iRow);
	l_rect.top = l_rowPos.yTop;
	l_rect.bottom = l_rowPos.yBottom;
	l_rect.left = 0;

	l_rect.right = GetRightGridSize(a_hWnd);
	::InvalidateRect(a_hWnd, &l_rect, TRUE); 
}


//	---------------------------------------------------------------------------------------
//	Invalidates given col
//
static void 
InvalidateCol(
	HWND a_hWnd, 
	long a_iCol
	)
{
	RECT l_rect ;	// invalidated rectprostok¹t

	T_COLPOS l_colPos = GetColPos(a_hWnd, a_iCol);
	l_rect.top = 0;
	l_rect.bottom = GetLastRowPos(a_hWnd).yBottom;
	l_rect.left = l_colPos.xLeft;
	l_rect.right = l_colPos.xRight;

	::InvalidateRect(a_hWnd, &l_rect, TRUE); 
}


void 
InvalidateCell(HWND a_hWnd, long a_iRow, long a_iCol)
{
	RECT l_rect = GetCellRect(a_hWnd, a_iRow, a_iCol);
	::InvalidateRect(a_hWnd, &l_rect, TRUE);
}


//	---------------------------------------------------------------------------------------
//	Invalidates permanent drawing region
//
static void 
InvalidateDrawRectV(
	HWND a_hWnd
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	RECT l_rect2Inv ;		// obszar do przerysowania
	RECT l_rectClient ;	// ca³y obszar klienta
	::GetClientRect(a_hWnd, &l_rectClient) ;

	l_rect2Inv.top = l_pData->GetFixedRowsHeight();
	l_rect2Inv.left = 0;
	if (CalculateRows2Draw(a_hWnd) < l_pData->GetRowsCount())
	{
		l_rect2Inv.bottom = l_rectClient.bottom ;
	}
	else
	{
		T_ROWPOS l_rowPos = GetLastRowPos(a_hWnd);
		l_rect2Inv.bottom = l_rowPos.yBottom;
	}

	l_rect2Inv.right = GetRightGridSize(a_hWnd);
	
	InvalidateRect(a_hWnd, &l_rect2Inv, FALSE) ;

}


//	---------------------------------------------------------------------------------------
//	returns right position of last element on the window
//
static long 
GetRightGridSize(
	HWND a_hWnd
	)
{
	return GetColPos(a_hWnd, GetLastVisibleCol(a_hWnd)).xRight;
}


//	---------------------------------------------------------------------------------------
//	Moves list left or right
//
static void 
HScroll(
	HWND a_hWnd, 
	long a_iScroll
	)
{
	if (a_iScroll == 0)
	{
		return;
	}

	RListData* l_pData = GetRListCtrlData(a_hWnd);
	long l_iFirstVisCol = l_pData->GetFirstVisCol();

	long l_iRightBefore = GetRightGridSize(a_hWnd) ;	// pobranie prawej krawêdzie grid'a sprzed zmian

	l_pData->SetFirstVisCol(l_iFirstVisCol + a_iScroll);
	if (l_pData->GetFirstVisCol() < l_pData->GetFixedCols())
	{
		l_pData->SetFirstVisCol(l_pData->GetFixedCols());
	}
	long l_iCols = l_pData->GetColumnsCount();
	if (l_pData->GetFirstVisCol() >= l_iCols)
	{
		l_pData->SetFirstVisCol(l_iCols - 1);
	}

	// to see as much columns as posible
	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);
	long l_iLastCol = GetColToSeeLast(a_hWnd, l_rect.right - l_rect.left);
	if (l_pData->GetFirstVisCol() > l_iLastCol)
	{
		l_pData->SetFirstVisCol(l_iLastCol);
	}

	if (l_pData->GetFirstVisCol() == l_iFirstVisCol)
	{
		return;
	}

	l_pData->SetRectCalculated(FALSE);

	//trzeba zmieniæ pobieranie l_iRight na w³aœciwe!!!
	long l_iRightAfter = GetRightGridSize(a_hWnd) ;	// pobranie prawej krawêdzie grid'a sprzed zmian
	InvalidateDrawRectH(a_hWnd, max(l_iRightAfter, l_iRightBefore));
	::UpdateWindow(a_hWnd);
}


//	---------------------------------------------------------------------------------------
//	Moves list left or right
//
static void 
HScrollTo(
	HWND a_hWnd, 
	long a_iPos
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	HScroll(a_hWnd, a_iPos - l_pData->GetFirstVisCol()) ;
}


//	---------------------------------------------------------------------------------------
//	Moves list to first row
//
static void	
First(
	HWND a_hWnd
	)
{
	SelectRow(a_hWnd, 1);
}


//	---------------------------------------------------------------------------------------
//	Moves list to last row
//
static void
Last(
	HWND a_hWnd
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);

	SelectRow(a_hWnd, l_pData->GetRowsCount() - 1);
}


//	---------------------------------------------------------------------------------------
//	Invalidates proper rect
//
static void 
InvalidateDrawRectH(
	HWND a_hWnd, 
	long a_iRight
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	RECT l_rect2Inv ;		// obszar do przerysowania
	RECT l_rectClient ;	// ca³y obszar klienta
	::GetClientRect(a_hWnd, &l_rectClient) ;

	l_rect2Inv.top = 0 ;
	l_rect2Inv.left = l_pData->GetFixedColsWidth();
	long l_iListHeight = GetLastRowPos(a_hWnd).yBottom;
	if (l_iListHeight > l_rectClient.bottom - l_rectClient.top)
	{
		l_rect2Inv.bottom = l_rectClient.bottom ;
	}
	else
	{
		l_rect2Inv.bottom = l_iListHeight;
	}

	l_rect2Inv.right = a_iRight + 1 ;
	
	::InvalidateRect(a_hWnd, &l_rect2Inv, TRUE) ;
}


//	---------------------------------------------------------------------------------------
//	Calculates if window has to have scrollbars.
//
static void 
CalculateDrawRect(
	HWND a_hWnd
	)
{

	RListData* l_pData = GetRListCtrlData(a_hWnd);
	// obszar policzony - spadamy
	if (l_pData->GetRectCalculated())
	{
		return;
	}

	// jak tu doszliœmy to na pewno obszar obliczymy
	l_pData->SetRectCalculated(TRUE);

	// calculates real (with scrollbars) client size
	SIZE l_sizeClient = GetClientSize(a_hWnd);

	long l_iColForLast = GetColToSeeLast(a_hWnd, l_sizeClient.cx);
	long l_iRowForLast = GetRowToSeeLast(a_hWnd, l_sizeClient.cy);
	BOOL l_bHorzBar = l_iColForLast != l_pData->GetFixedCols();
	BOOL l_bVertBar = l_iRowForLast != l_pData->GetFixedRows();

	// nie ma Vert, nie ma te¿ Hor - sprawa prosta
	if ((!l_bVertBar) && (!l_bHorzBar))
	{
#ifdef _WIN32_WCE
		::SetScrollRange(a_hWnd, SB_HORZ, 1, 1, TRUE);
		::SetScrollRange(a_hWnd, SB_VERT, 1, 1, TRUE);
#else
		::ShowScrollBar(a_hWnd, SB_BOTH, FALSE);

#endif
		l_pData->SetFirstVisCol(l_pData->GetFixedCols());
		l_pData->SetFirstVisRow(l_pData->GetFixedRows());
		return ;
	}

	// jest Ver, nie ma Hor - trzeba to g³êbiej sprawdziæ
	if (l_bVertBar && (!l_bHorzBar))
	{
		l_iColForLast = GetColToSeeLast(a_hWnd, l_sizeClient.cx - c_dxVScroll);
		l_bHorzBar = l_iColForLast != l_pData->GetFixedCols();
	}
	else if ((!l_bVertBar) && l_bHorzBar)
	{
		l_iRowForLast = GetRowToSeeLast(a_hWnd, l_sizeClient.cy - c_dyHScroll);
		l_bVertBar = l_iRowForLast != l_pData->GetFixedRows();
	}

	if (l_bVertBar && l_bHorzBar)
	{
		l_iRowForLast = GetRowToSeeLast(a_hWnd, l_sizeClient.cy - c_dyHScroll);
		l_iColForLast = GetColToSeeLast(a_hWnd, l_sizeClient.cx - c_dxVScroll);
	}


#ifndef _WIN32_WCE
	::ShowScrollBar(a_hWnd, SB_HORZ, l_bHorzBar);
	::ShowScrollBar(a_hWnd, SB_VERT, l_bVertBar);
#endif

	if (l_bHorzBar)
	{
		if (l_pData->GetFirstVisCol() > l_iColForLast)
		{
			l_pData->SetFirstVisCol(l_iColForLast);
		}
		::SetScrollRange(a_hWnd, SB_HORZ, 1, 
			l_iColForLast - l_pData->GetFixedCols() + 1, FALSE);
		::SetScrollPos(a_hWnd, SB_HORZ, l_pData->GetFirstVisCol() - l_pData->GetFixedCols() + 1, TRUE) ;
	}
	else
	{
#ifdef _WIN32_WCE
		::SetScrollRange(a_hWnd, SB_HORZ, 0, 0, TRUE);
#endif
		l_pData->SetFirstVisCol(l_pData->GetFixedCols());
	}

	if (l_bVertBar)
	{
		if (l_pData->GetFirstVisRow() > l_iRowForLast)
		{
			l_pData->SetFirstVisRow(l_iRowForLast);
		}
		::SetScrollRange(a_hWnd, SB_VERT, 1, 
			l_iRowForLast - l_pData->GetFixedRows() + 1, FALSE);
		::SetScrollPos(a_hWnd, SB_VERT, l_pData->GetFirstVisRow() - l_pData->GetFixedRows() + 1, TRUE);
	}
	else
	{
#ifdef _WIN32_WCE
		::SetScrollRange(a_hWnd, SB_VERT, 1, 1, TRUE);
#endif
		l_pData->SetFirstVisRow(l_pData->GetFixedRows());
	}
}


//	---------------------------------------------------------------------------------------
//	WM_VSCROLL message handler
//
void 
OnVScroll(
	HWND a_hWnd,
	UINT a_iSBCode
	) 
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	// not working when in edit mode
	if (l_pData->GetWndEdit() != NULL)
	{
		return;
	}

	SCROLLINFO l_scrollinfo;
	::ZeroMemory(&l_scrollinfo, sizeof(SCROLLINFO));
    l_scrollinfo.cbSize = sizeof(SCROLLINFO);
	long l_iPos;
	if (a_iSBCode == SB_THUMBTRACK)
	{
		l_scrollinfo.fMask = SIF_TRACKPOS;
		::GetScrollInfo(a_hWnd, SB_VERT, &l_scrollinfo);
		l_iPos = l_scrollinfo.nTrackPos;
	}
	else
	{
		l_scrollinfo.fMask = SIF_POS;
		::GetScrollInfo(a_hWnd, SB_VERT, &l_scrollinfo);
		l_iPos = l_scrollinfo.nPos;
	}

	switch (a_iSBCode)
	{
	case SB_LINEDOWN:
		VScroll(a_hWnd, 1) ;
		break;

	case SB_LINEUP:
		VScroll(a_hWnd, -1) ;
		break;

	case SB_PAGEDOWN:
		VScroll(a_hWnd, CalculateRows2Draw(a_hWnd)) ;
		break ;
	case SB_PAGEUP:
		VScroll(a_hWnd, -CalculateRows2Draw(a_hWnd)) ;
		break;
	
	case SB_BOTTOM:
		VScrollTo(a_hWnd, 1) ;
		break;

	case SB_TOP:
		VScrollTo(a_hWnd, l_pData->GetRowsCount()) ;
		break;

	case SB_THUMBPOSITION:
		VScrollTo(a_hWnd, l_iPos) ;
		break;

	case SB_THUMBTRACK:
		VScrollTo(a_hWnd, l_iPos) ;
		break;
	}
}


//	---------------------------------------------------------------------------------------
//	WM_VSCROLL message handler
//
static void 
OnHScroll(
	HWND a_hWnd,
	UINT a_iSBCode
	) 
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	// not working when in edit mode
	if (l_pData->GetWndEdit() != NULL)
	{
		return;
	}

	SCROLLINFO l_scrollinfo;
	::ZeroMemory(&l_scrollinfo, sizeof(SCROLLINFO));
    l_scrollinfo.cbSize = sizeof(SCROLLINFO);
	long l_iPos;
	if (a_iSBCode == SB_THUMBTRACK)
	{
		l_scrollinfo.fMask = SIF_TRACKPOS;
		::GetScrollInfo(a_hWnd, SB_HORZ, &l_scrollinfo);
		l_iPos = l_scrollinfo.nTrackPos;
	}
	else
	{
		l_scrollinfo.fMask = SIF_POS;
		::GetScrollInfo(a_hWnd, SB_HORZ, &l_scrollinfo);
		l_iPos = l_scrollinfo.nPos;
	}

	switch (a_iSBCode)
	{
	case SB_LINELEFT:
		HScroll(a_hWnd, -1) ;
		break ;

	case SB_LINERIGHT:
		HScroll(a_hWnd, 1);
		break ;

	case SB_PAGELEFT:
		HScroll(a_hWnd, -1) ;
		break ;
	case SB_PAGERIGHT:
		HScroll(a_hWnd, 1) ;
		break ;

	case SB_LEFT:
		HScrollTo(a_hWnd, 1) ;
		break ;

	case SB_RIGHT:
		HScrollTo(a_hWnd, l_pData->GetColumnsCount()) ;
		break ;

	case SB_THUMBPOSITION:
		HScrollTo(a_hWnd, l_iPos) ;
		break ;

	case SB_THUMBTRACK:
		HScrollTo(a_hWnd, l_iPos) ;
		break ;

	}
}


//	---------------------------------------------------------------------------------------
//	Returns left and right positions of columns (on screen)
//
static T_COLPOS
GetColPos(
	HWND a_hWnd,	//IN window
	long a_iCol		//IN column number
	)
{
	ASSERT(a_iCol >= 0);

	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	T_COLPOS l_pos = {0, 0};
	long l_iCol;
	long l_iFixedCols = l_pData->GetFixedCols();
	long l_iStartCol = l_pData->GetFirstVisCol();
	long l_iLastVisibleCol = GetLastVisibleCol(a_hWnd);

	if (l_iLastVisibleCol < a_iCol)
	{
		return l_pos;
	}

	for (l_iCol = 0; l_iCol < min(a_iCol, l_iFixedCols); l_iCol++)
	{
		l_pos.xLeft += l_pData->GetColumn(l_iCol).GetWidth();
	}

	if ((a_iCol < l_iFixedCols) || (a_iCol == 0))
	{
		l_pos.xRight = l_pos.xLeft + l_pData->GetColumn(a_iCol).GetWidth();
		return l_pos;
	}

	l_pos.xLeft = l_pData->GetFixedColsWidth();

	for (l_iCol = l_iStartCol; l_iCol < a_iCol; l_iCol++)
	{
		l_pos.xLeft += l_pData->GetColumn(l_iCol).GetWidth();
	}

	if (
		(l_pData->TestFlags(LMB_FILLLASTCOL)) &&
		(a_iCol == l_pData->GetColumnsCount() - 1)
		)
	{
		// the end of client rect
		RECT l_rectClient;
		::GetClientRect(a_hWnd, &l_rectClient);
		l_pos.xRight = l_rectClient.right;
	}
	else
	{
		l_pos.xRight = l_pos.xLeft + l_pData->GetColumn(a_iCol).GetWidth();
	}
	return l_pos;
}


//	---------------------------------------------------------------------------------------
//	Returns top and bottom client positions of row 
//
T_ROWPOS 
GetRowPos(
	HWND a_hWnd,	//IN window
	long a_iRow		//IN row number
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	T_ROWPOS l_pos = {0, 0};
	long l_yStart = 0;
	long l_iFixedRows = l_pData->GetFixedRows();
	long l_iRow;
	for (l_iRow = 0; l_iRow < min(a_iRow, l_iFixedRows); l_iRow++)
	{
		l_yStart += l_pData->GetRowHeight(l_iRow);
	}

	if (a_iRow < l_iFixedRows)
	{
		l_pos.yTop = l_yStart;
		l_pos.yBottom = l_pos.yTop + l_pData->GetRowHeight(a_iRow);

		return l_pos;
	}

	long l_iStartRow = l_pData->GetFirstVisRow();
	long l_iRows2Draw = CalculateRows2Draw(a_hWnd);
	if ((a_iRow < l_iStartRow) || (a_iRow >= l_iStartRow + l_iRows2Draw))
	{
		return l_pos;
	}


	for (l_iRow = l_iStartRow; l_iRow < a_iRow; l_iRow++)
	{
		l_yStart += l_pData->GetRowHeight(l_iRow);
	}

	l_pos.yTop = l_yStart;
	l_pos.yBottom = l_pos.yTop + l_pData->GetRowHeight(a_iRow);
	return l_pos;
}


//	---------------------------------------------------------------------------------------
//	Returns cell position in window
//
static RECT
GetCellRect(
	HWND a_hWnd, 
	long a_iRow, 
	long a_iCol
	)
{
	T_COLPOS l_posCol = GetColPos(a_hWnd, a_iCol);
	T_ROWPOS l_posRow = GetRowPos(a_hWnd, a_iRow);

	RECT l_rect = {l_posCol.xLeft, l_posRow.yTop, l_posCol.xRight, l_posRow.yBottom};
	return l_rect;
}


//	---------------------------------------------------------------------------------------
//	Returns position of last visible row
//
static T_ROWPOS 
GetLastRowPos(
	HWND a_hWnd
	)
{
	return GetRowPos(a_hWnd, GetLastVisibleRow(a_hWnd));
}


//	---------------------------------------------------------------------------------------
//	Returns position of last visible col
//
static T_COLPOS 
GetLastColPos(
	HWND a_hWnd
	)
{
	return GetColPos(a_hWnd, GetLastVisibleCol(a_hWnd));
}


//	---------------------------------------------------------------------------------------
//	Returns last col to draw in wnd
//
static long 
GetLastVisibleCol(
	HWND a_hWnd
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	RECT l_rect;
	::GetWindowRect(a_hWnd, &l_rect);
	long l_iWinWidth = l_rect.right - l_rect.left;
	long l_iFixedColsWidth = l_pData->GetFixedColsWidth();

	// all columns are fixed - lets count their width
	if (l_iWinWidth < l_iFixedColsWidth)
	{
		long l_iWidth = 0;
		for (long l_iCol = 0; l_iCol < l_pData->GetFixedCols(); l_iCol++)
		{
			l_iWidth += l_pData->GetColumn(l_iCol).GetWidth();
			if (l_iWidth >= l_iWinWidth)
			{
				return l_iCol;
			}
		}
		ASSERT(FALSE);
	}
	else
	{
		long l_iWidth = l_iFixedColsWidth;
		for (long l_iCol = l_pData->GetFirstVisCol(); l_iCol < l_pData->GetColumnsCount(); l_iCol++)
		{
			l_iWidth += l_pData->GetColumn(l_iCol).GetWidth();
			if (l_iWidth >= l_iWinWidth)
			{
				return l_iCol;
			}
		}
	}

	return l_pData->GetColumnsCount() - 1;
}


//	---------------------------------------------------------------------------------------
//	Returns last row to draw in wnd
//
static long 
GetLastVisibleRow(
	HWND a_hWnd
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);
	long l_iWinHeight = l_rect.bottom - l_rect.top;
	long l_iHeight = 0;

	long l_iStartRow = l_pData->GetFirstVisRow();
	long l_iRow;

	// fixed rows
	for (l_iRow = 0; l_iRow < l_pData->GetFixedRows(); l_iRow++)
	{
		l_iHeight += l_pData->GetRowHeight(l_iRow);
		if (l_iHeight > l_iWinHeight)
		{
			return (l_iRow == 0) ? 0 : l_iRow - 1;
		}
		else if (l_iHeight == l_iWinHeight)
		{
			return l_iRow;
		}
	}

	// not fixed rows
	long l_iRowsCount = l_pData->GetRowsCount();
	for (l_iRow = l_iStartRow; l_iRow < l_iRowsCount; l_iRow++)
	{
		l_iHeight += l_pData->GetRowHeight(l_iRow);
		if (l_iHeight > l_iWinHeight)
		{
			return (l_iRow == 0) ? 0 : l_iRow - 1;
		}
		else if (l_iHeight == l_iWinHeight)
		{
			return l_iRow;
		}
	}
	// last row
	return l_pData->GetRowsCount() - 1;
}


//	---------------------------------------------------------------------------------------
//	returns TRUE if point inside grid 
//
static BOOL 
HitInRange(
	HWND a_hWnd, 
	const POINT& a_pt
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	int l_iWidth = GetRightGridSize(a_hWnd);
	int l_iHeight = GetLastRowPos(a_hWnd).yBottom;

	return ((a_pt.y >= l_pData->GetFixedRowsHeight()) && 
		(a_pt.x <= l_iWidth + l_pData->GetFixedColsWidth()) && 
		(a_pt.y <= l_iHeight + l_pData->GetFixedRowsHeight())) ;
}


//	---------------------------------------------------------------------------------------
//	returns TRUE if point is on any row grid in control, also on top border of ctrl
//
static BOOL
MouseOnGridRow(
	HWND a_hWnd, 
	const POINT& a_pt
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);


	long l_iStartRow = l_pData->GetFirstVisRow();
	long l_iRows2Draw = CalculateRows2Draw(a_hWnd);


	// top border of control
	T_ROWPOS l_rowPos = GetRowPos(a_hWnd, 0);
	if ((a_pt.y >= l_rowPos.yTop - 1) && (a_pt.y <= l_rowPos.yTop + 1))
	{
		return TRUE;
	}		

	long l_iRow;
	// header borders
	for (l_iRow = 0; l_iRow < l_pData->GetFixedRows(); l_iRow++)
	{
		T_ROWPOS l_rowPos = GetRowPos(a_hWnd, l_iRow);
		if ((a_pt.y >= l_rowPos.yBottom - 1) && (a_pt.y <= l_rowPos.yBottom + 1))
		{
			return TRUE;
		}		
	}

	// other rows grid
	for (l_iRow = l_iStartRow; l_iRow < l_iStartRow + l_iRows2Draw; l_iRow++)
	{
		T_ROWPOS l_rowPos = GetRowPos(a_hWnd, l_iRow);
		// bingo! - it is on border
		if ((a_pt.y >= l_rowPos.yBottom - 1) && (a_pt.y <= l_rowPos.yBottom + 1))
		{
			return TRUE;
		}
	}

	// it is not on border
	return FALSE;
}



//	---------------------------------------------------------------------------------------
//	returns TRUE if point is on any column grid in control
//
static BOOL 
MouseOnGridCol(
	HWND a_hWnd, 
	const POINT& a_pt
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);

	long l_iStartCol = l_pData->GetFirstVisCol();
	long l_iLastCol = GetLastVisibleCol(a_hWnd);

	long l_iAt;
	for (l_iAt = l_iStartCol; l_iAt <= l_iLastCol; l_iAt++)
	{
		T_COLPOS l_colPos = GetColPos(a_hWnd, l_iAt);
		// bingo! - it is on border
		if ((a_pt.x >= l_colPos.xRight - 1) && (a_pt.x <= l_colPos.xRight + 1))
		{
			return TRUE;
		}
	}

	// it is not on border
	return FALSE;
}


//	---------------------------------------------------------------------------------------
//	returns TRUE if point is on any column or row grid in control
//
BOOL 
MouseOnGrid(
	HWND a_hWnd, 
	const POINT& a_pt
	)
{
	return MouseOnGridRow(a_hWnd, a_pt) || MouseOnGridCol(a_hWnd, a_pt);
}


//	---------------------------------------------------------------------------------------
//	returns TRUE if point is on border between columns headers
//
BOOL 
MouseOnHeaderGridCol(
	HWND a_hWnd,
	const POINT& a_pt
	)
{
	return (TrackedColFromPoint(a_hWnd, a_pt) >= 0);
}


//	---------------------------------------------------------------------------------------
//	returns TRUE if point is on border between rows
//
static BOOL 
MouseOnHeaderGridRow(
	HWND a_hWnd,
	const POINT& a_pt
	)
{
	return (TrackedRowFromPoint(a_hWnd, a_pt) >= 0);
}


void SetMouseCell(HWND a_hWnd, const POINT& a_pt, long a_iRow, long a_iCol)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);

	RCell l_cellOld = l_pData->GetMouseCell();
	RCell l_cellNew(a_iRow, a_iCol);

	if (l_cellOld.Equal(l_cellNew))
	{
		return;
	}

	RLCNMMOUSE l_nmMouse;

	if (l_cellOld.IsNull())
	{
		l_pData->SetMouseCell(a_iRow, a_iCol);
		l_nmMouse.nmhdr.code = RLCN_MOUSECELLENTER;
		l_nmMouse.cell.iRow = l_cellNew.GetRow();
		l_nmMouse.cell.iCol = l_cellNew.GetCol();
		l_nmMouse.pt = a_pt;
		
		HCURSOR l_hCur = ::LoadCursor(NULL, IDC_ARROW);
		::SetCursor(l_hCur);

		Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmMouse));
	}
	else if (l_cellNew.IsNull())
	{
		l_nmMouse.nmhdr.code = RLCN_MOUSECELLLEAVE;
		l_nmMouse.cell.iRow = l_cellOld.GetRow();
		l_nmMouse.cell.iCol = l_cellOld.GetCol();
		l_nmMouse.pt = a_pt;
		HCURSOR l_hCur = ::LoadCursor(NULL, IDC_ARROW);
		::SetCursor(l_hCur);

		Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmMouse));
		l_pData->SetMouseCell(a_iRow, a_iCol);
	}
	else
	{
		l_nmMouse.nmhdr.code = RLCN_MOUSECELLLEAVE;
		l_nmMouse.cell.iRow = l_cellOld.GetRow();
		l_nmMouse.cell.iCol = l_cellOld.GetCol();
		l_nmMouse.pt = a_pt;
		Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmMouse));

		l_pData->SetMouseCell(a_iRow, a_iCol);
	
		l_nmMouse.nmhdr.code = RLCN_MOUSECELLENTER;
		l_nmMouse.cell.iRow = l_cellNew.GetRow();
		l_nmMouse.cell.iCol = l_cellNew.GetCol();
		l_nmMouse.pt = a_pt;
		HCURSOR l_hCur = ::LoadCursor(NULL, IDC_ARROW);
		::SetCursor(l_hCur);
		Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmMouse));
	}
}


//	---------------------------------------------------------------------------------------
//	sets tracked column width
//
static void
SetTrackedColumnWidth(
	HWND a_hWnd, 
	const POINT& a_pt
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	long l_iCol = l_pData->GetTrackedColRow();
	T_COLPOS l_pos = GetColPos(a_hWnd, l_iCol);
	// new point have to be on right side of left corner of column
	ASSERT(l_pos.xLeft < a_pt.x);

	l_pData->SetColumnWidth(l_iCol, a_pt.x - l_pos.xLeft);
}


//	---------------------------------------------------------------------------------------
//	sets tracked row height
//
static void
SetTrackedRowHeight(
	HWND a_hWnd, 
	const POINT& a_pt
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	long l_iRow = l_pData->GetTrackedColRow();
	T_ROWPOS l_pos = GetRowPos(a_hWnd, l_iRow);
	// new point have to be on right side of left corner of column
	ASSERT(l_pos.yTop < a_pt.y);

	if (l_pData->TestFlags(LMB_SAMEROWSHEIGHT))
	{
		l_pData->SetAllRowsHeight(a_pt.y - l_pos.yTop);
	}
	else
	{
		l_pData->SetRowHeight(l_iRow, a_pt.y - l_pos.yTop);
	}
}


//	---------------------------------------------------------------------------------------
//	Invalidates tracked column, checks also if VScroll is needed
//
static void 
InvalidateTrackedCol(
	HWND a_hWnd, 
	long a_iStartTracked
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);

	BOOL l_bRowOnScreen = IsRowOnScreen(a_hWnd, l_pData->GetSelRow());
	long l_iFromEnd = 0;

	// jest na ekranie, ale który od do³u
	if (l_bRowOnScreen)
	{
		l_iFromEnd = CalculateRows2Draw(a_hWnd) - 
			(l_pData->GetSelRow() - l_pData->GetFirstVisRow());
	}
	
	if (l_bRowOnScreen && (!IsRowOnScreen(a_hWnd, l_pData->GetSelRow())))
	{
		l_pData->SetFirstVisRow(l_pData->GetSelRow() - CalculateRows2Draw(a_hWnd) 
			+ l_iFromEnd);
		InvalidateDrawRectV(a_hWnd);
	}

	// przerysowanie obszaru zmienionego (nag³ówek i grid na prawo 
	// od punktu startu trackowania)
	RECT l_rectClient;
	GetClientRect(a_hWnd, &l_rectClient);
	RECT l_rectHeader = {a_iStartTracked, 0, 
		l_rectClient.right - l_rectClient.left, l_rectClient.bottom - l_rectClient.top};
	InvalidateRect(a_hWnd, &l_rectHeader, TRUE);

}


//	---------------------------------------------------------------------------------------
//	Invalidates tracked row, checks also if HScroll is needed
//
void 
InvalidateTrackedRow(
	HWND a_hWnd, 
	long a_iStartTracked
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
#pragma todo("This code should probably be changed to be same as in InvalidateTrackedCol")
	RECT l_rectClient;
	GetClientRect(a_hWnd, &l_rectClient);
	RECT l_rectLeftCol = {0, a_iStartTracked, 
		l_rectClient.right - l_rectClient.left, l_rectClient.bottom - l_rectClient.top};

	// if all rows have the same height rect must invalidated starting just after fixed rows.
	if (l_pData->TestFlags(LMB_SAMEROWSHEIGHT))
	{
		T_ROWPOS l_rowPos = GetRowPos(a_hWnd, l_pData->GetFixedRows());
		l_rectLeftCol.top = l_rowPos.yTop;
	}
	InvalidateRect(a_hWnd, &l_rectLeftCol, TRUE);

}


//	---------------------------------------------------------------------------------------
//	Returns TRUE if row is on screen
//
static BOOL 
IsRowOnScreen(
	HWND a_hWnd,
	long a_iRow
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (a_iRow < l_pData->GetFirstVisRow())
	{
		return FALSE ;
	}
	if (a_iRow >= l_pData->GetFirstVisRow() + CalculateRows2Draw(a_hWnd))
	{
		return FALSE ;
	}
	return TRUE ;
}


//	---------------------------------------------------------------------------------------
//	Returns row from point. If point is not in range returns -1. 
//  Checks only y position - dos not care if point is on left column or on empty space
//  Returns 0 if point is on header
//
static long 
GetRowFromPoint(
	HWND a_hWnd, 
	const POINT& a_pt
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);

	long l_iStartRow = l_pData->GetFirstVisRow();
	long l_iLastRow = GetLastVisibleRow(a_hWnd);
	long l_iRow;

	// fixed rows
	for (l_iRow = 0; l_iRow < l_pData->GetFixedRows(); l_iRow++)
	{
		T_ROWPOS l_pos = GetRowPos(a_hWnd, l_iRow);
		if ((a_pt.y >= l_pos.yTop) && (a_pt.y < l_pos.yBottom))
		{
			return l_iRow;
		}

	}

	// not fixed rows
	for (l_iRow = l_iStartRow; l_iRow <= l_iLastRow; l_iRow++)
	{
		T_ROWPOS l_pos = GetRowPos(a_hWnd, l_iRow);
		if ((a_pt.y >= l_pos.yTop) && (a_pt.y < l_pos.yBottom))
		{
			return l_iRow;
		}
	}
	return -1;
}


//	---------------------------------------------------------------------------------------
//	Returns col from point. If point is not in range returns -1. 
//  Returns col also if point is on fixed column.
//
static long 
GetColFromPoint(
	HWND a_hWnd, 
	const POINT& a_pt
	)
{

	const RListData* l_pData = GetRListCtrlData(a_hWnd);

	long l_iStartCol = l_pData->GetFirstVisCol();
	long l_iLastCol = GetLastVisibleCol(a_hWnd);
	long l_iCol;

	// fixed columns
	for (l_iCol = 0; l_iCol < l_pData->GetFixedCols(); l_iCol++)
	{
		T_COLPOS l_pos = GetColPos(a_hWnd, l_iCol);
		if ((a_pt.x >= l_pos.xLeft) && (a_pt.x < l_pos.xRight))
		{
			return l_iCol;
		}

	}

	// not fixed columns
	for (l_iCol = l_iStartCol; l_iCol <= l_iLastCol; l_iCol++)
	{
		T_COLPOS l_pos = GetColPos(a_hWnd, l_iCol);
		if ((a_pt.x >= l_pos.xLeft) && (a_pt.x < l_pos.xRight))
		{
			return l_iCol;
		}
	}
	return -1;
}


//	---------------------------------------------------------------------------------------
//	Returns cell (row and col numbers) from point
//
RLCELLPOS
GetCellFromPoint(
	HWND a_hWnd, 
	const POINT& a_pt,
	bool  a_bRect
	)
{
	RLCELLPOS l_cell;

	l_cell.cell.iRow = GetRowFromPoint(a_hWnd, a_pt);
	l_cell.cell.iCol = GetColFromPoint(a_hWnd, a_pt);
	if (a_bRect && (l_cell.cell.iRow >= 0) && (l_cell.cell.iCol >= 0))
	{
		l_cell.rect = GetCellRect(a_hWnd, l_cell.cell.iRow, l_cell.cell.iCol);
	}
	return l_cell;
}


//	---------------------------------------------------------------------------------------
//	Returns tracked row from point
//
static long
TrackedRowFromPoint(
	HWND a_hWnd, 
	const POINT& a_pt
	)
{
	// if point below header - lets get out of function
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (a_pt.x > l_pData->GetFixedColsWidth()) 
	{
		return -1;
	}


	long l_iStartRow = l_pData->GetFirstVisRow();
	long l_iRows2Draw = CalculateRows2Draw(a_hWnd);

	long l_iRow;
	for (l_iRow = l_iStartRow; l_iRow < l_iStartRow + l_iRows2Draw; l_iRow++)
	{
		T_ROWPOS l_rowPos = GetRowPos(a_hWnd, l_iRow);
		// bingo! - it is on border
		if ((a_pt.y >= l_rowPos.yBottom - 1) && (a_pt.y <= l_rowPos.yBottom + 1))
		{
			return l_iRow;
		}
	}

	// it is not on border
	return -1;
}


//	---------------------------------------------------------------------------------------
//	Invalidates tracked region
//
static void 
InvalidateTrackCol(
	HWND a_hWnd,
	long a_xStart, 
	long a_xEnd
	)
{
	RECT l_rect2Inv ;	// obszar do przerysowania
	const RListData* l_pData = GetRListCtrlData(a_hWnd);

	l_rect2Inv.top = 1 ;
	l_rect2Inv.bottom = GetLastRowPos(a_hWnd).yBottom;

	if (a_xStart < a_xEnd)
	{
		l_rect2Inv.left = a_xStart;
		l_rect2Inv.right = a_xEnd + l_pData->GetTrackLineWidth();
		if (l_pData->GetTrackLineInverted())
		{
			l_rect2Inv.right += l_pData->GetTrackLineWidth();
		}
		else
		{
			l_rect2Inv.left -= l_pData->GetTrackLineWidth();
		}
	}
	else
	{
		l_rect2Inv.left = a_xEnd;
		l_rect2Inv.right = a_xStart + l_pData->GetTrackLineWidth();
	}
	RECT l_rectClient;
	::GetClientRect(a_hWnd, &l_rectClient);
	if (l_rect2Inv.bottom > l_rectClient.bottom)
	{
		l_rect2Inv.bottom = l_rectClient.bottom;
	}
	::InvalidateRect(a_hWnd, &l_rect2Inv, TRUE) ;
}


//	---------------------------------------------------------------------------------------
//	Invalidates tracked region for row
//
static void 
InvalidateTrackRow(
	HWND a_hWnd,
	long a_yStart, 
	long a_yEnd
	)
{
	RECT l_rect2Inv ;	// obszar do przerysowania
	const RListData* l_pData = GetRListCtrlData(a_hWnd);

	l_rect2Inv.left = 1 ;
	l_rect2Inv.right = GetLastColPos(a_hWnd).xRight;

	if (a_yStart < a_yEnd)
	{
		l_rect2Inv.top = a_yStart;
		l_rect2Inv.bottom = a_yEnd + l_pData->GetTrackLineWidth();
		if (l_pData->GetTrackLineInverted())
		{
			l_rect2Inv.bottom += l_pData->GetTrackLineWidth();
		}
		else
		{
			l_rect2Inv.top -= l_pData->GetTrackLineWidth();
		}
	}
	else
	{
		l_rect2Inv.top = a_yEnd;
		l_rect2Inv.bottom = a_yStart + l_pData->GetTrackLineWidth();
	}
	RECT l_rectClient;
	::GetClientRect(a_hWnd, &l_rectClient);
	if (l_rect2Inv.bottom > l_rectClient.bottom)
	{
		l_rect2Inv.bottom = l_rectClient.bottom;
	}
	::InvalidateRect(a_hWnd, &l_rect2Inv, TRUE) ;
}


//	---------------------------------------------------------------------------------------
//	Message handler of WM_CAPTURECHANGED
//
void OnCaptureChanged(HWND a_hWnd, HWND /*a_hWndNew*/)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (l_pData->IsTrackingCol() || l_pData->IsTrackingRow())
	{
		FinishTracking(a_hWnd);
		return;
	}
	else if (l_pData->IsTrackingMouse())
	{
		l_pData->SetTrackingMouse(false);
#ifndef RLCF_NOTOOLTIP
		RCellTip_Pop(l_pData->GetCellTip());
#endif

	}

}


//	---------------------------------------------------------------------------------------
//	What messages we want to intercept
//
LRESULT OnGetDlgCode(HWND /*a_hWnd*/, LPMSG a_pMsg)
{
	// WM_KEYUP must also be captured to not cause beep after pressing TAB
	if	(
		(a_pMsg != NULL) && 
		(a_pMsg->message == WM_KEYDOWN || a_pMsg->message == WM_KEYUP)
		)
	{
		if	(
			((int)a_pMsg->wParam == VK_DOWN)	|| 
			((int)a_pMsg->wParam == VK_UP)		||
			((int)a_pMsg->wParam == VK_LEFT)	|| 
			((int)a_pMsg->wParam == VK_RIGHT)	
			)
		{
			return DLGC_WANTMESSAGE;
		}
	}

	return 0;

}


//	---------------------------------------------------------------------------------------
//	Finishes tracking column
//
static void	
FinishTracking(
	HWND a_hWnd
	)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	ASSERT((l_pData->IsTrackingCol() >= 0) || (l_pData->IsTrackingRow() >= 0));
	
	long l_iColRow = l_pData->GetTrackedColRow();
	ASSERT(l_iColRow >= 0);

	::ClipCursor(NULL);
	l_pData->SetRectCalculated(FALSE);

	if (l_pData->IsTrackingCol())
	{
		T_COLPOS l_pos = GetColPos(a_hWnd, l_iColRow);
		InvalidateTrackedCol(a_hWnd, l_pos.xLeft);
	}
	else
	{
		T_ROWPOS l_pos = GetRowPos(a_hWnd, l_iColRow);
		InvalidateTrackedRow(a_hWnd, l_pos.yTop);
	}
	l_pData->IsTrackingCol() ? l_pData->SetTrackingCol(FALSE) : l_pData->SetTrackingRow(FALSE);
}


//	---------------------------------------------------------------------------------------
//	Clips cursor on col
//
static void	
ClipCursorCol(
	HWND a_hWnd, 
	long a_iCol
	)
{
	RECT l_rectClip;
	RECT l_rectWnd;
	::GetWindowRect(a_hWnd, &l_rectWnd);

	const RListData* l_pData = GetRListCtrlData(a_hWnd);

	T_COLPOS l_colPos = GetColPos(a_hWnd, a_iCol);

	POINT l_ptLeftUpper = {l_colPos.xLeft + 1, 0};
	POINT l_ptRightLower = {0, l_pData->GetFixedRowsHeight()};
	::ClientToScreen(a_hWnd, &l_ptLeftUpper);
	::ClientToScreen(a_hWnd, &l_ptRightLower);

	l_rectClip.top = l_ptLeftUpper.y;
	l_rectClip.bottom = l_ptRightLower.y;
	l_rectClip.left = l_ptLeftUpper.x;
	l_rectClip.right = l_rectWnd.right;
	if (HasVertScrollBar(a_hWnd))
	{
		l_rectClip.right -= c_dxVScroll;
	}
	::ClipCursor(&l_rectClip);
}


//	---------------------------------------------------------------------------------------
//	Clips cursor on row
//
static void	
ClipCursorRow(
	HWND a_hWnd, 
	long a_iRow
	)
{
	RECT l_rectClip;
	RECT l_rectWnd;
	::GetWindowRect(a_hWnd, &l_rectWnd);

	const RListData* l_pData = GetRListCtrlData(a_hWnd);

	T_ROWPOS l_rowPos = GetRowPos(a_hWnd, a_iRow);

	POINT l_ptLeftUpper = {0, l_rowPos.yTop + 1};
	POINT l_ptRightLower = {l_pData->GetFixedColsWidth(), 0};
	::ClientToScreen(a_hWnd, &l_ptLeftUpper);
	::ClientToScreen(a_hWnd, &l_ptRightLower);

	l_rectClip.top = l_ptLeftUpper.y;
	l_rectClip.bottom = l_rectWnd.bottom;
	l_rectClip.left = l_ptLeftUpper.x;
	l_rectClip.right = l_ptRightLower.x;
	if (HasHorzScrollBar(a_hWnd))
	{
		l_rectClip.bottom -= c_dyHScroll;
	}
	::ClipCursor(&l_rectClip);
}


//	---------------------------------------------------------------------------------------
//	Returns col nr from point
//
static long
TrackedColFromPoint(
	HWND a_hWnd, 
	const POINT& a_pt
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	// if point below header - lets get out of function
	if (a_pt.y > l_pData->GetFixedRowsHeight()) 
	{
		return -1;
	}

	long l_iStartCol = l_pData->GetFirstVisCol();
	long l_iLastCol = GetLastVisibleCol(a_hWnd);

	long l_iAt;
	for (l_iAt = l_iStartCol; l_iAt <= l_iLastCol; l_iAt++)
	{
		T_COLPOS l_colPos = GetColPos(a_hWnd, l_iAt);
		// bingo! - it is on border
		if ((a_pt.x >= l_colPos.xRight - 1) && (a_pt.x <= l_colPos.xRight + 1))
		{
			return l_iAt;
		}
	}

	// it is not on border
	return -1;
}


//	---------------------------------------------------------------------------------------
//	Calculates which column must be shown to see last column
//
static long 
GetHScrollLastCol(
	HWND a_hWnd, 
	long a_dx
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	long l_dxReal = a_dx - l_pData->GetFixedColsWidth();
	long l_iLastCol = l_pData->GetFixedCols();

	long l_iCol;
	long l_dxWidth = 0;
	for (l_iCol = l_pData->GetColumnsCount() - 1; l_iCol >= l_iLastCol; l_iCol--)
	{
		l_dxWidth += l_pData->GetColumn(l_iCol).GetWidth();
		if (l_dxReal < l_dxWidth)
		{
			return (l_iCol == l_pData->GetColumnsCount() - 1) ? l_iCol : l_iCol + 1;
		}
	}
	return l_pData->GetFixedCols();
}


//	---------------------------------------------------------------------------------------
//	Calculates row which must be first on list to see full last row
//
static long
GetRowToSeeLast(
	HWND a_hWnd, 
	long a_dy
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	long l_iRow;
	long l_iFixedRows = l_pData->GetFixedRows();
	long l_dyHeight = l_pData->GetFixedRowsHeight();
	long l_iLastRow = l_pData->GetRowsCount() - 1;
	for (l_iRow = l_iLastRow; l_iRow >= l_iFixedRows; l_iRow--)
	{
		l_dyHeight += l_pData->GetRowHeight(l_iRow);
		if (a_dy < l_dyHeight)
		{
			return (l_iRow == l_iLastRow) ? l_iRow : l_iRow + 1;
		}
	}
	return l_iFixedRows;
}


//	---------------------------------------------------------------------------------------
//	Calculates col which must be first on list to see full last column
//
static long
GetColToSeeLast(
	HWND a_hWnd, 
	long a_dx
	)
{
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	long l_iCol;
	long l_iFixedCols = l_pData->GetFixedCols();
	long l_dxWidth = l_pData->GetFixedColsWidth();
	long l_iLastCol = l_pData->GetColumnsCount() - 1;
	for (l_iCol = l_iLastCol; l_iCol >= l_iFixedCols; l_iCol--)
	{
		l_dxWidth += l_pData->GetColumn(l_iCol).GetWidth();
		if (a_dx < l_dxWidth)
		{
			return (l_iCol == l_iLastCol) ? l_iCol : l_iCol + 1;
		}
	}
	return l_iFixedCols;
}


//	---------------------------------------------------------------------------------------
//	Calculates client size (also with scrollbars)
//
static SIZE 
GetClientSize(
	HWND a_hWnd
	)
{
	RECT l_rectWin;
	::GetClientRect(a_hWnd, &l_rectWin);

	// if there is scroll bar in window - real window rect must be increased by
	// scrollbar width or height
	if (HasVertScrollBar(a_hWnd))
	{
		l_rectWin.right += c_dxVScroll;
	}

	if (HasHorzScrollBar(a_hWnd))
	{
		l_rectWin.bottom += c_dyHScroll;
	}

	SIZE l_size = { l_rectWin.right - l_rectWin.left, l_rectWin.bottom - l_rectWin.top };
	return l_size;
}


void CellTip_Pop(HWND a_hWnd)
{
#ifndef RLCF_NOTOOLTIP
	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	RCellTip_Pop(l_pData->GetCellTip());
#endif
}


#ifndef RLCF_NOTOOLTIP
//	---------------------------------------------------------------------------------------
//	shows CellTip if has to.
//
static void
CellTip(
	HWND a_hWnd, 
	const POINT& a_pt
	)
{
	RLCELLPOS l_cell;
	SIZE l_size;
	if (!HasToShowCellTip(a_hWnd, a_pt, &l_cell, &l_size))
	{
		return;
	}

	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	LPCTSTR l_psText;
	l_pData->GetData(l_cell.cell.iRow, l_cell.cell.iCol, &l_psText);

	// must change to screen coordinates
	POINT l_pt = {l_cell.rect.left, l_cell.rect.top};
	::ClientToScreen(a_hWnd, &l_pt);

	RCellTip_SetSize(l_pData->GetCellTip(), &l_size);
	::SetWindowText(l_pData->GetCellTip(), l_psText);
	::SendMessage(l_pData->GetCellTip(), WM_SETFONT, reinterpret_cast<WPARAM>(l_pData->GetFont()), FALSE);
	RCellTip_PopUp(l_pData->GetCellTip(), &l_pt);
}


//	---------------------------------------------------------------------------------------
//	Checks if have to show celltip
//
BOOL HasToShowCellTip(
	HWND a_hWnd, 
	const POINT& a_pt,
	LPRLCELLPOS a_pCell,
	LPSIZE a_pSize
	)
{
	a_pSize->cx = -1;
	a_pSize->cy = -1;
	const RListData* l_pData = GetRListCtrlData(a_hWnd);

	// no tooltips, no problem
	if (l_pData->TestFlags(LMB_NOTOOLTIPS))
	{
		return FALSE;
	}

	RLCELLPOS l_cell = GetCellFromPoint(a_hWnd, a_pt, true);
	if ((l_cell.cell.iCol < 0) || (l_cell.cell.iRow < 0))
	{
		return FALSE;
	}

	// drawing rect must be deflated by text margin
	::InflateRect(&(l_cell.rect), -l_pData->GetTextMargin(), 0);

	// text for cell
	LPCTSTR l_psText;
	l_pData->GetData(l_cell.cell.iRow, l_cell.cell.iCol, &l_psText);

#pragma todo("not able to properly calculate needed size for drawing rect, because don't know used font")
	HDC l_hDC = ::GetDC(a_hWnd);
	HFONT l_hFontOld = (HFONT)::SelectObject(l_hDC, l_pData->GetFont());

	RECT l_rectText = {0, 0, 0, 0};
	int l_iHeight = ::DrawText(l_hDC, l_psText, -1, &l_rectText, 
		DT_CALCRECT | DT_EXTERNALLEADING | DT_NOPREFIX | DT_SINGLELINE);
	ASSERT(l_iHeight > 0);
	UNUSED(l_iHeight);
	::SelectObject(l_hDC, l_hFontOld);
	::ReleaseDC(a_hWnd, l_hDC);

	if ((RectWidth(l_rectText) < RectWidth(l_cell.rect)) && (RectHeight(l_rectText) < RectHeight(l_cell.rect)))
	{
		return FALSE;
	}
	
	if (RectWidth(l_rectText) < RectWidth(l_cell.rect))
	{
		a_pSize->cx = RectWidth(l_cell.rect);
	}
	else if (RectHeight(l_rectText) < RectHeight(l_cell.rect))
	{
		a_pSize->cy = RectHeight(l_cell.rect);
	}

	*a_pCell = l_cell;
	return TRUE;
}


#endif


void OnLButtonDblClk(HWND a_hWnd, UINT /*a_iKeys*/, const POINT& a_pt)
{
	RLCNMMOUSE l_nmMouse;
	l_nmMouse.nmhdr.code = RLCN_LDBLCLICK;

	RLCELLPOS l_cellpos = GetCellFromPoint(a_hWnd, a_pt, true);
	l_nmMouse.cell = l_cellpos.cell;
	l_nmMouse.pt = a_pt;

	if (Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmMouse)))
	{
		return;
	}

	const RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (l_pData->TestFlags(LMB_EDITABLE))
	{
		if ((l_cellpos.cell.iRow >= 0) && (l_cellpos.cell.iCol >= 0))
		{
			HWND l_hWndEdit;
			RECT l_rectEdit;
			if (CanEditCell(a_hWnd, l_cellpos.cell, &l_hWndEdit, &l_rectEdit))
			{
				EditCell(a_hWnd, l_cellpos.cell, l_hWndEdit, &l_rectEdit);
				return;
			}
		}
	}
	// not editable - maybe selected?
	if (l_cellpos.cell.iRow >= l_pData->GetFixedRows())
	{
		RLCNMSELECTED l_nmSelected;
		l_nmSelected.nmhdr.code = RLCN_SELECTED;
		l_nmSelected.iRow = l_cellpos.cell.iRow;
		Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmSelected));
	}
}


bool CanEditCell(HWND a_hWnd, const CELL& a_cell, HWND* a_phWndEdit, LPRECT a_pRect)
{
	RLCNMEDIT l_nmedit;
	*a_phWndEdit = NULL;

	ZeroMemory(&l_nmedit, sizeof(l_nmedit));
	l_nmedit.rectEdit = GetCellRect(a_hWnd, a_cell.iRow, a_cell.iCol);
	if (!NotifyStart(a_hWnd, a_cell, &l_nmedit))
	{
		return false;
	}

	if (l_nmedit.hwndEdit == NULL || !::IsWindow(l_nmedit.hwndEdit))
	{
		return false;
	}
	*a_phWndEdit = l_nmedit.hwndEdit;
	*a_pRect = l_nmedit.rectEdit;
	return true;
}


void EditCell(HWND a_hWnd, const CELL& a_cell, HWND a_hWndEdit, LPRECT a_pRectEdit)
{
	ASSERT(::IsWindow(a_hWndEdit));
#ifndef _WIN32_WCE

	SelectRow(a_hWnd, a_cell.iRow);
	::EnableScrollBar(a_hWnd, SB_BOTH, ESB_DISABLE_BOTH);
	// start editing 
	::SetWindowLong(a_hWndEdit, GWL_EXSTYLE, WS_EX_NOPARENTNOTIFY | ::GetWindowLong(a_hWndEdit, GWL_EXSTYLE));
	::SetWindowPos(a_hWndEdit, NULL, a_pRectEdit->left, a_pRectEdit->top, 
		RectWidth(*a_pRectEdit) - 1, RectHeight(*a_pRectEdit), SWP_NOZORDER | SWP_SHOWWINDOW);

	::SetFocus(a_hWndEdit);
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (l_pData->IsTrackingMouse())
	{
		::ReleaseCapture();
	}
	RLCELLPOS l_cellPos;
	l_cellPos.cell = a_cell;
	l_cellPos.rect = *a_pRectEdit;
	l_pData->SetCellPosEdit(l_cellPos);
	l_pData->SetWndEdit(a_hWndEdit);
	// for Esc, Tab and Enter Keys in Dialog
	s_hook = ::SetWindowsHookEx(WH_MSGFILTER, HookMessageProc, NULL, ::GetCurrentThreadId());
#endif

}


void EndEdit(HWND a_hWndEdit, WNDPROC a_wndprocDef, bool a_bCommit, bool a_bContinue, bool a_bReverse)
{
#ifdef _WIN32_WCE
	::SetWindowLong(a_hWndEdit, GWL_WNDPROC, reinterpret_cast<LONG>(a_wndprocDef));
#else
#pragma warning(disable: 4244)
	::SetWindowLongPtr(a_hWndEdit, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(a_wndprocDef));
#pragma warning(default: 4244)
#endif
	HWND l_hWndList = ::GetParent(a_hWndEdit);
	RListData* l_pData = GetRListCtrlData(l_hWndList);
	::SendMessage(l_hWndList, RLCM_EDITEND, a_bCommit, 0);
	if (a_bContinue)
	{
		// looking for next/prev cell to edit
		RRoundIter<long> l_iterCol(l_pData->GetColumnsCount(), l_pData->GetCellPosEdit().cell.iCol);
		long l_iRow = l_pData->GetSelRow();

// easier to implement it this way, so ignore warning "conditional expression is constatnt"
#pragma warning(disable:4127)
		while (true)
#pragma warning(default: 4127)
		{
			if (a_bReverse)
			{
				if (l_iRow == 0)
				{
					break;	// there is no more cell to edit
				}
				l_iterCol--;
				if (l_iterCol == (l_pData->GetColumnsCount() - 1))
				{
					l_iRow--;
				}
			}
			else
			{
				l_iterCol++;
				if (l_iterCol == 0)
				{
					l_iRow++;
				}
				if (l_iRow >= l_pData->GetRowsCount())
				{
					break;	// there is no more cell to edit
				}
			}
			CELL l_cell;
			l_cell.iRow = l_iRow;
			l_cell.iCol = l_iterCol;
			HWND l_hWndEdit;
			RECT l_rectEdit;
			if (CanEditCell(l_hWndList, l_cell, &l_hWndEdit, &l_rectEdit))
			{
				SelectRow(l_hWndList, l_iRow);
				EditCell(l_hWndList, l_cell, l_hWndEdit, &l_rectEdit);
				break;
			}
		}
	}
}


bool NotifyStart(HWND a_hWnd, const CELL& a_cell, LPRLCNMEDIT a_pnmEdit)
{
	a_pnmEdit->cell = a_cell;
	a_pnmEdit->nmhdr.code = RLCN_STARTEDIT;
	return (Notify(a_hWnd, reinterpret_cast<LPNMHDR>(a_pnmEdit)) != 0);
}


bool NotifyEnd(HWND a_hWnd, const CELL& a_cell, LPRLCNMEDIT a_pnmEdit)
{
	a_pnmEdit->cell = a_cell;
	a_pnmEdit->nmhdr.code = RLCN_ENDEDIT;
	return (Notify(a_hWnd, reinterpret_cast<LPNMHDR>(a_pnmEdit)) != 0);
}


LRESULT Notify(HWND a_hWnd, LPNMHDR a_pNmhdr)
{
	HWND l_hParent = ::GetParent(a_hWnd);
	if (l_hParent == NULL)
	{
		l_hParent = a_hWnd;
	}

	long l_idCtrl = ::GetDlgCtrlID(a_hWnd);
	a_pNmhdr->hwndFrom = a_hWnd;
	a_pNmhdr->idFrom = l_idCtrl;
	return ::SendMessage(l_hParent, WM_NOTIFY, l_idCtrl, reinterpret_cast<LPARAM>(a_pNmhdr));
}


LRESULT OnNotify(HWND a_hWnd, int /*a_idCtrl*/, LPNMHDR a_pNmHdr)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (a_pNmHdr->hwndFrom == l_pData->GetTooltipWnd())
	{
		OnNotifyTooltip(a_hWnd, a_pNmHdr);
	}

	return 0;
}


void OnNotifyTooltip(HWND a_hWnd, LPNMHDR a_pNmHdr)
{
#ifndef RLCF_NOTOOLTIP
	switch (a_pNmHdr->code)
	{
	case TTN_GETDISPINFO:
		{
			LPNMTTDISPINFO l_pNmInfo = reinterpret_cast<LPNMTTDISPINFO>(a_pNmHdr);
			::SendMessage(l_pNmInfo->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
			RListData* l_pData = GetRListCtrlData(a_hWnd);
			l_pNmInfo->lpszText = l_pData->GetTooltip();
		}
		break;
	}
#endif
}



void OnEditEnd(HWND a_hWnd, BOOL a_bCommit)
{
#ifndef _WIN32_WCE
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (!l_pData->IsEditCell())
	{
		return;
	}

	ASSERT(l_pData->TestFlags(LMB_EDITABLE));
	RLCNMEDIT l_nmedit;
	l_nmedit.bCommit = a_bCommit;
	l_nmedit.hwndEdit = l_pData->GetWndEdit();
	NotifyEnd(a_hWnd, l_pData->GetCellPosEdit().cell, &l_nmedit);
	l_pData->SetWndEdit(NULL);
	::EnableScrollBar(a_hWnd, SB_BOTH, ESB_ENABLE_BOTH);
	::UnhookWindowsHookEx(s_hook);
#endif
}


void OnSetTooltip(HWND a_hWnd, LPCTSTR a_sTooltip)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	l_pData->SetTooltip(a_sTooltip);
	SetTooltip(a_hWnd);
}


UINT OnGetTooltip(HWND a_hWnd, LPTSTR a_sTooltip, UINT a_iSize)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	ZeroMemory(a_sTooltip, a_iSize);
	UINT l_iSize = _tcslen(l_pData->GetTooltip());
	_tcsncpy_s(a_sTooltip, a_iSize, l_pData->GetTooltip(), l_iSize);
	return l_iSize + 1; // with terminating 0
}


void OnRefresh(HWND a_hWnd)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	l_pData->SetRectCalculated(FALSE);
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}


LRESULT OnGetFont(HWND a_hWnd)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	return reinterpret_cast<LRESULT>(l_pData->GetFont());
}


void OnSetFont(HWND a_hWnd, HFONT a_hFont)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	l_pData->SetFont(a_hFont);
}

LRESULT OnGetTextMargin(HWND a_hWnd)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	return static_cast<LRESULT>(l_pData->GetTextMargin());
}


void OnSetTextMargin(HWND a_hWnd, short a_nMargin)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	l_pData->SetTextMargin(a_nMargin);
}


void OnSetSelected(HWND a_hWnd, UINT a_iRow)
{
	SelectRow(a_hWnd, a_iRow);
}


LRESULT OnGetSelected(HWND a_hWnd)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	return l_pData->GetSelRow();
}


void OnInvalidateRow(HWND a_hWnd, long a_iRow)
{
	InvalidateRow(a_hWnd, a_iRow);
}


void OnInvalidateCol(HWND a_hWnd, long a_iCol)
{
	InvalidateCol(a_hWnd, a_iCol);
}


void OnInvalidateCell(HWND a_hWnd, long a_iRow, long a_iCol)
{
	InvalidateCell(a_hWnd, a_iRow, a_iCol);
}


void OnEditCell(HWND a_hWnd, long a_iRow, long a_iCol)
{
	CELL l_cell;
	l_cell.iRow = a_iRow;
	l_cell.iCol = a_iCol;

	HWND l_hWndEdit;
	RECT l_rectEdit;
	if (CanEditCell(a_hWnd, l_cell, &l_hWndEdit, &l_rectEdit))
	{
		EditCell(a_hWnd, l_cell, l_hWndEdit, &l_rectEdit);
	}
}

void OnGetHoverCell(HWND a_hWnd, LPCELL a_pCell)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	a_pCell->iRow = l_pData->GetMouseCell().GetRow();
	a_pCell->iCol = l_pData->GetMouseCell().GetCol();
}

#ifdef RLCF_NOTOOLTIP

void SetTooltip(HWND a_hWnd)
{
}

#else
LRESULT CALLBACK HookMessageProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code != MSGF_DIALOGBOX) 
	{
		return ::CallNextHookEx(s_hook, code, wParam, lParam);
	}
	
	LPMSG l_lpMsg = reinterpret_cast<LPMSG>(lParam);
	if (l_lpMsg->message == WM_KEYDOWN) 
	{
		if (l_lpMsg->wParam == VK_RETURN || l_lpMsg->wParam == VK_ESCAPE || l_lpMsg->wParam == VK_TAB)
		{
			::SendMessage(l_lpMsg->hwnd, l_lpMsg->message, l_lpMsg->wParam, l_lpMsg->lParam);
			return 1;
		}
	}
	return ::CallNextHookEx(s_hook, code, wParam, lParam);
}


void SetTooltip(HWND a_hWnd)
{
	RListData* l_pData = GetRListCtrlData(a_hWnd);
	if (l_pData->GetTooltip() == NULL)
	{
		return;
	}

	l_pData->SetTooltipWnd(::CreateWindowEx(WS_EX_TOPMOST,
		TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		a_hWnd, NULL, RCommon_GetInstance(), NULL));
	::SetWindowPos(l_pData->GetTooltipWnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);

	TOOLINFO l_ti;
	l_ti.cbSize = sizeof(TOOLINFO);
	l_ti.uFlags = TTF_SUBCLASS;
	l_ti.hwnd = a_hWnd;
	l_ti.hinst = RCommon_GetInstance();
	l_ti.uId = 1;
	l_ti.lpszText = LPSTR_TEXTCALLBACK;
	l_ti.rect.left = l_rect.left;    
	l_ti.rect.top = l_rect.top;
	l_ti.rect.right = l_rect.right;
	l_ti.rect.bottom = l_rect.bottom;
	::SendMessage(l_pData->GetTooltipWnd(), TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&l_ti));	

}
#endif
