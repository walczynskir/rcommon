// REmoWnd.cpp: implementation of the REmoWnd class.
// I'm using abbreviation emo for emoticon
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "REmoWnd.h"
#include "REmoData.h"
#include "DrawUtl.h"
#include "RKeyboard.h"
#include "REmoDownWnd.h"
#include "RTheme.h"
#include "RMemDC.h"



static const long c_iWindowOfs = sizeof(REmoData*) - 4;


static inline REmoData* GetREmoData(HWND a_hWnd);


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
static inline void OnThemeChanged(HWND a_hWnd);
static inline void OnActivate(HWND a_hWnd, UINT a_iState);
static inline void OnActivateApp(HWND a_hWnd, BOOL a_bActivate);
static inline void OnEnable(HWND a_hWnd, BOOL a_bEnable);

static inline void OnEmoChanged(HWND a_hWnd, TSelect* a_pSel);
static inline void OnHideDrop(HWND a_hWnd);
static inline void OnSetSel(HWND a_hWnd, TSelect* a_pSel, bool a_bNotify);
static inline void OnGetEmo(HWND a_hWnd, HBITMAP* a_pBmp);
static inline void OnGetTranspEmo(HWND a_hWnd, COLORREF a_clrBack, HBITMAP* a_pBmp);

static inline void OnSetTooltip(HWND a_hWnd, LPTSTR a_psTooltip);
static inline BOOL OnSetStruct(HWND a_hWnd, LPTEmoStruct a_pEmoStruct);

// drawing procedures
static inline void Draw(HWND a_hWnd, HDC a_hDC);
static inline void DrawTriangle(HWND a_hWnd, HDC a_hDC, const RECT& a_rect, bool a_bPushed);
static inline void DrawSep(HWND a_hWnd, HDC a_hDC, const RECT& a_rect);
static inline void DrawEmo(HWND a_hWnd, HDC a_hDC);
static inline void GetEmoRect(HWND a_hWnd, LPRECT a_pRectEmo);

static void DropDown(HWND a_hWnd, bool a_bDown);
static inline void SetEmo(HWND a_hWnd, const TSelect& a_sel, bool a_bNotify);

static inline void NotifyPushed(HWND a_hWnd);
static void Notify(HWND a_hWnd, const TSelect& a_selOld, const TSelect& a_selNew, UINT a_iCode);

static HBITMAP GetBitmap(HWND a_hWnd);

static inline bool IsDropDown(HWND a_hWnd);

#define IsTbStyle(a_hWnd) ((::GetWindowLong(a_hWnd, GWL_STYLE) & RECS_TBSTYLE) != 0)
// #define IsTbStyle(a_hWnd) (false)

static const short cc_dySep = 7;
static const short cc_dxTriangle = 12;

//	---------------------------------------------------------------------------------------
//	Main REmoWnd procedure
//
LRESULT CALLBACK 
REmoWnd_WndProc(
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
			TRACE0("KeyDown\n");
			OnKeyDown(a_hWnd, a_wParam, a_lParam);
			break;

		case WM_KEYUP:
			OnKeyUp(a_hWnd, a_wParam, a_lParam);
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

		case WM_DESTROY:
			OnDestroy(a_hWnd);
			break;

		case WM_ENABLE:
			OnEnable(a_hWnd, (a_wParam == TRUE));
			break;

		case EDM_EMOCHANGED:
			OnEmoChanged(a_hWnd, reinterpret_cast<TSelect*>(a_lParam));
			break;

		case EDM_HIDEDROP:
			OnHideDrop(a_hWnd);
			break;

		case REM_SETSEL:
			OnSetSel(a_hWnd, reinterpret_cast<TSelect*>(a_lParam), (a_wParam == TRUE));
			break;

		case REM_GETEMO:
			OnGetEmo(a_hWnd, reinterpret_cast<HBITMAP*>(a_lParam));
			break;

		case REM_GETTRANSPEMO:
			OnGetTranspEmo(a_hWnd, static_cast<COLORREF>(a_wParam), reinterpret_cast<HBITMAP*>(a_lParam));
			break;

		case REM_SETTOOLTIP:
			OnSetTooltip(a_hWnd, reinterpret_cast<LPTSTR>(a_lParam));
			break;

		case REM_SETSTRUCT:
			return OnSetStruct(a_hWnd, reinterpret_cast<LPTEmoStruct>(a_lParam));

		default:
			return DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
   }
   return 0;
}


//	---------------------------------------------------------------------------------------
//	Creation of REmoWnd window
//
HWND	// Handle of created window or NULL if failed
REmoWnd_Create(
	DWORD		a_iStyle,		//WE style,
	int			a_x,			// horizontal position of window
	int			a_y,			// vertical position of window
	int			a_iWidth,		// window width
	int			a_iHeight,		// window height
	HWND		a_hWndParent,   // handle to parent or owner window
	LPVOID		a_lpParam			// pointer to window-creation data
	)
{
	HWND l_hWnd = ::CreateWindow(REmoWnd_ClassName, _T(""), a_iStyle,
		a_x, a_y, a_iWidth, a_iHeight, a_hWndParent, NULL, RCommon_GetInstance(), a_lpParam);

	return l_hWnd;
}


//	---------------------------------------------------------------------------------------
//	sets control's color
//
void
OnSetSel(
	HWND	 a_hWnd,   // IN window handle 
	TSelect* a_pSel,	 // IN new emoticons
	bool     a_bNotify // IN send notify
	)
{
	ASSERT(::IsWindow(a_hWnd));
	ASSERT(a_pSel != NULL);
	SetEmo(a_hWnd, *a_pSel, a_bNotify == TRUE);
}


//	---------------------------------------------------------------------------------------
//	returns selected emo
//
void	
OnGetEmo(
	HWND a_hWnd,    //IN window handle
	HBITMAP* a_pBmp //OUT selected emo
	)
{
	ASSERT(::IsWindow(a_hWnd));
	*a_pBmp = GetBitmap(a_hWnd);
}


//	---------------------------------------------------------------------------------------
//	returns selected emo with given background color
//
void OnGetTranspEmo(
	HWND a_hWnd, 
	COLORREF a_clrBack, 
	HBITMAP* a_pBmp
	)
{
	ASSERT(::IsWindow(a_hWnd));
	REmoData* l_pData = GetREmoData(a_hWnd);
	HBITMAP l_hBmp = GetBitmap(a_hWnd);
	*a_pBmp = RDraw::ReplaceColor(l_hBmp, l_pData->m_clrBackBitmap, a_clrBack);
}


//	---------------------------------------------------------------------------------------
//	Returns control's data
//
static REmoData*	//OUT pointer to this data
GetREmoData(
	HWND a_hWnd	//IN 
	)
{
#pragma warning(disable: 4312)
	return reinterpret_cast<REmoData*>(::GetWindowLongPtr(a_hWnd, c_iWindowOfs));
#pragma warning(default: 4312)
}


//	---------------------------------------------------------------------------------------
//	Sets control's data
//
static void	
SetREmoData(
	HWND a_hWnd,	//IN
	REmoData* a_pData	//IN
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
	REmoData* l_pData = new REmoData();
	SetREmoData(a_hWnd, l_pData);
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
	const REmoData* l_pData = GetREmoData(a_hWnd);
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
	REmoData* l_pData = GetREmoData(a_hWnd);
	if (IsDropDown(a_hWnd))
	{
		return;
	}
	if (l_pData->IsNormal())
	{
		::SetFocus(a_hWnd);
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
	REmoData* l_pData = GetREmoData(a_hWnd);
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
			NotifyPushed(a_hWnd);
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

	REmoData* l_pData = GetREmoData(a_hWnd);	
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
	REmoData* l_pData = GetREmoData(a_hWnd);
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
	REmoData* l_pData = GetREmoData(a_hWnd);
	if (l_pData->IsClicked())
	{
		l_pData->SetNormal();
	}
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	DropDown(a_hWnd, false);

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
	REmoData* l_pData = GetREmoData(a_hWnd);
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
void OnKeyUp(HWND a_hWnd, WPARAM a_iKey, LPARAM /*a_iFlags*/)
{
	REmoData* l_pData = GetREmoData(a_hWnd);
	if (IsDropDown(a_hWnd) && (a_iKey != VK_F4))
	{
		if (a_iKey == VK_ESCAPE)
		{
			REmoDownWnd_Hide(l_pData->GetDownWnd());
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
//	OnCaptureChanged
//
static 
void OnCaptureChanged(
	HWND a_hWnd
	)
{
	REmoData* l_pData = GetREmoData(a_hWnd);
	l_pData->SetNormal();
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}


//	---------------------------------------------------------------------------
//	
//
static void 
OnActivate(
	HWND a_hWnd,
	UINT a_iState
	)
{
	if (a_iState == WA_INACTIVE)
	{
		DropDown(a_hWnd, false);
	}
}


//	---------------------------------------------------------------------------
//	
//
static void 
OnActivateApp(
	HWND a_hWnd,
	BOOL a_bActivate
	)
{
	if (!a_bActivate)
	{
		DropDown(a_hWnd, false);
	}
}


//	---------------------------------------------------------------------------
//	OnEmoChanged
//
static void 
OnEmoChanged(
	HWND a_hWnd, 
	TSelect* a_pSel
	)
{
	SetEmo(a_hWnd, *a_pSel, true);
	RECT l_rectEmo;
	::GetEmoRect(a_hWnd, &l_rectEmo);
	::RedrawWindow(a_hWnd, &l_rectEmo, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
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
static void 
Draw(
	HWND a_hWnd, 
	HDC a_hDC
	)
{
	RECT l_rectWin;
	::GetClientRect(a_hWnd, &l_rectWin);

	const REmoData* l_pData = GetREmoData(a_hWnd);

	RTheme l_theme;
	if (l_theme.OpenData(a_hWnd, L"BUTTON") != NULL)
	{
		// horror! but don't have better idea, how to remove separator :-(
		if (IsTbStyle(a_hWnd))
		{
			RTheme l_themeTb;
			l_themeTb.OpenData(a_hWnd, L"REBAR");
			l_themeTb.DrawBackground(a_hDC, RP_BAND,
						0, &l_rectWin, NULL);
			l_themeTb.CloseData();
//			for (int l_y = 0; l_y <= RectHeight(l_rectWin); l_y++)
			{
//				::SetPixel(a_hDC, 3, l_y, ::GetPixel(a_hDC, 4, l_y));
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

	DrawEmo(a_hWnd, a_hDC);
}


//	---------------------------------------------------------------------------
//	Rejestracja klasy
//
ATOM 
REmoWnd_RegisterClass()
{
	WNDCLASSEX l_wcex;
	// Initialize global strings

	l_wcex.cbSize			= sizeof(WNDCLASSEX); 
	l_wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS | CS_PARENTDC;
	l_wcex.lpfnWndProc		= REmoWnd_WndProc;
	l_wcex.cbClsExtra		= 0;
	l_wcex.cbWndExtra		= sizeof(REmoData*);
	l_wcex.hInstance		= RCommon_GetInstance();
	l_wcex.hIcon			= NULL;
	l_wcex.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
	l_wcex.hbrBackground	= NULL;
	l_wcex.lpszMenuName		= NULL;
	l_wcex.lpszClassName	= REmoWnd_ClassName;
	l_wcex.hIconSm			= NULL;

	return ::RegisterClassEx(&l_wcex);
}


//	---------------------------------------------------------------------------
//	Utworzenie okna
//
HWND
REmoWnd_Create(
	HWND a_hWndParent
	)
{
	DWORD l_dwStyle = WS_THICKFRAME | WS_OVERLAPPED | WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | 0x08021;
	DWORD l_dwStyleEx = WS_EX_TOOLWINDOW;

	HWND l_hWnd = ::CreateWindowEx(l_dwStyleEx, REmoWnd_ClassName, _T(""), l_dwStyle, 
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
	REmoData* l_pData = GetREmoData(a_hWnd);
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
	REmoData* l_pData = GetREmoData(a_hWnd);
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
//	Draws emo
//
void DrawEmo(HWND a_hWnd, HDC a_hDC)
{
	HBITMAP l_hBmp = GetBitmap(a_hWnd);
	if (l_hBmp == NULL)
	{
		return;
	}
	REmoData* l_pData = GetREmoData(a_hWnd);

	HDC l_hDC = ::CreateCompatibleDC(a_hDC);
	HBITMAP l_hBmpOld = static_cast<HBITMAP>(::SelectObject(l_hDC, l_hBmp));

	RECT l_rectEmo;
	GetEmoRect(a_hWnd, &l_rectEmo);
	::TransparentBlt(a_hDC, l_rectEmo.left, l_rectEmo.top, RectWidth(l_rectEmo), RectHeight(l_rectEmo), 
		l_hDC, 0, 0, l_pData->m_sizeEmo.cx, l_pData->m_sizeEmo.cy, l_pData->m_clrBackBitmap);

	::SelectObject(l_hDC, l_hBmpOld);
	::DeleteDC(l_hDC);
}


//	---------------------------------------------------------------------------
//	Shows/hides dropped window
//
void 
DropDown(
	HWND a_hWnd,
	bool a_bDown	//IN TRUE - show dropped window
	)
{
	REmoData* l_pData = GetREmoData(a_hWnd);
	if (l_pData->m_hBitmap == NULL)
	{
		return;
	}
	HWND l_hDownWnd = l_pData->GetDownWnd();
	if (a_bDown)
	{
		if (l_hDownWnd == NULL)
		{
			REmoDownWnd_RegisterClass();
			l_hDownWnd = REmoDownWnd_Create(a_hWnd, l_pData);
			l_pData->SetDownWnd(l_hDownWnd);
		}
		RECT l_rect;
		::GetWindowRect(a_hWnd, &l_rect);
		REmoDownWnd_DropDown(l_hDownWnd, &l_rect);
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else
	{
		if (l_hDownWnd != NULL)
		{
			REmoDownWnd_Hide(l_hDownWnd);
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
	const REmoData* l_pData = GetREmoData(a_hWnd);
	return (::IsWindow(l_pData->GetDownWnd())) && (::IsWindowVisible(l_pData->GetDownWnd()));
}


//	---------------------------------------------------------------------------
//	sets emo and sends notification to parent
//
void
SetEmo(
	HWND  a_hWnd,         //IN window
	const TSelect& a_sel, //IN new emoticon
	bool  a_bNotify       //IN notify that color changed
	)
{
	REmoData* l_pData = GetREmoData(a_hWnd);
	if ((a_sel.iRow <= 0) || (a_sel.iRow > l_pData->m_iRows) ||
		(a_sel.iCol <= 0) || (a_sel.iCol > l_pData->m_iCols)
		)
	{
		return;
	}

	TSelect l_selOld;
	l_selOld.iRow = l_pData->m_sel.iRow;
	l_selOld.iCol = l_pData->m_sel.iCol;

	l_pData->m_sel.iRow = a_sel.iRow;
	l_pData->m_sel.iCol = a_sel.iCol;
	if (l_pData->m_hSelBitmap != NULL)
	{
		::DeleteObject(l_pData->m_hSelBitmap);
		l_pData->m_hSelBitmap = NULL;
	}

	if (a_bNotify)
	{
		Notify(a_hWnd, l_selOld, a_sel, RECN_CHANGED);
	}
}


//	---------------------------------------------------------------------------
//	notify parent about pushing button
//
void NotifyPushed(HWND a_hWnd)
{
	REmoData* l_pData = GetREmoData(a_hWnd);
	Notify(a_hWnd, l_pData->m_sel, l_pData->m_sel, RECN_PUSHED);
}


//	---------------------------------------------------------------------------
//	notify parent
//
void Notify(HWND a_hWnd, const TSelect& a_selOld, const TSelect& a_selNew, UINT a_iCode)
{
	long l_idCtrl = ::GetDlgCtrlID(a_hWnd);
	RENMCHG l_nmhdr;
	l_nmhdr.selOld.iRow = a_selOld.iRow;
	l_nmhdr.selOld.iCol = a_selOld.iCol;
	l_nmhdr.nmhdr.hwndFrom = a_hWnd;
	l_nmhdr.nmhdr.code = a_iCode;
	l_nmhdr.nmhdr.idFrom = l_idCtrl;
	l_nmhdr.selNew.iRow = a_selNew.iRow;
	l_nmhdr.selNew.iCol = a_selNew.iCol;

	HWND l_hParent = ::GetParent(a_hWnd);
	if (l_hParent == NULL)
	{
		l_hParent = a_hWnd;
	}
	::SendMessage(l_hParent, WM_NOTIFY, (WPARAM)l_idCtrl, (LPARAM)&l_nmhdr);
}


void OnSetTooltip(HWND a_hWnd, LPTSTR a_psTooltip)
{
	REmoData* l_pData = GetREmoData(a_hWnd);
	l_pData->m_psTooltip = a_psTooltip;
	if ((_tcslen(a_psTooltip) == 0) && (l_pData->m_hTooltip != NULL))
	{
		::DestroyWindow(l_pData->m_hTooltip);
	}
	else if (_tcslen(a_psTooltip) != 0)
	{
		if (l_pData->m_hTooltip == NULL)
		{
			INITCOMMONCONTROLSEX l_icc;
			l_icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
			l_icc.dwICC  = ICC_BAR_CLASSES;
			::InitCommonControlsEx(&l_icc);
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


BOOL OnSetStruct(HWND a_hWnd, LPTEmoStruct a_pEmoStruct)
{
	REmoData* l_pData = GetREmoData(a_hWnd);
	BITMAP l_bmp;
	::GetObject(a_pEmoStruct->hBitmap, sizeof(l_bmp), &l_bmp);
	// bitmap is too small!
	if (l_pData->m_hSelBitmap != NULL)
	{
		::DeleteObject(l_pData->m_hSelBitmap);
		l_pData->m_hSelBitmap = NULL;
	}

	if ((l_bmp.bmWidth < static_cast<LONG>(2 * a_pEmoStruct->iCols)) || (l_bmp.bmHeight < static_cast<LONG>(2 * a_pEmoStruct->iRows)))
	{
		l_pData->m_hBitmap = NULL;
		return FALSE;
	}
	l_pData->m_iRows = a_pEmoStruct->iRows;
	l_pData->m_iCols = a_pEmoStruct->iCols;
	l_pData->m_sizeEmo.cx = l_bmp.bmWidth / l_pData->m_iCols; 
	l_pData->m_sizeEmo.cy = l_bmp.bmHeight / l_pData->m_iRows; 
	l_pData->m_hBitmap = a_pEmoStruct->hBitmap;
	l_pData->m_clrBackBitmap = a_pEmoStruct->clrBackBitmap;

	l_pData->m_sel.iRow = 1;
	l_pData->m_sel.iCol = 1;
	return TRUE;
}


void GetEmoRect(HWND a_hWnd, LPRECT a_pRectEmo)
{
	REmoData* l_pData = GetREmoData(a_hWnd);

	RECT l_rectDraw;
	::GetClientRect(a_hWnd, &l_rectDraw);
	l_rectDraw.right = l_rectDraw.right - cc_dxTriangle - 2;

	// simplest case - emo lesser than draw rect
	if ((RectWidth(l_rectDraw) >= l_pData->m_sizeEmo.cx + 4) && (RectHeight(l_rectDraw) >= l_pData->m_sizeEmo.cy + 4))
	{
		// only center it
		a_pRectEmo->left = (RectWidth(l_rectDraw) - l_pData->m_sizeEmo.cx) / 2;
		a_pRectEmo->top = (RectHeight(l_rectDraw) - l_pData->m_sizeEmo.cy) / 2;
		a_pRectEmo->right = a_pRectEmo->left + l_pData->m_sizeEmo.cx;
		a_pRectEmo->bottom = a_pRectEmo->top + l_pData->m_sizeEmo.cy;
		return;
	}

	// we must keep proportions
	double l_fAspect = ((double)l_pData->m_sizeEmo.cx) / ((double)l_pData->m_sizeEmo.cy);
	// dx or dy as a base?
	double l_fWidthAspect = ((double)RectWidth(l_rectDraw) / (double)l_pData->m_sizeEmo.cx);
	double l_fHeightAspect = ((double)RectHeight(l_rectDraw) / (double)l_pData->m_sizeEmo.cy);
	if (l_fWidthAspect > l_fHeightAspect)
	{
		a_pRectEmo->top = 2;
		a_pRectEmo->bottom = RectHeight(l_rectDraw) - 2;
		UINT l_iWidth = static_cast<UINT>(RectHeight(*a_pRectEmo) * l_fAspect);
		a_pRectEmo->left = (RectWidth(l_rectDraw) - l_iWidth) / 2;
		a_pRectEmo->right = a_pRectEmo->left + l_iWidth;
	}
	else
	{
		a_pRectEmo->left = 2;
		a_pRectEmo->right = RectWidth(l_rectDraw) - 2;
		UINT l_iHeight = static_cast<UINT>(RectWidth(*a_pRectEmo) / l_fAspect);
		a_pRectEmo->top = (RectHeight(l_rectDraw) - l_iHeight) / 2;
		a_pRectEmo->bottom = a_pRectEmo->top + l_iHeight;
	}
}


HBITMAP GetBitmap(HWND a_hWnd)
{
	REmoData* l_pData = GetREmoData(a_hWnd);
	if (l_pData->m_hSelBitmap != NULL)
	{
		return l_pData->m_hSelBitmap;
	}

	if (l_pData->m_hBitmap == NULL)
	{
		return NULL;
	}

	l_pData->m_hSelBitmap = RDraw::GetBitmap(l_pData->m_hBitmap, 
		(l_pData->m_sel.iCol - 1) * l_pData->m_sizeEmo.cx, (l_pData->m_sel.iRow - 1) * l_pData->m_sizeEmo.cy,
		l_pData->m_sizeEmo.cx, l_pData->m_sizeEmo.cy);
	return l_pData->m_hSelBitmap;
}
