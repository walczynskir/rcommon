// RPinWnd.cpp: implementation of the RPinWnd control.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "RPinWnd.h"
#include "RPinData.h"
#include "drawutl.h"
#include "celltip.h"
#include <windowsx.h>


static const long c_iWindowOfs = sizeof(RPinData*) - 4;

static inline RPinData* GetData(HWND a_hWnd);

// messages
static inline BOOL OnCreate(HWND a_hWnd, LPCREATESTRUCT a_lpStruct);
static inline void OnNcDestroy(HWND a_hWnd);
static inline void OnNcPaint(HWND a_hWnd, HRGN a_hRgn);
static inline bool OnNcLButtonDown(HWND a_hWnd, WPARAM a_iHitTest, int a_x, int a_y);
static inline void OnLButtonUp(HWND a_hWnd);
static inline LRESULT OnNcMouseMove(HWND a_hWnd, WPARAM a_iHitTest, int a_x, int a_y);
static inline void OnMouseMove(HWND a_hWnd, int a_x, int a_y);
static inline LRESULT OnNcMouseLeave(HWND a_hWnd);
static inline LRESULT OnNcHitTest(HWND a_hWnd, int a_x, int a_y);
static inline LRESULT OnNcCalcSize(HWND a_hWnd, BOOL a_bCalc, LPARAM a_lParam);
static inline void OnGetMinMaxInfo(HWND a_hWnd, LPMINMAXINFO a_pMinMaxInfo);
static inline void OnCaptureChanged(HWND a_hWnd);
static inline void OnSize(HWND a_hWnd, long a_dxWidth, long a_dyHeight);
static inline void OnPinChange(HWND a_hWnd);
inline static void OnSetFocus(HWND a_hWnd);

static inline void OnSetWnd(HWND a_hWnd, HWND a_hWndInner);
static inline void OnGetWnd(HWND a_hWnd, HWND* a_phWndInner);
static inline void OnSetCaptionColor(HWND a_hWnd, int a_iCnt, COLORREF* a_pColors);
static inline void OnGetCaptionColor(HWND a_hWnd, int a_iCnt, COLORREF* a_pColors);
static inline void OnAddCaptionIcon(HWND a_hWnd, LPRADDCICON a_pAddCIcon);
static inline void OnAnimate(HWND a_hWnd, bool a_bShow);
static inline void OnShow(HWND a_hWnd, short a_nStyle, LPRECT a_pRect);
static inline void OnHide(HWND a_hWnd, short a_nStyle);
static inline void OnSetDelay(HWND a_hWnd, int a_iDelay);
static inline LRESULT OnGetDelay(HWND a_hWnd);

static inline void Draw(HWND a_hWnd, HDC a_hDC, LPCRECT a_pRect);
static inline void DrawBorder(HWND a_hWnd, HDC a_hDC);
static inline void DrawCaption(HWND a_hWnd, HDC a_hDC);
static inline void DrawCaptionIcon(HWND a_hWnd, HDC a_hDC);
static inline void DrawPin(HWND a_hWnd, HDC a_hDC);
static inline void DrawClose(HWND a_hWnd, HDC a_hDC);
static inline void DrawAdditional(HWND a_hWnd, HDC a_hDC);
static void RedrawFrameIcon(HWND a_hWnd, int a_iHitTest);

static void Show(HWND a_hWnd, LPRECT a_pRect);
static void Hide(HWND a_hWnd);

static void ScreenToWindow(HWND a_hWnd, LPRECT a_pRect);
static void GetCaptionRect(HWND a_hWnd, LPRECT a_pRect);
static void GetCaptionIconRect(HWND a_hWnd, LPRECT a_pRect);
static void GetCloseRect(HWND a_hWnd, LPRECT a_pRect);
static void GetPinRect(HWND a_hWnd, LPRECT a_pRect);
static void GetAdditIconRect(HWND a_hWnd, int a_iIcon, LPRECT a_pRect);
static void GetRectByHitTest(HWND a_hWnd, int a_iHitTest, LPRECT a_pRect);
static long HitTest(HWND a_hWnd, int a_x, int a_y);
static inline void GetCaptionTextPosSize(HWND a_hWnd, const LPPOINT a_pPt, const LPSIZE a_pSize);
static inline int GetTitleAngle(HWND a_hWnd);
static LRESULT Notify(HWND a_hWnd, LPNMHDR a_pNmHdr);
static inline void NotifyClicked(HWND a_hWnd, int a_iHitTest);
static inline void NotifyCaptionIcon(HWND a_hWnd);
static inline void NotifyClose(HWND a_hWnd);
static inline void NotifyPin(HWND a_hWnd);
static inline LONG GetMinTrackSize(HWND a_hWnd);

#define HasStyle(a_hWnd, a_iStyle) ((::GetWindowLong(a_hWnd, GWL_STYLE) & a_iStyle) == a_iStyle)

// own hit-test code
#define HTCAPTIONICON   100
#define HTPIN			101
#define HTFIRST			102

#define HitTestOnIcon(a_iHitTest) ((a_iHitTest == HTCLOSE) || (a_iHitTest == HTPIN) || (a_iHitTest >= HTFIRST))


//	---------------------------------------------------------------------------
//	Rejestracja klasy
//
ATOM RPinWnd_RegisterClass(void)
{

	WNDCLASSEX l_wcex;
	l_wcex.cbSize			= sizeof(WNDCLASSEX); 
	l_wcex.style			= CS_GLOBALCLASS | CS_CLASSDC;
	l_wcex.lpfnWndProc		= RPinWnd_WndProc;
	l_wcex.cbClsExtra		= 0;
	l_wcex.cbWndExtra		= sizeof(RPinData*);
	l_wcex.hInstance		= RCommon_GetInstance();
	l_wcex.hIcon			= NULL;
	l_wcex.hCursor			= NULL;
	l_wcex.hbrBackground	= NULL;
	l_wcex.lpszMenuName		= NULL;
	l_wcex.lpszClassName	= RPinWnd_ClassName;
	l_wcex.hIconSm			= NULL;

	return ::RegisterClassEx(&l_wcex);
}


//	---------------------------------------------------------------------------------------
//	Main RPinWnd procedure
//
LRESULT CALLBACK 
RPinWnd_WndProc(
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

	case WM_NCPAINT:
		OnNcPaint(a_hWnd, reinterpret_cast<HRGN>(a_wParam));
		break;

	case WM_NCCALCSIZE:
		return OnNcCalcSize(a_hWnd, a_wParam == TRUE, a_lParam);

	case WM_NCLBUTTONDOWN:
		if (!OnNcLButtonDown(a_hWnd, a_wParam, LOWORD(a_lParam), HIWORD(a_lParam)))
		{
			return ::DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
		}
		return TRUE;

	case WM_LBUTTONUP:
		OnLButtonUp(a_hWnd);
		break;

	case WM_NCMOUSEMOVE:
		return OnNcMouseMove(a_hWnd, a_wParam, GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam));

	case WM_MOUSEMOVE:
		OnMouseMove(a_hWnd, LOWORD(a_lParam), HIWORD(a_lParam));
		break;

	case WM_NCMOUSELEAVE:
		return OnNcMouseLeave(a_hWnd);

	case WM_CAPTURECHANGED:
		OnCaptureChanged(a_hWnd);
		break;

	case WM_SIZE:
		OnSize(a_hWnd, LOWORD(a_lParam), HIWORD(a_lParam));
		break;
	
	case WM_NCDESTROY:
		OnNcDestroy(a_hWnd);
		break;

	case WM_NCHITTEST:
		return OnNcHitTest(a_hWnd, LOWORD(a_lParam), HIWORD(a_lParam));

	case WM_GETMINMAXINFO:
		OnGetMinMaxInfo(a_hWnd, reinterpret_cast<LPMINMAXINFO>(a_lParam));
		break;

	case WM_SETFOCUS:
		OnSetFocus(a_hWnd);
		break;

	case RPWM_SETWND:
		OnSetWnd(a_hWnd, reinterpret_cast<HWND>(a_lParam));
		break;

	case RPWM_GETWND:
		OnGetWnd(a_hWnd, reinterpret_cast<HWND*>(a_lParam));
		break;

	case RPWM_SETCAPTIONCOLOR:
		OnSetCaptionColor(a_hWnd, static_cast<int>(a_wParam), reinterpret_cast<COLORREF*>(a_lParam));
		break;

	case RPWM_GETCAPTIONCOLOR:
		OnGetCaptionColor(a_hWnd, static_cast<int>(a_wParam), reinterpret_cast<COLORREF*>(a_lParam));
		break;

	case RPWM_ADDCAPTIONICON:
		OnAddCaptionIcon(a_hWnd, reinterpret_cast<LPRADDCICON>(a_lParam));
		break;

	case RPWM_SHOW:
		OnShow(a_hWnd, static_cast<short>(a_wParam), reinterpret_cast<LPRECT>(a_lParam));
		break;

	case RPWM_HIDE:
		OnHide(a_hWnd, static_cast<short>(a_wParam));
		break;

	case RPWM_SETDELAY:
		OnSetDelay(a_hWnd, static_cast<int>(a_lParam));
		break;

	case RPWM_GETDELAY:
		return OnGetDelay(a_hWnd);

	default:
		return ::DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
   }
   return 0;
}


//	---------------------------------------------------------------------------------------
//	Creation of RPinWnd window
//
HWND	// Handle of created window or NULL if failed
RPinWnd_CreateEx(
	DWORD		a_iStyle,		// style
	DWORD		a_iStyleEx,		// extended style
	LPCTSTR     a_sTitle,		// Title
	int			a_x,			// horizontal position of window
	int			a_y,			// vertical position of window
	int			a_iWidth,		// window width
	int			a_iHeight,		// window height
	HWND		a_hWndParent,   // handle to parent or owner window
	LPVOID		a_lpParam			// pointer to window-creation data
	)
{
	HWND l_hWnd = ::CreateWindowEx(a_iStyleEx, RPinWnd_ClassName, a_sTitle, a_iStyle | WS_CLIPCHILDREN,
		a_x, a_y, a_iWidth, a_iHeight, a_hWndParent, NULL, RCommon_GetInstance(), a_lpParam);


	return l_hWnd;
}


//	---------------------------------------------------------------------------------------
//	Returns control's data
//
RPinData*	//OUT pointer to this data
GetData(
	HWND a_hWnd	//IN 
	)
{
#pragma warning(disable: 4312)
	return reinterpret_cast<RPinData*>(::GetWindowLongPtr(a_hWnd, c_iWindowOfs));
#pragma warning(default: 4312)
}


//	---------------------------------------------------------------------------------------
//	Sets control's data
//
void SetData(
	HWND a_hWnd,	//IN
	RPinData* a_pData	//IN
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
	RPinData* l_pData = new RPinData();
	if (l_pData == NULL)
	{
		return FALSE;
	}
	SetData(a_hWnd, l_pData);

	RCellTip_Register();
	l_pData->m_hTip = RCellTip_Create(a_hWnd, RCTS_FRAME | RCTS_ANIMATE);
	RTIPDELAY l_tipDelay;
	l_tipDelay.iHide = 5000;
	l_tipDelay.iShow = 500;
	RCellTip_SetDelay(l_pData->m_hTip, &l_tipDelay);

	TCHAR l_sBuf[1024];
	::LoadString(RCommon_GetInstance(), IDS_PIN_OFF, l_sBuf, ArraySize(l_sBuf));
	l_pData->m_sPinOff = l_sBuf;
	::LoadString(RCommon_GetInstance(), IDS_PIN_ON, l_sBuf, ArraySize(l_sBuf));
	l_pData->m_sPinOn = l_sBuf;
	return TRUE;
}


//	---------------------------------------------------------------------------------------
//	Postmortem:-)
//
void OnNcDestroy(
	HWND a_hWnd		//IN
	)
{
	const RPinData* l_pData = GetData(a_hWnd);
	TRACKMOUSEEVENT l_track = {sizeof(TRACKMOUSEEVENT), TME_CANCEL | TME_NONCLIENT | TME_LEAVE | TME_HOVER, a_hWnd, HOVER_DEFAULT};
	::TrackMouseEvent(&l_track);
	delete l_pData;
}


void OnNcPaint(HWND a_hWnd, HRGN a_hRgn)
{
	HDC l_hdc = ::GetDCEx(a_hWnd, a_hRgn, DCX_WINDOW | DCX_INTERSECTRGN | DCX_CLIPSIBLINGS);
	//HDC l_hdc = ::GetWindowDC(a_hWnd);
	// don't want double buffering - it's very slow on big screens :(
	RECT l_rect;
	::GetRgnBox(a_hRgn, &l_rect);
	::GetWindowRect(a_hWnd, &l_rect);
	Draw(a_hWnd, l_hdc, NULL);
	::ReleaseDC(a_hWnd, l_hdc);
}


LRESULT OnNcCalcSize(HWND a_hWnd, BOOL a_bCalc, LPARAM a_lParam)
{
	LPNCCALCSIZE_PARAMS l_pParams = reinterpret_cast<LPNCCALCSIZE_PARAMS>(a_lParam);
	LPRECT l_pRectProp = &(l_pParams->rgrc[0]);
	RECT l_rectWin;
	if (a_bCalc)
	{
		::SetRect(&l_rectWin, l_pParams->lppos->x, l_pParams->lppos->y, l_pParams->lppos->x + l_pParams->lppos->cx, l_pParams->lppos->y + l_pParams->lppos->cy);
	}
	else
	{
		::GetWindowRect(a_hWnd, &l_rectWin);
	}
	if (HasStyle(a_hWnd, PS_TITLELEFT))
	{
		l_pRectProp->left = l_rectWin.left + ::GetSystemMetrics(SM_CXFRAME) + ::GetSystemMetrics(SM_CYSMCAPTION);
		l_pRectProp->right = l_rectWin.right - ::GetSystemMetrics(SM_CXFRAME);
		l_pRectProp->top = l_rectWin.top + ::GetSystemMetrics(SM_CYFRAME);
		l_pRectProp->bottom = l_rectWin.bottom - ::GetSystemMetrics(SM_CYFRAME);
	}
	else if (HasStyle(a_hWnd, PS_TITLERIGHT))
	{
		l_pRectProp->left = l_rectWin.left + ::GetSystemMetrics(SM_CXFRAME);
		l_pRectProp->right = l_rectWin.right - ::GetSystemMetrics(SM_CXFRAME) - ::GetSystemMetrics(SM_CYSMCAPTION);
		l_pRectProp->top = l_rectWin.top + ::GetSystemMetrics(SM_CYFRAME);
		l_pRectProp->bottom = l_rectWin.bottom - ::GetSystemMetrics(SM_CYFRAME);
	}
	else if (HasStyle(a_hWnd, PS_TITLEBOTTOM))
	{
		l_pRectProp->left = l_rectWin.left + ::GetSystemMetrics(SM_CXFRAME);
		l_pRectProp->right = l_rectWin.right - ::GetSystemMetrics(SM_CXFRAME);
		l_pRectProp->top = l_rectWin.top + ::GetSystemMetrics(SM_CYFRAME);
		l_pRectProp->bottom = l_rectWin.bottom - ::GetSystemMetrics(SM_CYFRAME) - ::GetSystemMetrics(SM_CYSMCAPTION);
	}
	else
	{
		l_pRectProp->left = l_rectWin.left + ::GetSystemMetrics(SM_CXFRAME);
		l_pRectProp->right = l_rectWin.right - ::GetSystemMetrics(SM_CXFRAME);
		l_pRectProp->top = l_rectWin.top + ::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CYSMCAPTION);
		l_pRectProp->bottom = l_rectWin.bottom - ::GetSystemMetrics(SM_CYFRAME);
	}
	return 0;
}


void OnCaptureChanged(HWND a_hWnd)
{
	RPinData* l_pData = GetData(a_hWnd);
	l_pData->m_iPushedAt = 0;
}


void OnSize(HWND a_hWnd, long /*a_dxWidth*/, long /*a_dyHeight*/)
{
	RPinData* l_pData = GetData(a_hWnd);
	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);
	::SetWindowPos(l_pData->m_hWnd, NULL, 0, 0,
		RectWidth(l_rect), RectHeight(l_rect), SWP_NOZORDER);
}


//	---------------------------------------------------------------------------
//	Min size
//
void OnGetMinMaxInfo(HWND a_hWnd, LPMINMAXINFO a_pMinMaxInfo)
{
	a_pMinMaxInfo->ptMaxSize.x = ::GetSystemMetrics(SM_CXSCREEN);
	a_pMinMaxInfo->ptMaxSize.y = ::GetSystemMetrics(SM_CYSCREEN);
	a_pMinMaxInfo->ptMaxPosition.x = 0;
	a_pMinMaxInfo->ptMaxPosition.y = 0;
	if (HasStyle(a_hWnd, PS_TITLELEFT) || HasStyle(a_hWnd, PS_TITLERIGHT))
	{
		a_pMinMaxInfo->ptMinTrackSize.x = ::GetSystemMetrics(SM_CYSMCAPTION) + 2 * ::GetSystemMetrics(SM_CXFRAME);
		a_pMinMaxInfo->ptMinTrackSize.y = GetMinTrackSize(a_hWnd);
	}
	else
	{
		a_pMinMaxInfo->ptMinTrackSize.x = GetMinTrackSize(a_hWnd);
		a_pMinMaxInfo->ptMinTrackSize.y = ::GetSystemMetrics(SM_CYSMCAPTION) + 2 * ::GetSystemMetrics(SM_CYFRAME);
	}
	a_pMinMaxInfo->ptMaxTrackSize.x = ::GetSystemMetrics(SM_CXSCREEN);
	a_pMinMaxInfo->ptMaxTrackSize.y = ::GetSystemMetrics(SM_CYSCREEN);

}


//	---------------------------------------------------------------------------
//	Really draws PinWnd control
//
void Draw(HWND a_hWnd, HDC a_hDC, LPCRECT /*a_pRect*/)
{
	DrawBorder(a_hWnd, a_hDC);
	DrawCaption(a_hWnd, a_hDC);
	DrawCaptionIcon(a_hWnd, a_hDC);
	DrawClose(a_hWnd, a_hDC);
	DrawPin(a_hWnd, a_hDC);
	DrawAdditional(a_hWnd, a_hDC);
}


void DrawBorder(HWND a_hWnd, HDC a_hDC)
{
	RPinData* l_pData = GetData(a_hWnd);
	RECT l_rect;
	GetWindowRect(a_hWnd, &l_rect);
	HBRUSH l_hBrush = ::CreateSolidBrush(l_pData->m_clrCaption[0]);
	RECT l_rectDraw;
	// left
	::SetRect(&l_rectDraw, 0, 0, ::GetSystemMetrics(SM_CXFRAME), RectHeight(l_rect));
	::FillRect(a_hDC, &l_rectDraw, l_hBrush);

	// top
	::SetRect(&l_rectDraw, 0, 0, RectWidth(l_rect), ::GetSystemMetrics(SM_CYFRAME));
	::FillRect(a_hDC, &l_rectDraw, l_hBrush);

	// right
	::SetRect(&l_rectDraw, RectWidth(l_rect) - ::GetSystemMetrics(SM_CXFRAME), 0, RectWidth(l_rect), RectHeight(l_rect));
	::FillRect(a_hDC, &l_rectDraw, l_hBrush);

	// bottom
	::SetRect(&l_rectDraw, 0, RectHeight(l_rect) - ::GetSystemMetrics(SM_CYFRAME), RectWidth(l_rect), RectHeight(l_rect));
	::FillRect(a_hDC, &l_rectDraw, l_hBrush);

	::DeleteObject(l_hBrush);
}


void DrawCaption(HWND a_hWnd, HDC a_hDC)
{
	RPinData* l_pData = GetData(a_hWnd);

	SIZE l_size;
	RECT l_rect;
	GetCaptionRect(a_hWnd, &l_rect);
	ScreenToWindow(a_hWnd, &l_rect);

	TRIVERTEX    l_vert[2];
	l_vert[0].x		 = l_rect.left;
	l_vert[0].y      = l_rect.top;
	l_vert[0].Red    = GetRValue(l_pData->m_clrCaption[0]) * 256;
	l_vert[0].Green  = GetGValue(l_pData->m_clrCaption[0]) * 256;
	l_vert[0].Blue   = GetBValue(l_pData->m_clrCaption[0]) * 256;
	l_vert[0].Alpha  = 0x0000;

	l_vert[1].x      = l_rect.right;
	l_vert[1].y      = l_rect.bottom; 
	l_vert[1].Red    = GetRValue(l_pData->m_clrCaption[1]) * 256;
	l_vert[1].Green  = GetGValue(l_pData->m_clrCaption[1]) * 256;
	l_vert[1].Blue   = GetBValue(l_pData->m_clrCaption[1]) * 256;
	l_vert[1].Alpha  = 0x0000;

	GRADIENT_RECT  l_gradRect;
	l_gradRect.UpperLeft  = 0;
	l_gradRect.LowerRight = 1;
	ULONG l_iFillStyle = 
		HasStyle(a_hWnd, PS_TITLELEFT) || HasStyle(a_hWnd, PS_TITLERIGHT) ? 
			GRADIENT_FILL_RECT_H : GRADIENT_FILL_RECT_V;
	::GradientFill(a_hDC, l_vert, ArraySize(l_vert), &l_gradRect, 1, l_iFillStyle);


	POINT l_pt;
	GetCaptionTextPosSize(a_hWnd, &l_pt, &l_size);
	int l_iBkModeOrg = ::SetBkMode(a_hDC, TRANSPARENT);
	COLORREF l_clrTextOrg = ::SetTextColor(a_hDC, RGB(0, 0, 0));
	TCHAR l_sTitle[100];
	::GetWindowText(a_hWnd, l_sTitle, ArraySize(l_sTitle));

	RDraw::DrawTextRotated(a_hDC, l_sTitle, -1, &l_pt, &l_size, 
		DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS, GetTitleAngle(a_hWnd));
	::SetTextColor(a_hDC, l_clrTextOrg);
	::SetBkMode(a_hDC, l_iBkModeOrg); 
}


void DrawClose(HWND a_hWnd, HDC a_hDC)
{
	RPinData* l_pData = GetData(a_hWnd);
	RECT l_rect;
	GetCloseRect(a_hWnd, &l_rect);
	ScreenToWindow(a_hWnd, &l_rect);
	if (((l_pData->m_iHoverAt == HTCLOSE) && (l_pData->m_iPushedAt == 0)) || ((l_pData->m_iPushedAt == HTCLOSE) && (l_pData->m_iPushedAt == l_pData->m_iHoverAt)))
	{
		RDraw::DrawFrame(a_hDC, l_rect, RGB(0, 0, 0));
		RECT l_rectInflate = l_rect;
		::InflateRect(&l_rectInflate, -1, -1);
		RDraw::FillSolidRect(a_hDC, l_rectInflate, 
			(l_pData->m_iPushedAt == HTCLOSE) ? l_pData->m_clrCaption[3] : l_pData->m_clrCaption[2]);
	}

	int l_iDiff = (RectWidth(l_rect) - l_pData->m_sizePin.cx) / 2;
	ImageList_Draw(l_pData->m_hImagePin, 2, a_hDC, l_rect.left + l_iDiff, l_rect.top + l_iDiff, ILD_TRANSPARENT);
}


void DrawCaptionIcon(HWND a_hWnd, HDC a_hDC)
{
	RPinData* l_pData = GetData(a_hWnd);
	if (l_pData->m_hImageCaption == NULL)
	{
		return;
	}
	RECT l_rect;
	GetCaptionIconRect(a_hWnd, &l_rect);
	ScreenToWindow(a_hWnd, &l_rect);
	ImageList_Draw(l_pData->m_hImageCaption, 0, a_hDC, l_rect.left, l_rect.top, ILD_TRANSPARENT);
}


void DrawPin(HWND a_hWnd, HDC a_hDC)
{
	RPinData* l_pData = GetData(a_hWnd);
	RECT l_rect;
	GetPinRect(a_hWnd, &l_rect);
	ScreenToWindow(a_hWnd, &l_rect);
	if (((l_pData->m_iHoverAt == HTPIN) && (l_pData->m_iPushedAt == 0)) || ((l_pData->m_iPushedAt == HTPIN) && (l_pData->m_iPushedAt == l_pData->m_iHoverAt)))
	{
		RDraw::DrawFrame(a_hDC, l_rect, RGB(0, 0, 0));
		RECT l_rectInflate = l_rect;
		::InflateRect(&l_rectInflate, -1, -1);
		RDraw::FillSolidRect(a_hDC, l_rectInflate, 
			(l_pData->m_iPushedAt == HTPIN) ? l_pData->m_clrCaption[3] : l_pData->m_clrCaption[2]);
	}
	int l_iDiff = (RectWidth(l_rect) - l_pData->m_sizePin.cx) / 2;
	ImageList_Draw(l_pData->m_hImagePin, l_pData->m_bOnTop ? 1 : 0, a_hDC, l_rect.left + l_iDiff, l_rect.top + l_iDiff, ILD_TRANSPARENT);
}


void DrawAdditional(HWND a_hWnd, HDC a_hDC)
{
	RPinData* l_pData = GetData(a_hWnd);
	RECT l_rect;
	for (int l_iIcon = 0; l_iIcon < l_pData->m_iCntAddit; l_iIcon++)
	{
		GetAdditIconRect(a_hWnd, l_iIcon, &l_rect);
		ScreenToWindow(a_hWnd, &l_rect);
		if (((l_pData->m_iHoverAt == (HTFIRST + l_iIcon)) && (l_pData->m_iPushedAt == 0)) || ((l_pData->m_iPushedAt == (HTFIRST + l_iIcon)) && (l_pData->m_iPushedAt == l_pData->m_iHoverAt)))
		{
			RDraw::DrawFrame(a_hDC, l_rect, RGB(0, 0, 0));
			RECT l_rectInflate = l_rect;
			::InflateRect(&l_rectInflate, -1, -1);
			RDraw::FillSolidRect(a_hDC, l_rectInflate, 
				(l_pData->m_iPushedAt == (HTFIRST + l_iIcon)) ? l_pData->m_clrCaption[3] : l_pData->m_clrCaption[2]);
		}
		int l_iDiff = (RectWidth(l_rect) - l_pData->m_sizeAddit.cx) / 2;
		ImageList_Draw(l_pData->m_hImageAddit, l_iIcon, a_hDC, l_rect.left + l_iDiff, l_rect.top + l_iDiff, ILD_TRANSPARENT);
	}

}

bool OnNcLButtonDown(HWND a_hWnd, WPARAM a_iHitTest, int /*a_x*/, int /*a_y*/)
{
	RPinData* l_pData = GetData(a_hWnd);
	if (a_iHitTest == HTCAPTIONICON)
	{
		NotifyCaptionIcon(a_hWnd);
		return TRUE;
	}
	int l_iPushedAt = 0;
	TRACKMOUSEEVENT l_track = {sizeof(TRACKMOUSEEVENT), TME_CANCEL | TME_NONCLIENT | TME_LEAVE | TME_HOVER, a_hWnd, HOVER_DEFAULT};
	::TrackMouseEvent(&l_track);
	l_pData->m_bHovering = false;

	bool l_bPushed = ((a_iHitTest == HTCLOSE) || (a_iHitTest == HTPIN) ||(a_iHitTest >= HTFIRST));
	RCellTip_Pop(l_pData->m_hTip);
	if (l_bPushed)
	{
		l_iPushedAt = static_cast<int>(a_iHitTest);
	}
	if (l_pData->m_iPushedAt != l_iPushedAt)
	{
		l_pData->m_iPushedAt = l_iPushedAt;
		RedrawFrameIcon(a_hWnd, l_iPushedAt);
	}
	if (l_bPushed)
	{
		::SetCapture(a_hWnd);
	}
	return l_bPushed;
}


void OnLButtonUp(HWND a_hWnd)
{
	RPinData* l_pData = GetData(a_hWnd);
	POINT l_pt;
	::GetCursorPos(&l_pt);
	int l_iHitTest = HitTest(a_hWnd, l_pt.x, l_pt.y);
	l_iHitTest = (l_iHitTest == l_pData->m_iPushedAt) ? l_iHitTest : 0;
	if (::GetCapture() == a_hWnd)
	{
		::ReleaseCapture();
	}
	if (HitTestOnIcon(l_pData->m_iPushedAt))
	{
		int l_iPushedAt = l_pData->m_iPushedAt;
		l_pData->m_iPushedAt = 0;
		RedrawFrameIcon(a_hWnd, l_iPushedAt);
	}

	if (l_iHitTest == HTCLOSE)
	{
		NotifyClose(a_hWnd);
		Hide(a_hWnd);
	}
	else if (l_iHitTest == HTPIN)
	{
		NotifyPin(a_hWnd);
		OnPinChange(a_hWnd);
	}
	else if (l_iHitTest >= HTFIRST)
	{
		NotifyClicked(a_hWnd, static_cast<int>(l_iHitTest));
	}
	//return ((a_iHitTest == HTCLOSE) || (a_iHitTest == HTPIN) ||(a_iHitTest >= HTFIRST));
}


void OnMouseMove(HWND a_hWnd, int /*a_x*/, int /*a_y*/)
{
	if (!(::GetCapture() == a_hWnd))
	{
		return;
	}

	RPinData* l_pData = GetData(a_hWnd);
	POINT l_pt;
	::GetCursorPos(&l_pt);
	int l_iHitTest = HitTest(a_hWnd, l_pt.x, l_pt.y);
	if (l_iHitTest != l_pData->m_iHoverAt)
	{
		l_pData->m_iHoverAt = l_iHitTest;
		// it happens only when mouse was pushed on icon, so we must redraw only pushed
		// icon
		RedrawFrameIcon(a_hWnd, l_pData->m_iPushedAt);
	}
		
}


LRESULT OnNcMouseMove(HWND a_hWnd, WPARAM /*a_iHitTest*/, int a_x, int a_y)
{
	RPinData* l_pData = GetData(a_hWnd);
	if (!l_pData->m_bHovering)
	{
		l_pData->m_bHovering = true;
		TRACKMOUSEEVENT l_track = {sizeof(TRACKMOUSEEVENT), TME_LEAVE | TME_NONCLIENT, a_hWnd, 1};
		::TrackMouseEvent(&l_track);
	}

	int l_iHitTest = HitTest(a_hWnd, a_x, a_y);
	if (l_iHitTest == l_pData->m_iHoverAt)
	{
		return 0;
	}	

	RCellTip_Pop(l_pData->m_hTip);
	if (HitTestOnIcon(l_iHitTest) || HitTestOnIcon(l_pData->m_iHoverAt))
	{
		RECT l_rect;
		HRGN l_hRgn = NULL;
		if (HitTestOnIcon(l_iHitTest))
		{
			GetRectByHitTest(a_hWnd, l_iHitTest, &l_rect);
			l_hRgn = ::CreateRectRgn(l_rect.left, l_rect.top, l_rect.right, l_rect.bottom);
		}
		if (HitTestOnIcon(l_pData->m_iHoverAt))
		{
			GetRectByHitTest(a_hWnd, l_pData->m_iHoverAt, &l_rect);
			HRGN l_hRgnTemp = ::CreateRectRgn(l_rect.left, l_rect.top, l_rect.right, l_rect.bottom);
			if (l_hRgn == NULL)
			{
				l_hRgn = l_hRgnTemp;
			}
			else
			{
				::CombineRgn(l_hRgn, l_hRgn, l_hRgnTemp, RGN_OR);
				::DeleteObject(l_hRgnTemp);
			}
		}
		l_pData->m_iHoverAt = l_iHitTest;
		::SendMessage(a_hWnd, WM_NCPAINT, reinterpret_cast<WPARAM>(l_hRgn), 0);
		::DeleteObject(l_hRgn);
		if ((l_iHitTest == HTPIN) || (l_iHitTest >= HTFIRST))
		{
			SIZE l_size;
			if (l_iHitTest == HTPIN)
			{
				::SetWindowText(l_pData->m_hTip, l_pData->m_bOnTop ? l_pData->m_sPinOff.c_str() : l_pData->m_sPinOn.c_str());
			}
			else
			{
				::SetWindowText(l_pData->m_hTip, l_pData->m_sTipsAddit.at(l_iHitTest - HTFIRST).c_str());
			}
			RCellTip_GetSize(l_pData->m_hTip, &l_size);
			HCURSOR l_hCur = ::GetCursor();
			ICONINFO l_ii;
			::GetIconInfo(l_hCur, &l_ii);
			POINT l_pt;
			l_pt.x = a_x + ::GetSystemMetrics(SM_CXCURSOR) - l_ii.xHotspot;
			l_pt.y = a_y;
			if (l_pt.x + l_size.cx >= ::GetSystemMetrics(SM_CXSCREEN))
			{
				l_pt.x = ::GetSystemMetrics(SM_CXSCREEN) - l_size.cx;
				l_pt.y += ::GetSystemMetrics(SM_CYCURSOR) - l_ii.yHotspot;
			}
			if (l_pt.y + l_size.cy >= ::GetSystemMetrics(SM_CYSCREEN))
			{
				l_pt.y = a_y - l_size.cy - 1;
			}
			RCellTip_PopUp(l_pData->m_hTip, &l_pt);
		}
	}

	return 0;
}


LRESULT OnNcMouseLeave(HWND a_hWnd)
{
	RPinData* l_pData = GetData(a_hWnd);
	l_pData->m_bHovering = false;
	int l_iHoverAt = l_pData->m_iHoverAt;
	l_pData->m_iHoverAt = 0;
	RedrawFrameIcon(a_hWnd, l_iHoverAt);
	RCellTip_Pop(l_pData->m_hTip);
	return 0;
}


LRESULT OnNcHitTest(HWND a_hWnd, int a_x, int a_y)
{
	return HitTest(a_hWnd, a_x, a_y);
}


void OnSetFocus(HWND a_hWnd)
{
	NMHDR l_nmhdr;
	l_nmhdr.code = RPN_SETFOCUS;
	Notify(a_hWnd, &l_nmhdr);
}


void OnSetWnd(HWND a_hWnd, HWND a_hInnerWnd)
{
	ASSERT(::IsWindow(a_hWnd));
	GetData(a_hWnd)->m_hWnd = a_hInnerWnd;
	::SetParent(a_hInnerWnd, a_hWnd);
}


void OnGetWnd(HWND a_hWnd, HWND* a_phWndInner)
{
	ASSERT(::IsWindow(a_hWnd));
	ASSERT(a_phWndInner != NULL);
	*a_phWndInner = GetData(a_hWnd)->m_hWnd;
}


void OnSetCaptionColor(HWND a_hWnd, int a_iCnt, COLORREF* a_pColors)
{
	ASSERT(::IsWindow(a_hWnd));
	ASSERT(a_pColors != NULL);
	RPinData* l_pData = GetData(a_hWnd);
	for (int l_iAt = 0; l_iAt < a_iCnt, l_iAt < ArraySize(l_pData->m_clrCaption); l_iAt++)
	{
		l_pData->m_clrCaption[l_iAt] = a_pColors[l_iAt];
	}
}


void OnGetCaptionColor(HWND a_hWnd, int a_iCnt, COLORREF* a_pColors)
{
	ASSERT(::IsWindow(a_hWnd));
	ASSERT(a_pColors != NULL);
	RPinData* l_pData = GetData(a_hWnd);
	for (int l_iAt = 0; l_iAt < a_iCnt, l_iAt < ArraySize(l_pData->m_clrCaption); l_iAt++)
	{
		a_pColors[l_iAt] = l_pData->m_clrCaption[l_iAt];
	}
}


void OnAddCaptionIcon(HWND a_hWnd, LPRADDCICON a_pAddCIcon)
{
	ASSERT(::IsWindow(a_hWnd));
	ASSERT(a_pAddCIcon != NULL);
	RPinData* l_pData = GetData(a_hWnd);
	HBITMAP l_hBmp;
	
	if (!a_pAddCIcon->bCaption)
	{
		l_pData->m_iCntAddit = a_pAddCIcon->iCnt;
	}
	if (a_pAddCIcon->hInst != NULL)
	{
		l_hBmp = ::LoadBitmap(a_pAddCIcon->hInst, MAKEINTRESOURCE(a_pAddCIcon->bmp.nID));
		if (a_pAddCIcon->tip.arrIdTip != NULL)
		{
			if (a_pAddCIcon->tip.arrIdTip != NULL)
			{
				if (a_pAddCIcon->bCaption)
				{
					TCHAR l_sBuf[1024];
					::LoadString(a_pAddCIcon->hInst, a_pAddCIcon->tip.arrIdTip[0], l_sBuf, ArraySize(l_sBuf));
					l_pData->m_sTipCaption = l_sBuf;
				}
				else
				{
					l_pData->m_sTipsAddit.clear();
					for (int l_iAt = 0; l_iAt < l_pData->m_iCntAddit; l_iAt++)
					{
						TCHAR l_sBuf[1024];
						::LoadString(a_pAddCIcon->hInst, a_pAddCIcon->tip.arrIdTip[l_iAt], l_sBuf, ArraySize(l_sBuf));
						l_pData->m_sTipsAddit.push_back(l_sBuf);
					}
				}
			}
		}
	}
	else
	{
		l_hBmp = a_pAddCIcon->bmp.hBmp;
		if (a_pAddCIcon->tip.arrStrTip != NULL)
		{
			if (a_pAddCIcon->bCaption)
			{
				l_pData->m_sTipCaption = a_pAddCIcon->tip.arrStrTip[0];
			}
			else
			{
				l_pData->m_sTipsAddit.clear();
				for (int l_iAt = 0; l_iAt < l_pData->m_iCntAddit; l_iAt++)
				{
					l_pData->m_sTipsAddit.push_back(a_pAddCIcon->tip.arrStrTip[l_iAt]);
				}
			}
		}
	}
	ASSERT(l_hBmp != NULL);
	BITMAP l_bmp;
	::GetObject(l_hBmp, sizeof(l_bmp), &l_bmp);
	if (a_pAddCIcon->bCaption)
	{
		l_pData->m_sizeCaption.cx = l_bmp.bmWidth;
		l_pData->m_sizeCaption.cy = l_bmp.bmHeight;
		// must have same width and height
		ASSERT(l_pData->m_sizeCaption.cx == l_pData->m_sizeCaption.cy);
	}
	else
	{
		l_pData->m_sizeAddit.cx = l_bmp.bmWidth / l_pData->m_iCntAddit;
		l_pData->m_sizeAddit.cy = l_bmp.bmHeight;
		// must have same width and height
		ASSERT(l_pData->m_sizeAddit.cx == l_pData->m_sizeAddit.cy);
	}

	if (a_pAddCIcon->bCaption)
	{
		if (l_pData->m_hImageCaption != NULL)
		{
			ImageList_Destroy(l_pData->m_hImageCaption);
			l_pData->m_hImageCaption = NULL;
		}
		l_pData->m_hImageCaption = ImageList_Create(l_pData->m_sizeCaption.cx, l_pData->m_sizeCaption.cy, ILC_COLOR32 | ILC_MASK,
				1, 1);
		ImageList_AddMasked(l_pData->m_hImageCaption, l_hBmp, a_pAddCIcon->clrMask);
		if (a_pAddCIcon->hInst != NULL)
		{
			::DeleteObject(l_hBmp);
		}
	}
	else
	{
		if (l_pData->m_hImageAddit != NULL)
		{
			ImageList_Destroy(l_pData->m_hImageAddit);
			l_pData->m_hImageAddit = NULL;
		}
		l_pData->m_hImageAddit = ImageList_Create(l_pData->m_sizeAddit.cx, l_pData->m_sizeAddit.cy, ILC_COLOR32 | ILC_MASK,
				a_pAddCIcon->iCnt, a_pAddCIcon->iCnt);
		ImageList_AddMasked(l_pData->m_hImageAddit, l_hBmp, a_pAddCIcon->clrMask);
		if (a_pAddCIcon->hInst != NULL)
		{
			::DeleteObject(l_hBmp);
		}
	}
}


void OnShow(HWND a_hWnd, short a_nStyle, LPRECT a_pRect)
{
	ASSERT(::IsWindow(a_hWnd));
	GetData(a_hWnd)->m_nAniStyle = a_nStyle;
	Show(a_hWnd, a_pRect);
}


void OnHide(HWND a_hWnd, short a_nStyle)
{
	ASSERT(::IsWindow(a_hWnd));
	GetData(a_hWnd)->m_nAniStyle = a_nStyle;
	Hide(a_hWnd);
}


void OnSetDelay(HWND a_hWnd, int a_iDelay)
{
	ASSERT(::IsWindow(a_hWnd));
	GetData(a_hWnd)->m_iDelay = a_iDelay;
}


LRESULT OnGetDelay(HWND a_hWnd)
{
	ASSERT(::IsWindow(a_hWnd));
	return GetData(a_hWnd)->m_iDelay;
}


//
// 
//
long HitTest(HWND a_hWnd, int a_x, int a_y)
{
	RECT l_rect;

	POINT l_pt;
	l_pt.x = a_x;
	l_pt.y = a_y;
	GetCloseRect(a_hWnd, &l_rect);
	if (::PtInRect(&l_rect, l_pt))
	{
		return HTCLOSE;
	}

	GetPinRect(a_hWnd, &l_rect);
	if (::PtInRect(&l_rect, l_pt))
	{
		return HTPIN;
	}

	GetCaptionIconRect(a_hWnd, &l_rect);
	if (::PtInRect(&l_rect, l_pt))
	{
		return HTCAPTIONICON;
	}

	RPinData* l_pData = GetData(a_hWnd);
	for (int l_iIcon = 0; l_iIcon < l_pData->m_iCntAddit; l_iIcon++)
	{
		GetAdditIconRect(a_hWnd, l_iIcon, &l_rect);
		if (::PtInRect(&l_rect, l_pt))
		{
			return HTFIRST + l_iIcon;
		}
	}

	GetCaptionRect(a_hWnd, &l_rect);
	if (::PtInRect(&l_rect, l_pt))
	{
		return HTCAPTION;
	}

	// border
	::GetWindowRect(a_hWnd, &l_rect);
	RECT l_rectBorder;
	::SetRect(&l_rectBorder, l_rect.left, l_rect.top, 
		l_rect.left + ::GetSystemMetrics(SM_CXFRAME), l_rect.top + ::GetSystemMetrics(SM_CYFRAME));
	if (::PtInRect(&l_rectBorder, l_pt))
	{
		return HTTOPLEFT;
	}
	::SetRect(&l_rectBorder, l_rect.left, l_rect.bottom - ::GetSystemMetrics(SM_CYFRAME), 
		l_rect.left + ::GetSystemMetrics(SM_CXFRAME), l_rect.bottom);
	if (::PtInRect(&l_rectBorder, l_pt))
	{
		return HTBOTTOMLEFT;
	}
	::SetRect(&l_rectBorder, l_rect.right - ::GetSystemMetrics(SM_CXFRAME), l_rect.top, 
		l_rect.right, l_rect.top + ::GetSystemMetrics(SM_CYFRAME));
	if (::PtInRect(&l_rectBorder, l_pt))
	{
		return HTTOPRIGHT;
	}
	::SetRect(&l_rectBorder, l_rect.right - ::GetSystemMetrics(SM_CXFRAME), l_rect.bottom - ::GetSystemMetrics(SM_CYFRAME), 
		l_rect.right, l_rect.bottom);
	if (::PtInRect(&l_rectBorder, l_pt))
	{
		return HTBOTTOMRIGHT;
	}

	::SetRect(&l_rectBorder, l_rect.left, l_rect.top + ::GetSystemMetrics(SM_CYFRAME), 
		l_rect.left + ::GetSystemMetrics(SM_CXFRAME), l_rect.bottom - ::GetSystemMetrics(SM_CYFRAME));
	if (::PtInRect(&l_rectBorder, l_pt))
	{
		return HTLEFT;
	}
	::SetRect(&l_rectBorder, l_rect.right - ::GetSystemMetrics(SM_CXFRAME), l_rect.top + ::GetSystemMetrics(SM_CYFRAME), 
		l_rect.right, l_rect.bottom - ::GetSystemMetrics(SM_CYFRAME));
	if (::PtInRect(&l_rectBorder, l_pt))
	{
		return HTRIGHT;
	}
	::SetRect(&l_rectBorder, l_rect.left + ::GetSystemMetrics(SM_CXFRAME), l_rect.top, 
		l_rect.right - ::GetSystemMetrics(SM_CXFRAME), l_rect.top + ::GetSystemMetrics(SM_CYFRAME));
	if (::PtInRect(&l_rectBorder, l_pt))
	{
		return HTTOP;
	}
	::SetRect(&l_rectBorder, l_rect.left + ::GetSystemMetrics(SM_CXFRAME), l_rect.bottom - ::GetSystemMetrics(SM_CYFRAME), 
		l_rect.right - ::GetSystemMetrics(SM_CXFRAME), l_rect.bottom);
	if (::PtInRect(&l_rectBorder, l_pt))
	{
		return HTBOTTOM;
	}

	return HTCLIENT;
}


//
// in screen coordinates
//
void GetCaptionRect(HWND a_hWnd, LPRECT a_pRect)
{
	::GetWindowRect(a_hWnd, a_pRect);
	if (HasStyle(a_hWnd, PS_TITLELEFT))
	{
		a_pRect->left += ::GetSystemMetrics(SM_CXFRAME);
		a_pRect->right = a_pRect->left + ::GetSystemMetrics(SM_CYSMCAPTION);
		a_pRect->top += ::GetSystemMetrics(SM_CYFRAME);
		a_pRect->bottom -= ::GetSystemMetrics(SM_CYFRAME);
	}
	else if (HasStyle(a_hWnd, PS_TITLERIGHT))
	{
		a_pRect->right -= ::GetSystemMetrics(SM_CXFRAME);
		a_pRect->left = a_pRect->right - ::GetSystemMetrics(SM_CYSMCAPTION);
		a_pRect->top += ::GetSystemMetrics(SM_CYFRAME);
		a_pRect->bottom -= ::GetSystemMetrics(SM_CYFRAME);
	}
	else if (HasStyle(a_hWnd, PS_TITLEBOTTOM))
	{
		a_pRect->left += ::GetSystemMetrics(SM_CXFRAME);
		a_pRect->right -= ::GetSystemMetrics(SM_CXFRAME);
		a_pRect->bottom -= ::GetSystemMetrics(SM_CYFRAME);
		a_pRect->top = a_pRect->bottom - ::GetSystemMetrics(SM_CYSMCAPTION);
	}
	else
	{
		a_pRect->left += ::GetSystemMetrics(SM_CXFRAME);
		a_pRect->right -= ::GetSystemMetrics(SM_CXFRAME);
		a_pRect->top += ::GetSystemMetrics(SM_CYFRAME);
		a_pRect->bottom = a_pRect->top + ::GetSystemMetrics(SM_CYSMCAPTION);
	}
}


//
// in screen coordinates
//
void GetCaptionIconRect(HWND a_hWnd, LPRECT a_pRect)
{
	GetCaptionRect(a_hWnd, a_pRect);
	RPinData* l_pData = GetData(a_hWnd);
	if (HasStyle(a_hWnd, PS_TITLELEFT))
	{
		a_pRect->bottom = a_pRect->top + l_pData->m_sizeCaption.cy;
	}
	else if (HasStyle(a_hWnd, PS_TITLERIGHT))
	{
		a_pRect->bottom = a_pRect->top + l_pData->m_sizeCaption.cy;
	}
	else if (HasStyle(a_hWnd, PS_TITLEBOTTOM))
	{
		a_pRect->right = a_pRect->left + l_pData->m_sizeCaption.cx;
	}
	else
	{
		a_pRect->right = a_pRect->left + l_pData->m_sizeCaption.cx;
	}
}

//
// in screen coordinates
//
void GetCloseRect(HWND a_hWnd, LPRECT a_pRect)
{
	::GetWindowRect(a_hWnd, a_pRect);
	if (HasStyle(a_hWnd, PS_TITLELEFT))
	{
		a_pRect->left += ::GetSystemMetrics(SM_CXFRAME);
		a_pRect->right = a_pRect->left + ::GetSystemMetrics(SM_CYSMCAPTION);
		a_pRect->bottom = a_pRect->bottom - ::GetSystemMetrics(SM_CYFRAME);
		a_pRect->top = a_pRect->bottom - ::GetSystemMetrics(SM_CYSMCAPTION);
	}
	else if (HasStyle(a_hWnd, PS_TITLERIGHT))
	{
		a_pRect->right -= ::GetSystemMetrics(SM_CXFRAME);
		a_pRect->left =  a_pRect->right - ::GetSystemMetrics(SM_CYSMCAPTION);
		a_pRect->bottom = a_pRect->bottom - ::GetSystemMetrics(SM_CYFRAME);
		a_pRect->top = a_pRect->bottom - ::GetSystemMetrics(SM_CYSMCAPTION);
	}
	else if (HasStyle(a_hWnd, PS_TITLEBOTTOM))
	{
		a_pRect->right -= ::GetSystemMetrics(SM_CXFRAME);
		a_pRect->left =  a_pRect->right - ::GetSystemMetrics(SM_CYSMCAPTION);
		a_pRect->bottom = a_pRect->bottom - ::GetSystemMetrics(SM_CYFRAME);
		a_pRect->top = a_pRect->bottom - ::GetSystemMetrics(SM_CYSMCAPTION);
	}
	else
	{
		a_pRect->right -= ::GetSystemMetrics(SM_CXFRAME);
		a_pRect->left =  a_pRect->right - ::GetSystemMetrics(SM_CYSMCAPTION);
		a_pRect->top = a_pRect->top + ::GetSystemMetrics(SM_CYFRAME);
		a_pRect->bottom = a_pRect->top + ::GetSystemMetrics(SM_CYSMCAPTION);
	}
}


//
// in screen coordinates
//
void GetPinRect(HWND a_hWnd, LPRECT a_pRect)
{
	GetCloseRect(a_hWnd, a_pRect);
	if (HasStyle(a_hWnd, PS_TITLELEFT) || HasStyle(a_hWnd, PS_TITLERIGHT))
	{
		a_pRect->top -= ::GetSystemMetrics(SM_CYSMCAPTION);
		a_pRect->bottom -= ::GetSystemMetrics(SM_CYSMCAPTION);
	}
	else
	{
		a_pRect->left -= ::GetSystemMetrics(SM_CYSMCAPTION);
		a_pRect->right -= ::GetSystemMetrics(SM_CYSMCAPTION);
	}
}


//
// in screen coordinates
//
void GetAdditIconRect(HWND a_hWnd, int a_iIcon, LPRECT a_pRect)
{
	GetPinRect(a_hWnd, a_pRect);
	if (HasStyle(a_hWnd, PS_TITLELEFT) || HasStyle(a_hWnd, PS_TITLERIGHT))
	{
		a_pRect->top -= (::GetSystemMetrics(SM_CYSMCAPTION) * (a_iIcon + 1));
		a_pRect->bottom -= (::GetSystemMetrics(SM_CYSMCAPTION) * (a_iIcon + 1));
	}
	else
	{
		a_pRect->left -= (::GetSystemMetrics(SM_CYSMCAPTION) * (a_iIcon + 1));
		a_pRect->right -= (::GetSystemMetrics(SM_CYSMCAPTION) * (a_iIcon + 1));
	}
}


void GetRectByHitTest(HWND a_hWnd, int a_iHitTest, LPRECT a_pRect)
{
	if (a_iHitTest == HTPIN)
	{
		GetPinRect(a_hWnd, a_pRect);
	}
	else if (a_iHitTest == HTCLOSE)
	{
		GetCloseRect(a_hWnd, a_pRect);
	}
	else if (a_iHitTest >= HTFIRST)
	{
		GetAdditIconRect(a_hWnd, a_iHitTest - HTFIRST, a_pRect);
	}
	else
	{
		ASSERT(FALSE);
	}
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


void NotifyClose(HWND a_hWnd)
{
	NMHDR l_nmhdr;
	l_nmhdr.code = RPN_CLOSE;
	Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmhdr));
}


void NotifyPin(HWND a_hWnd)
{
	NMHDR l_nmhdr;
	l_nmhdr.code = RPN_PIN;
	Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmhdr));
}


void NotifyClicked(HWND a_hWnd, int a_iHitTest)
{
	RCCNMPWCLK l_nmhdr;
	l_nmhdr.iIcon = a_iHitTest - HTFIRST;
	l_nmhdr.nmhdr.code = RPN_CLICKED;
	Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmhdr));
}


void NotifyCaptionIcon(HWND a_hWnd)
{
	NMHDR l_nmhdr;
	l_nmhdr.code = RPN_CAPTIONICON;
	Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nmhdr));
}


void OnPinChange(HWND a_hWnd)
{
	RPinData* l_pData = GetData(a_hWnd);
	l_pData->m_bOnTop = (!l_pData->m_bOnTop);
	::SetWindowPos(a_hWnd, 
		l_pData->m_bOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
	RedrawFrameIcon(a_hWnd, HTPIN);
}



//
// in window ccordinates
//
void GetCaptionTextPosSize(HWND a_hWnd, const LPPOINT a_pPt, const LPSIZE a_pSize)
{
	RECT l_rect;
	::GetWindowRect(a_hWnd, &l_rect);
	RPinData* l_pData = GetData(a_hWnd);
	int l_iAdditSize = 0;
	if (HasStyle(a_hWnd, PS_TITLELEFT))
	{
		a_pPt->x = ::GetSystemMetrics(SM_CYSMCAPTION) + ::GetSystemMetrics(SM_CXFRAME);
		a_pPt->y = ::GetSystemMetrics(SM_CYFRAME) + 2;
		a_pSize->cx = RectHeight(l_rect) - 2 * ::GetSystemMetrics(SM_CYFRAME) - 4;
		a_pSize->cy = ::GetSystemMetrics(SM_CYSMCAPTION);
		l_iAdditSize = l_pData->m_sizeAddit.cy;
		if (l_pData->m_hImageCaption != NULL)
		{
			a_pPt->y += l_pData->m_sizeCaption.cy + 2;
			a_pSize->cx -= (l_pData->m_sizeCaption.cy + 2);
		}
	}
	else if (HasStyle(a_hWnd, PS_TITLERIGHT))
	{
		a_pPt->x = RectWidth(l_rect) - ::GetSystemMetrics(SM_CXFRAME);
		a_pPt->y = ::GetSystemMetrics(SM_CYFRAME) + 2;
		a_pSize->cx = RectHeight(l_rect) - 2 * ::GetSystemMetrics(SM_CYFRAME) - 4;
		a_pSize->cy = ::GetSystemMetrics(SM_CYSMCAPTION);
		l_iAdditSize = l_pData->m_sizeAddit.cy;
		if (l_pData->m_hImageCaption != NULL)
		{
			a_pPt->y += l_pData->m_sizeCaption.cy + 2;
			a_pSize->cx -= (l_pData->m_sizeCaption.cy + 2);
		}
	}
	else if (HasStyle(a_hWnd, PS_TITLEBOTTOM))
	{
		a_pPt->x = ::GetSystemMetrics(SM_CXFRAME) + 2;
		a_pPt->y = RectHeight(l_rect) - ::GetSystemMetrics(SM_CYFRAME) - ::GetSystemMetrics(SM_CYSMCAPTION);
		a_pSize->cx = RectWidth(l_rect) - 2 * ::GetSystemMetrics(SM_CXFRAME) - 4;
		a_pSize->cy = ::GetSystemMetrics(SM_CYSMCAPTION);
		l_iAdditSize = l_pData->m_sizeAddit.cx;
		if (l_pData->m_hImageCaption != NULL)
		{
			a_pPt->x += l_pData->m_sizeCaption.cx + 2;
			a_pSize->cx -= (l_pData->m_sizeCaption.cx + 2);
		}
	}
	else
	{
		a_pPt->x = ::GetSystemMetrics(SM_CXFRAME) + 4;
		a_pPt->y = ::GetSystemMetrics(SM_CYFRAME);
		a_pSize->cx = RectWidth(l_rect) - 2 * ::GetSystemMetrics(SM_CXFRAME) - 4;
		a_pSize->cy = ::GetSystemMetrics(SM_CYSMCAPTION);
		l_iAdditSize = l_pData->m_sizeAddit.cx;
		if (l_pData->m_hImageCaption != NULL)
		{
			a_pPt->x += l_pData->m_sizeCaption.cx + 2;
			a_pSize->cx -= (l_pData->m_sizeCaption.cx + 2);
		}
	}
	a_pSize->cx -= ((l_pData->m_sizePin.cx * 2) + l_pData->m_iCntAddit * l_iAdditSize);
}


int GetTitleAngle(HWND a_hWnd)
{
	if (HasStyle(a_hWnd, PS_TITLELEFT) || HasStyle(a_hWnd, PS_TITLERIGHT))
	{
		return 90;
	}
	return 0;
}


void Show(HWND a_hWnd, LPRECT a_pRect)
{
	if (!IsWindowVisible(a_hWnd))
	{
		RPinData* l_pData = GetData(a_hWnd);
		::SetWindowPos(a_hWnd, HWND_TOP, a_pRect->left, a_pRect->top, 
			RectWidth(*a_pRect), RectHeight(*a_pRect), SWP_NOACTIVATE);
		RDraw::AnimateWindow(a_hWnd, l_pData->m_iDelay, l_pData->m_nAniStyle);
	}

}


void Hide(HWND a_hWnd)
{
	RPinData* l_pData = GetData(a_hWnd);
	l_pData->m_iHoverAt = 0;
	RDraw::AnimateWindow(a_hWnd, l_pData->m_iDelay, GetData(a_hWnd)->m_nAniStyle | RAW_HIDE);
}


void ScreenToWindow(HWND a_hWnd, LPRECT a_pRect)
{
	RECT l_rectWin;
	::GetWindowRect(a_hWnd, &l_rectWin);
	SIZE l_size;
	l_size.cx = RectWidth(*a_pRect);
	l_size.cy = RectHeight(*a_pRect);
	a_pRect->left -= l_rectWin.left;
	a_pRect->right = a_pRect->left + l_size.cx;
	a_pRect->top -= l_rectWin.top;
	a_pRect->bottom = a_pRect->top + l_size.cy;
}


LONG GetMinTrackSize(HWND a_hWnd)
{
	RPinData* l_pData = GetData(a_hWnd);
	if (HasStyle(a_hWnd, PS_TITLELEFT) || HasStyle(a_hWnd, PS_TITLERIGHT))
	{
		return 2 * ::GetSystemMetrics(SM_CYFRAME) + 4 + l_pData->m_sizePin.cy * 2 + 
			l_pData->m_sizeAddit.cy * l_pData->m_iCntAddit + l_pData->m_sizeCaption.cy + 20;
	}
	else
	{
		return 2 * ::GetSystemMetrics(SM_CXFRAME) + 4 + l_pData->m_sizePin.cx * 2 + 
			l_pData->m_sizeAddit.cx * l_pData->m_iCntAddit + l_pData->m_sizeCaption.cx + 20;
	}
}


void RedrawFrameIcon(HWND a_hWnd, int a_iHitTest)
{
	if (HitTestOnIcon(a_iHitTest))
	{
		RECT l_rect;
		GetRectByHitTest(a_hWnd, a_iHitTest, &l_rect);
		HRGN l_hRgn = ::CreateRectRgn(l_rect.left, l_rect.top, l_rect.right, l_rect.bottom);
		::SendMessage(a_hWnd, WM_NCPAINT, reinterpret_cast<WPARAM>(l_hRgn), 0);
		::DeleteObject(l_hRgn);
	}
}