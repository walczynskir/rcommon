// RSplitter.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "RSplitterData.h"
#include "RSplitterWnd.h"
#include "RMemDC.h"
#include "RTrackerWnd.h"
#include "celltip.h"
#include <windowsx.h>
#include <stdio.h>

#define WinSize(a_hWnd, a_rect) (IsHorizontal(a_hWnd) ? RectHeight(a_rect) : RectWidth(a_rect))

static const long c_iWindowOfs = sizeof(RSplitterData*) - sizeof(int);
static inline RSplitterData* GetRData(HWND a_hWnd);

static inline LRESULT OnCreate(HWND a_hWnd);
static inline void OnNcDestroy(HWND a_hWnd);
static inline void OnPaint(HWND a_hWnd);
static inline void OnShowWindow(HWND a_hWnd, bool a_bShow);
static inline void OnSize(HWND a_hWnd, long a_dxWidth, long a_dyHeight);
static inline void OnMouseMove(HWND a_hWnd, UINT a_iFlags, long a_x, long a_y);
static inline void OnLButtonDown(HWND a_hWnd, UINT a_iFlags, long a_x, long a_y);
static inline void OnLButtonUp(HWND a_hWnd, UINT a_iFlags, long a_x, long a_y);
static inline void OnRButtonDown(HWND a_hWnd, UINT a_iFlags, long a_x, long a_y);
static inline void OnCaptureChanged(HWND a_hWnd);
static inline void OnSetRedraw(HWND a_hWnd, bool a_bRedraw);

static inline void OnSetWnd(HWND a_hWnd, BYTE a_btIdx, HWND a_hInnerWnd);
static inline void OnGetWnd(HWND a_hWnd, BYTE a_btIdx, HWND* a_phWndInner);
static inline void OnSetSize(HWND a_hWnd, BYTE a_btIdx, long a_iSize);
static inline void OnSetPSize(HWND a_hWnd, BYTE a_btIdx, double a_fPerc);
static inline long OnGetSize(HWND a_hWnd, BYTE a_btIdx);
static inline void OnSplitEqual(HWND a_hWnd);
static inline void OnRefresh(HWND a_hWnd);
static inline LRESULT OnSetBitmap(HWND a_hWnd, LPRSSETBITMAP a_pSetBitmap);
static inline void OnSetThickness(HWND a_hWnd, BYTE a_btThickness);

static inline void Draw(HWND a_hWnd, HDC a_hDC);
static inline void DrawStd(HWND a_hWnd, HDC a_hDC);
static inline void DrawBarOnly(HWND a_hWnd, HDC a_hDC);

static void SetPSize(HWND a_hWnd, BYTE a_btIdx, double a_fPerc);
static void SetSize(HWND a_hWnd, BYTE a_btIdx, long a_iSize);

static long GetPaneSize(HWND a_hWnd, BYTE a_btIdx);
static long GetBarPos(HWND a_hWnd);
static void GetPercentDrag(HWND a_hWnd, LPTSTR a_sPercent, int a_iMax);
static void GetPercentDrag(HWND a_hWnd, double& a_fPerc);

static bool MouseOnSplitterBar(HWND a_hWnd, long a_x, long a_y);
static void Track(HWND a_hWnd);
static void ChangeLayout(HWND a_hWnd);

static void SetCursor(HWND a_hWnd);
static LRESULT RerouteMessage(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);

#define IsHorizontal(a_hWnd) (!(::GetWindowLong(a_hWnd, GWL_STYLE) & RSCS_VERTICAL))
#define IsReroute(a_hWnd) ((::GetWindowLong(a_hWnd, GWL_STYLE) & RSCS_REROUTEMSG))
#define IsPercentSize(a_hWnd) ((::GetWindowLong(a_hWnd, GWL_STYLE) & RSCS_PERCENTSIZE)) 
#define IsNoTip(a_hWnd) ((::GetWindowLong(a_hWnd, GWL_STYLE) & RSCS_NOTIP)) 
#define IsBlackTracker(a_hWnd) ((::GetWindowLong(a_hWnd, GWL_STYLE) & RSCS_BLACKTRACKER)) 
#define IsNoTrackerBorders(a_hWnd) ((::GetWindowLong(a_hWnd, GWL_STYLE) & RSCS_NOTRACKERBORDER)) 


ATOM RSplitterWnd_RegisterClass()
{
	WNDCLASSEX l_wcex;
	l_wcex.cbSize = sizeof(WNDCLASSEX); 
	l_wcex.style		 = CS_GLOBALCLASS;
	l_wcex.lpfnWndProc	 = (WNDPROC)RSplitterWnd_WndProc;
	l_wcex.cbClsExtra	 = 0;
	l_wcex.cbWndExtra	 = sizeof(RSplitterData*);
	l_wcex.hInstance	 = RCommon_GetInstance();
	l_wcex.hIcon		 = NULL;
	l_wcex.hCursor		 = NULL;
	l_wcex.hbrBackground = NULL;
	l_wcex.lpszMenuName	 = NULL;
	l_wcex.lpszClassName = RSplitterWnd_ClassName;
	l_wcex.hIconSm		 = NULL;
	return RegisterClassEx(&l_wcex);
}


LRESULT CALLBACK RSplitterWnd_WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam)
{

	switch (a_iMsg) 
	{
	case WM_CREATE:
		return OnCreate(a_hWnd);

	case WM_PAINT:
		OnPaint(a_hWnd);
		break;

	case WM_MOUSEMOVE:
		OnMouseMove(a_hWnd, (UINT)a_wParam, GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam));
		break;

	case WM_LBUTTONDOWN:
		OnLButtonDown(a_hWnd, (UINT)a_wParam, GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam));
		break;

	case WM_LBUTTONUP:
		OnLButtonUp(a_hWnd, (UINT)a_wParam, GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam));
		break;

	case WM_RBUTTONDOWN:
		OnRButtonDown(a_hWnd, (UINT)a_wParam, GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam));
		break;

	case WM_SHOWWINDOW:
		OnShowWindow(a_hWnd, (bool)(a_wParam != 0));
		break;

	case WM_SIZE:
		OnSize(a_hWnd, LOWORD(a_lParam), HIWORD(a_lParam));
		break;

	case WM_NCDESTROY:
		OnNcDestroy(a_hWnd);
		break;

	case WM_CAPTURECHANGED:
		OnCaptureChanged(a_hWnd);
		break;

	case WM_NOTIFY:
		return RerouteMessage(a_hWnd, WM_NOTIFY, a_wParam, a_lParam);

	case WM_COMMAND:
		return RerouteMessage(a_hWnd, WM_COMMAND, a_wParam, a_lParam);

	case WM_SETREDRAW:
		OnSetRedraw(a_hWnd, (a_wParam == TRUE));
		break;

	case RSCM_SETWND:
		OnSetWnd(a_hWnd, (BYTE)a_wParam, (HWND)a_lParam);
		break;

	case RSCM_GETWND:
		OnGetWnd(a_hWnd, (BYTE)a_wParam, (HWND*)a_lParam);
		break;

	case RSCM_SETSIZE:
		OnSetSize(a_hWnd, (BYTE)a_wParam, (long)a_lParam);
		break;

	case RSCM_SETPSIZE:
		OnSetPSize(a_hWnd, (BYTE)a_wParam, *(double*)a_lParam);
		break;

	case RSCM_GETSIZE:
		return OnGetSize(a_hWnd, (BYTE)a_wParam);

	case RSCM_SPLITEQUAL:
		OnSplitEqual(a_hWnd);
		break;

	case RSCM_REFRESH:
		OnRefresh(a_hWnd);
		break;

	case RSCM_SETBITMAP:
		return OnSetBitmap(a_hWnd, reinterpret_cast<LPRSSETBITMAP>(a_lParam));

	case RSCM_SETTHICKNESS:
		OnSetThickness(a_hWnd, static_cast<BYTE>(a_wParam));
		break;

	default:
		return ::DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
	}
	return 0;
}


RSplitterData* GetRData(HWND a_hWnd)
{
	ASSERT(a_hWnd != NULL);
#pragma warning(disable: 4312)
	return (RSplitterData*)::GetWindowLongPtr(a_hWnd, c_iWindowOfs);
#pragma warning(default: 4312)
}



LRESULT OnCreate(HWND a_hWnd)
{
	RSplitterData* l_pData = new RSplitterData();
	if (l_pData == NULL)
	{
		return -1;
	}
#pragma warning(disable: 4244)
	::SetWindowLongPtr(a_hWnd, c_iWindowOfs, (LONG_PTR)l_pData);
#pragma warning(default: 4244)
	return 0;
}


void OnNcDestroy(HWND a_hWnd)
{
	RSplitterData* l_pData = GetRData(a_hWnd);
	delete l_pData;
}


void OnPaint(HWND a_hWnd)
{
	if (!GetRData(a_hWnd)->m_bRedraw)
	{
		return;
	}
	PAINTSTRUCT l_ps;
	HDC l_hdc;
	l_hdc = ::BeginPaint(a_hWnd, &l_ps);

	Draw(a_hWnd, l_hdc);

	::EndPaint(a_hWnd, &l_ps);
}


void Draw(HWND a_hWnd, HDC a_hDC)
{
	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);
	RSplitterData* l_pData = GetRData(a_hWnd);

	long l_iSize = GetPaneSize(a_hWnd, 0);
	if (IsHorizontal(a_hWnd))
	{
		long l_dxWidth = RectWidth(l_rect);
		RECT l_rectBar = { 0, l_iSize, l_dxWidth, l_iSize +  + l_pData->m_btBarThickness};
		if (::RectVisible(a_hDC, &l_rectBar))
		{
			if (IsNoTrackerBorders(a_hWnd))
			{
				RECT l_rect = { 0, l_iSize, l_dxWidth, l_iSize + l_pData->m_btBarThickness};
				::FillRect(a_hDC, &l_rect, l_pData->m_hBrush);
			}
			else
			{
				RECT l_rect = { 0, l_iSize, l_dxWidth, l_iSize + 1};
				::FillRect(a_hDC, &l_rect, l_pData->m_hBrushBorder);
				l_rect.top = l_iSize + 1;
				l_rect.bottom = l_iSize + 1 + l_pData->m_btBarThickness;
				::FillRect(a_hDC, &l_rect, l_pData->m_hBrush);

				l_rect.top = l_iSize + l_pData->m_btBarThickness - 1;
				l_rect.bottom = l_rect.top + 1;
				::FillRect(a_hDC, &l_rect, l_pData->m_hBrushBorder);
			}
		}
	}
	else	// vertical
	{
		long l_dyHeight = RectHeight(l_rect);
		RECT l_rectBar = { l_iSize, 0, l_iSize + l_pData->m_btBarThickness, l_dyHeight };
		if (::RectVisible(a_hDC, &l_rectBar))
		{
			if (IsNoTrackerBorders(a_hWnd))
			{
				RECT l_rect = { l_iSize, 0, l_iSize + l_pData->m_btBarThickness, l_dyHeight };
				::FillRect(a_hDC, &l_rect, l_pData->m_hBrush);
			}
			else
			{
				RECT l_rect = { l_iSize, 0, l_iSize + 1, l_dyHeight };
				::FillRect(a_hDC, &l_rect, l_pData->m_hBrushBorder);

				l_rect.left = l_iSize + 1;
				l_rect.right = l_iSize + 1 + l_pData->m_btBarThickness;
				::FillRect(a_hDC, &l_rect, l_pData->m_hBrush);

				l_rect.left = l_iSize + l_pData->m_btBarThickness - 1;
				l_rect.right = l_rect.left + 1;
				::FillRect(a_hDC, &l_rect, l_pData->m_hBrushBorder);
			}
		}
	}
}


void OnMouseMove(HWND a_hWnd, UINT /*a_iFlags*/, long a_x, long a_y)
{
	RSplitterData* l_pData = GetRData(a_hWnd);
	if (l_pData->m_bDragging)
	{
		l_pData->m_iDrag = (IsHorizontal(a_hWnd) ? a_y : a_x);
		l_pData->m_iDrag = max(l_pData->m_btBias, l_pData->m_iDrag);
		RECT l_rect;
		::GetClientRect(a_hWnd, &l_rect);
		l_pData->m_iDrag = min(l_pData->m_iDrag, 
			(IsHorizontal(a_hWnd) ? RectHeight(l_rect) : RectWidth(l_rect)) - l_pData->m_btBarThickness + l_pData->m_btBias);

		Track(a_hWnd);
	}
	else if (MouseOnSplitterBar(a_hWnd, a_x, a_y))
	{
		HCURSOR l_hCur = ::LoadCursor(RCommon_GetInstance(), MAKEINTRESOURCE(IsHorizontal(a_hWnd) ? IDC_SPLITV : IDC_SPLITH));
		::SetCursor(l_hCur);
	}
}


void OnLButtonDown(HWND a_hWnd, UINT /*a_iFlags*/, long a_x, long a_y)
{
	if (!MouseOnSplitterBar(a_hWnd, a_x, a_y)) 
	{
		return;
	}

	SetCursor(a_hWnd);

	RSplitterData* l_pData = GetRData(a_hWnd);

	l_pData->m_bDragging = true;
	l_pData->m_iDrag = GetPaneSize(a_hWnd, 0);
	l_pData->m_btBias = static_cast<BYTE>((IsHorizontal(a_hWnd) ? a_y : a_x) - l_pData->m_iDrag);
	Track(a_hWnd);
}


void OnLButtonUp(HWND a_hWnd, UINT /*a_iFlags*/, long /*a_x*/, long /*a_y*/)
{
	RSplitterData* l_pData = GetRData(a_hWnd);
	if (l_pData->m_bDragging)
	{
		::ReleaseCapture();	// dragging state will be changed in oncapturechanged
		RECT l_rect;
		::GetClientRect(a_hWnd, &l_rect);
		long l_iPos = l_pData->m_iDrag - l_pData->m_btBias;

		if (IsPercentSize(a_hWnd))
		{
			double l_fDrag;
			GetPercentDrag(a_hWnd, l_fDrag);
			SetPSize(a_hWnd, 0, l_fDrag);
		}
		else
		{
			SetSize(a_hWnd, 0, l_iPos);
		}
		ChangeLayout(a_hWnd);
	}
}


void OnRButtonDown(HWND a_hWnd, UINT /*a_iFlags*/, long a_x, long a_y)
{
	POINT l_pt;
	l_pt.x = a_x;
	l_pt.y = a_y;
	::ClientToScreen(a_hWnd, &l_pt);
	HMENU l_hPopup = ::CreatePopupMenu();
	for (int l_idx = 1; l_idx < 10; l_idx++)
	{
		TCHAR l_sMenu[100];
		_sntprintf_s(l_sMenu, ArraySize(l_sMenu), ArraySize(l_sMenu), _T("%d/%d"), l_idx * 10, 100 - l_idx * 10);
		::AppendMenu(l_hPopup, MF_STRING | MF_BYPOSITION, l_idx, l_sMenu);
	}
	int l_iCmd = ::TrackPopupMenuEx(l_hPopup, TPM_LEFTALIGN | TPM_RETURNCMD, l_pt.x, l_pt.y, a_hWnd, NULL);
	if (l_iCmd == 0)
	{
		return;
	}
	SetPSize(a_hWnd, 0, l_iCmd * 10);

}


void OnShowWindow(HWND a_hWnd, bool a_bShow)
{
	if (a_bShow)
	{
		ChangeLayout(a_hWnd);
	}
}


void OnSize(HWND a_hWnd, long /*a_dxWidth*/, long /*a_dyHeight*/)
{
	ChangeLayout(a_hWnd);
}


void Track(HWND a_hWnd)
{
	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);
	RSplitterData* l_pData = GetRData(a_hWnd);
	POINT l_pt;
	SIZE l_size;

	if (IsHorizontal(a_hWnd))
	{
		l_pt.x = 0;
		l_pt.y = l_pData->m_iDrag - l_pData->m_btBias;
		l_size.cx = RectWidth(l_rect);
		l_size.cy = l_pData->m_btBarThickness;
	}
	else
	{
		l_pt.x = l_pData->m_iDrag - l_pData->m_btBias;
		l_pt.y = 0;
		l_size.cx = l_pData->m_btBarThickness;
		l_size.cy = RectHeight(l_rect);
	}
	::ClientToScreen(a_hWnd, &l_pt);
	if (l_pData->m_hTracker == NULL)
	{
		RTrackerWnd_RegisterClass();
		l_pData->CreateTrackerBrush(!IsBlackTracker(a_hWnd));
		l_pData->m_hTracker = RTrackerWnd_Create(l_pData->m_hBrushTracker);
	}
	::SetWindowPos(l_pData->m_hTracker, HWND_TOPMOST, 
		l_pt.x, l_pt.y, l_size.cx, l_size.cy, SWP_SHOWWINDOW | SWP_NOACTIVATE);
	if (!IsNoTip(a_hWnd))
	{
		if (l_pData->m_hTip == NULL)
		{
			RCellTip_Register();
			l_pData->m_hTip = RCellTip_Create(l_pData->m_hTracker, RCTS_CONSTSIZE);
			::SetWindowText(l_pData->m_hTip, _T("100.0%"));
			RCellTip_CalculateSize(l_pData->m_hTip);
		}
		TCHAR l_sPercent[10];
		GetPercentDrag(a_hWnd, l_sPercent, ArraySize(l_sPercent));
		::SetWindowText(l_pData->m_hTip, l_sPercent);
		::GetCursorPos(&l_pt);
		if (IsHorizontal(a_hWnd))
		{
			l_pt.y += l_pData->m_btBarThickness - l_pData->m_btBias;
		}
		else
		{
			l_pt.x += l_pData->m_btBarThickness - l_pData->m_btBias;
		}

		if (::IsWindowVisible(l_pData->m_hTip))
		{
			RCellTip_Move(l_pData->m_hTip, &l_pt);
			::RedrawWindow(l_pData->m_hTip, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		else
		{
			RCellTip_PopUp(l_pData->m_hTip, &l_pt);
		}
	}
}


void ChangeLayout(HWND a_hWnd)
{
	RSplitterData* l_pData = GetRData(a_hWnd);
	if ((l_pData->m_hWndTop == NULL) || (l_pData->m_hWndBottom == NULL))
	{
		return;
	}

	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);

	HDWP l_hdwp = ::BeginDeferWindowPos(2);
	long l_iSize;

	l_iSize = GetPaneSize(a_hWnd, 0);

	if (IsHorizontal(a_hWnd))
	{
		l_hdwp = ::DeferWindowPos(l_hdwp, l_pData->m_hWndTop, 
			NULL, 0, 0, RectWidth(l_rect), l_iSize, SWP_NOZORDER | SWP_SHOWWINDOW);
		l_hdwp = ::DeferWindowPos(l_hdwp, l_pData->m_hWndBottom,
			NULL, 0, l_iSize + l_pData->m_btBarThickness, RectWidth(l_rect), 
			RectHeight(l_rect) - l_iSize - l_pData->m_btBarThickness, SWP_NOZORDER | SWP_SHOWWINDOW);
	}
	else
	{
		l_hdwp = ::DeferWindowPos(l_hdwp, l_pData->m_hWndTop, 
			NULL, 0, 0, l_iSize, RectHeight(l_rect), SWP_NOZORDER | SWP_SHOWWINDOW);
		l_hdwp = ::DeferWindowPos(l_hdwp, l_pData->m_hWndBottom,
			NULL, l_iSize + l_pData->m_btBarThickness, 0, 
			RectWidth(l_rect) - l_iSize - l_pData->m_btBarThickness, RectHeight(l_rect), SWP_NOZORDER | SWP_SHOWWINDOW);

	}
	::EndDeferWindowPos(l_hdwp);
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE);
}


//	---------------------------------------------------------------------------------------
//	Creation of RSplitterWnd window
//
HWND	// Handle of created window or NULL if failed
RSplitterWnd_Create(
	DWORD		a_iStyle,		// style,
	LPCTSTR     a_sTitle,		// window title
	int			a_x,			// horizontal position of window
	int			a_y,			// vertical position of window
	int			a_iWidth,		// window width
	int			a_iHeight,		// window height
	HWND		a_hWndParent,   // handle to parent or owner window
	LPVOID		a_lpParam		// pointer to window-creation data
	)
{
	HWND l_hWnd = ::CreateWindow(RSplitterWnd_ClassName, a_sTitle, a_iStyle,
		a_x, a_y, a_iWidth, a_iHeight, a_hWndParent, NULL, RCommon_GetInstance(), a_lpParam);

	return l_hWnd;
}


void OnSetWnd(HWND a_hWnd, BYTE a_btIdx, HWND a_hInnerWnd)
{
	ASSERT(a_hWnd != NULL);
	RSplitterData* l_pData = GetRData(a_hWnd);
	ASSERT(l_pData != NULL);
	ASSERT((a_btIdx == 0) || (a_btIdx == 1));
	if (a_btIdx == 0)
	{
		l_pData->m_hWndTop = a_hInnerWnd;
	}
	else
	{
		l_pData->m_hWndBottom = a_hInnerWnd;
	}
	::SetParent(a_hInnerWnd, a_hWnd);
}


void OnGetWnd(HWND a_hWnd, BYTE a_btIdx, HWND* a_phWndInner)
{
	ASSERT(a_hWnd != NULL);
	ASSERT(a_phWndInner != NULL);
	RSplitterData* l_pData = GetRData(a_hWnd);
	ASSERT(l_pData != NULL);
	ASSERT((a_btIdx == 0) || (a_btIdx == 1));
	if (a_btIdx == 0)
	{
		*a_phWndInner = l_pData->m_hWndTop;
	}
	else
	{
		*a_phWndInner = l_pData->m_hWndBottom;
	}
}


bool MouseOnSplitterBar(HWND a_hWnd, long a_x, long a_y)
{
	RSplitterData* l_pData = GetRData(a_hWnd);
	long l_iPos = IsHorizontal(a_hWnd) ? a_y : a_x;
	return (l_iPos >= GetBarPos(a_hWnd)) && (l_iPos <= GetBarPos(a_hWnd) + l_pData->m_btBarThickness);
}


void OnSetSize(HWND a_hWnd, BYTE a_btIdx, long a_iSize)
{
	ASSERT(::IsWindow(a_hWnd));
	SetSize(a_hWnd,a_btIdx, a_iSize);
}


void 
SetSize(HWND a_hWnd, BYTE a_btIdx, long a_iSize)
{
	ASSERT(a_iSize >= 0);
	ASSERT((a_btIdx == 0) || (a_btIdx == 1));

	RSplitterData* l_pData = GetRData(a_hWnd);
	ASSERT(l_pData != NULL);

	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);
	long l_iWinSize = WinSize(a_hWnd, l_rect);
	if (IsPercentSize(a_hWnd))
	{
		if (a_btIdx == 0)
		{
			l_pData->SetPSize(double(a_iSize) / l_iWinSize * 100);
		}
		else
		{
			l_pData->SetPSize(double(100) - double(a_iSize) / l_iWinSize * 100);
		}
		ChangeLayout(a_hWnd);
	}
	else
	{
		long l_iOldSize = l_pData->GetSize().iSize;
		if (a_btIdx == 0)
		{
			if (l_iWinSize <= l_pData->m_btBarThickness)
			{
				l_pData->SetSize(0L);
			}
			else if (a_iSize > l_iWinSize - l_pData->m_btBarThickness)
			{
				l_pData->SetSize(l_iWinSize - l_pData->m_btBarThickness);
			}
			else
			{
				l_pData->SetSize(a_iSize);
			}
		}
		else
		{
			long l_iWinSize = WinSize(a_hWnd, l_rect);
			if (l_iWinSize <= l_pData->m_btBarThickness)
			{
				l_pData->SetSize(0L);
			}
			else if (a_iSize > l_iWinSize - l_pData->m_btBarThickness)
			{
				l_pData->SetSize(0L);
			}
			else
			{
				l_pData->SetSize(l_iWinSize - l_pData->m_btBarThickness - a_iSize);
			}
		}
		if (l_iOldSize != l_pData->GetSize().iSize)
		{
			ChangeLayout(a_hWnd);
		}
	}
}


void OnSetPSize(HWND a_hWnd, BYTE a_btIdx, double a_fPerc)
{
	ASSERT(a_hWnd != NULL);
	SetPSize(a_hWnd, a_btIdx, a_fPerc);
}


long OnGetSize(HWND a_hWnd, BYTE a_btIdx)
{
	ASSERT(a_hWnd != NULL);
	ASSERT((a_btIdx == 0) || (a_btIdx == 1));
	RSplitterData* l_pData = GetRData(a_hWnd);
	ASSERT(l_pData != NULL);


	if (a_btIdx == 0)
	{
		if (IsPercentSize(a_hWnd))
		{
			RECT l_rect;
			::GetClientRect(a_hWnd, &l_rect);
			long l_iWinSize = WinSize(a_hWnd, l_rect);
			return (long)((l_iWinSize / 100) * l_pData->GetSize().fPerc);
		}
		else
		{
			return l_pData->GetSize().iSize;
		}
	}
	else
	{
		RECT l_rect;
		::GetClientRect(a_hWnd, &l_rect);
		long l_iWinSize = WinSize(a_hWnd, l_rect);
		if (IsPercentSize(a_hWnd))
		{
			return (long)((l_iWinSize / 100) * (1 - l_pData->GetSize().fPerc));
		}
		else
		{
			return l_iWinSize - l_pData->GetSize().iSize - l_pData->m_btBarThickness;
		}
	}
}

void SetCursor(HWND a_hWnd)
{
	HCURSOR l_hCur = ::LoadCursor(RCommon_GetInstance(), MAKEINTRESOURCE(IsHorizontal(a_hWnd) ? IDC_SPLITV : IDC_SPLITH));
	::SetCursor(l_hCur);
	::SetCapture(a_hWnd);
}


void OnCaptureChanged(HWND a_hWnd)
{
	RSplitterData* l_pData = GetRData(a_hWnd);
	// to 'hide'tracker
	if (l_pData->m_bDragging)
	{
		::ShowWindow(l_pData->m_hTracker, SW_HIDE);
		if (l_pData->m_hTip != NULL)
		{
			RCellTip_Pop(l_pData->m_hTip);
		}
		l_pData->m_bDragging = false;
	}
}


void OnSetRedraw(HWND a_hWnd, bool a_bRedraw)
{
	ASSERT(::IsWindow(a_hWnd));
	RSplitterData* l_pData = GetRData(a_hWnd);
	l_pData->m_bRedraw = a_bRedraw;
	::SendMessage(l_pData->m_hWndTop, WM_SETREDRAW, a_bRedraw ? TRUE : FALSE, 0L);
	::SendMessage(l_pData->m_hWndBottom, WM_SETREDRAW, a_bRedraw ? TRUE : FALSE, 0L);
}


void OnSplitEqual(HWND a_hWnd)
{
	SetPSize(a_hWnd, 0, 50.0);
	ChangeLayout(a_hWnd);
}


LRESULT RerouteMessage(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam)
{
	if (IsReroute(a_hWnd))
	{
		HWND l_hWndParent = ::GetParent(a_hWnd);
		if (l_hWndParent != a_hWnd)
		{
			return ::SendMessage(l_hWndParent, a_iMsg, a_wParam, a_lParam);
		}
	}
	return 0;
}


void OnRefresh(HWND a_hWnd)
{
	ChangeLayout(a_hWnd);
}


LRESULT OnSetBitmap(HWND a_hWnd, LPRSSETBITMAP a_pSetBitmap)
{
	RSplitterData* l_pData = GetRData(a_hWnd);
	if (a_pSetBitmap == NULL)
	{
		l_pData->ReleaseBrush();
		l_pData->SetSimpleBrush();
		return TRUE;
	}

	HBITMAP l_hBmp;
	if (a_pSetBitmap->hInst == NULL)
	{
		l_hBmp = a_pSetBitmap->bmp.hBmp;
	}
	else
	{
		l_hBmp = reinterpret_cast<HBITMAP>(::LoadImage(a_pSetBitmap->hInst, MAKEINTRESOURCE(a_pSetBitmap->bmp.nID), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR));
	}

	if (l_hBmp == NULL)
	{
		return FALSE;
	}

	l_pData->ReleaseBrush();
	l_pData->m_hBrush = ::CreatePatternBrush(l_hBmp);

	if (a_pSetBitmap->hInst != NULL)
	{
		::DeleteObject(l_hBmp);
	}
	return TRUE;
}


void OnSetThickness(HWND a_hWnd, BYTE a_btThickness)
{
	RSplitterData* l_pData = GetRData(a_hWnd);
	l_pData->m_btBarThickness = a_btThickness;
}


long GetPaneSize(HWND a_hWnd, BYTE a_btIdx)
{
	ASSERT((a_btIdx == 0) || (a_btIdx == 1));
	RSplitterData* l_pData = GetRData(a_hWnd);
	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);

	if (IsPercentSize(a_hWnd))
	{
		// we must calculate
		long l_iWinSize = WinSize(a_hWnd, l_rect);
		l_iWinSize -= l_pData->m_btBarThickness;
		double l_fPerc = l_pData->GetSize().fPerc;
		if (a_btIdx == 0)
		{
			return (long)(double(l_iWinSize) / 100 * l_fPerc);
		}
		else
		{
			return (long)(double(l_iWinSize) / 100 * (100 - l_fPerc));
		}
		
	}
	else
	{
		if (a_btIdx == 0)
		{
			return l_pData->GetSize().iSize;
		}
		else
		{
			long l_iWinSize = WinSize(a_hWnd, l_rect);
			return max(l_iWinSize - l_pData->GetSize().iSize - l_pData->m_btBarThickness, 0);
		}
	}
}


long GetBarPos(HWND a_hWnd)
{
	return GetPaneSize(a_hWnd, 0);
}


void SetPSize(HWND a_hWnd, BYTE a_btIdx, double a_fPerc)
{
	ASSERT((a_fPerc >= 0) && (a_fPerc <= 100));
	ASSERT((a_btIdx == 0) || (a_btIdx == 1));

	RSplitterData* l_pData = GetRData(a_hWnd);
	ASSERT(l_pData != NULL);

	if (IsPercentSize(a_hWnd))
	{
		if (a_btIdx == 0)
		{
			l_pData->SetPSize(a_fPerc);
		}
		else
		{
			l_pData->SetPSize(100 - a_fPerc);
		}
	}
	else
	{
		RECT l_rect;
		::GetClientRect(a_hWnd, &l_rect);
		long l_iSize = long(double(WinSize(a_hWnd, l_rect) - l_pData->m_btBarThickness) / 100 * a_fPerc);
		if (a_btIdx == 0)
		{
			l_pData->SetSize(l_iSize);
		}
		else
		{
			l_pData->SetSize(WinSize(a_hWnd, l_rect) - l_iSize - l_pData->m_btBarThickness);
		}

	}
	ChangeLayout(a_hWnd);
}


void 
GetPercentDrag(HWND a_hWnd, double& a_fDrag)
{
	RSplitterData* l_pData = GetRData(a_hWnd);
	ASSERT(l_pData != NULL);
	a_fDrag = static_cast<double>(l_pData->m_iDrag - l_pData->m_btBias) / static_cast<double>(GetPaneSize(a_hWnd, 0) + GetPaneSize(a_hWnd, 1));
	a_fDrag *= 100;
	a_fDrag = min(a_fDrag, 100.0);
	a_fDrag = max(a_fDrag, 0.0);
}


void 
GetPercentDrag(HWND a_hWnd, LPTSTR a_sPercent, int a_iMax)
{
	double l_fSize;
	GetPercentDrag(a_hWnd, l_fSize);
	_sntprintf_s(a_sPercent, a_iMax, a_iMax, _T("%.1f%%"), l_fSize);
}