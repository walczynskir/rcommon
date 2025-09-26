// RHyperlinkWnd.cpp: implementation of the RHyperlinkWnd control.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "RHyperlinkWnd.h"
#include "RMemDC.h"
#include "celltip.h"
#include "rstring.h"
#include <shellapi.h>
#include <commctrl.h>


class RHyperlinkData
{
public:
	RHyperlinkData(void) : m_sUrl(_T(""))
	{
		m_bFollowed = false;
		m_bHover = false;
		m_hFont = NULL;
		m_hCursor = NULL;
		m_hTip = NULL;
		m_hDefCursor = ::LoadCursor(NULL, IDC_HAND);
		SetFont(static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT)));
		m_colors.clrLink = RGB(0, 0, 255);	// Blue
		m_colors.clrActive = RGB(0, 128, 128);	// Dark cyan
		m_colors.clrVisited = RGB(128, 0, 128);	// Purple
		m_colors.clrHover = RGB(255, 0, 0);	// Red
	}

	~RHyperlinkData(void)
	{
		if (m_hDefCursor != NULL)
		{
			::DestroyCursor(m_hDefCursor);
		}
		DeleteFontObj();
		if (m_hTip != NULL)
		{
			::DestroyWindow(m_hTip);
		}
	}

	// make font underlined
	void SetFont(HFONT a_hFont)
	{
		LOGFONT l_lf;
		if (::GetObject(a_hFont, sizeof(l_lf), &l_lf) == 0)
		{
			return;
		}
		l_lf.lfUnderline = TRUE;
		DeleteFontObj();
		m_hFont = ::CreateFontIndirect(&l_lf);
	}

	HFONT GetFont() const { return m_hFont; }
	void DeleteFontObj() 
	{ 
		if (m_hFont != NULL)
		{
			::DeleteObject(m_hFont);
			m_hFont = NULL;
		}
	}

	RHYPERLINKCOLORS m_colors;
	tstring  m_sUrl;
	bool     m_bFollowed;
	bool	 m_bHover;
	HCURSOR  m_hCursor;
	HCURSOR  m_hDefCursor;
	HWND	 m_hTip;
private:
	HFONT    m_hFont;
};

static const long c_iWindowOfs = sizeof(RHyperlinkData*) - 4;

static inline RHyperlinkData* GetData(HWND a_hWnd);

// messages
static inline BOOL OnCreate(HWND a_hWnd, LPCREATESTRUCT a_lpStruct);
static inline void OnNcDestroy(HWND a_hWnd);
static inline void OnPaint(HWND a_hWnd);
static inline void OnMouseMove(HWND a_hWnd, int a_xPos, int a_yPos);
static inline void OnMouseLeave(HWND a_hWnd);
static inline void OnLButtonUp(HWND a_hWnd, int a_x, int a_y);
static inline LRESULT OnSetCursor(HWND a_hWnd);

static inline void OnSetFont(HWND a_hWnd, HFONT a_hFont, bool a_bRedraw);
static inline LRESULT OnGetFont(HWND a_hWnd);
static inline void OnSetColors(HWND a_hWnd, LPRHYPERLINKCOLORS a_pColors, bool a_bRedraw);
static inline void OnGetColors(HWND a_hWnd, LPRHYPERLINKCOLORS a_pColors);
static inline void OnSetFollowed(HWND a_hWnd, bool a_bFollowed, bool a_bRedraw);
static inline BOOL OnGetFollowed(HWND a_hWnd);
static inline LRESULT OnSetText(HWND a_hWnd, LPARAM a_lParam);
static inline LRESULT OnSetUrl(HWND a_hWnd, LPCTSTR a_sUrl);
static inline LRESULT OnGetUrl(HWND a_hWnd, UINT a_iMaxLen, LPTSTR a_psUrl);
static inline LRESULT OnGetUrlLength(HWND a_hWnd);
static inline void OnChangeCursor(HWND a_hWnd, HCURSOR a_hCursor);
static inline LRESULT OnGetCursor(HWND a_hWnd);

static inline void Draw(HWND a_hWnd, HDC a_hDC);

static void AdjustWindow(HWND a_hWnd);

static inline LRESULT Notify(HWND a_hWnd, LPNMHDR a_pNmHdr);
static HINSTANCE GotoURL(const tstring& a_sURL, int a_iShowCmd);
static void FollowLink(HWND a_hWnd);

#define HasWindowStyle(a_hWnd, a_iStyle) ((::GetWindowLong(a_hWnd, GWL_STYLE) & a_iStyle) == a_iStyle)


//TODO: Focusable
//TODO: Notify commands

//	---------------------------------------------------------------------------
//	Rejestracja klasy
//
ATOM RHyperlinkWnd_RegisterClass(void)
{

	WNDCLASSEX l_wcex;
	l_wcex.cbSize			= sizeof(WNDCLASSEX); 
	l_wcex.style			= CS_DBLCLKS | CS_GLOBALCLASS | CS_PARENTDC;
	l_wcex.lpfnWndProc		= RHyperlinkWnd_WndProc;
	l_wcex.cbClsExtra		= 0;
	l_wcex.cbWndExtra		= sizeof(RHyperlinkData*);
	l_wcex.hInstance		= RCommon_GetInstance();
	l_wcex.hIcon			= NULL;
	l_wcex.hCursor			= NULL;
	l_wcex.hbrBackground	= NULL;
	l_wcex.lpszMenuName		= NULL;
	l_wcex.lpszClassName	= RHyperlinkWnd_ClassName;
	l_wcex.hIconSm			= NULL;

	return ::RegisterClassEx(&l_wcex);
}


//	---------------------------------------------------------------------------------------
//	Main RHyperlinkWnd procedure
//
LRESULT CALLBACK 
RHyperlinkWnd_WndProc(
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

		case WM_LBUTTONUP:
			OnLButtonUp(a_hWnd, LOWORD(a_lParam), HIWORD(a_lParam));
			break;

		case WM_MOUSEMOVE:
			OnMouseMove(a_hWnd, LOWORD(a_lParam), HIWORD(a_lParam));
			break;

		case WM_MOUSELEAVE:
			OnMouseLeave(a_hWnd);
			break;

		case WM_SETCURSOR:
			return OnSetCursor(a_hWnd);

		case RHM_SETCOLORS:
			OnSetColors(a_hWnd, reinterpret_cast<LPRHYPERLINKCOLORS>(a_lParam), a_wParam == TRUE);
			break;

		case RHM_GETCOLORS:
			OnGetColors(a_hWnd, reinterpret_cast<LPRHYPERLINKCOLORS>(a_lParam));
			break;

		case RHM_SETFOLLOWED:
			OnSetFollowed(a_hWnd, a_lParam == TRUE, a_wParam == TRUE);
			break;

		case RHM_GETFOLLOWED:
			return OnGetFollowed(a_hWnd);

		case RHM_SETURL:
			return OnSetUrl(a_hWnd, reinterpret_cast<LPCTSTR>(a_lParam));

		case RHM_GETURL:
			return OnGetUrl(a_hWnd, static_cast<UINT>(a_wParam), reinterpret_cast<LPTSTR>(a_lParam));

		case RHM_GETURLLENGTH:
			return OnGetUrlLength(a_hWnd);

		case RHM_SETCURSOR:
			OnChangeCursor(a_hWnd, reinterpret_cast<HCURSOR>(a_lParam));
			break;

		case RHM_GETCURSOR:
			return OnGetCursor(a_hWnd);

		case WM_SETTEXT:
			return OnSetText(a_hWnd, a_lParam);

		case WM_SETFONT:
			OnSetFont(a_hWnd, reinterpret_cast<HFONT>(a_wParam), (LOWORD(a_lParam) == TRUE));
			break;

		case WM_GETFONT:
			return OnGetFont(a_hWnd);

		case WM_NCDESTROY:
			OnNcDestroy(a_hWnd);
			break;

		default:
			return ::DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
   }
   return 0;
}


//	---------------------------------------------------------------------------------------
//	Creation of RPinWnd window
//
HWND	// Handle of created window or NULL if failed
RHyperlinkWnd_CreateEx(
	DWORD		a_iStyle,		// style
	DWORD		a_iStyleEx,		// extended style
	LPCTSTR     a_sText,		// URL
	int			a_x,			// horizontal position of window
	int			a_y,			// vertical position of window
	int			a_iWidth,		// window width
	int			a_iHeight,		// window height
	HWND		a_hWndParent,   // handle to parent or owner window
	LPVOID		a_lpParam			// pointer to window-creation data
	)
{
	HWND l_hWnd = ::CreateWindowEx(a_iStyleEx, RHyperlinkWnd_ClassName, a_sText, a_iStyle,
		a_x, a_y, a_iWidth, a_iHeight, a_hWndParent, NULL, RCommon_GetInstance(), a_lpParam);
	ASSERT(GetData(l_hWnd) != NULL);

	return l_hWnd;
}


//	---------------------------------------------------------------------------------------
//	Returns control's data
//
static RHyperlinkData*	//OUT pointer to this data
GetData(
	HWND a_hWnd	//IN 
	)
{
#pragma warning(disable: 4312)
	return reinterpret_cast<RHyperlinkData*>(::GetWindowLongPtr(a_hWnd, c_iWindowOfs));
#pragma warning(default: 4312)
}


//	---------------------------------------------------------------------------------------
//	Sets control's data
//
static void	
SetData(
	HWND a_hWnd,	//IN
	RHyperlinkData* a_pData	//IN
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
	RHyperlinkData* l_pData = new RHyperlinkData();
	SetData(a_hWnd, l_pData);
	AdjustWindow(a_hWnd);
	return TRUE;
}


//	---------------------------------------------------------------------------------------
//	Postmortem:-)
//
void OnNcDestroy(
	HWND a_hWnd		//IN
	)
{
	const RHyperlinkData* l_pData = GetData(a_hWnd);
	delete l_pData;
}


//	---------------------------------------------------------------------------------------
//	Font setting
//
void OnSetFont(HWND a_hWnd, HFONT a_hFont, bool a_bRedraw)
{
	ASSERT(a_hWnd != NULL);
	ASSERT(::IsWindow(a_hWnd));
	if (a_hFont != NULL)
	{
		GetData(a_hWnd)->SetFont(a_hFont);
	}
	AdjustWindow(a_hWnd);

	if (a_bRedraw)
	{
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}
}


//	---------------------------------------------------------------------------------------
//	Font getting
//
LRESULT OnGetFont(HWND a_hWnd)
{
	ASSERT(a_hWnd != NULL);
	ASSERT(::IsWindow(a_hWnd));
	return reinterpret_cast<LRESULT>(GetData(a_hWnd)->GetFont());
}


//	---------------------------------------------------------------------------------------
//	Drawing - WM_PAINT message handler (all drawing here)
//
void OnPaint(
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
//	Really draws HyperlinkWnd control
//
void Draw(
	HWND a_hWnd, 
	HDC a_hDC
	)
{
	RHyperlinkData* l_pData = GetData(a_hWnd);
	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);
	HFONT l_hFontOld = (HFONT)::SelectObject(a_hDC, l_pData->GetFont());
	UINT l_iDrawStyle = DT_VCENTER | DT_SINGLELINE;
	if (HasWindowStyle(a_hWnd, HS_CENTER))
	{
		l_iDrawStyle = l_iDrawStyle | DT_CENTER;
	}
	else if (HasWindowStyle(a_hWnd, HS_RIGHT))
	{
		l_iDrawStyle = l_iDrawStyle | DT_RIGHT;
	}
	else
	{
		l_iDrawStyle = l_iDrawStyle | DT_LEFT;
	}
	COLORREF l_clr = 
		l_pData->m_bHover ? l_pData->m_colors.clrHover : 
			l_pData->m_bFollowed ? l_pData->m_colors.clrVisited : l_pData->m_colors.clrLink;

	COLORREF l_clrOld = ::SetTextColor(a_hDC, l_clr);
	int l_iBkModeOld = ::SetBkMode(a_hDC, TRANSPARENT);

	TCHAR l_sText[1024];
	::GetWindowText(a_hWnd, l_sText, ArraySize(l_sText));
	::DrawText(a_hDC, l_sText, _tcslen(l_sText), 
		&l_rect, l_iDrawStyle);
	::SetTextColor(a_hDC, l_clrOld);
	::SetBkMode(a_hDC, l_iBkModeOld);
	::SelectObject(a_hDC, l_hFontOld);
}


void OnSetColors(HWND a_hWnd, LPRHYPERLINKCOLORS a_pColors, bool a_bRedraw)
{
	ASSERT(::IsWindow(a_hWnd));
	memcpy(&(GetData(a_hWnd)->m_colors), a_pColors, sizeof(RHYPERLINKCOLORS));
	if (a_bRedraw)
	{
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}
}


void OnGetColors(HWND a_hWnd, LPRHYPERLINKCOLORS a_pColors)
{
	ASSERT(::IsWindow(a_hWnd));
	memcpy(a_pColors, &(GetData(a_hWnd)->m_colors), sizeof(RHYPERLINKCOLORS));
}


void OnSetFollowed(HWND a_hWnd, bool a_bFollowed, bool a_bRedraw)
{
	ASSERT(::IsWindow(a_hWnd));
	GetData(a_hWnd)->m_bFollowed = a_bFollowed;
	if (a_bRedraw)
	{
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

BOOL OnGetFollowed(HWND a_hWnd)
{
	ASSERT(::IsWindow(a_hWnd));
	return GetData(a_hWnd)->m_bFollowed;
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


LRESULT OnSetText(HWND a_hWnd, LPARAM a_lParam)
{
	ASSERT(::IsWindow(a_hWnd));
	LRESULT l_res = ::DefWindowProc(a_hWnd, WM_SETTEXT, 0, a_lParam);
	AdjustWindow(a_hWnd);

	return l_res;
}


LRESULT OnSetUrl(HWND a_hWnd, LPCTSTR a_sUrl)
{
	ASSERT(::IsWindow(a_hWnd));
	GetData(a_hWnd)->m_sUrl = a_sUrl;
	if (GetData(a_hWnd)->m_hTip != NULL)
	{
		::SetWindowText(GetData(a_hWnd)->m_hTip, a_sUrl);
	}
	return TRUE;
}


LRESULT OnGetUrl(HWND a_hWnd, UINT a_iMaxLen, LPTSTR a_psUrl)
{
	ASSERT(::IsWindow(a_hWnd));
	_tcsncpy_s(a_psUrl, a_iMaxLen, GetData(a_hWnd)->m_sUrl.c_str(), GetData(a_hWnd)->m_sUrl.length());
	return min(GetData(a_hWnd)->m_sUrl.length(), a_iMaxLen);
}


//	---------------------------------------------------------------------------------------
//	returns length of URL address
//
LRESULT OnGetUrlLength(HWND a_hWnd)
{
	ASSERT(::IsWindow(a_hWnd));
	return GetData(a_hWnd)->m_sUrl.length();
}


//	---------------------------------------------------------------------------------------
//	sets cursor for HyperlinkWnd
//
void OnChangeCursor(HWND a_hWnd, HCURSOR a_hCursor)
{
	ASSERT(::IsWindow(a_hWnd));
	GetData(a_hWnd)->m_hCursor = a_hCursor;
}


//	---------------------------------------------------------------------------------------
//	returns current control cursor (if not default)
//
LRESULT OnGetCursor(HWND a_hWnd)
{
	ASSERT(::IsWindow(a_hWnd));
	return reinterpret_cast<LPARAM>(GetData(a_hWnd)->m_hCursor);

}


//	---------------------------------------------------------------------------------------
//	WM_MOUSEMOVE handler
//
void OnMouseMove(HWND a_hWnd, int a_xPos, int a_yPos)
{
	RHyperlinkData* l_pData = GetData(a_hWnd);	
	if (!l_pData->m_bHover)
	{
		l_pData->m_bHover = true;
		TRACKMOUSEEVENT l_track = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, a_hWnd, 1};
		::TrackMouseEvent(&l_track);
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
		if (!HasWindowStyle(a_hWnd, HS_NOTIP))
		{
			if (l_pData->m_hTip == NULL)
			{
				RCellTip_Register();
				l_pData->m_hTip = RCellTip_Create(a_hWnd, RCTS_FRAME | RCTS_ANIMATE);
				::SetWindowText(l_pData->m_hTip, l_pData->m_sUrl.c_str());
			}
			POINT l_pt = {a_xPos, a_yPos};
			::ClientToScreen(a_hWnd, &l_pt);
			l_pt.x += (::GetSystemMetrics(SM_CXCURSOR) / 2);
			l_pt.y += (::GetSystemMetrics(SM_CYCURSOR) / 2);
			RCellTip_PopUp(l_pData->m_hTip, &l_pt);
		}
		return;
	}

}


//	---------------------------------------------------------------------------------------
//	WM_MOUSELEAVE handler
//
void OnMouseLeave(HWND a_hWnd)
{
	RHyperlinkData* l_pData = GetData(a_hWnd);
	if (l_pData->m_bHover)
	{
		l_pData->m_bHover = false;
		if (l_pData->m_hTip != NULL)
		{
			RCellTip_Pop(l_pData->m_hTip);
		}
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}
}


void OnLButtonUp(HWND a_hWnd, int /*a_x*/, int /*a_y*/)
{
	NMHDR l_nmhdr;
	l_nmhdr.code = NM_CLICK;
	l_nmhdr.idFrom = ::GetDlgCtrlID(a_hWnd);
	l_nmhdr.hwndFrom = a_hWnd;

	HWND l_hParent = ::GetParent(a_hWnd);
	if (l_hParent == NULL)
	{
		l_hParent = a_hWnd;
	}

	if (::SendMessage(l_hParent, WM_NOTIFY, l_nmhdr.idFrom, reinterpret_cast<LPARAM>(&l_nmhdr)) == 0)
	{
		FollowLink(a_hWnd);
	}
}


// ---------------------------------------------------------
// Sets cursor if needed
// 
LRESULT OnSetCursor(HWND a_hWnd)
{
	RHyperlinkData* l_pData = GetData(a_hWnd);
	if (l_pData->m_hCursor != NULL)
	{
		::SetCursor(l_pData->m_hCursor);
		return TRUE;
	}
	else if (l_pData->m_hDefCursor != NULL)
	{
		::SetCursor(l_pData->m_hDefCursor);
		return TRUE;
	}
	return FALSE;
}


// ---------------------------------------------------------
// GotoURL Function
// 
HINSTANCE GotoURL(const tstring& a_sURL, int a_iShowCmd)
{
    SHELLEXECUTEINFO l_sei;
	ZeroMemory(&l_sei, sizeof(l_sei));
	l_sei.cbSize = sizeof(l_sei);
	l_sei.fMask = 0;
	l_sei.hwnd = ::GetDesktopWindow();
	l_sei.lpVerb = _T("open");
	l_sei.lpFile = a_sURL.c_str();
	l_sei.nShow = a_iShowCmd;
    ::ShellExecuteEx(&l_sei);
	return l_sei.hInstApp;
//return ::ShellExecute(NULL, _T("open"), a_sURL.c_str(), NULL, NULL, a_iShowCmd);
}


// ---------------------------------------------------------
// FollowLink function
// 
void FollowLink(HWND a_hWnd) 
{	
	RHyperlinkData* l_pData = GetData(a_hWnd);
	HINSTANCE l_hRes = GotoURL(l_pData->m_sUrl, SW_SHOW);
#pragma warning (disable: 4311)	
    if (reinterpret_cast<int>(l_hRes) <= HINSTANCE_ERROR) 
#pragma warning (default: 4311)	
	{
        MessageBeep(MB_ICONEXCLAMATION);	// Unable to follow link
		return;
    } 
	// Mark link as followed and repaint window
	l_pData->m_bFollowed = TRUE;		
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

// ---------------------------------------------------------
//
void AdjustWindow(HWND a_hWnd)
{	
	RHyperlinkData* l_pData = GetData(a_hWnd);

    RECT l_rectWnd;
	::GetWindowRect(a_hWnd, &l_rectWnd);
	
	// Get the current client rect
	RECT l_rectClient;
	::GetClientRect(a_hWnd, &l_rectClient);

	// Calc borders size based on window and client rects
	int l_iBorderWidth = RectWidth(l_rectWnd) - RectWidth(l_rectClient);
	int l_iBorderHeight = RectHeight(l_rectWnd) - RectHeight(l_rectClient);

	HDC l_hDC = ::GetDC(a_hWnd);
	HFONT l_hFontOld = static_cast<HFONT>(::SelectObject(l_hDC, l_pData->GetFont()));
	SIZE l_size;

	TCHAR l_sText[1024];
	::GetWindowText(a_hWnd, l_sText, ArraySize(l_sText));
	::GetTextExtentPoint32(l_hDC, l_sText, _tcslen(l_sText), &l_size);

	if (HasWindowStyle(a_hWnd, HS_CENTER))
	{
		// we must move right and down our control by half of differences of size (dx and dy)
		int l_dxMove = (RectWidth(l_rectWnd) - (l_size.cx + l_iBorderWidth)) / 2;
		int l_dyMove = (RectHeight(l_rectWnd) - (l_size.cy + l_iBorderHeight)) / 2;
		POINT l_pt = { l_rectWnd.left, l_rectWnd.top };
		if (::GetParent(a_hWnd) != NULL)
		{
			::ScreenToClient(::GetParent(a_hWnd), &l_pt);
		}

		::SetWindowPos(a_hWnd, NULL, l_pt.x + l_dxMove, l_pt.y + l_dyMove, l_size.cx + l_iBorderWidth, l_size.cy + l_iBorderHeight, SWP_NOZORDER | SWP_NOREDRAW);
	}
	else if (HasWindowStyle(a_hWnd, HS_RIGHT))
	{
		// we must move right and down our control by differences of size (dx and dy)
		int l_dxMove = (RectWidth(l_rectWnd) - (l_size.cx + l_iBorderWidth));
		int l_dyMove = (RectHeight(l_rectWnd) - (l_size.cy + l_iBorderHeight));
		POINT l_pt = { l_rectWnd.left, l_rectWnd.top };
		if (::GetParent(a_hWnd) != NULL)
		{
			::ScreenToClient(::GetParent(a_hWnd), &l_pt);
		}

		::SetWindowPos(a_hWnd, NULL, l_pt.x + l_dxMove, l_pt.y + l_dyMove, l_size.cx + l_iBorderWidth, l_size.cy + l_iBorderHeight, SWP_NOZORDER | SWP_NOREDRAW);
	}
	else
	{
		::SetWindowPos(a_hWnd, NULL, 0, 0, l_size.cx + l_iBorderWidth, l_size.cy + l_iBorderHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
	}

	::SelectObject(l_hDC, l_hFontOld);
	::ReleaseDC(a_hWnd, l_hDC);
}



