// RBtnWnd.cpp: implementation of the RBtnWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "RBtnWnd.h"
#include "RKeyboard.h"
#include <commctrl.h>
#include "DrawUtl.h"

#ifdef _WIN32_WCE
#include <aygshell.h>
#else
#include "celltip.h"
#include "RTheme.h"
#include "RMemDC.h"
#include "RoundIter.h"
#include <Vssym32.h>
#endif


static const UINT cc_iDefTipShowTime = 3000;
static const UINT cc_iDefTipHideTime = 5000;

class RBtnData
{
public:
	RBtnData(void) : m_sTip(_T("")), m_hRgnBmp(NULL), m_hFont(NULL), m_clrText(RGB(0, 0, 0))
	{
		m_hImageList = NULL;
		m_bHovering = false;
		m_state = e_Normal;
		m_hBrushBtnFace =  ::CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
		m_hTip = NULL;
		m_tipDelay.iShow = cc_iDefTipShowTime;
		m_tipDelay.iHide = cc_iDefTipHideTime;
	}

	~RBtnData(void)
	{
		if (m_hBrushBtnFace != NULL)
		{
			::DeleteObject(m_hBrushBtnFace);
			m_hBrushBtnFace = NULL;
		}
		SetImageList(NULL);
		SetBmpRgn(NULL);
	}

	bool IsHovering() const { return m_bHovering; };
	void SetHovering(bool a_bHovering) { m_bHovering = a_bHovering; };

	bool IsClicked(void) const { return (m_state == e_Clicked); };
	bool IsPressed(void) const { return (m_state == e_Pressed); };
	bool IsNormal(void) const { return (m_state == e_Normal); };
	void SetClicked(void) { m_state = e_Clicked; };
	void SetPressed(void) { m_state = e_Pressed; };
	void SetNormal(void) { m_state = e_Normal; };
	void SetImageList(HIMAGELIST a_hImageList) 
	{ 
		if (m_hImageList != NULL)
		{
			ImageList_Destroy(m_hImageList);
		}
		m_hImageList = a_hImageList; 
	};

	int GetImageIdx(void) const 
	{
		ASSERT(m_hImageList != NULL); // probably you didn't call RBtnWnd_SetBitmap
		int l_iCount = ImageList_GetImageCount(m_hImageList);
		ASSERT(l_iCount > 0);
		if (l_iCount == 1)
		{
			return 0;
		}
		else if (l_iCount == 2)
		{
			return ((IsHovering() || (!IsNormal())) ?  1 : 0);
		}
		else
		{
			if (IsClicked() || IsPressed())
			{
				return 1;
			}
			else if (IsHovering())
			{
				return 2;
			}
			else
			{
				return 0;
			}
		}
	}

	HIMAGELIST GetImageList(void) const { return m_hImageList; };

	HRGN GetBmpRgn(void) const { return m_hRgnBmp; };
	void SetBmpRgn(HRGN a_hRgn) 
	{ 
		if (m_hRgnBmp != NULL)
		{
			::DeleteObject(m_hRgnBmp);
			m_hRgnBmp = NULL;
		}
		m_hRgnBmp = a_hRgn; 
	};

	HBRUSH m_hBrushBtnFace;
	HWND m_hTip;
	tstring m_sTip;
	RBTNTIPDELAY m_tipDelay;
	HFONT        m_hFont;
	COLORREF     m_clrText;

private:
	HRGN		m_hRgnBmp;
	bool		m_bHovering;
	HIMAGELIST	m_hImageList;
	enum 
	{
		e_Normal,
		e_Clicked,	// clicked by mouse 
		e_Pressed	// pushed by key (space)
	} m_state;
};

static const long c_iWindowOfs = sizeof(RBtnData*) - 4;

static inline RBtnData* GetRBtnData(HWND a_hWnd);


// messages
static inline BOOL OnCreate(HWND a_hWnd, LPCREATESTRUCT a_lpStruct);
static inline void OnDestroy(HWND a_hWnd);
static inline void OnPaint(HWND a_hWnd);
static inline LRESULT OnSetBitmap(HWND a_hWnd, LPTSetBitmap a_pSetBmp);
static inline void OnGetBitmap(HWND a_hWnd, LPTSetBitmap a_pSetBmp);
static inline void OnSetTip(HWND a_hWnd, LPTSTR a_sTip, LPRBTNTIPDELAY a_pTipDelay);
static inline void OnLButtonDown(HWND a_hWnd, int a_x, int a_y);
static inline void OnLButtonUp(HWND a_hWnd, int a_x, int a_y);
static inline void OnMouseMove(HWND a_hWnd, int a_xPos, int a_yPos);
static inline void OnMouseLeave(HWND a_hWnd);
static inline void OnCaptureChanged(HWND a_hWnd);
static inline void OnSetFocus(HWND a_hWnd);
static inline void OnKillFocus(HWND a_hWnd);
static inline void OnKeyDown(HWND a_hWnd, UINT a_iChar, LPARAM a_lParam);
static inline void OnKeyUp(HWND a_hWnd, UINT a_iChar);
static inline void OnChar(HWND a_hWnd, UINT a_iChar, LPARAM a_lParam);
static inline void OnSetFont(HWND a_hWnd, HFONT a_hFont, bool a_bRedraw);
static inline LRESULT OnGetFont(HWND a_hWnd);
static inline void OnSetTextColor(HWND a_hWnd, COLORREF a_clrText);
static inline LRESULT OnGetTextColor(HWND a_hWnd);

static inline void Draw(HWND a_hWnd, HDC a_hDC);
static inline LRESULT Notify(HWND a_hWnd, LPNMHDR a_pNmHdr);
static inline void ShowTip(HWND a_hWnd);

#define IsTbStyle(a_hWnd) ((::GetWindowLong(a_hWnd, GWL_STYLE) & RBCS_TBSTYLE) != 0)
#define IsRgn(a_hWnd) ((::GetWindowLong(a_hWnd, GWL_STYLE) & RBCS_RGN) != 0)


//	---------------------------------------------------------------------------------------
//	Main RBtnWnd procedure
//
LRESULT CALLBACK 
RBtnWnd_WndProc(
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
			OnLButtonDown(a_hWnd, LOWORD(a_lParam), HIWORD(a_lParam));
			break;

		case WM_LBUTTONUP:
			OnLButtonUp(a_hWnd, LOWORD(a_lParam), HIWORD(a_lParam));
			break;

		case WM_MOUSEMOVE:
			OnMouseMove(a_hWnd, LOWORD(a_lParam), HIWORD(a_lParam));
			break;

		case WM_MOUSELEAVE:
			OnMouseLeave(a_hWnd);
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

		case WM_KEYDOWN:
			OnKeyDown(a_hWnd, static_cast<UINT>(a_wParam), a_lParam);
			break;

		case WM_KEYUP:
			OnKeyUp(a_hWnd, static_cast<UINT>(a_wParam));
			break;

		case WM_CHAR:
			OnChar(a_hWnd, static_cast<UINT>(a_wParam), a_lParam);
			break;

		case WM_SETFONT:
			OnSetFont(a_hWnd, reinterpret_cast<HFONT>(a_wParam), LOWORD(a_lParam) == TRUE);
			break;

		case WM_GETFONT:
			return OnGetFont(a_hWnd);

		case RBWM_SETBITMAP:
			return OnSetBitmap(a_hWnd, reinterpret_cast<LPTSetBitmap>(a_lParam));

		case RBWM_GETBITMAP:
			OnGetBitmap(a_hWnd, reinterpret_cast<LPTSetBitmap>(a_lParam));
			break;

		case RBWM_SETTIP:
			OnSetTip(a_hWnd, reinterpret_cast<LPTSTR>(a_lParam), reinterpret_cast<LPRBTNTIPDELAY>(a_wParam));
			break;

		case RBWM_SETTEXTCOLOR:
			OnSetTextColor(a_hWnd, static_cast<COLORREF>(a_lParam));
			break;

		case RBWM_GETTEXTCOLOR:
			return OnGetTextColor(a_hWnd);
			
		case WM_DESTROY:
			OnDestroy(a_hWnd);
			break;

		default:
			return ::DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
   }
   return 0;
}


//	---------------------------------------------------------------------------------------
//	Creation of RBtnWnd window
//
HWND	// Handle of created window or NULL if failed
RBtnWnd_CreateEx(
	DWORD		a_iStyle,		// style
	DWORD		a_iStyleEx,		// extended style
	LPCTSTR     a_sCaption,		// caption
	int			a_x,			// horizontal position of window
	int			a_y,			// vertical position of window
	int			a_iWidth,		// window width
	int			a_iHeight,		// window height
	HWND		a_hWndParent,   // handle to parent or owner window
	UINT        a_idCtrl		// id ctrl
	)
{
	HWND l_hWnd = ::CreateWindowEx(a_iStyleEx, RBtnWnd_ClassName, a_sCaption, a_iStyle,
		a_x, a_y, a_iWidth, a_iHeight, a_hWndParent, reinterpret_cast<HMENU>(static_cast<LONG_PTR>(a_idCtrl)), RCommon_GetInstance(), NULL);

	return l_hWnd;
}


//	---------------------------------------------------------------------------------------
//	Returns control's data
//
RBtnData*	//OUT pointer to this data
GetRBtnData(
	HWND a_hWnd	//IN 
	)
{
#pragma warning(disable: 4312)
	return reinterpret_cast<RBtnData*>(::GetWindowLongPtr(a_hWnd, c_iWindowOfs));
#pragma warning(default: 4312)
}


//	---------------------------------------------------------------------------------------
//	Sets control's data
//
void	
SetRBtnData(
	HWND a_hWnd,	//IN
	RBtnData* a_pData	//IN
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
	RBtnData* l_pData = new RBtnData();
	SetRBtnData(a_hWnd, l_pData);
	return TRUE;
}


//	---------------------------------------------------------------------------------------
//	Postmortem:-)
//
void OnDestroy(
	HWND a_hWnd		//IN
	)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);
	if (l_pData->m_hTip != NULL)
	{
		::DestroyWindow(l_pData->m_hTip);
		l_pData->m_hTip = NULL;
	}

	delete l_pData;
}


void OnPaint(HWND a_hWnd)
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
//	Really draws BtnWnd control
//
void Draw(
	HWND a_hWnd, 
	HDC a_hDC
	)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);

	RECT l_rectWin;
	::GetClientRect(a_hWnd, &l_rectWin);

	RTheme l_theme;
	if (l_theme.OpenData(a_hWnd, L"BUTTON") != NULL)
	{		
		l_theme.DrawParentBackground(a_hWnd, a_hDC, &l_rectWin);
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
		RECT l_rectFocus(l_rectWin);
		::InflateRect(&l_rectFocus, -3, -3);
		::DrawFocusRect(a_hDC, &l_rectFocus);
	}
	int l_dx;
	int l_dy;
	ImageList_GetIconSize(l_pData->GetImageList(), &l_dx, &l_dy);

	ImageList_Draw(l_pData->GetImageList(), l_pData->GetImageIdx(), 
		a_hDC, max(0, (RectWidth(l_rectWin) - l_dx) / 2),
		max(0, (RectHeight(l_rectWin) - l_dy) / 2), 
		ILD_TRANSPARENT);
	TCHAR l_sText[1024];
	::GetWindowText(a_hWnd, l_sText, ArraySize(l_sText));
	if (_tclen(l_sText) > 0)
	{
		::SetBkMode(a_hDC, TRANSPARENT);
		COLORREF l_clrOld = ::SetTextColor(a_hDC, l_pData->m_clrText);
		HFONT l_hFont = NULL;
		if (l_pData->m_hFont != NULL)
		{
			l_hFont = reinterpret_cast<HFONT>(::SelectObject(a_hDC, l_pData->m_hFont));
		}
		::DrawText(a_hDC, l_sText, -1, &l_rectWin, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		if (l_hFont != NULL)
		{
			::SelectObject(a_hDC, l_hFont);
		}
		::SetTextColor(a_hDC, l_clrOld);
		::SetBkMode(a_hDC, BKMODE_LAST);
	}
}


//	---------------------------------------------------------------------------
//	Rejestracja klasy
//
ATOM 
RBtnWnd_RegisterClass()
{

#ifdef _WIN32_WCE
	WNDCLASS l_wndClass;
#else
	WNDCLASSEX l_wndClass;
	l_wndClass.cbSize = sizeof(WNDCLASSEX); 
	l_wndClass.hIconSm	   = NULL;
#endif

	l_wndClass.style			= CS_DBLCLKS | CS_GLOBALCLASS | CS_OWNDC;
	l_wndClass.lpfnWndProc		= RBtnWnd_WndProc;
	l_wndClass.cbClsExtra		= 0;
	l_wndClass.cbWndExtra		= sizeof(RBtnData*);
	l_wndClass.hInstance		= RCommon_GetInstance();
	l_wndClass.hIcon			= NULL;
	l_wndClass.hCursor			= NULL;
	l_wndClass.hbrBackground	= NULL;
	l_wndClass.lpszMenuName		= NULL;
	l_wndClass.lpszClassName	= RBtnWnd_ClassName;

#ifdef _WIN32_WCE
	return ::RegisterClass(&l_wndClass);
#else
	#ifndef RBWF_NOTOOLTIP
		RCellTip_Register();
	#endif
	return ::RegisterClassEx(&l_wndClass);
#endif
}


//	---------------------------------------------------------------------------
//	LButtonDown
//
void OnLButtonDown(HWND a_hWnd, int /*a_x*/, int /*a_y*/)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);
	if (l_pData->IsNormal())
	{
		l_pData->SetClicked();
		::SetCapture(a_hWnd);
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}

	NMHDR l_nmhdr;
	l_nmhdr.code = NM_LDOWN;

	Notify(a_hWnd, &l_nmhdr);
}


//	---------------------------------------------------------------------------
//	LButtonUp
//
void OnLButtonUp(HWND a_hWnd, int /*a_x*/, int /*a_y*/)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);
	if (!l_pData->IsClicked())
	{
		::ReleaseCapture();	// cannot be before IsClicked, because it sends CaptureChanged 
							// message, which changes state of button
		return;
	}
	::ReleaseCapture();

	NMHDR l_nmhdr;
	l_nmhdr.code = NM_CLICK;
	Notify(a_hWnd, &l_nmhdr);
	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);
	l_pData->SetNormal();
	::SetFocus(a_hWnd);
}


//	---------------------------------------------------------------------------------------
//	WM_MOUSEMOVE handler
//
void
OnMouseMove(
	HWND a_hWnd, 
	int a_xPos, 
	int a_yPos
	)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);	
	if (!l_pData->IsHovering())
	{
		l_pData->SetHovering(true);
		TRACKMOUSEEVENT l_track = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, a_hWnd, 1};
		::TrackMouseEvent(&l_track);
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
		ShowTip(a_hWnd);

		return;
	}

	// code below for situation, when left mouse button is presses
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
void
OnMouseLeave(
	HWND a_hWnd
	)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);
	l_pData->SetHovering(false);

	RCellTip_Pop(l_pData->m_hTip);

	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}


LRESULT
OnSetBitmap(HWND a_hWnd, LPTSetBitmap a_pSetBmp)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);
	ASSERT(a_pSetBmp->iCount > 0);
	HBITMAP l_hBmp = ((a_pSetBmp->hInst != NULL) ? 
		::LoadBitmap(a_pSetBmp->hInst, MAKEINTRESOURCE(a_pSetBmp->idBmp)) :
		a_pSetBmp->hBmp);
	if (l_hBmp == NULL)
	{
		return FALSE;
	}

	BITMAP l_bmp;
	int l_iSts = ::GetObject(l_hBmp, sizeof(l_bmp), &l_bmp);
	if (l_iSts == 0)
	{
		if (a_pSetBmp->hInst != NULL)
		{
			::DeleteObject(l_hBmp);
		}
		return FALSE;
	}

	l_pData->SetImageList(ImageList_Create(l_bmp.bmWidth / a_pSetBmp->iCount, 
		l_bmp.bmHeight, ILC_COLOR32 | ILC_MASK, a_pSetBmp->iCount, 0));
	if (l_pData->GetImageList() == NULL)
	{
		if (a_pSetBmp->hInst != NULL)
		{
			::DeleteObject(l_hBmp);
		}
		return FALSE;
	}

	if (IsRgn(a_hWnd))
	{
		l_pData->SetBmpRgn(RDraw::CreateRegionFromBitmap(l_hBmp, a_pSetBmp->clrMask));
		RECT l_rectWin;
		::GetWindowRect(a_hWnd, &l_rectWin);
		int l_dx;
		int l_dy;
		ImageList_GetIconSize(l_pData->GetImageList(), &l_dx, &l_dy);
		::OffsetRgn(l_pData->GetBmpRgn(), max(0, (RectWidth(l_rectWin) - l_dx) / 2),max(0, (RectHeight(l_rectWin) - l_dy) / 2));
		::SetWindowRgn(a_hWnd, l_pData->GetBmpRgn(), FALSE);
	}
	ImageList_AddMasked(l_pData->GetImageList(), l_hBmp, a_pSetBmp->clrMask);
	if (a_pSetBmp->hInst != NULL)
	{
		::DeleteObject(l_hBmp);
	}
	return TRUE;
}


void OnGetBitmap(HWND a_hWnd, LPTSetBitmap a_pSetBmp)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);
	a_pSetBmp->hInst = NULL;
	a_pSetBmp->iCount = ImageList_GetImageCount(l_pData->GetImageList());
	a_pSetBmp->idBmp = 0;
	a_pSetBmp->clrMask = 0;
}


void OnSetTip(HWND a_hWnd, LPTSTR a_sTip, LPRBTNTIPDELAY a_pTipDelay)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);
	l_pData->m_sTip = a_sTip;
	l_pData->m_tipDelay.iShow = a_pTipDelay->iShow;
	l_pData->m_tipDelay.iHide = a_pTipDelay->iShow + a_pTipDelay->iHide;
}


void OnSetTextColor(HWND a_hWnd, COLORREF a_clrText)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);
	l_pData->m_clrText = a_clrText;
}


LRESULT OnGetTextColor(HWND a_hWnd)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);
	return static_cast<LRESULT>(l_pData->m_clrText);
}


LRESULT Notify(HWND a_hWnd, LPNMHDR a_pNmHdr)
{
	HWND l_hParent = ::GetParent(a_hWnd);
	if (l_hParent == NULL)
	{
		l_hParent = a_hWnd;
	}
	
	a_pNmHdr->idFrom = ::GetDlgCtrlID(a_hWnd);
	a_pNmHdr->hwndFrom = a_hWnd;

	return ::SendMessage(l_hParent, WM_NOTIFY, static_cast<WPARAM>(a_pNmHdr->idFrom), 
		reinterpret_cast<LPARAM>(a_pNmHdr));
}


//	---------------------------------------------------------------------------
//	OnCaptureChanged
//
void 
OnCaptureChanged(
	HWND a_hWnd
	)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);
	l_pData->SetNormal();
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}


static void OnSetFocus(HWND a_hWnd)
{
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}
			

static void OnKillFocus(HWND a_hWnd)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);
	l_pData->SetNormal();
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}


static void OnKeyDown(HWND a_hWnd, UINT a_iChar, LPARAM a_lParam)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);
	if ((a_iChar == VK_SPACE) && (l_pData->IsNormal()))
	{
		l_pData->SetPressed();
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}
	NMKEY l_nmkey;
	l_nmkey.hdr.code = NM_KEYDOWN;
	l_nmkey.nVKey = a_iChar;
	l_nmkey.uFlags = a_lParam;

	if (Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmkey)) == 0)
	{
		NMHDR l_nmhdr;
		l_nmhdr.code = NM_CLICK;
		Notify(a_hWnd, &l_nmhdr);
	}
}


static void OnKeyUp(HWND a_hWnd, UINT a_iChar)
{
	if (a_iChar == VK_SPACE)
	{
		RBtnData* l_pData = GetRBtnData(a_hWnd);
		l_pData->SetNormal();
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}
}


static void OnChar(HWND a_hWnd, UINT a_iChar, LPARAM a_lParam)
{
	NMKEY l_nmkey;
	l_nmkey.hdr.code = NM_CHAR;
	l_nmkey.nVKey = a_iChar;
	l_nmkey.uFlags = a_lParam;

	Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmkey));
}


void OnSetFont(HWND a_hWnd, HFONT a_hFont, bool a_bRedraw)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);
	l_pData->m_hFont = a_hFont;
	if (a_bRedraw)
	{
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}

}


LRESULT OnGetFont(HWND a_hWnd)
{
	return reinterpret_cast<LRESULT>(GetRBtnData(a_hWnd)->m_hFont);
}


void ShowTip(HWND a_hWnd)
{
	RBtnData* l_pData = GetRBtnData(a_hWnd);
	if (l_pData->m_sTip.length() == 0)
	{
		return;
	}
	if (l_pData->m_hTip == NULL)
	{
		l_pData->m_hTip = RCellTip_Create(a_hWnd, RCTS_ANIMATE | RCTS_FRAME);
		RTIPDELAY l_tipDelay;
		l_tipDelay.iShow = l_pData->m_tipDelay.iShow;
		l_tipDelay.iHide = l_pData->m_tipDelay.iHide;
		RCellTip_SetDelay(l_pData->m_hTip, &l_tipDelay);
	}

	if (!::IsWindowVisible(l_pData->m_hTip))
	{
		::SetWindowText(l_pData->m_hTip, l_pData->m_sTip.c_str());
		RECT l_rect;
		::GetWindowRect(a_hWnd, &l_rect);
		POINT l_pt = {l_rect.right + 1, l_rect.bottom + 1};
		RCellTip_PopUp(l_pData->m_hTip, &l_pt);
	}
}