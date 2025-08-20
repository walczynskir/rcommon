// RColorDownWnd.cpp: implementation of the RColorDownWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "rstddef.h"
#include "RColorWnd.h"
#include "RColorDownWnd.h"
#include "RColorDownData.h"
#include "DrawUtl.h"
#include "RKeyboard.h"
#include <commdlg.h>
#include "RTheme.h"


static const long c_iWindowOfs = sizeof(RColorDownData*) - 4;


// messages
static inline void OnNcDestroy(HWND a_hWnd);
static inline void OnPaint(HWND a_hWnd);
static inline void OnGetMinMaxInfo(HWND a_hWnd, LPMINMAXINFO a_pMMI);
static inline void OnKeyDown(HWND a_hWnd, int a_iKey);
static inline void OnKeyUp(HWND a_hWnd, int a_iKey);
static inline void OnSysKeyDown(HWND a_hWnd, int a_iKey);
static inline void OnLButtonDown(HWND a_hWnd, LPPOINT a_pPoint);
static inline void OnLButtonUp(HWND a_hWnd, LPPOINT a_pPoint);
static inline void OnMouseMove(HWND a_hWnd, int a_xPos, int a_yPos);
static inline void OnShowWindow(HWND a_hWnd, BOOL a_bShow);
static inline void OnUpdateUIState(HWND a_hWnd, WORD a_iAction, WORD a_iElem);
static inline void OnActivateApp(HWND a_hWnd, bool a_bActivate);

static BOOL IsStdColor(HWND a_hWnd, COLORREF a_color);


static inline int GetColorWidth(HWND a_hWnd);
static inline int GetColorHeight(HWND a_hWnd);
static inline int GetWidth(HWND a_hWnd);
static inline int GetHeight(HWND a_hWnd);

static void GetButtonRect(HWND a_hWnd, LPRECT a_pRect);
static void GetNotStdColorRect(HWND a_hWnd, LPRECT a_pRect);

static inline void NotifyOwner(HWND a_hWnd, COLORREF a_color);
static void DropDown(HWND a_hWnd, BOOL a_bDown, LPCRECT a_pRect, COLORREF a_color);

//drawing procedures
static inline void DrawColorRects(HWND a_hWnd, HDC a_hDC);
static inline void DrawColorRect(HWND a_hWnd, HDC a_hDC, long a_x, long a_y, COLORREF a_dwColor);
static inline void DrawSep(HWND a_hWnd, HDC a_hDC);
static inline void DrawButton(HWND a_hWnd, HDC a_hDC);

// moving
static inline void MoveLeft(HWND a_hWnd);
static inline void MoveRight(HWND a_hWnd);
static inline void MoveUp(HWND a_hWnd);
static inline void MoveDown(HWND a_hWnd);
static inline void MoveTab(HWND a_hWnd);

static void SelectColor(HWND a_hWnd, COLORREF a_color);

static void InvalidateColor(HWND a_hWnd, COLORREF a_color);
static void InvalidateButton(HWND a_hWnd);

static void ColorToRect(HWND a_hWnd, COLORREF a_color, LPRECT a_pRect);
static void GetRowCol(HWND a_hWnd, COLORREF a_color, UINT* a_pRow, UINT* a_pCol);
static inline COLORREF RowColToColor(HWND a_hWnd, int a_iRow, int a_iCol);

static void SelectButton(HWND a_hWnd);
static void PushButton(HWND a_hWnd, RColorDownData::T_STATE a_state);

static void ShowColorDlg(HWND a_hWnd);

static inline BOOL PointOnColor(HWND a_hWnd, const LPPOINT a_pPoint, COLORREF* a_pColor);
static inline BOOL PointOnButton(HWND a_hWnd, const LPPOINT a_pPoint);
static void ColorIndexToRect(HWND a_hWnd, UINT a_iIdx, LPRECT a_pRect);

static inline RColorDownData* GetRColorDownData(HWND a_hWnd);

static LRESULT CALLBACK HookCreateProc(int a_nCode, WPARAM a_wParam, LPARAM a_lParam);
static HHOOK s_hHook = NULL;

#define IsWordStyle(a_hWnd) ((::GetWindowLong(a_hWnd, GWL_STYLE) & RCCS_WORDSTYLE) != 0)


static const int s_dxColor = 16;
static const int s_dyColor = 16;
static const int c_iSepSpace = 3;
static const int c_iButtonSpace = 2;
static const int c_dxButton = 52;
static const int c_iButtonHeight = 20;
static const int c_iRectSpace = 2;



//	---------------------------------------------------------------------------
//	Rejestracja klasy
//
ATOM 
RColorDownWnd_RegisterClass()
{


	WNDCLASSEX l_wcex;
	// Initialize global strings

	l_wcex.cbSize			= sizeof(WNDCLASSEX); 
	l_wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
	l_wcex.lpfnWndProc		= RColorDownWnd_WndProc;
	l_wcex.cbClsExtra		= 0;
	l_wcex.cbWndExtra		= sizeof(RColorDownData*);
	l_wcex.hInstance		= RCommon_GetInstance();
	l_wcex.hIcon			= NULL;
	l_wcex.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
	l_wcex.hbrBackground	= NULL;
	l_wcex.lpszMenuName		= NULL;
	l_wcex.lpszClassName	= RColorDownWnd_ClassName;
	l_wcex.hIconSm			= NULL;

	return ::RegisterClassEx(&l_wcex);
}


//	---------------------------------------------------------------------------
//	Utworzenie okna
//
HWND RColorDownWnd_Create(HWND a_hWndOwner, DWORD a_dwStyle, RColorData* a_pData)
{

	DWORD l_dwStyle = WS_THICKFRAME | WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | a_dwStyle & 0xFFFF;
	DWORD l_dwStyleEx = WS_EX_TOOLWINDOW;

	// because Windows sends WM_GETMINMAXINFO before WM_CREATE! :(
	s_hHook = ::SetWindowsHookEx(WH_CBT, HookCreateProc, ::GetModuleHandle(NULL), ::GetCurrentThreadId());
	HWND l_hWnd = ::CreateWindowEx(l_dwStyleEx, RColorDownWnd_ClassName, _T(""), l_dwStyle, 
		CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, ::GetDesktopWindow(), 0, RCommon_GetInstance(), a_pData);
	::UnhookWindowsHookEx(s_hHook);

	RColorDownData* l_pData = GetRColorDownData(l_hWnd);
	l_pData->SetOwner(a_hWndOwner);
	return l_hWnd;
}


//	---------------------------------------------------------------------------
//	Wyœwietlenie okna
//
void
RColorDownWnd_DropDown(
	HWND a_hWnd, 
	LPCRECT a_pRect,
	COLORREF a_color
	)
{
	DropDown(a_hWnd, TRUE, a_pRect, a_color);
}



//	---------------------------------------------------------------------------
//	Schowanie okna
//
void
RColorDownWnd_Hide(
	HWND a_hWnd
	)
{
	DropDown(a_hWnd, FALSE, NULL, 0);
}


//	---------------------------------------------------------------------------------------
//	Main RColorWnd procedure
//
LRESULT CALLBACK 
RColorDownWnd_WndProc(
	HWND a_hWnd, 
	UINT a_iMsg, 
	WPARAM a_wParam, 
	LPARAM a_lParam
	)
{	
	switch (a_iMsg) 
	{
		case WM_GETMINMAXINFO:
			OnGetMinMaxInfo(a_hWnd, reinterpret_cast<LPMINMAXINFO>(a_lParam));
			break;

		case WM_PAINT:
			OnPaint(a_hWnd);
			break;

		case WM_KEYUP:
			OnKeyUp(a_hWnd, (int)a_wParam);
			break;

		case WM_KEYDOWN:
			OnKeyDown(a_hWnd, (int)a_wParam);
			break;

		case WM_SYSKEYDOWN:
			OnSysKeyDown(a_hWnd, (int)a_wParam);
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

		case WM_UPDATEUISTATE:
			OnUpdateUIState(a_hWnd, HIWORD(a_wParam), LOWORD(a_wParam));
			break;

		default:
			return DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
   }
   return 0;
}


//	---------------------------------------------------------------------------------------
//	Returns control's data
//
RColorDownData*	GetRColorDownData(HWND a_hWnd)
{
#pragma warning(disable: 4312)
	return reinterpret_cast<RColorDownData*>(::GetWindowLongPtr(a_hWnd, c_iWindowOfs));
#pragma warning(default: 4312)
}


//	---------------------------------------------------------------------------------------
//	Sets control's data
//
static void	
SetRColorDownData(
	HWND a_hWnd,	//IN
	RColorDownData* a_pData	//IN
	)
{
#pragma warning(disable: 4244)
	::SetWindowLongPtr(a_hWnd, c_iWindowOfs, reinterpret_cast<LONG_PTR>(a_pData));
#pragma warning(default: 4244)
}


//	---------------------------------------------------------------------------------------
//	Postmortem:-)
//
void OnNcDestroy(HWND a_hWnd)
{
	RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	delete l_pData;
}


//	---------------------------------------------------------------------------
//	On WM_GETMINMAXINFO
//
void OnGetMinMaxInfo(HWND a_hWnd, LPMINMAXINFO a_pMMI)
{
	a_pMMI->ptMaxSize.x = GetWidth(a_hWnd);
	a_pMMI->ptMaxSize.y = GetHeight(a_hWnd);
	a_pMMI->ptMinTrackSize.x  = GetWidth(a_hWnd);
	a_pMMI->ptMinTrackSize.y  = GetHeight(a_hWnd);
	a_pMMI->ptMaxTrackSize.x  = GetWidth(a_hWnd);
	a_pMMI->ptMaxTrackSize.y  = GetHeight(a_hWnd);
}


//	---------------------------------------------------------------------------
//	WM_KEYUP
//
static void 
OnKeyUp(
	HWND a_hWnd, 
	int a_iKey
	)
{
	if (!::IsWindowVisible(a_hWnd))
	{
		return;
	}

	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	if (a_iKey == VK_RETURN)
	{
		NotifyOwner(a_hWnd, l_pData->GetSelColor());
		DropDown(a_hWnd, FALSE, NULL, 0);
	}
	else if (a_iKey == VK_LEFT)
	{
		MoveLeft(a_hWnd);
	}
	else if (a_iKey == VK_RIGHT)
	{
		MoveRight(a_hWnd);
	}
	else if (a_iKey == VK_UP)
	{
		MoveUp(a_hWnd);
	}
	else if (a_iKey == VK_DOWN)
	{
		MoveDown(a_hWnd);
	}
	else if (a_iKey == VK_TAB)
	{
		MoveTab(a_hWnd);
	}
	else if ((a_iKey == VK_SPACE) && l_pData->IsButtonSelected())
	{
		ShowColorDlg(a_hWnd);
	}
}


//	---------------------------------------------------------------------------
//	WM_KEYDOWN
//
static void 
OnKeyDown(
	HWND a_hWnd, 
	int a_iKey
	)
{
	if (!::IsWindowVisible(a_hWnd))
	{
		return;
	}

	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	if (l_pData->IsButtonClicked() || l_pData->IsButtonPressed())
	{
		return;
	}
	if ((a_iKey == VK_SPACE) && l_pData->IsButtonSelected())
	{
		PushButton(a_hWnd, RColorDownData::e_Pressed);
	}
}


//	---------------------------------------------------------------------------
//	OnSysKeyDown
//
static void 
OnSysKeyDown(
	HWND a_hWnd,
	int a_iKey
	)
{
	ASSERT(::IsWindowVisible(a_hWnd));
	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);

	if (a_iKey == l_pData->m_cAccell)
	{
		ShowColorDlg(a_hWnd);
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
	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);

	// if point not in rect, we are capturing mouse, so lbutton is down
	// so if not clicked on owner lets hide us
	if (!::PtInRect(&l_rect, l_pt))
	{
		// check if we clicked on ColorButton
		RECT l_rectOwner;
		::GetWindowRect(l_pData->GetOwner(), &l_rectOwner);
		if (!::PtInRect(&l_rectOwner, l_pt))
		{
			DropDown(a_hWnd, FALSE, NULL, 0);
		}
		return;
	}

	// clicked on button
	if (l_pData->IsButtonNormal())
	{
		if (PointOnButton(a_hWnd, a_pPoint))
		{
			PushButton(a_hWnd, RColorDownData::e_Clicked);
			return;
		}
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

	RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	// mo¿e odklikniêto na przycisk
	if (l_pData->IsButtonClicked())
	{
		if (PointOnButton(a_hWnd, a_pPoint))
		{
			l_pData->SetButtonState(RColorDownData::e_Normal | RColorDownData::e_Selected);
			InvalidateButton(a_hWnd);
			::UpdateWindow(a_hWnd);
			ShowColorDlg(a_hWnd);
			DropDown(a_hWnd, FALSE, NULL, 0);
		}
		else
		{
			l_pData->SetButtonState(RColorDownData::e_Normal | RColorDownData::e_Selected);
		}
		return;
	}

	COLORREF l_color;
	if (PointOnColor(a_hWnd, a_pPoint, &l_color))
	{
		NotifyOwner(a_hWnd, l_pData->GetSelColor());
		DropDown(a_hWnd, FALSE, NULL, 0);
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
	COLORREF l_color;
	POINT l_pt = {a_xPos, a_yPos};
	RColorDownData* l_pData = GetRColorDownData(a_hWnd);

	if (l_pData->IsButtonPressed())
	{
		return;
	}
	if (l_pData->IsButtonClicked())
	{
		if (!PointOnButton(a_hWnd, &l_pt) && l_pData->IsButtonPushed())
		{
			l_pData->SetButtonState(RColorDownData::e_Normal | RColorDownData::e_Selected | RColorDownData::e_Clicked);
			InvalidateButton(a_hWnd);			
		}
		else if (PointOnButton(a_hWnd, &l_pt) && !l_pData->IsButtonPushed())
		{
			l_pData->SetButtonState(RColorDownData::e_Pushed | RColorDownData::e_Selected | RColorDownData::e_Clicked);
			InvalidateButton(a_hWnd);			
		}
		::UpdateWindow(a_hWnd);
		return;
	}
	if (PointOnColor(a_hWnd, &l_pt, &l_color))
	{
		if (l_color != l_pData->GetSelColor())
		{
			InvalidateColor(a_hWnd, l_color);
			InvalidateColor(a_hWnd, l_pData->GetSelColor());
			l_pData->SetSelColor(l_color);
			::UpdateWindow(a_hWnd);
		}
		return;
	}
	if (PointOnButton(a_hWnd, &l_pt) && !l_pData->IsHovering())
	{
		l_pData->SetHovering(true);
		InvalidateButton(a_hWnd);
		::UpdateWindow(a_hWnd);
		return;
	}
	if (!PointOnButton(a_hWnd, &l_pt) && l_pData->IsHovering())
	{
		l_pData->SetHovering(false);
		InvalidateButton(a_hWnd);
		::UpdateWindow(a_hWnd);
		return;
	}

	
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
		RColorDownData* l_pData = GetRColorDownData(a_hWnd);
		::PostMessage(l_pData->GetOwner(), CDM_HIDEDROP, 0, 0);
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
		COLORREF l_clrBk = IsWordStyle(a_hWnd) ? RGB(246, 246, 246) :  ::GetSysColor(COLOR_BTNFACE);
		RDraw::FillSolidRect(l_hMemDC, l_rectClip, l_clrBk);
	}
	// real drawings
	DrawColorRects(a_hWnd, l_hMemDC);
	DrawSep(a_hWnd, l_hMemDC);
	DrawButton(a_hWnd, l_hMemDC);

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
//	Draws color rectangles
//
void DrawColorRects(HWND a_hWnd, HDC a_hDC)
{
	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	UINT l_iElems = l_pData->GetRows() * l_pData->GetColumns();
	UINT l_iAt;
	for (l_iAt = 0; l_iAt < l_iElems; l_iAt++)
	{
		RECT l_rect;
		ColorIndexToRect(a_hWnd, l_iAt, &l_rect);

		DrawColorRect(a_hWnd, a_hDC, l_rect.left, l_rect.top, 
			l_pData->GetColor(l_iAt));
	}
	if (!IsStdColor(a_hWnd, l_pData->GetColor()))
	{
		RECT l_rect;
		GetNotStdColorRect(a_hWnd, &l_rect);
		DrawColorRect(a_hWnd, a_hDC, l_rect.left, l_rect.top, 
			l_pData->GetColor());
	}
}


//	---------------------------------------------------------------------------
//	Draws one color
//
void DrawColorRect(HWND a_hWnd,	HDC a_hDC, long a_x, long a_y, COLORREF a_dwColor)
{

	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	RECT l_rect = {a_x, a_y, a_x + s_dxColor, a_y + s_dyColor};
	if  (a_dwColor != l_pData->GetSelColor())
	{
		RDraw::DrawSunkenRect(a_hDC, l_rect);
	}
	else
	{
		RECT l_rectSel = l_rect;
		::InflateRect(&l_rectSel, 1, 1);
		RDraw::Draw3DRect(a_hDC, l_rectSel, RGB(0, 0, 0), RGB(0, 0, 0));
		::InflateRect(&l_rectSel, -1, -1);
		RDraw::Draw3DRect(a_hDC, l_rectSel, RGB(255, 255, 255), RGB(255, 255, 255));
		::InflateRect(&l_rectSel, -1, -1);
		RDraw::Draw3DRect(a_hDC, l_rectSel, RGB(0, 0, 0), RGB(0, 0, 0));
	}

	::InflateRect(&l_rect, -2, -2);
	RDraw::FillSolidRect(a_hDC, l_rect, a_dwColor);
}


//	---------------------------------------------------------------------------
//	Draws separating line
//
void DrawSep(HWND a_hWnd, HDC a_hDC)
{
	POINT l_ptStart;
	l_ptStart.x = 0;
	l_ptStart.y = GetColorHeight(a_hWnd) + c_iSepSpace;
	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);
	RDraw::DrawSunkenLine(a_hDC, l_ptStart, RectWidth(l_rect), FALSE);
}


//	---------------------------------------------------------------------------
//	Draws button
//
void DrawButton(HWND a_hWnd, HDC a_hDC)
{
	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	RECT l_rect;
	GetButtonRect(a_hWnd, &l_rect);

	l_rect.left = 0;
	l_rect.right = l_rect.left + c_dxButton;
	l_rect.top = GetColorHeight(a_hWnd) + c_iSepSpace + 2 + c_iButtonSpace;
	l_rect.bottom = l_rect.top + c_iButtonHeight;

	HFONT l_hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	ASSERT(l_hFont != NULL);
	HFONT l_hFontOld = (HFONT)::SelectObject(a_hDC, l_hFont);

	RTheme l_theme;
	DWORD l_dwTextFlags = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
	if (!l_pData->m_bShowPrefix)
	{
		l_dwTextFlags |= DT_HIDEPREFIX;
	}

	if (l_theme.OpenData(a_hWnd, L"BUTTON") != NULL)
	{
		int l_iState;
		if (l_pData->IsButtonNormal())
		{
			if (l_pData->IsHovering())
			{
				l_iState = PBS_HOT;
			}
			else
			{
				l_iState = PBS_NORMAL;
			}
		}
		else 
		{
			l_iState = PBS_PRESSED;
		}

		HRESULT l_hr = l_theme.DrawBackground(a_hDC, BP_PUSHBUTTON,
				l_iState, &l_rect, NULL);
		RECT l_rectContent;
		l_hr = l_theme.GetBackgroundContentRect(a_hDC, BP_PUSHBUTTON, 
				l_iState, &l_rect, &l_rectContent);

		l_hr = l_theme.DrawText(a_hDC, BP_PUSHBUTTON, l_iState,
			l_pData->m_sBtnText, static_cast<int>(_tcslen(l_pData->m_sBtnText)), 
			l_dwTextFlags, 0, &l_rectContent);
	}
	else
	{
		RDraw::DrawButtonBorder(a_hDC, l_rect, !l_pData->IsButtonNormal(), false, false, false);
		int l_iBkMode = ::SetBkMode(a_hDC, TRANSPARENT);


		l_rect.top += 2;

		::DrawText(a_hDC, l_pData->m_sBtnText, static_cast<int>(_tcslen(l_pData->m_sBtnText)), &l_rect, l_dwTextFlags);
		::SetBkMode(a_hDC, l_iBkMode);

	}


	::SelectObject(a_hDC, l_hFontOld);
	::DeleteObject(l_hFont);
	if (l_pData->IsButtonSelected())
	{
		RECT l_rectFocus = l_rect; 
		::InflateRect(&l_rectFocus, -2, -2);
		::DrawFocusRect(a_hDC, &l_rectFocus);
	}
}


//	---------------------------------------------------------------------------
//	Chcecks if color is in colors table
//
BOOL IsStdColor(HWND a_hWnd, COLORREF a_color)
{
	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	UINT l_iAt;
	for (l_iAt = 0; l_iAt < l_pData->GetColorsCount(); l_iAt++)
	{
		if (l_pData->GetColor(l_iAt) == a_color)
		{
			return TRUE;
		}
	}
	return FALSE;
}



//	---------------------------------------------------------------------------
//	Calculates 
//
int GetColorWidth(HWND a_hWnd)
{
	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	return 
		l_pData->GetColumns() * s_dxColor +			
		(l_pData->GetColumns() - 1) * c_iRectSpace;	
}


//	---------------------------------------------------------------------------
//	Calculates
//
int GetColorHeight(HWND a_hWnd)
{
	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	return 
		l_pData->GetRows() * s_dyColor +				
		(l_pData->GetRows() - 1) * c_iRectSpace;		
}


//	---------------------------------------------------------------------------
//	Calculates button rect
//
void GetButtonRect(HWND a_hWnd, LPRECT a_pRect)
{
	a_pRect->top = GetColorHeight(a_hWnd) + c_iSepSpace + 2 + c_iButtonSpace;
	a_pRect->left = 0;
	a_pRect->bottom = a_pRect->top + c_iButtonHeight;
	a_pRect->right = a_pRect->left + c_dxButton;
}


//	---------------------------------------------------------------------------
//	Sends info about color change to its owner
void 
NotifyOwner(
	HWND a_hWnd,
	COLORREF a_color
	)
{
	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	::SendMessage(l_pData->GetOwner(), CDM_COLORCHANGED, (WPARAM)a_color, 0);
}


//	---------------------------------------------------------------------------
//	Shows or hides window
//
void DropDown(HWND a_hWnd, BOOL a_bDown, LPCRECT a_pRect, COLORREF a_color)
{
	RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	if (a_bDown)
	{
		if (!::IsWindowVisible(a_hWnd))
		{
			l_pData->SetColor(a_color);
			l_pData->SetSelColor(a_color);
			_ASSERT(a_pRect != NULL);
			::SetWindowPos(a_hWnd, HWND_TOPMOST, a_pRect->left, a_pRect->bottom, 
				0, 0, SWP_SHOWWINDOW);
			::SetCapture(a_hWnd);
		}
	}
	else
	{
		if (::IsWindowVisible(a_hWnd))
		{
			::ReleaseCapture();
			::ShowWindow(a_hWnd, SW_HIDE);
			l_pData->SetButtonState(RColorDownData::e_Normal);
		}
	}
}


//	---------------------------------------------------------------------------
//	Moves selected color left
//
void MoveLeft(HWND a_hWnd)
{
	RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	if (l_pData->IsButtonSelected())
	{
		SelectColor(a_hWnd, l_pData->GetColorsCount() - 1);
	}
	else
	{
		if (IsStdColor(a_hWnd, l_pData->GetSelColor()))
		{
			UINT l_iRow;
			UINT l_iCol;
			GetRowCol(a_hWnd, l_pData->GetSelColor(), &l_iRow, &l_iCol);
			if (l_iCol == 1)
			{
				l_iCol = l_pData->GetColumns();
			}
			else
			{
				l_iCol--;
			}
			SelectColor(a_hWnd, RowColToColor(a_hWnd, l_iRow, l_iCol));
		}
		else
		{
			SelectButton(a_hWnd);
		}
	}
}


//	---------------------------------------------------------------------------
//	Move right
//
static void
MoveRight(
	HWND a_hWnd
	)
{
	RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	if (l_pData->IsButtonSelected())
	{
		SelectColor(a_hWnd, l_pData->GetColor(0));
	}
	else
	{
		UINT l_iRow;
		UINT l_iCol;
		GetRowCol(a_hWnd, l_pData->GetSelColor(), &l_iRow, &l_iCol);
		if (l_iCol == l_pData->GetColumns())
		{
			l_iCol = 1;
		}
		else
		{
			l_iCol++;
		}
		SelectColor(a_hWnd, RowColToColor(a_hWnd, l_iRow, l_iCol));
	}
}


//	---------------------------------------------------------------------------
//	Moves up
//
static void
MoveUp(
	HWND a_hWnd
	)
{
	RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	if (l_pData->IsButtonSelected())
	{
		SelectColor(a_hWnd, l_pData->GetColorsCount() - 1);
	}
	else
	{
		UINT l_iRow;
		UINT l_iCol;
		if (IsStdColor(a_hWnd, l_pData->GetSelColor()))
		{
			GetRowCol(a_hWnd, l_pData->GetSelColor(), &l_iRow, &l_iCol);
			if (l_iRow == 1)
			{
				l_iRow = l_pData->GetRows();
			}
			else
			{
				l_iRow--;
			}
		}
		else
		{
			l_iRow = l_pData->GetRows();
			l_iCol = l_pData->GetColumns();
		}
		SelectColor(a_hWnd, RowColToColor(a_hWnd, l_iRow, l_iCol));
	}
}


//	---------------------------------------------------------------------------
//	Moves down
//
void MoveDown(HWND a_hWnd)
{
	RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	if (l_pData->IsButtonSelected())
	{
		SelectColor(a_hWnd, l_pData->GetColor(0));
	}
	else
	{
		UINT l_iRow;
		UINT l_iCol;
		if (IsStdColor(a_hWnd, l_pData->GetSelColor()))
		{
			GetRowCol(a_hWnd, l_pData->GetSelColor(), &l_iRow, &l_iCol);
			if (l_iRow == l_pData->GetRows())
			{
				l_iRow = 1;
			}
			else
			{
				l_iRow++;
			}
		}
		else
		{
			l_iRow = 1;
			l_iCol = 1;
		}
		SelectColor(a_hWnd, RowColToColor(a_hWnd, l_iRow, l_iCol));
	}
}


//	---------------------------------------------------------------------------
//	Ruch po TAB-ie
//
static void
MoveTab(
	HWND a_hWnd
	)
{
	BOOL l_bShift = IsShiftPressed();

	RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	if (l_pData->IsButtonSelected())
	{
		if (l_bShift)
		{
			SelectColor(a_hWnd, RowColToColor(a_hWnd, 1, 1));
		}
		else
		{
			SelectColor(a_hWnd, l_pData->GetSelColor());
		}
	}
	else
	{
		if (IsStdColor(a_hWnd, l_pData->GetSelColor()))
		{
			SelectButton(a_hWnd);		
		}
		else
		{
			if (l_bShift)
			{
				SelectButton(a_hWnd);
			}
			else
			{
				SelectColor(a_hWnd, RowColToColor(a_hWnd, l_pData->GetRows(), l_pData->GetColumns()));
			}
		}
	}
}


//	---------------------------------------------------------------------------
//	Color selecting
//
void SelectColor(HWND a_hWnd, COLORREF a_color)
{

	RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	InvalidateColor(a_hWnd, a_color);
	InvalidateColor(a_hWnd, l_pData->GetSelColor());
	l_pData->SetSelColor(a_color);
	if (l_pData->IsButtonSelected())
	{
		InvalidateButton(a_hWnd);
	}
	l_pData->SetButtonState(RColorDownData::e_Normal);

	::UpdateWindow(a_hWnd);
}


//	---------------------------------------------------------------------------
//	Color invalidation
//
static void
InvalidateColor(
	HWND a_hWnd, 
	COLORREF a_color
	)
{
	RECT l_rectColor;
	ColorToRect(a_hWnd, a_color, &l_rectColor);

	::InflateRect(&l_rectColor, 1, 1);
	::InvalidateRect(a_hWnd, &l_rectColor, FALSE);
}


//	---------------------------------------------------------------------------
//	Invalidates button
//
static void
InvalidateButton(
	HWND a_hWnd
	)
{
	RECT l_rect;
	GetButtonRect(a_hWnd, &l_rect);
	::InvalidateRect(a_hWnd, &l_rect, FALSE);
}


//	---------------------------------------------------------------------------
//	Transforms color to rect
//
void ColorToRect(HWND a_hWnd, COLORREF a_color, LPRECT a_pRect)
{
	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	UINT l_iAt;
	for (l_iAt = 0; l_iAt < l_pData->GetColorsCount(); l_iAt++)
	{
		if (l_pData->GetColor(l_iAt) == a_color)
		{
			ColorIndexToRect(a_hWnd, l_iAt, a_pRect);
			return;
		}
	}

	GetNotStdColorRect(a_hWnd, a_pRect);
}



//	---------------------------------------------------------------------------
//	Calculates row and column for given color
//
void GetRowCol(HWND a_hWnd, COLORREF a_color, UINT* a_pRow, UINT* a_pCol)
{
	_ASSERT(a_pRow != NULL);
	_ASSERT(a_pCol != NULL);
	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);

	UINT l_iAt;
	for (l_iAt = 0; l_iAt < l_pData->GetColorsCount(); l_iAt++)
	{
		if (l_pData->GetColor(l_iAt) == a_color)
		{
			*a_pRow = l_iAt / l_pData->GetColumns() + 1;
			*a_pCol = l_iAt % l_pData->GetColumns() + 1;
			return;
		}
	}
	_ASSERT(FALSE);	
}


//	---------------------------------------------------------------------------
//	returns color for given row and column
//
COLORREF RowColToColor(HWND a_hWnd, int a_iRow, int a_iCol)
{
	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	int l_iAt = (a_iRow - 1) * l_pData->GetColumns() + a_iCol - 1;

	return l_pData->GetColor(l_iAt);
}


//	---------------------------------------------------------------------------
//	Selects button
//
void SelectButton(HWND a_hWnd)
{
	RColorDownData* l_pData = GetRColorDownData(a_hWnd);

	l_pData->SetButtonState(RColorDownData::e_Normal | RColorDownData::e_Selected);
	InvalidateColor(a_hWnd, l_pData->GetSelColor());
	InvalidateButton(a_hWnd);
	::UpdateWindow(a_hWnd);
}


//	---------------------------------------------------------------------------
//	Wciœniêcie przycisku
//
static void
PushButton(
	HWND a_hWnd,
	RColorDownData::T_STATE a_state		// e_Clicked or e_Pressed
	)
{
	RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	l_pData->SetButtonState(static_cast<BYTE>(RColorDownData::e_Pushed | RColorDownData::e_Selected | a_state));
	InvalidateButton(a_hWnd);
	::UpdateWindow(a_hWnd);
}


//	---------------------------------------------------------------------------
// Shows color dialog
//
void ShowColorDlg(HWND a_hWnd)
{

	RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	DropDown(a_hWnd, FALSE, NULL, 0);

	CHOOSECOLOR l_chooseColor;
	memset(&l_chooseColor, 0, sizeof(l_chooseColor));
	l_chooseColor.lStructSize = sizeof(l_chooseColor);
	l_chooseColor.hwndOwner = l_pData->GetOwner();
	l_chooseColor.hInstance = NULL;
	l_chooseColor.rgbResult = l_pData->GetColor();
	l_chooseColor.lpCustColors = l_pData->GetSavedColors();
	l_chooseColor.Flags = CC_ANYCOLOR | CC_RGBINIT | CC_FULLOPEN;
	l_chooseColor.lCustData = NULL;
	l_chooseColor.lpfnHook = NULL;
	l_chooseColor.lpTemplateName = NULL;
	if (!ChooseColor(&l_chooseColor))
	{
		return;
	}

	NotifyOwner(a_hWnd, l_chooseColor.rgbResult);
}


//	---------------------------------------------------------------------------
//	Is point on color and which
//
BOOL PointOnColor(HWND a_hWnd, const LPPOINT a_pPoint, COLORREF* a_pColor)
{
	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	UINT l_iAt;
	for (l_iAt = 0; l_iAt < l_pData->GetColorsCount(); l_iAt++)
	{
		RECT l_rect;
		ColorIndexToRect(a_hWnd, l_iAt, &l_rect);
		if (::PtInRect(&l_rect, *a_pPoint))
		{
			*a_pColor = l_pData->GetColor(l_iAt);
			return TRUE;
		}
	}
	// maybe it is on not std color

	RECT l_rect;
	GetNotStdColorRect(a_hWnd, &l_rect);
	if (!IsStdColor(a_hWnd, l_pData->GetColor()) && ::PtInRect(&l_rect, *a_pPoint))
	{
		*a_pColor = l_pData->GetColor();
		return TRUE;
	}
	return FALSE;
}


//	---------------------------------------------------------------------------
//	Is point on button
//
BOOL PointOnButton(HWND a_hWnd,	const LPPOINT a_pPoint)
{
	RECT l_rect;
	GetButtonRect(a_hWnd, &l_rect);
	return ::PtInRect(&l_rect, *a_pPoint);
}


//	---------------------------------------------------------------------------
//	Obliczenie szerokoœci
//
int GetWidth(HWND a_hWnd)
{
//	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	return ::GetSystemMetrics(SM_CYFRAME) * 2 + max(
		GetColorWidth(a_hWnd) + c_iRectSpace, 
		c_dxButton + s_dxColor + 2 * c_iRectSpace);										
}


//	---------------------------------------------------------------------------
//	Obliczenie wysokoœci
//
int GetHeight(HWND a_hWnd)
{
	return 
		2 +									// na obwódki wybranych kolorów
		GetColorHeight(a_hWnd) +					// kolory
		c_iSepSpace +						// przestrzeñ do linii
		2 +									// linia
		c_iButtonSpace +					// przestrzeñ do przycisku
		c_iButtonHeight +					// wysokoœæ przycisku
		2 * ::GetSystemMetrics(SM_CYFRAME);	// ramka
}


//	---------------------------------------------------------------------------
//	returns not std color rect
//
void GetNotStdColorRect(HWND a_hWnd, LPRECT a_pRect)
{
	RECT l_rectButton;
	GetButtonRect(a_hWnd, &l_rectButton);
	RECT l_rect;
//	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
//	ColorIndexToRect(a_hWnd, l_pData->GetColumns() - 1, &l_rect);
	::GetClientRect(a_hWnd, &l_rect);
	a_pRect->left = l_rect.right - s_dxColor - c_iRectSpace;
	a_pRect->top = l_rectButton.top + c_iRectSpace;
	a_pRect->right = a_pRect->left + s_dxColor;
	a_pRect->bottom = a_pRect->top + s_dyColor;
}


//	---------------------------------------------------------------------------
//	OnUpdateUIState
//
void OnUpdateUIState(HWND a_hWnd, WORD a_iAction, WORD a_iElem)
{
	if ((a_iAction == UIS_INITIALIZE) && (a_iElem == UISF_HIDEACCEL))
	{
		GetRColorDownData(a_hWnd)->m_bShowPrefix = true;
		InvalidateButton(a_hWnd);
		::UpdateWindow(a_hWnd);
	}
}


void OnActivateApp(HWND a_hWnd, bool a_bActivate)
{
	if (!a_bActivate)
	{
		DropDown(a_hWnd, false, NULL, 0);
	}
}


void ColorIndexToRect(HWND a_hWnd, UINT a_iIdx, LPRECT a_pRect)
{
	const RColorDownData* l_pData = GetRColorDownData(a_hWnd);
	UINT l_iCol = a_iIdx % l_pData->GetColumns();
	UINT l_iRow = a_iIdx / l_pData->GetColumns();
	a_pRect->left = l_iCol * (s_dxColor + c_iRectSpace) + 1;
	a_pRect->top = l_iRow * (s_dyColor + c_iRectSpace) + 1;
	a_pRect->right = a_pRect->left + s_dxColor + c_iRectSpace;
	a_pRect->bottom = a_pRect->top + s_dyColor + c_iRectSpace;
}


LRESULT CALLBACK HookCreateProc(int a_nCode, WPARAM a_wParam, LPARAM a_lParam)
{
	if ((a_nCode != HCBT_CREATEWND) || (s_hHook == NULL))
	{
		return ::CallNextHookEx(s_hHook, a_nCode, a_wParam, a_lParam);
	}

	LPCBT_CREATEWND l_pCbt = reinterpret_cast<LPCBT_CREATEWND>(a_lParam);
	const RColorData* l_pData = reinterpret_cast<RColorData*>(l_pCbt->lpcs->lpCreateParams);
	RColorDownData* l_pDownData = new RColorDownData(l_pData);
	SetRColorDownData(reinterpret_cast<HWND>(a_wParam), l_pDownData);
	return ::CallNextHookEx(s_hHook, a_nCode, a_wParam, a_lParam);
}


