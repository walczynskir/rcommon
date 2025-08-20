// REmoDownWnd.cpp: implementation of the REmoDownWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "REmoWnd.h"
#include "REmoData.h"
#include "REmoDownWnd.h"
#include "DrawUtl.h"
#include "RKeyboard.h"
#include <commdlg.h>
#include "RTheme.h"


class REmoDownData
{
public:
	REmoDownData(const REmoData* a_pEmoData)
	{
		m_hWndOwner = NULL;
		m_selNew.iRow = 1; 
		m_selNew.iCol = 1; 
		m_pEmoData = a_pEmoData;
	}

	const REmoData* m_pEmoData;
	TSelect m_selNew;
	HWND m_hWndOwner;	
};

static const long c_iWindowOfs = sizeof(REmoDownData*) - 4;
static inline REmoDownData* GetREmoDownData(HWND a_hWnd);

// messages
static inline BOOL OnCreate(HWND a_hWnd, LPCREATESTRUCT a_lpStruct);
static inline void OnNcDestroy(HWND a_hWnd);
static inline void OnPaint(HWND a_hWnd);
static inline void OnGetMinMaxInfo(HWND a_hWnd, LPMINMAXINFO a_pMMI);
static inline void OnLButtonDown(HWND a_hWnd, LPPOINT a_pPoint);
static inline void OnLButtonUp(HWND a_hWnd, LPPOINT a_pPoint);
static inline void OnMouseMove(HWND a_hWnd, int a_xPos, int a_yPos);
static inline void OnShowWindow(HWND a_hWnd, BOOL a_bShow);
static inline void OnActivateApp(HWND a_hWnd, bool a_bActivate);

static inline int GetWidth(HWND a_hWnd);
static inline int GetHeight(HWND a_hWnd);

static inline void NotifyOwner(HWND a_hWnd, const TSelect& a_sel);
static void DropDown(HWND a_hWnd, BOOL a_bDown, LPCRECT a_pRect);

//drawing procedures
static inline void DrawBitmap(HWND a_hWnd, HDC a_hDC);
static inline void DrawGrid(HWND a_hWnd, HDC a_hDC);
static inline void DrawSel(HWND a_hWnd, HDC a_hDC);

static void SelectEmo(HWND a_hWnd, const TSelect& a_sel);
static void InvalidateEmo(HWND a_hWnd, const TSelect& a_sel);

static bool PointToEmo(HWND a_hWnd, const POINT& a_pt, TSelect* a_pSel);
static void EmoToRect(HWND a_hWnd, const TSelect& a_sel, LPRECT a_pRectEmo);

static inline void CalculateOpenPlace(HWND a_hWnd, LPCRECT a_pRect, LPPOINT a_pPt);


#define ChangeOpenPlace(a_hWnd) ((::GetWindowLong(a_hWnd, GWL_STYLE) & RECS_CHANGEOPENPLACE) != 0)


//	---------------------------------------------------------------------------
//	Rejestracja klasy
//
ATOM 
REmoDownWnd_RegisterClass()
{
	WNDCLASSEX l_wcex;
	// Initialize global strings

	l_wcex.cbSize			= sizeof(WNDCLASSEX); 
	l_wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;;
	l_wcex.lpfnWndProc		= REmoDownWnd_WndProc;
	l_wcex.cbClsExtra		= 0;
	l_wcex.cbWndExtra		= sizeof(REmoDownData*);
	l_wcex.hInstance		= RCommon_GetInstance();
	l_wcex.hIcon			= NULL;
	l_wcex.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
	l_wcex.hbrBackground	= NULL;
	l_wcex.lpszMenuName		= NULL;
	l_wcex.lpszClassName	= REmoDownWnd_ClassName;
	l_wcex.hIconSm			= NULL;

	return ::RegisterClassEx(&l_wcex);
}


//	---------------------------------------------------------------------------
//	Utworzenie okna
//
HWND
REmoDownWnd_Create(
	HWND a_hWndOwner,
	const REmoData* a_pEmoData
	)
{
	DWORD l_dwStyle = WS_THICKFRAME | WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | 0x08021;
	DWORD l_dwStyleEx = WS_EX_TOOLWINDOW;

	HWND l_hWnd = ::CreateWindowEx(l_dwStyleEx, REmoDownWnd_ClassName, _T(""), l_dwStyle, 
		CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, ::GetDesktopWindow(), 0, RCommon_GetInstance(), LPVOID(a_pEmoData));
	REmoDownData* l_pData = GetREmoDownData(l_hWnd);
	l_pData->m_hWndOwner = a_hWndOwner;
	return l_hWnd;
}


//	---------------------------------------------------------------------------
//	Wyœwietlenie okna
//
void
REmoDownWnd_DropDown(
	HWND a_hWnd, 
	LPCRECT a_pRect
	)
{
	DropDown(a_hWnd, TRUE, a_pRect);
}



//	---------------------------------------------------------------------------
//	Wyœwietlenie okna
//
void
REmoDownWnd_Hide(
	HWND a_hWnd
	)
{
	DropDown(a_hWnd, FALSE, NULL);
}


//	---------------------------------------------------------------------------------------
//	Main REmoWnd procedure
//
LRESULT CALLBACK 
REmoDownWnd_WndProc(
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

		case WM_KEYDOWN:
			TRACE0("EmoDownKeyDown\n");
			break;

		case WM_GETMINMAXINFO:
			OnGetMinMaxInfo(a_hWnd, (LPMINMAXINFO)a_lParam);
			break;

		case WM_PAINT:
			OnPaint(a_hWnd);
			break;

		case WM_LBUTTONDOWN:
			{
				POINT l_pt;
				POINTSTOPOINT(l_pt, MAKEPOINTS(a_lParam))
				OnLButtonDown(a_hWnd, &l_pt);
			}
			break;

		case WM_LBUTTONUP:
			{
				POINT l_pt;
				POINTSTOPOINT(l_pt, MAKEPOINTS(a_lParam))
				OnLButtonUp(a_hWnd, &l_pt);
			}
			break;

		case WM_MOUSEMOVE:
			OnMouseMove(a_hWnd, LOWORD(a_lParam), HIWORD(a_lParam));
			break;

		case WM_SHOWWINDOW:
			OnShowWindow(a_hWnd, (BOOL)a_wParam);
			break;

		case WM_ACTIVATEAPP:
			OnActivateApp(a_hWnd, a_wParam != 0);
			break;

		case WM_NCDESTROY:
			OnNcDestroy(a_hWnd);
			break;

		case WM_CHAR:
			return DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);

		default:
			return DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
   }
   return 0;
}


//	---------------------------------------------------------------------------------------
//	Returns control's data
//
static REmoDownData*	//OUT pointer to this data
GetREmoDownData(
	HWND a_hWnd	//IN 
	)
{
#pragma warning(disable: 4312)
	return reinterpret_cast<REmoDownData*>(::GetWindowLongPtr(a_hWnd, c_iWindowOfs));
#pragma warning(default: 4312)
}


//	---------------------------------------------------------------------------------------
//	Sets control's data
//
static void	
SetREmoDownData(
	HWND a_hWnd,	//IN
	REmoDownData* a_pData	//IN
	)
{
#pragma warning(disable: 4244)
	::SetWindowLongPtr(a_hWnd, c_iWindowOfs, reinterpret_cast<LONG_PTR>(a_pData));
#pragma warning(default: 4244)
}




//	---------------------------------------------------------------------------------------
//	Initiation of window data
//
static BOOL	//OUT TRUE - success
OnCreate(
	HWND a_hWnd,	//IN 
	LPCREATESTRUCT a_lpStruct
	)
{
	REmoDownData* l_pData = new REmoDownData(static_cast<const REmoData*>(a_lpStruct->lpCreateParams));
	SetREmoDownData(a_hWnd, l_pData);
	return TRUE;
}


//	---------------------------------------------------------------------------------------
//	Postmortem:-)
//
static void
OnNcDestroy(
	HWND a_hWnd		//IN
	)
{
	REmoDownData* l_pData = GetREmoDownData(a_hWnd);
	delete l_pData;
}


//	---------------------------------------------------------------------------
//	On WM_GETMINMAXINFO
//
void 
OnGetMinMaxInfo(
	HWND a_hWnd,
	LPMINMAXINFO a_pMMI
	)
{
	if (GetREmoDownData(a_hWnd) != NULL)
	{
		a_pMMI->ptMaxSize.x = GetWidth(a_hWnd);
		a_pMMI->ptMaxSize.y = GetHeight(a_hWnd);
		a_pMMI->ptMinTrackSize.x  = GetWidth(a_hWnd);
		a_pMMI->ptMinTrackSize.y  = GetHeight(a_hWnd);
		a_pMMI->ptMaxTrackSize.x  = GetWidth(a_hWnd);
		a_pMMI->ptMaxTrackSize.y  = GetHeight(a_hWnd);
	}
}


//	---------------------------------------------------------------------------
//	LButtonDown
//
static void 
OnLButtonDown(
	HWND a_hWnd, 
	LPPOINT a_pPoint 
	)
{
	RECT l_rect;
	POINT l_pt = *a_pPoint;

	::GetWindowRect(a_hWnd, &l_rect);
	::ClientToScreen(a_hWnd, &l_pt);

	// if point not in rect, we are capturing mouse, so lbutton is down
	// so if not clicked on owner lets hide us
	if (!::PtInRect(&l_rect, l_pt))
	{
		DropDown(a_hWnd, FALSE, NULL);
		return;
	}

}


//	---------------------------------------------------------------------------
//	LButtonUp
//
static void 
OnLButtonUp(
	HWND a_hWnd, 
	LPPOINT a_pPoint
	)
{
	TSelect l_sel;
	if (PointToEmo(a_hWnd, *a_pPoint, &l_sel))
	{
		NotifyOwner(a_hWnd, l_sel);
		DropDown(a_hWnd, FALSE, NULL);
	}
}


//	---------------------------------------------------------------------------------------
//	WM_MOUSEMOVE handler
//
static void
OnMouseMove(
	HWND a_hWnd, 
	int a_xPos, 
	int a_yPos
	)
{
	POINT l_pt = {a_xPos, a_yPos};
	TSelect l_sel;
	if (!PointToEmo(a_hWnd, l_pt, &l_sel))
	{
		return;
	}
	REmoDownData* l_pData = GetREmoDownData(a_hWnd);
	if ((l_sel.iRow == l_pData->m_selNew.iRow) && (l_sel.iCol == l_pData->m_selNew.iCol))
	{
		return;
	}
	InvalidateEmo(a_hWnd, l_pData->m_selNew);
	l_pData->m_selNew.iRow = l_sel.iRow;
	l_pData->m_selNew.iCol = l_sel.iCol;
	InvalidateEmo(a_hWnd, l_sel);
	::UpdateWindow(a_hWnd);
}


//	---------------------------------------------------------------------------------------
//	WM_SHOWWINDOW handler
//
static void
OnShowWindow(
	HWND a_hWnd, 
	BOOL a_bShow
	)
{
	if (!a_bShow)
	{
		REmoDownData* l_pData = GetREmoDownData(a_hWnd);
		::PostMessage(l_pData->m_hWndOwner, EDM_HIDEDROP, 0, 0);
	}
}


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

	RECT l_rectClip;
	::GetClipBox(l_hdc, &l_rectClip);

	COLORREF l_clrBk = ::GetSysColor(COLOR_BTNFACE);
#ifdef _DEBUG 
	HDC l_hMemDC = l_hdc;
#else	
	// create compatible offscreen bitmap
	HDC l_hMemDC = ::CreateCompatibleDC(l_hdc);
	HBITMAP l_hBmp = ::CreateCompatibleBitmap(l_hdc, 
		l_rectClip.right - l_rectClip.left, l_rectClip.bottom - l_rectClip.top);
    HBITMAP l_hBmpOld = (HBITMAP)::SelectObject(l_hMemDC, l_hBmp);
	::SetWindowOrgEx(l_hMemDC, l_rectClip.left, l_rectClip.top, NULL);
	// copy real dc to memdc
    ::BitBlt(l_hMemDC, l_rectClip.left, l_rectClip.top, 
		l_rectClip.right - l_rectClip.left, l_rectClip.bottom - l_rectClip.top,
		l_hdc, l_rectClip.left, l_rectClip.top, SRCCOPY);
#endif

	RECT l_rectWin;
	::GetClientRect(a_hWnd, &l_rectWin);

	RTheme l_theme;
	if (l_theme.OpenData(a_hWnd, L"MENU") != NULL)
	{
		TRACE0("Needs to be checked, as needed to be replaced, see commented for previous version");
		//l_theme.DrawBackground(l_hMemDC, MP_MENUBARDROPDOWN,
		//	MS_NORMAL, &l_rectWin, &l_rectClip);
		l_theme.DrawBackground(l_hMemDC, HP_HEADERDROPDOWN,
			HDDS_NORMAL, &l_rectWin, &l_rectClip);
	}
	else
	{
		RDraw::FillSolidRect(l_hMemDC, l_rectClip, l_clrBk);
	}
	DrawBitmap(a_hWnd, l_hMemDC);
	DrawGrid(a_hWnd, l_hMemDC);

	DrawSel(a_hWnd, l_hMemDC);

#ifndef _DEBUG
	// Copy the offscreen bitmap onto the screen.
    ::BitBlt(l_hdc, l_rectClip.left, l_rectClip.top, 
		l_rectClip.right - l_rectClip.left, l_rectClip.bottom - l_rectClip.top,
		l_hMemDC, l_rectClip.left, l_rectClip.top, SRCCOPY);

	//Swap back the original bitmap.
	::SelectObject(l_hMemDC, l_hBmpOld);
	::DeleteObject(l_hBmp);
#endif

	::EndPaint(a_hWnd, &l_ps);
}



//	---------------------------------------------------------------------------
//	Draws bitmap
//
void
DrawBitmap(
	HWND a_hWnd,
	HDC a_hDC
	)
{
	const REmoDownData* l_pData = GetREmoDownData(a_hWnd);

	HDC l_hdcMem = ::CreateCompatibleDC(a_hDC);
	::SelectObject(l_hdcMem, l_pData->m_pEmoData->m_hBitmap);
	BITMAP l_bmp;
	::GetObject(l_pData->m_pEmoData->m_hBitmap, sizeof(l_bmp), &l_bmp);
	::TransparentBlt(a_hDC, 0, 0, l_bmp.bmWidth, l_bmp.bmHeight, 
			l_hdcMem, 0, 0, l_bmp.bmWidth, l_bmp.bmHeight, 
			l_pData->m_pEmoData->m_clrBackBitmap);
	::DeleteDC(l_hdcMem);
}


//	---------------------------------------------------------------------------
//	Draws grid
//
void DrawGrid(HWND a_hWnd, HDC a_hDC)
{
	const REmoDownData* l_pData = GetREmoDownData(a_hWnd);
	HPEN l_hPen = ::CreatePen(PS_DOT, 0, RGB(255, 255, 255));
	HPEN l_hPenOld = static_cast<HPEN>(::SelectObject(a_hDC, l_hPen));
	// vertical
	UINT l_dyHeight = l_pData->m_pEmoData->m_iRows * l_pData->m_pEmoData->m_sizeEmo.cy;
	for (UINT l_iLine = 1; l_iLine < l_pData->m_pEmoData->m_iCols; l_iLine++)
	{
		::MoveToEx(a_hDC, l_iLine * l_pData->m_pEmoData->m_sizeEmo.cx, 0, NULL);
		::LineTo(a_hDC, l_iLine * l_pData->m_pEmoData->m_sizeEmo.cx, l_dyHeight);
	}
	// horizontal
	UINT l_dxWidth = l_pData->m_pEmoData->m_iCols * l_pData->m_pEmoData->m_sizeEmo.cx;
	for (UINT l_iLine = 1; l_iLine < l_pData->m_pEmoData->m_iRows; l_iLine++)
	{
		::MoveToEx(a_hDC, 0, l_iLine * l_pData->m_pEmoData->m_sizeEmo.cy, NULL);
		::LineTo(a_hDC, l_dxWidth, l_iLine * l_pData->m_pEmoData->m_sizeEmo.cy);
	}
	::SelectObject(a_hDC, l_hPenOld);
	::DeleteObject(l_hPen);
}


void DrawSel(HWND a_hWnd, HDC a_hDC)
{
	const REmoDownData* l_pData = GetREmoDownData(a_hWnd);
	RECT l_rect;
	EmoToRect(a_hWnd, l_pData->m_selNew, &l_rect);
	
	RTheme l_theme;
	
	if (l_theme.OpenData(a_hWnd, L"TOOLBAR") != NULL)
	{
		l_theme.DrawEdge(a_hDC, TP_BUTTON,
			TS_HOT, &l_rect, EDGE_RAISED, BF_RECT, NULL);
	}
	else
	{
		RDraw::Draw3DRect(a_hDC, l_rect, RGB(255, 255, 255), RGB(0, 0, 0));
		::InflateRect(&l_rect, -1, -1);
		RDraw::Draw3DRect(a_hDC, l_rect, RGB(255, 255, 255), RGB(0, 0, 0));
	}
}


//	---------------------------------------------------------------------------
//	Sends info about color change to its owner
void 
NotifyOwner(
	HWND a_hWnd,
	const TSelect& a_sel
	)
{
	const REmoDownData* l_pData = GetREmoDownData(a_hWnd);
	::SendMessage(l_pData->m_hWndOwner, EDM_EMOCHANGED, 0, reinterpret_cast<LPARAM>(&a_sel));
}


//	---------------------------------------------------------------------------
//	Shows or hides window
//
void 
DropDown(
	HWND a_hWnd,	
	BOOL a_bDown,
	LPCRECT a_pRect
	)
{
	if (a_bDown)
	{
		if (!::IsWindowVisible(a_hWnd))
		{
			_ASSERT(a_pRect != NULL);
			POINT l_pt;
			CalculateOpenPlace(a_hWnd, a_pRect, &l_pt);
			::SetWindowPos(a_hWnd, HWND_TOPMOST, l_pt.x, l_pt.y, 0, 0, SWP_SHOWWINDOW);
			::SetCapture(a_hWnd);
		}
	}
	else
	{
		if (::IsWindowVisible(a_hWnd))
		{
			::ReleaseCapture();
			::ShowWindow(a_hWnd, SW_HIDE);
		}
	}
}


//	---------------------------------------------------------------------------
//	Emoticon invalidation
//
static void
InvalidateEmo(
	HWND a_hWnd, 
	const TSelect& a_sel
	)
{
	RECT l_rectEmo;
	EmoToRect(a_hWnd, a_sel, &l_rectEmo);
	::InvalidateRect(a_hWnd, &l_rectEmo, FALSE);
}


//	---------------------------------------------------------------------------
//	Obliczenie szerokoœci
//
static int 
GetWidth(HWND a_hWnd)
{
	REmoDownData* l_pData = GetREmoDownData(a_hWnd);
	BITMAP l_bmp;
	::GetObject(l_pData->m_pEmoData->m_hBitmap, sizeof(l_bmp), &l_bmp);
	return ::GetSystemMetrics(SM_CYFRAME) * 2 + 
		l_bmp.bmWidth;
}


//	---------------------------------------------------------------------------
//	Obliczenie wysokoœci
//
int 
GetHeight(HWND a_hWnd)
{
	REmoDownData* l_pData = GetREmoDownData(a_hWnd);
	BITMAP l_bmp;
	::GetObject(l_pData->m_pEmoData->m_hBitmap, sizeof(l_bmp), &l_bmp);
	return ::GetSystemMetrics(SM_CYFRAME) * 2 + 
		l_bmp.bmHeight;
}


void EmoToRect(HWND a_hWnd, const TSelect& a_sel, LPRECT a_pRectEmo)
{
	REmoDownData* l_pData = GetREmoDownData(a_hWnd);
	a_pRectEmo->left = (a_sel.iCol - 1) * l_pData->m_pEmoData->m_sizeEmo.cx;
	a_pRectEmo->top = (a_sel.iRow - 1) * l_pData->m_pEmoData->m_sizeEmo.cy;
	a_pRectEmo->right = a_pRectEmo->left + l_pData->m_pEmoData->m_sizeEmo.cx;
	a_pRectEmo->bottom = a_pRectEmo->top + l_pData->m_pEmoData->m_sizeEmo.cy;
}


bool PointToEmo(HWND a_hWnd, const POINT& a_pt, TSelect* a_pSel)
{
	REmoDownData* l_pData = GetREmoDownData(a_hWnd);
	RECT l_rect;
	POINT l_pt = a_pt;

	::GetWindowRect(a_hWnd, &l_rect);
	::ClientToScreen(a_hWnd, &l_pt);
	if (!::PtInRect(&l_rect, l_pt))
	{
		return false;
	}
	a_pSel->iRow = (a_pt.y) / l_pData->m_pEmoData->m_sizeEmo.cy + 1;
	a_pSel->iCol = (a_pt.x) / l_pData->m_pEmoData->m_sizeEmo.cx + 1;
	return true;
}


void OnActivateApp(HWND a_hWnd, bool a_bActivate)
{
	if (!a_bActivate)
	{
		DropDown(a_hWnd, false, NULL);
	}
}


void CalculateOpenPlace(HWND a_hWnd, LPCRECT a_pRect, LPPOINT a_pPt)
{
	REmoDownData* l_pData = GetREmoDownData(a_hWnd);
	if (!ChangeOpenPlace(l_pData->m_hWndOwner))
	{
		a_pPt->x = a_pRect->left;
		a_pPt->y = a_pRect->bottom;
		return;
	}

	int l_dxScreen = ::GetSystemMetrics(SM_CXSCREEN);
	int l_dyScreen = ::GetSystemMetrics(SM_CYSCREEN);
	// x - pos
	if (a_pRect->left + GetWidth(a_hWnd) <= l_dxScreen)
	{
		a_pPt->x = a_pRect->left;
	}
	else if (a_pRect->right - GetWidth(a_hWnd) >= 0)
	{
		a_pPt->x = a_pRect->right - GetWidth(a_hWnd);
	}
	else
	{
		a_pPt->x = a_pRect->left;
	}

	// y - pos
	if (a_pRect->bottom + GetHeight(a_hWnd) <= l_dyScreen)
	{
		a_pPt->y = a_pRect->bottom;
	}
	else if (a_pRect->top - GetHeight(a_hWnd) >= 0)
	{
		a_pPt->y = a_pRect->top - GetHeight(a_hWnd);
	}
	else
	{
		a_pPt->y = a_pRect->bottom;
	}
}