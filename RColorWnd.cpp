// RColorWnd.cpp: implementation of the RColorWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "RColorWnd.h"
#include "RColorData.h"
#include "DrawUtl.h"
#include "RKeyboard.h"
#include <commctrl.h>
#include "RColorDownWnd.h"
#include "RTheme.h"
#include "RMemDC.h"


static const long c_iWindowOfs = sizeof(RColorData*) - 4;


static inline RColorData* GetRColorData(HWND a_hWnd);


// messages
static inline BOOL OnCreate(HWND a_hWnd, LPCREATESTRUCT a_lpStruct);
static inline void OnDestroy(HWND a_hWnd);
static inline void OnPaint(HWND a_hWnd);
static inline void OnLButtonDown(HWND a_hWnd, LPPOINT a_pPoint);
static inline void OnLButtonUp(HWND a_hWnd, LPPOINT a_pPoint);
static inline void OnMouseMove(HWND a_hWnd, int a_xPos, int a_yPos);

static inline void OnMouseLeave(HWND a_hWnd);

static inline void OnSetFocus(HWND a_hWnd);
static inline void OnKillFocus(HWND a_hWnd);
static inline void OnCaptureChanged(HWND a_hWnd);
static inline void OnKeyUp(HWND a_hWnd, WPARAM a_iKey, LPARAM a_iFlags);
static inline void OnKeyDown(HWND a_hWnd, WPARAM a_iKey, LPARAM a_iFlags);
static inline void OnSysKeyDown(HWND a_hWnd, WPARAM a_iKey, LPARAM a_iFlags);
static inline void OnThemeChanged(HWND a_hWnd);
static inline void OnActivate(HWND a_hWnd, UINT a_iState);
static inline void OnActivateApp(HWND a_hWnd, BOOL a_bActivate);
static inline void OnUpdateUIState(HWND a_hWnd, WPARAM a_wParam);
static inline void OnEnable(HWND a_hWnd, BOOL a_bEnable);
static inline void OnColorChanged(HWND a_hWnd, COLORREF a_clr);
static inline void OnHideDrop(HWND a_hWnd);
static inline LRESULT OnGetDlgCode(HWND a_hWnd, LPMSG a_pMsg);
static inline void OnSetColor(HWND a_hWnd, COLORREF a_clr, bool a_bNotify);
static inline LRESULT OnGetColor(HWND a_hWnd);
static inline void OnSetTooltip(HWND a_hWnd, LPTSTR a_psTooltip);
static inline void OnSetBitmap(HWND a_hWnd, HBITMAP a_hBitmap);
static inline UINT OnSetColumns(HWND a_hWnd, UINT a_iColumns);

// drawing procedures
static inline void Draw(HWND a_hWnd, HDC a_hDC);
static inline void DrawTriangle(HWND a_hWnd, HDC a_hDC, const RECT& a_rect, bool a_bPushed);
static inline void DrawSep(HWND a_hWnd, HDC a_hDC, const RECT& a_rect);
static inline void DrawColor(HWND a_hWnd, HDC a_hDC, const RECT& a_rect);

static void DropDown(HWND a_hWnd, bool a_bDown);
static inline COLORREF SetColor(HWND a_hWnd, COLORREF a_clr, bool a_bNotify);

static inline bool IsDropDown(HWND a_hWnd);

#define IsTbStyle(a_hWnd) ((::GetWindowLong(a_hWnd, GWL_STYLE) & RCCS_TBSTYLE) != 0)
#define IsWordStyle(a_hWnd) ((::GetWindowLong(a_hWnd, GWL_STYLE) & RCCS_WORDSTYLE) != 0)

static const short cc_dySep = 7;
static const short cc_dxTriangle = 12;

//	---------------------------------------------------------------------------------------
//	Main RColorWnd procedure
//
LRESULT CALLBACK 
RColorWnd_WndProc(
	HWND a_hWnd, 
	UINT a_iMsg, 
	WPARAM a_wParam, 
	LPARAM a_lParam
	)
{
	switch (a_iMsg) 
	{
		case WM_CREATE:
			return (OnCreate(a_hWnd, (LPCREATESTRUCT)a_lParam) ? 0 : -1);

		case WM_GETDLGCODE:
			return OnGetDlgCode(a_hWnd, (LPMSG)a_lParam);

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

		case WM_MOUSELEAVE:
			OnMouseLeave(a_hWnd);
			break;

		case WM_KEYDOWN:
			OnKeyDown(a_hWnd, a_wParam, a_lParam);
			break;

		case WM_KEYUP:
			OnKeyUp(a_hWnd, a_wParam, a_lParam);
			break;

		case WM_SYSKEYDOWN:
			OnSysKeyDown(a_hWnd, a_wParam, a_lParam);
			break;

		case WM_CAPTURECHANGED:
			OnCaptureChanged(a_hWnd);
			break;

		case WM_SETFOCUS:
			OnSetFocus(a_hWnd);
			break;

		case WM_KILLFOCUS:
			OnKillFocus(a_hWnd);
			break;

		case WM_THEMECHANGED:
			OnThemeChanged(a_hWnd);
			break;

		case WM_UPDATEUISTATE:
			OnUpdateUIState(a_hWnd, a_wParam);
			break;

		case WM_DESTROY:
			OnDestroy(a_hWnd);
			break;

		case WM_ENABLE:
			OnEnable(a_hWnd, (a_wParam == TRUE));
			break;

		case CDM_COLORCHANGED:
			OnColorChanged(a_hWnd, (COLORREF)a_wParam);
			break;

		case CDM_HIDEDROP:
			OnHideDrop(a_hWnd);
			break;

		case RCM_SETCOLOR:
			OnSetColor(a_hWnd, static_cast<COLORREF>(a_lParam), (a_wParam == TRUE));
			break;

		case RCM_GETCOLOR:
			return OnGetColor(a_hWnd);

		case RCM_SETTOOLTIP:
			OnSetTooltip(a_hWnd, reinterpret_cast<LPTSTR>(a_lParam));
			break;

		case RCM_SETBITMAP:
			OnSetBitmap(a_hWnd, reinterpret_cast<HBITMAP>(a_lParam));
			break;

		case RCM_SETCOLUMNS:
			return static_cast<LRESULT>(OnSetColumns(a_hWnd, static_cast<UINT>(a_lParam)));

		default:
			return DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
   }
   return 0;
}


//	---------------------------------------------------------------------------------------
//	Creation of RColorWnd window
//
HWND	// Handle of created window or NULL if failed
RColorWnd_Create(
	DWORD		a_iStyle,		//WE style,
	int			a_x,			// horizontal position of window
	int			a_y,			// vertical position of window
	int			a_iWidth,		// window width
	int			a_iHeight,		// window height
	HWND		a_hWndParent,   // handle to parent or owner window
	LPVOID		a_lpParam			// pointer to window-creation data
	)
{
	HWND l_hWnd = ::CreateWindow(RColorWnd_ClassName, _T(""), a_iStyle,
		a_x, a_y, a_iWidth, a_iHeight, a_hWndParent, NULL, RCommon_GetInstance(), a_lpParam);

	return l_hWnd;
}


//	---------------------------------------------------------------------------------------
//	sets control's color
//
void
OnSetColor(
	HWND	 a_hWnd,   // IN window handle 
	COLORREF a_clr,	   // IN new color
	bool     a_bNotify // IN send notify
	)
{
	SetColor(a_hWnd, a_clr, a_bNotify);
}


//	---------------------------------------------------------------------------------------
//	returns control's color
//
LRESULT	// current color
OnGetColor(
	HWND	 a_hWnd  // window handle
	)
{
	ASSERT(::IsWindow(a_hWnd));
	return static_cast<LRESULT>(GetRColorData(a_hWnd)->GetColor());
}


//	---------------------------------------------------------------------------------------
//	Returns control's data
//
static RColorData*	//OUT pointer to this data
GetRColorData(
	HWND a_hWnd	//IN 
	)
{
#pragma warning(disable: 4312)
	return reinterpret_cast<RColorData*>(::GetWindowLongPtr(a_hWnd, c_iWindowOfs));
#pragma warning(default: 4312)
}


//	---------------------------------------------------------------------------------------
//	Sets control's data
//
static void	
SetRColorData(
	HWND a_hWnd,	//IN
	RColorData* a_pData	//IN
	)
{
#pragma warning(disable: 4311)
	::SetWindowLongPtr(a_hWnd, c_iWindowOfs, reinterpret_cast<LONG>(a_pData));
#pragma warning(default: 4311)
}




//	---------------------------------------------------------------------------------------
//	Initiation of window data
//
BOOL OnCreate(HWND a_hWnd, LPCREATESTRUCT /*a_lpStruct*/)
{
	RColorData* l_pData = new RColorData();
	SetRColorData(a_hWnd, l_pData);
	return TRUE;
}


//	---------------------------------------------------------------------------------------
//	Postmortem:-)
//
static void
OnDestroy(
	HWND a_hWnd		//IN
	)
{
	const RColorData* l_pData = GetRColorData(a_hWnd);
	if (::IsWindow(l_pData->GetDownWnd()))
	{
		::DestroyWindow(l_pData->GetDownWnd());
	}

	TRACKMOUSEEVENT l_track = {sizeof(TRACKMOUSEEVENT), TME_CANCEL | TME_LEAVE | TME_HOVER, a_hWnd, HOVER_DEFAULT};
	::TrackMouseEvent(&l_track);

	delete l_pData;

}


//	---------------------------------------------------------------------------
//	LButtonDown
//
void OnLButtonDown(HWND a_hWnd, LPPOINT /*a_pPoint*/)
{
	RColorData* l_pData = GetRColorData(a_hWnd);
	if (IsDropDown(a_hWnd))
	{
		return;
	}
	if (l_pData->IsNormal())
	{
		if (!IsTbStyle(a_hWnd))
		{
			::SetFocus(a_hWnd);
		}
		l_pData->SetClicked();
		::SetCapture(a_hWnd);
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
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
	RColorData* l_pData = GetRColorData(a_hWnd);
	if (!l_pData->IsClicked())
	{
		::ReleaseCapture();	// cannot be before IsClicked, because it sends CaptureChanged 
							// message, which changes state of button
		return;
	}
	::ReleaseCapture();

	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);
	l_pData->SetNormal();
	if (IsTbStyle(a_hWnd))
	{
		l_rect.right -= cc_dxTriangle;
		if (::PtInRect(&l_rect, *a_pPoint))
		{
			SetColor(a_hWnd, l_pData->GetColor(), true);
			return;
		}
		else
		{
			DropDown(a_hWnd, true);
		}
	}
	else
	{
		if (::PtInRect(&l_rect, *a_pPoint))
		{
			DropDown(a_hWnd, true);
		}
	}
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
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

	RColorData* l_pData = GetRColorData(a_hWnd);	
	if (!l_pData->IsHovering())
	{
		l_pData->SetHovering(true);
		TRACKMOUSEEVENT l_track = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, a_hWnd, 1};
		::TrackMouseEvent(&l_track);
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
		::SendMessage(l_pData->m_hTooltip, TTM_POPUP, TRUE, 0);
		return;
	}

	if (!LeftMouseButtonPressed())
	{
		return;
	}
	if (::GetCapture() != a_hWnd)
	{
		return;
	}
	if (l_pData->IsPressed())
	{
		return;
	}
	RECT l_rect;
	::GetWindowRect(a_hWnd, &l_rect);
	POINT l_pt = {a_xPos, a_yPos};
	::ClientToScreen(a_hWnd, &l_pt);
	BOOL l_bOnBtn = ::PtInRect(&l_rect, l_pt);
	if (l_pData->IsClicked() && !l_bOnBtn)
	{
		l_pData->SetNormal();
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (!l_pData->IsClicked() && l_bOnBtn)
	{
		l_pData->SetClicked();
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}

}


//	---------------------------------------------------------------------------------------
//	WM_MOUSELEAVE handler
//
static void
OnMouseLeave(
	HWND a_hWnd
	)
{
	RColorData* l_pData = GetRColorData(a_hWnd);
	l_pData->SetHovering(false);
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}


//	---------------------------------------------------------------------------
//	SetFocus
//
static void 
OnSetFocus(
	HWND a_hWnd
	)
{
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}
			


//	---------------------------------------------------------------------------
//	KillFocus
//
static 
void OnKillFocus(
	HWND a_hWnd
	)
{
	RColorData* l_pData = GetRColorData(a_hWnd);
	if (l_pData->IsClicked())
	{
		l_pData->SetNormal();
	}
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

}
			

//	---------------------------------------------------------------------------
//	KeyDown
//
static
void OnKeyDown(
	HWND a_hWnd, 
	WPARAM a_iKey,
	LPARAM a_iFlags
	)
{
	RColorData* l_pData = GetRColorData(a_hWnd);
	if (IsDropDown(a_hWnd))
	{
		::SendMessage(l_pData->GetDownWnd(), WM_KEYDOWN, a_iKey, a_iFlags);
	}
	else if (a_iKey == VK_SPACE)
	{
		if (l_pData->IsNormal())
		{
			l_pData->SetPressed();
			::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}

}


//	---------------------------------------------------------------------------
//	KeyUp
//
void OnKeyUp(HWND a_hWnd, WPARAM a_iKey, LPARAM a_iFlags)
{
	RColorData* l_pData = GetRColorData(a_hWnd);
	if (IsDropDown(a_hWnd) && (a_iKey != VK_F4))
	{
		if (a_iKey == VK_ESCAPE)
		{
			RColorDownWnd_Hide(l_pData->GetDownWnd());
		}
		else
		{
			::SendMessage(l_pData->GetDownWnd(), WM_KEYUP, (WPARAM)a_iKey, a_iFlags);
		}
	}
	else if (a_iKey == VK_F4)
	{

		DropDown(a_hWnd, !IsDropDown(a_hWnd));
	}
	else if (a_iKey == VK_SPACE)
	{
		l_pData->SetNormal();
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
		DropDown(a_hWnd, true);
	}

}


//	---------------------------------------------------------------------------
//	SysKeyDown
//
void OnSysKeyDown(HWND a_hWnd, WPARAM a_iKey, LPARAM a_iFlags)
{
	const RColorData* l_pData = GetRColorData(a_hWnd);
	if (IsDropDown(a_hWnd))
	{
		::SendMessage(l_pData->GetDownWnd(), WM_SYSKEYDOWN, (WPARAM)a_iKey, a_iFlags);
	}

}


//	---------------------------------------------------------------------------
//	OnThemeChanged
//
void OnThemeChanged(
	HWND a_hWnd
	)
{
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}

void OnEnable(HWND a_hWnd, BOOL /*a_bEnable*/)
{
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}


//	---------------------------------------------------------------------------
//	OnUpdateUIState
//
void OnUpdateUIState(HWND a_hWnd, WPARAM a_wParam)
{
	RColorData* l_pData = GetRColorData(a_hWnd);
	if (l_pData->GetDownWnd() != NULL)
	{
		::SendMessage(l_pData->GetDownWnd(), WM_UPDATEUISTATE, a_wParam, 0);
	}
}


//	---------------------------------------------------------------------------
//	OnCaptureChanged
//
void 
OnCaptureChanged(
	HWND a_hWnd
	)
{
	RColorData* l_pData = GetRColorData(a_hWnd);
	l_pData->SetNormal();
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}


//	---------------------------------------------------------------------------
//	
//
void OnActivate(HWND a_hWnd, UINT a_iState)
{
	if (a_iState == WA_INACTIVE)
	{
		DropDown(a_hWnd, false);
	}
}


//	---------------------------------------------------------------------------
//	
//
void OnActivateApp(HWND a_hWnd,	BOOL a_bActivate)
{
	if (!a_bActivate)
	{
		DropDown(a_hWnd, false);
	}
}


//	---------------------------------------------------------------------------
//	OnColorChanged
//
void OnColorChanged(HWND a_hWnd, COLORREF a_clr)
{
	SetColor(a_hWnd, a_clr, true);
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

}


//	---------------------------------------------------------------------------
//	OnHideDrop
//
static void 
OnHideDrop(
	HWND a_hWnd
	)
{
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}


//	---------------------------------------------------------------------------------------
//	What messages we want to intercept
//
LRESULT OnGetDlgCode(HWND a_hWnd, LPMSG a_pMsg)
{
	if (!IsDropDown(a_hWnd))
	{
		return 0;
	}

	// WM_KEYUP must also be captured to not cause beep after pressing TAB
	if	(
		(a_pMsg != NULL) && 
		(a_pMsg->message == WM_KEYDOWN || a_pMsg->message == WM_KEYUP)
		)
	{
		if	(
			((int)a_pMsg->wParam == VK_TAB)		|| 
			((int)a_pMsg->wParam == VK_DOWN)	|| 
			((int)a_pMsg->wParam == VK_UP)		||
			((int)a_pMsg->wParam == VK_LEFT)	|| 
			((int)a_pMsg->wParam == VK_RIGHT)	||
			((int)a_pMsg->wParam == VK_ESCAPE)	||
			((int)a_pMsg->wParam == VK_RETURN)
			)
		{
			return DLGC_WANTMESSAGE;
		}
	}

	return 0;

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

	RECT l_rectWin;
	::GetClientRect(a_hWnd, &l_rectWin);
	{	
		// prepare correct DC
#ifdef _DEBUG 
		HDC l_hMemDC = l_hdc;
#else
		RMemDC l_memDC(l_hdc, &l_rectWin);
		HDC l_hMemDC = l_memDC;
#endif
		
		Draw(a_hWnd, l_hMemDC);
	}
	::EndPaint(a_hWnd, &l_ps);
}


//	---------------------------------------------------------------------------
//	Really draws color button
//
void 
Draw(
	HWND a_hWnd, 
	HDC a_hDC
	)
{
	RECT l_rectWin;
	::GetClientRect(a_hWnd, &l_rectWin);

	const RColorData* l_pData = GetRColorData(a_hWnd);

	RTheme l_theme;
	if (l_theme.OpenData(a_hWnd, L"BUTTON") != NULL)
	{		
		l_theme.DrawParentBackground(a_hWnd, a_hDC, &l_rectWin);
		// horror! but don't have better idea, how to remove separator :-(
		if (IsTbStyle(a_hWnd))
		{
			for (int l_y = 0; l_y <= RectHeight(l_rectWin); l_y++)
			{
				::SetPixel(a_hDC, 3, l_y, ::GetPixel(a_hDC, 4, l_y));
			}
		}
		long l_iStyle = ::GetWindowLong(a_hWnd, GWL_STYLE);
		int l_iState;
		if (!l_pData->IsNormal())
		{
			l_iState = PBS_PRESSED;
		}
		else if (l_pData->IsHovering())
		{
			l_iState = IsTbStyle(a_hWnd) ? PBS_DISABLED : PBS_HOT;
		}
		else if (l_iStyle & WS_DISABLED)
		{
			l_iState = PBS_DISABLED;
		}
		else
		{
			l_iState = IsTbStyle(a_hWnd) ? PBS_DISABLED : PBS_NORMAL;
		}

		if (!IsTbStyle(a_hWnd) || l_pData->IsHovering())
		{
			l_theme.DrawBackground(a_hDC, BP_PUSHBUTTON,
						l_iState, &l_rectWin, NULL);
		}
		l_theme.CloseData();
	}
	else
	{
		::FillRect(a_hDC, &l_rectWin, l_pData->m_hBrushBtnFace);
		if (!IsTbStyle(a_hWnd) || l_pData->IsHovering())
		{
			RDraw::DrawButtonBorder(a_hDC, l_rectWin, !l_pData->IsNormal(), IsDisabled(a_hWnd), IsTbStyle(a_hWnd), l_pData->IsHovering());
		}
	}
	
	if (IsFocused(a_hWnd))
	{
		if (!IsDropDown(a_hWnd))
		{
			RECT l_rectFocus(l_rectWin);
			::InflateRect(&l_rectFocus, -3, -3);
			::DrawFocusRect(a_hDC, &l_rectFocus);
		}
	}

	DrawTriangle(a_hWnd, a_hDC, l_rectWin, !l_pData->IsNormal());
	DrawSep(a_hWnd, a_hDC, l_rectWin);

	DrawColor(a_hWnd, a_hDC, l_rectWin);
}


//	---------------------------------------------------------------------------
//	Rejestracja klasy
//
ATOM 
RColorWnd_RegisterClass()
{


	WNDCLASSEX l_wcex;
	// Initialize global strings

	l_wcex.cbSize			= sizeof(WNDCLASSEX); 
	l_wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_PARENTDC;
	l_wcex.lpfnWndProc		= RColorWnd_WndProc;
	l_wcex.cbClsExtra		= 0;
	l_wcex.cbWndExtra		= sizeof(RColorData*);
	l_wcex.hInstance		= RCommon_GetInstance();
	l_wcex.hIcon			= NULL;
	l_wcex.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
	l_wcex.hbrBackground	= NULL;
	l_wcex.lpszMenuName		= NULL;
	l_wcex.lpszClassName	= RColorWnd_ClassName;
	l_wcex.hIconSm			= NULL;

	return ::RegisterClassEx(&l_wcex);
}


//	---------------------------------------------------------------------------
//	Utworzenie okna
//
HWND
RColorWnd_Create(
	HWND a_hWndParent
	)
{
	DWORD l_dwStyle = WS_THICKFRAME | WS_OVERLAPPED | WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | 0x08021;
	DWORD l_dwStyleEx = WS_EX_TOOLWINDOW;

	HWND l_hWnd = ::CreateWindowEx(l_dwStyleEx, RColorWnd_ClassName, _T(""), l_dwStyle, 
		CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, a_hWndParent, 0, RCommon_GetInstance(), NULL);
	return l_hWnd;
}


//	---------------------------------------------------------------------------
//	Narysowanie trójk¹ta
//
static void 
DrawTriangle(
	HWND a_hWnd, 
	HDC a_hDC, 
	const RECT& a_rect,
	bool a_bPushed
	)
{
	RColorData* l_pData = GetRColorData(a_hWnd);
	// wyczyszczenie t³a
	RECT l_rect;
	l_rect.top = a_rect.top + ((RectHeight(a_rect) / 2) - 1);
	l_rect.left = a_rect.right - cc_dxTriangle + 2;
	l_rect.bottom = l_rect.top + 4;
	l_rect.right = l_rect.left + 5;

	// obliczenie po³ozenia górnej krawêdzi
	l_rect.top += (a_bPushed ? 1 : 0);
	l_rect.bottom = l_rect.top + 1;

	::FillRect(a_hDC, &l_rect, l_pData->m_hBrushBlack);

	l_rect.left += 1;
	l_rect.right -= 1;
	l_rect.top += 1;
	l_rect.bottom += 1;
	::FillRect(a_hDC, &l_rect, l_pData->m_hBrushBlack);

	l_rect.left += 1;
	l_rect.right -= 1;
	l_rect.top += 1;
	l_rect.bottom += 1;
	::FillRect(a_hDC, &l_rect, l_pData->m_hBrushBlack);
}


//	---------------------------------------------------------------------------
//	Narysowanie separatora
//
static void 
DrawSep(
	HWND a_hWnd, 
	HDC a_hDC, 
	const RECT& a_rect
	)
{
	RColorData* l_pData = GetRColorData(a_hWnd);
	if (IsTbStyle(a_hWnd) && l_pData->IsNormal() && !l_pData->IsHovering())
	{
		return;
	}
	RECT l_rect = a_rect;
	::InflateRect(&l_rect, 0, -cc_dySep);
	l_rect.left = a_rect.right - cc_dxTriangle;
	l_rect.right = l_rect.left + 1;

	::FillRect(a_hDC, &l_rect, l_pData->m_hBrush3DShadow);
	l_rect.left += 1;
	l_rect.right += 1;
	::FillRect(a_hDC, &l_rect, l_pData->m_hBrushWhite);

}

//	---------------------------------------------------------------------------
//	Draws color or bitmap
//
void DrawColor(HWND a_hWnd, HDC a_hDC, const RECT& a_rect)
{
	RColorData* l_pData = GetRColorData(a_hWnd);
	if (l_pData->m_hBitmap == NULL)
	{
		RECT l_rectWin = a_rect;
		::SetRect(&l_rectWin, l_rectWin.left + 6, l_rectWin.top + 6, l_rectWin.right - 16, l_rectWin.bottom - 6);
		RDraw::Draw3DRect(a_hDC, l_rectWin, RGB(0, 0, 0), RGB(0, 0, 0));

		::InflateRect(&l_rectWin, -1, -1);
		::FillRect(a_hDC, &l_rectWin, l_pData->m_hBrushColor);
	}
	else
	{
		BITMAP l_bmp;
		if (!::GetObject(l_pData->m_hBitmap, sizeof(l_bmp), &l_bmp))
		{
			return;
		}
		HDC l_hdcCompat = ::CreateCompatibleDC(a_hDC);
		::SelectObject(l_hdcCompat, l_pData->m_hBitmap);
		RECT l_rectBmp = a_rect;
		::SetRect(&l_rectBmp, l_rectBmp.left + 6, l_rectBmp.top + 3, l_rectBmp.right - 16, l_rectBmp.bottom - 8);
		::BitBlt(a_hDC, l_rectBmp.left, l_rectBmp.top, l_bmp.bmWidth, l_bmp.bmHeight, 
			l_hdcCompat, 0, 0, SRCAND);
		::DeleteDC(l_hdcCompat);
		::SetRect(&l_rectBmp, l_rectBmp.left, l_rectBmp.top + l_bmp.bmHeight + 1, l_rectBmp.left + l_bmp.bmWidth, l_rectBmp.top + l_bmp.bmHeight + 4);
		::FillRect(a_hDC, &l_rectBmp, l_pData->m_hBrushColor);
	}
}


//	---------------------------------------------------------------------------
//	Shows/hides dropped window
//
void DropDown(HWND a_hWnd, bool a_bDown)
{
	RColorData* l_pData = GetRColorData(a_hWnd);
	HWND l_hDownWnd = l_pData->GetDownWnd();
	if (a_bDown)
	{
		if (l_hDownWnd == NULL)
		{
			RColorDownWnd_RegisterClass();
			l_hDownWnd = RColorDownWnd_Create(a_hWnd, ::GetWindowLong(a_hWnd, GWL_STYLE), l_pData);
			l_pData->SetDownWnd(l_hDownWnd);
		}
		RECT l_rect;
		::GetWindowRect(a_hWnd, &l_rect);
		RColorDownWnd_DropDown(l_hDownWnd, &l_rect, l_pData->GetColor());
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else
	{
		if (l_hDownWnd != NULL)
		{
			RColorDownWnd_Hide(l_hDownWnd);
		}
	}


}


//	---------------------------------------------------------------------------
//	Checks if DropDown window is visible
//
static bool 
IsDropDown(
	HWND a_hWnd
	)
{
	const RColorData* l_pData = GetRColorData(a_hWnd);
	return (::IsWindow(l_pData->GetDownWnd())) && (::IsWindowVisible(l_pData->GetDownWnd()));
}


//	---------------------------------------------------------------------------
//	sets control color and sends notification to parent
//
static COLORREF 
SetColor(
	HWND     a_hWnd,   //IN window
	COLORREF a_clr,	   //IN new color
	bool     a_bNotify //IN notify that color changed
	)
{
	_ASSERT(::IsWindow(a_hWnd));
	RColorData* l_pData = GetRColorData(a_hWnd);

	long l_idCtrl = ::GetDlgCtrlID(a_hWnd);
	RCCNMCHG l_nmhdr;
	l_nmhdr.clrOld = l_pData->GetColor();
	if (a_bNotify)
	{
		l_nmhdr.nmhdr.hwndFrom = a_hWnd;
		l_nmhdr.nmhdr.code = RCCN_CHANGED;
		l_nmhdr.nmhdr.idFrom = l_idCtrl;
		l_nmhdr.clrNew = a_clr;
	}

	l_pData->SetColor(a_clr);

	if (a_bNotify)
	{
		HWND l_hParent = ::GetParent(a_hWnd);
		if (l_hParent == NULL)
		{
			l_hParent = a_hWnd;
		}

		::SendMessage(l_hParent, WM_NOTIFY, (WPARAM)l_idCtrl, (LPARAM)&l_nmhdr);
	}
	return l_nmhdr.clrOld;
}


void OnSetTooltip(HWND a_hWnd, LPTSTR a_psTooltip)
{
	RColorData* l_pData = GetRColorData(a_hWnd);
	l_pData->m_psTooltip = a_psTooltip;
	if ((_tcslen(a_psTooltip) == 0) && (l_pData->m_hTooltip != NULL))
	{
		::DestroyWindow(l_pData->m_hTooltip);
	}
	else if (_tcslen(a_psTooltip) != 0)
	{
		if (l_pData->m_hTooltip == NULL)
		{
			l_pData->m_hTooltip = ::CreateWindowEx(WS_EX_TOPMOST,
				TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				a_hWnd, NULL, RCommon_GetInstance(), NULL);
		}
		::SetWindowPos(l_pData->m_hTooltip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		RECT l_rect;
		::GetClientRect(a_hWnd, &l_rect);
		
		TOOLINFO l_ti;
		l_ti.cbSize = sizeof(TOOLINFO);
		l_ti.uFlags = TTF_SUBCLASS;
		l_ti.hwnd = a_hWnd;
		l_ti.hinst = RCommon_GetInstance();
		l_ti.uId = 1;
		l_ti.lpszText = l_pData->m_psTooltip;
		l_ti.rect.left = l_rect.left;    
		l_ti.rect.top = l_rect.top;
		l_ti.rect.right = l_rect.right;
		l_ti.rect.bottom = l_rect.bottom;
		::SendMessage(l_pData->m_hTooltip, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&l_ti));	
	}
}


void OnSetBitmap(HWND a_hWnd, HBITMAP a_hBitmap)
{
	RColorData* l_pData = GetRColorData(a_hWnd);
	l_pData->m_hBitmap = a_hBitmap;
}


UINT OnSetColumns(HWND a_hWnd, UINT a_iColumns)
{
	RColorData* l_pData = GetRColorData(a_hWnd);
	UINT l_iColumns = l_pData->m_iColumns;
	l_pData->m_iColumns = a_iColumns;
	return l_iColumns;
}
