// CellTip.cpp : Defines the CellTip window
//

#include "stdafx.h"
#include "RTipData.h"
#include "celltip.h"
#include "DrawUtl.h"
#include "RMemDC.h"
#include "rstddef.h"
#include <commctrl.h>
#include "RSystemExc.h"
#include <crtdbg.h>


static const long c_iWindowOfs = sizeof(RTipData*) - 4;

#define IsConstDx(a_hWnd)     (HasWindowStyle(a_hWnd, RCTS_CONSTDX))
#define IsConstDy(a_hWnd)     (HasWindowStyle(a_hWnd, RCTS_CONSTDY))
#define IsConstSize(a_hWnd)   (IsConstDx(a_hWnd) && IsConstDy(a_hWnd))
#define IsFrame(a_hWnd)       (HasWindowStyle(a_hWnd, RCTS_FRAME))
#define IsAnimated(a_hWnd)    (HasWindowStyle(a_hWnd, RCTS_ANIMATE))


// messages
static inline BOOL	OnCreate(HWND a_hWnd);
static inline void	OnNcDestroy(HWND a_hWnd);
static inline void	OnPaint(HWND a_hWnd);
static inline void	OnPrintClient(HWND a_hWnd, HDC a_hDC);
static inline void  OnTimer(HWND a_hWnd, UINT a_idTimer);

static inline void  OnSetFont(HWND a_hWnd, HFONT a_hFont, bool a_bRedraw);
static inline void  OnSetText(HWND a_hWnd, LPCTSTR a_sText);
static inline LRESULT OnGetFont(HWND a_hWnd);
static inline LRESULT OnGetText(HWND a_hWnd, int a_iMax, LPTSTR a_psText);
static inline LRESULT OnGetTextLength(HWND a_hWnd);

static inline void    OnSetBkColor(HWND a_hWnd, COLORREF a_clr);
static inline LRESULT OnGetBkColor(HWND a_hWnd);
static inline void    OnSetTextColor(HWND a_hWnd, COLORREF a_clr);
static inline LRESULT OnGetTextColor(HWND a_hWnd);

static inline void    OnSetSize(HWND a_hWnd, const LPSIZE a_pSize);
static inline void    OnGetSize(HWND a_hWnd, LPSIZE a_pSize);
static inline void    OnPopUp(HWND a_hWnd, const LPPOINT a_pPt);
static inline void    OnMove(HWND a_hWnd, const LPPOINT a_pPt);
static inline void    OnPop(HWND a_hWnd);
static inline void	  OnCalculateSize(HWND a_hWnd);
static inline void	  OnSetDelay(HWND a_hWnd, const LPRTIPDELAY a_pDelay);
static inline void    OnGetDelay(HWND a_hWnd, LPRTIPDELAY a_pDelay);

static inline void	  Draw(HWND a_hWnd, HDC a_hDC);
static void ShowTip(HWND a_hWnd);
static void HideTip(HWND a_hWnd);

static void CalculateTipSize(HWND a_hWnd, bool a_bForce);
static RTipData* GetRCellTipData(HWND a_hWnd);
static inline void SetRCellTipData(HWND a_hWnd, RTipData* a_pData);

static const int cc_idTimerShow = 1;
static const int cc_idTimerHide = 2;

//	---------------------------------------------------------------------------------------
//	Registering celltip
//
ATOM RCellTip_Register()
{
	WNDCLASSEX l_wndClassEx;

	l_wndClassEx.cbSize = sizeof(WNDCLASSEX); 
	l_wndClassEx.style         = CS_OWNDC | CS_GLOBALCLASS;
	l_wndClassEx.lpfnWndProc   = RCellTip_WndProc;
	l_wndClassEx.cbClsExtra    = 0;
	l_wndClassEx.cbWndExtra	   = sizeof(RTipData*);;
	l_wndClassEx.hInstance     = RCommon_GetInstance();
	l_wndClassEx.hIcon         = NULL;
	l_wndClassEx.hCursor       = ::LoadCursor(NULL, IDC_ARROW);
	l_wndClassEx.hbrBackground = (HBRUSH)(COLOR_INFOBK + 1);
	l_wndClassEx.lpszMenuName  = NULL;
	l_wndClassEx.lpszClassName = RCellTip_ClassName;
	l_wndClassEx.hIconSm	   = NULL;

	return ::RegisterClassEx(&l_wndClassEx);
}


//	---------------------------------------------------------------------------------------
//	Creates celltip
//
HWND RCellTip_Create(HWND a_hWndOwner, DWORD a_dwStyle)
{
	ASSERT(a_hWndOwner != NULL);

 	DWORD l_dwStyle = WS_POPUP | a_dwStyle; 
	DWORD l_dwExStyle = WS_EX_TOPMOST | WS_EX_NOACTIVATE;

	HWND l_hWnd = ::CreateWindowEx(l_dwExStyle, RCellTip_ClassName, NULL, l_dwStyle, 
		CW_USEDEFAULT, 1, CW_USEDEFAULT, 1, 
		NULL, NULL, RCommon_GetInstance(), NULL);

	ASSERT(l_hWnd != NULL);

	// we are just after WM_CREATE, so RTipData already exists

	RTipData* l_pData = GetRCellTipData(l_hWnd);
	l_pData->SetOwnerWnd(a_hWndOwner);
	return l_hWnd;

}


//	---------------------------------------------------------------------------------------
//	Main window procedure
//
LRESULT CALLBACK 
RCellTip_WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam)
{
	switch (a_iMsg)
	{
	case WM_CREATE:
		if (!OnCreate(a_hWnd))
		{
			return -1;
		}
		break;

	case WM_PAINT:
		OnPaint(a_hWnd);
		break;

	case WM_PRINTCLIENT:
		OnPrintClient(a_hWnd, reinterpret_cast<HDC>(a_wParam));
		break;

	case WM_NCDESTROY:
		OnNcDestroy(a_hWnd);
		break;

	case WM_TIMER:
		OnTimer(a_hWnd, static_cast<UINT>(a_wParam));
		break;

	case WM_SETFONT:
		OnSetFont(a_hWnd, reinterpret_cast<HFONT>(a_wParam), (LOWORD(a_lParam) == TRUE));
		break;

	case WM_SETTEXT:
		OnSetText(a_hWnd, reinterpret_cast<LPCTSTR>(a_lParam));
		break;

	case WM_GETFONT:
		return OnGetFont(a_hWnd);

	case WM_GETTEXT:
		return OnGetText(a_hWnd, static_cast<int>(a_wParam), reinterpret_cast<LPTSTR>(a_lParam));

	case WM_GETTEXTLENGTH:
		return OnGetTextLength(a_hWnd);

	case RCTM_SETBKCOLOR:
		OnSetBkColor(a_hWnd, static_cast<COLORREF>(a_lParam));
		break;

	case RCTM_GETBKCOLOR:
		return OnGetBkColor(a_hWnd);

	case RCTM_SETTEXTCOLOR:
		OnSetTextColor(a_hWnd, static_cast<COLORREF>(a_lParam));
		break;

	case RCTM_GETTEXTCOLOR:
		return OnGetTextColor(a_hWnd);

	case RCTM_SETSIZE:
		OnSetSize(a_hWnd, reinterpret_cast<const LPSIZE>(a_lParam));
		break;

	case RCTM_GETSIZE:
		OnGetSize(a_hWnd, reinterpret_cast<LPSIZE>(a_lParam));
		break;

	case RCTM_POP:
		OnPop(a_hWnd);
		break;

	case RCTM_POPUP:
		OnPopUp(a_hWnd, reinterpret_cast<const LPPOINT>(a_lParam));
		break;

	case RCTM_MOVE:
		OnMove(a_hWnd, reinterpret_cast<const LPPOINT>(a_lParam));
		break;

	case RCTM_CALCULATESIZE:
		OnCalculateSize(a_hWnd);
		break;

	case RCTM_SETDELAY:
		OnSetDelay(a_hWnd, reinterpret_cast<LPRTIPDELAY>(a_lParam));
		break;

	case RCTM_GETDELAY:
		OnGetDelay(a_hWnd, reinterpret_cast<LPRTIPDELAY>(a_lParam));
		break;

	default:
		return ::DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
	}
	return 0;
}


//	---------------------------------------------------------------------------------------
//	returns celltip data 
//
RTipData*	//OUT data
GetRCellTipData(
	HWND a_hWnd	
	)
{
#pragma warning(disable: 4312)
	return reinterpret_cast<RTipData*>(::GetWindowLongPtr(a_hWnd, c_iWindowOfs));
#pragma warning(default: 4312)
}


//	---------------------------------------------------------------------------------------
//	sets celltip data
//
void	
SetRCellTipData(
	HWND a_hWnd,	
	RTipData* a_pData	
	)
{
#pragma warning(disable: 4244)
	::SetWindowLongPtr(a_hWnd, c_iWindowOfs, reinterpret_cast<LONG_PTR>(a_pData));
#pragma warning(default: 4244)
}


/*	---------------------------------------------------------------------------------------
	WM_CREATE handler
*/
BOOL	//OUT TRUE - success
OnCreate(
	HWND a_hWnd
	)
{
	RTipData* l_pData = new RTipData();
	SetRCellTipData(a_hWnd, l_pData);
	return TRUE;
}


//	---------------------------------------------------------------------------------------
//	Postmortem:-)
//
void
OnNcDestroy(
	HWND a_hWnd	
	)
{
	RTipData* l_pData = GetRCellTipData(a_hWnd);
	delete l_pData;
}


//	---------------------------------------------------------------------------------------
//	Drawing - WM_PAINT message handler (all drawing here)
//
void
OnPaint(
	HWND a_hWnd
	)
{
	PAINTSTRUCT l_ps;
	HDC l_hdc = ::BeginPaint(a_hWnd, &l_ps);

	{	// to allow destruction of RMemDC
#ifdef _DEBUG
	HDC l_dc = l_hdc;
#else
	RMemDC l_MemDC = RMemDC(l_hdc, NULL);
	HDC l_dc = l_MemDC;
#endif

	Draw(a_hWnd, l_dc);
	}
	
	::EndPaint(a_hWnd, &l_ps);
}


//	---------------------------------------------------------------------------------------
//	prints/draws entire control
//
void OnPrintClient(HWND a_hWnd, HDC a_hDC)
{
	Draw(a_hWnd, a_hDC);
}


void 
Draw(HWND a_hWnd, HDC a_hDC)
{
	const RTipData* l_pData = GetRCellTipData(a_hWnd);

	RECT l_rectWin;
	::GetClientRect(a_hWnd, &l_rectWin);
	
	if (IsFrame(a_hWnd))
	{
		RDraw::Draw3DRect(a_hDC, l_rectWin, RGB(0, 0, 0), RGB(0, 0, 0));
		::InflateRect(&l_rectWin, -1, -1);
	}
	RDraw::FillSolidRect(a_hDC, l_rectWin, l_pData->GetColorBk());

	RECT l_rectText = { 0, 0, 0, 0};
	long l_iTextLen = static_cast<long>(_tcslen(l_pData->GetText()));
	l_rectText.right = l_pData->GetSize().cx;
	l_rectText.bottom = l_pData->GetSize().cy;

	HFONT l_hFontOld = (HFONT)::SelectObject(a_hDC, l_pData->GetFont());
	COLORREF l_clrOldBk = ::SetBkColor(a_hDC, l_pData->GetColorBk());
	COLORREF l_clrOldText = ::SetTextColor(a_hDC, l_pData->GetColorText());
	::DrawText(a_hDC, l_pData->GetText(), l_iTextLen, &l_rectText, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	::SetTextColor(a_hDC, l_clrOldText);
	::SetBkColor(a_hDC, l_clrOldBk);
	::SelectObject(a_hDC, l_hFontOld);
}

//	---------------------------------------------------------------------------------------
//	OnTimer - only for setting mouse track notification. Don't no why, but it is not 
//  possible to start it in RCellTip_Show
//
void  
OnTimer(
	HWND a_hWnd,
	UINT a_idTimer
	)
{
	if (a_idTimer == cc_idTimerShow)
	{
		::KillTimer(a_hWnd, a_idTimer);
		ShowTip(a_hWnd);
	}
	else if (a_idTimer == cc_idTimerHide)
	{
		::KillTimer(a_hWnd, a_idTimer);
		HideTip(a_hWnd);
	}
}


//	---------------------------------------------------------------------------------------
//	sets text
//
void
OnSetText(HWND a_hWnd, LPCTSTR a_sText)
{
	RTipData* l_pData = GetRCellTipData(a_hWnd);
	l_pData->SetText(a_sText);
	CalculateTipSize(a_hWnd, false);
}


//	---------------------------------------------------------------------------------------
//	returns text
//
LRESULT OnGetText(HWND a_hWnd, int a_iMax, LPTSTR a_psText)
{
	if ((a_psText == NULL) || (a_iMax <= 0))
	{
		return 0;
	}

	const RTipData* l_pData = GetRCellTipData(a_hWnd);
	_tcsncpy_s(a_psText, a_iMax, l_pData->GetText(), a_iMax);
	return min(a_iMax, l_pData->GetTextLen());

}

LRESULT
OnGetTextLength(HWND a_hWnd)
{
	const RTipData* l_pData = GetRCellTipData(a_hWnd);
	return l_pData->GetTextLen();
}


//	---------------------------------------------------------------------------------------
//	sets font
//
void    
OnSetFont(HWND a_hWnd, HFONT a_hFont, bool a_bRedraw)
{
	RTipData* l_pData = GetRCellTipData(a_hWnd);
	l_pData->SetFont(a_hFont);
	CalculateTipSize(a_hWnd, false);
	if (a_bRedraw)
	{
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);
	}
}

//	---------------------------------------------------------------------------------------
//	returns font
//
LRESULT 
OnGetFont(HWND a_hWnd)
{
	const RTipData* l_pData = GetRCellTipData(a_hWnd);
	return reinterpret_cast<LRESULT>(l_pData->GetFont());
}


//	---------------------------------------------------------------------------------------
//	sets bk color
//
void    
OnSetBkColor(HWND a_hWnd, COLORREF a_clr)
{
	RTipData* l_pData = GetRCellTipData(a_hWnd);
	l_pData->SetColorBk(a_clr);
}

//	---------------------------------------------------------------------------------------
//	returns bk color
//
LRESULT 
OnGetBkColor(HWND a_hWnd)
{
	const RTipData* l_pData = GetRCellTipData(a_hWnd);
	return l_pData->GetColorBk();
}


//	---------------------------------------------------------------------------------------
//	sets text color
//
void   
OnSetTextColor(HWND a_hWnd, COLORREF a_clr)
{
	RTipData* l_pData = GetRCellTipData(a_hWnd);
	l_pData->SetColorText(a_clr);
}


//	---------------------------------------------------------------------------------------
//	returns text color
//
LRESULT 
OnGetTextColor(HWND a_hWnd)
{
	const RTipData* l_pData = GetRCellTipData(a_hWnd);
	return l_pData->GetColorText();
}


//	---------------------------------------------------------------------------------------
//	sets default size
//
void    
OnSetSize(HWND a_hWnd, const LPSIZE a_pSize)
{
	if (a_pSize == NULL)
	{
		return;
	}
	RTipData* l_pData = GetRCellTipData(a_hWnd);
	l_pData->SetSize(*a_pSize);
	::SetWindowPos(a_hWnd, NULL, 0, 0, l_pData->GetSize().cx, l_pData->GetSize().cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}


//	---------------------------------------------------------------------------------------
//	gets default size
//
void
OnGetSize(HWND a_hWnd, LPSIZE a_pSize)
{
	if (a_pSize == NULL)
	{
		return;
	}
	const RTipData* l_pData = GetRCellTipData(a_hWnd);
	*a_pSize = l_pData->GetSize();
}


//	---------------------------------------------------------------------------------------
//	Shows celltip
//
void 
OnPopUp(HWND a_hWnd, const LPPOINT a_pPt)
{
	ASSERT(::IsWindow(a_hWnd));
	RTipData* l_pData = GetRCellTipData(a_hWnd);
	l_pData->m_pt = *a_pPt;
	if (l_pData->m_tipDelay.iShow > 0)
	{
		::SetTimer(a_hWnd, cc_idTimerShow, l_pData->m_tipDelay.iShow, NULL);
	}
	if (l_pData->m_tipDelay.iHide > 0)
	{
		::SetTimer(a_hWnd, cc_idTimerHide, l_pData->m_tipDelay.iHide, NULL);
	}

	if (l_pData->m_tipDelay.iShow == 0)
	{
		ShowTip(a_hWnd);
	}
}


//	---------------------------------------------------------------------------------------
//	Moves celltip
//
void 
OnMove(HWND a_hWnd, const LPPOINT a_pPt)
{
	ASSERT(::IsWindow(a_hWnd));
	ASSERT(a_pPt != NULL);
	if (::IsWindowVisible(a_hWnd))
	{		
		::SetWindowPos(a_hWnd, 0, a_pPt->x, a_pPt->y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING | SWP_DEFERERASE);
	}
}


//	---------------------------------------------------------------------------------------
//	Hides celltip
//
void OnPop(HWND a_hWnd)
{
	HideTip(a_hWnd);
}


//	---------------------------------------------------------------------------------------
//	Forces calculating size of celltip
//
void OnCalculateSize(HWND a_hWnd)
{
	ASSERT(::IsWindow(a_hWnd));
	CalculateTipSize(a_hWnd, true);
}


void OnSetDelay(HWND a_hWnd, const LPRTIPDELAY a_pDelay)
{
	ASSERT(::IsWindow(a_hWnd));
	RTipData* l_pData = GetRCellTipData(a_hWnd);
	l_pData->m_tipDelay = *a_pDelay;
}


void OnGetDelay(HWND a_hWnd, LPRTIPDELAY a_pDelay)
{
	ASSERT(::IsWindow(a_hWnd));
	const RTipData* l_pData = GetRCellTipData(a_hWnd);
	*a_pDelay = l_pData->m_tipDelay;
}


//	---------------------------------------------------------------------------------------
//	calculates width of the window
//
void
CalculateTipSize(
	HWND a_hWnd,
	bool a_bForce
	) 
{
	if (IsConstSize(a_hWnd) && !a_bForce)
	{
		return;
	}
	RTipData* l_pData = GetRCellTipData(a_hWnd);
	RECT l_rectText = { 0, 0, 0, 100};

	HDC l_hdc = ::GetDC(a_hWnd);
	HFONT l_hFontOld = (HFONT)::SelectObject(l_hdc, l_pData->GetFont());
	::DrawText(l_hdc, l_pData->GetText(), -1, &l_rectText, DT_CALCRECT | DT_SINGLELINE);
	::SelectObject(l_hdc, l_hFontOld);
	::ReleaseDC(a_hWnd, l_hdc);

	if (!IsConstDx(a_hWnd) || a_bForce)
	{
		l_pData->SetDx(l_rectText.right - l_rectText.left + 2 + (IsFrame(a_hWnd) ? 2 : 0));
	}

	if (!IsConstDy(a_hWnd) || a_bForce)
	{
		l_pData->SetDy(l_rectText.bottom - l_rectText.top + (IsFrame(a_hWnd) ? 2 : 0));
	}
	::SetWindowPos(a_hWnd, NULL, 0, 0, l_pData->GetSize().cx, l_pData->GetSize().cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
}


void ShowTip(HWND a_hWnd)
{
	RTipData* l_pData = GetRCellTipData(a_hWnd);
	::SetWindowPos(a_hWnd, NULL, l_pData->m_pt.x, l_pData->m_pt.y, 
		l_pData->GetSize().cx, l_pData->GetSize().cy, 
		SWP_NOACTIVATE | SWP_NOZORDER);
	if (IsAnimated(a_hWnd))
	{
		::AnimateWindow(a_hWnd, 200, AW_SLIDE | AW_VER_POSITIVE | AW_BLEND);
	}
	else
	{
		::SetWindowPos(a_hWnd, HWND_TOPMOST, l_pData->m_pt.x, l_pData->m_pt.y, 
			l_pData->GetSize().cx, l_pData->GetSize().cy, 
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}
}


void HideTip(HWND a_hWnd)
{
	::KillTimer(a_hWnd, cc_idTimerShow);
	::KillTimer(a_hWnd, cc_idTimerHide);
	if (!::IsWindow(a_hWnd))
	{
		 return;
	}
	if (!::IsWindowVisible(a_hWnd))
	{
		return;
	}
	
	if (IsAnimated(a_hWnd))
	{
		::AnimateWindow(a_hWnd, 200, AW_HIDE | AW_SLIDE | AW_VER_NEGATIVE | AW_BLEND);
	}
	else
	{
		::ShowWindow(a_hWnd, SW_HIDE);
	}
}
