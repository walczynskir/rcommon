// RKeyWnd.cpp: implementation of the RKeyWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "RKeyWnd.h"
#include "RKeyData.h"
#include "RTheme.h"
#include "RMemDC.h"
#include "RKeyboard.h"

#define WM_THEMECHANGED                 0x031A

static const long c_iWindowOfs = sizeof(RKeyData*) - 4;
static HHOOK s_hook = NULL;


static inline RKeyData* GetRKeyData(HWND a_hWnd);
static inline int KeyToVirtCode(int a_iKey);
static LRESULT CALLBACK HookMessageProc(int code, WPARAM wParam, LPARAM lParam);


// messages
static inline BOOL OnCreate(HWND a_hWnd, LPCREATESTRUCT a_lpStruct);
static inline void OnDestroy(HWND a_hWnd);
static inline void OnPaint(HWND a_hWnd);
static inline void OnKeyDown(HWND a_hWnd, WPARAM a_iKey, LPARAM a_iFlags);
static inline void OnSetFocus(HWND a_hWnd);
static inline void OnSetFont(HWND a_hWnd, HFONT a_hFont, bool a_bRedraw);
static inline void OnKillFocus(HWND a_hWnd);
static inline void OnSetEmptyText(HWND a_hWnd, LPCTSTR a_sText);
static inline void OnGetEmptyText(HWND a_hWnd, LPTSTR a_psText, UINT a_iLen);
static inline void OnSetKey(HWND a_hWnd, LPRSHORTCUTKEY a_key);
static inline void OnGetKey(HWND a_hWnd, LPRSHORTCUTKEY a_key);
static inline LRESULT OnGetText(HWND a_hWnd, UINT a_iMaxLen, LPTSTR a_psStr);
static inline LRESULT OnGetTextLength(HWND a_hWnd);

static inline void Draw(HWND a_hWnd, HDC a_hDC);
static inline void GetDrawText(HWND a_hWnd, tstring* a_psDraw);
static inline void GetKeyDesc(LPRSHORTCUTKEY a_pKey, tstring* a_psDraw);

static inline LRESULT Notify(HWND a_hWnd, LPNMHDR a_pNmHdr);

static inline void CopyShortcutKey(LPRSHORTCUTKEY a_pKeyDst, LPRSHORTCUTKEY a_pKeySrc);

#define KeyPressed(vkey, code) (((::GetKeyState(vkey) & 0xF000) == 0xF000) ? code : 0)
#define HasStyle(hwnd, style) ((::GetWindowLong(hwnd, GWL_STYLE) & style) == style)

//	---------------------------------------------------------------------------------------
//	Main RKeyWnd procedure
//
LRESULT CALLBACK 
RKeyWnd_WndProc(
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

	    case WM_SYSKEYDOWN:
			OnKeyDown(a_hWnd, a_wParam, a_lParam);
			return ::DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);

		case WM_KEYDOWN:
			OnKeyDown(a_hWnd, a_wParam, a_lParam);
			break;

		case WM_KEYUP:
        case WM_CHAR:
			break;

		case WM_LBUTTONDOWN:
			::SetFocus(a_hWnd);
			break;

		case WM_THEMECHANGED:
			::RedrawWindow(a_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
			break;

		case WM_SETFOCUS:
			OnSetFocus(a_hWnd);
			break;

		case WM_KILLFOCUS:
			OnKillFocus(a_hWnd);
			break;

		case RCM_SETEMPTYTEXT:
			OnSetEmptyText(a_hWnd, reinterpret_cast<LPCTSTR>(a_lParam));
			break;

		case RCM_GETEMPTYTEXT:
			OnGetEmptyText(a_hWnd, reinterpret_cast<LPTSTR>(a_lParam), a_wParam);
			break;

		case RCM_SETKEY:
			OnSetKey(a_hWnd, reinterpret_cast<LPRSHORTCUTKEY>(a_lParam));
			break;

		case RCM_GETKEY:
			OnGetKey(a_hWnd, reinterpret_cast<LPRSHORTCUTKEY>(a_lParam));
			break;

		case WM_GETTEXT:
			return OnGetText(a_hWnd, static_cast<UINT>(a_wParam), reinterpret_cast<LPTSTR>(a_lParam));

		case WM_GETTEXTLENGTH:
			return OnGetTextLength(a_hWnd);

		case WM_SETFONT:
			OnSetFont(a_hWnd, reinterpret_cast<HFONT>(a_wParam), (a_lParam == TRUE));
			break;

		case WM_DESTROY:
			OnDestroy(a_hWnd);
			break;

		default:
			return ::DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
   }
   return 0;
}


//	---------------------------------------------------------------------------------------
//	Creation of RKeyWnd window
//
HWND	// Handle of created window or NULL if failed
RKeyWnd_CreateEx(
	DWORD		a_iStyle,		// style
	DWORD		a_iStyleEx,		// extended style
	int			a_x,			// horizontal position of window
	int			a_y,			// vertical position of window
	int			a_iWidth,		// window width
	int			a_iHeight,		// window height
	HWND		a_hWndParent,   // handle to parent or owner window
	LPVOID		a_lpParam			// pointer to window-creation data
	)
{
	HWND l_hWnd = ::CreateWindowEx(a_iStyleEx, RKeyWnd_ClassName, _T(""), a_iStyle,
		a_x, a_y, a_iWidth, a_iHeight, a_hWndParent, NULL, RCommon_GetInstance(), a_lpParam);

	return l_hWnd;
}


//	---------------------------------------------------------------------------------------
//	Returns control's data
//
static RKeyData*	//OUT pointer to this data
GetRKeyData(
	HWND a_hWnd	//IN 
	)
{
#pragma warning(disable: 4312)
	return reinterpret_cast<RKeyData*>(::GetWindowLongPtr(a_hWnd, c_iWindowOfs));
#pragma warning(default: 4312)
}


//	---------------------------------------------------------------------------------------
//	Sets control's data
//
static void	
SetRKeyData(
	HWND a_hWnd,	//IN
	RKeyData* a_pData	//IN
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
	RKeyData* l_pData = new RKeyData();
	SetRKeyData(a_hWnd, l_pData);
	return TRUE;
}


//	---------------------------------------------------------------------------------------
//	Postmortem:-)
//
void OnDestroy(
	HWND a_hWnd		//IN
	)
{
	const RKeyData* l_pData = GetRKeyData(a_hWnd);
	delete l_pData;
}


//	---------------------------------------------------------------------------------------
//	Font setting
//
void OnSetFont(HWND a_hWnd, HFONT a_hFont, bool a_bRedraw)
{
	ASSERT(a_hWnd != NULL);
	ASSERT(::IsWindow(a_hWnd));
	RKeyData* l_pData = GetRKeyData(a_hWnd);
	l_pData->m_hFont = a_hFont;
	if (a_bRedraw)
	{
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}
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
//	Really draws KeyWnd control
//
void Draw(
	HWND a_hWnd, 
	HDC a_hDC
	)
{
	RKeyData* l_pData = GetRKeyData(a_hWnd);
	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);
	RTheme l_theme;
	tstring l_sDraw;
	GetDrawText(a_hWnd, &l_sDraw);
	HFONT l_hFontOld = (HFONT)::SelectObject(a_hDC, l_pData->m_hFont);
	UINT l_iDrawStyle = DT_LEFT;
	if (HasStyle(a_hWnd, RKS_CENTER))
	{
		l_iDrawStyle = DT_CENTER;
	}
	else if (HasStyle(a_hWnd, RKS_RIGHT))
	{
		l_iDrawStyle = DT_RIGHT;
	}
	l_iDrawStyle = l_iDrawStyle | DT_VCENTER | DT_SINGLELINE;
	if (l_theme.OpenData(a_hWnd, L"EDIT") != NULL)
	{
		int l_iState = (::GetFocus() == a_hWnd) ? ETS_FOCUSED : ETS_NORMAL;
		l_theme.DrawBackground(a_hDC, EP_EDITTEXT, l_iState, &l_rect);
		l_theme.DrawBackground(a_hDC, EP_CARET, l_iState, &l_rect);
		RECT l_rectContent;
		l_theme.GetBackgroundContentRect(a_hDC, BP_PUSHBUTTON, 
				l_iState, &l_rect, &l_rectContent);
		l_rectContent.left += 2;

		if (l_pData->m_rkey.iKey == 0 && l_pData->m_rkey.iMod == 0)
		{
			COLORREF l_clrOld = ::SetTextColor(a_hDC, l_pData->m_clrText);
			::DrawText(a_hDC, l_sDraw.c_str(), static_cast<int>(l_sDraw.length()), &l_rectContent, l_iDrawStyle);
			::SetTextColor(a_hDC, l_clrOld);
		}
		else
		{
			l_theme.DrawText(a_hDC, EP_EDITTEXT, l_iState, l_sDraw.c_str(), static_cast<int>(l_sDraw.length()),
				l_iDrawStyle, 0, &l_rectContent);
		}
	}
	else
	{
		::FillRect(a_hDC, &l_rect, ::CreateSolidBrush(::GetSysColor(COLOR_WINDOW)));
		COLORREF l_clrOld = ::SetTextColor(a_hDC, l_pData->m_clrText);
		int l_iBkModeOld = ::SetBkMode(a_hDC, TRANSPARENT);
		::DrawText(a_hDC, l_sDraw.c_str(), static_cast<int>(l_sDraw.length()), 
			&l_rect, l_iDrawStyle);
		::SetBkMode(a_hDC, l_iBkModeOld);
		::SetTextColor(a_hDC, l_clrOld);
	}
	::SelectObject(a_hDC, l_hFontOld);
}


//	---------------------------------------------------------------------------
//	Rejestracja klasy
//
ATOM 
RKeyWnd_RegisterClass()
{

	WNDCLASSEX l_wcex;
	l_wcex.cbSize			= sizeof(WNDCLASSEX); 
	l_wcex.style			= CS_DBLCLKS | CS_GLOBALCLASS | CS_PARENTDC;
	l_wcex.lpfnWndProc		= RKeyWnd_WndProc;
	l_wcex.cbClsExtra		= 0;
	l_wcex.cbWndExtra		= sizeof(RKeyData*);
	l_wcex.hInstance		= RCommon_GetInstance();
	l_wcex.hIcon			= NULL;
	l_wcex.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
	l_wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW - 1);
	l_wcex.lpszMenuName		= NULL;
	l_wcex.lpszClassName	= RKeyWnd_ClassName;
	l_wcex.hIconSm			= NULL;

	return ::RegisterClassEx(&l_wcex);
}


void OnKeyDown(HWND a_hWnd, WPARAM a_iKey, LPARAM /*a_iFlags*/)
{
	RKeyData* l_pData = GetRKeyData(a_hWnd);
	BYTE l_btVirt = 0;
	l_btVirt |= KeyPressed(VK_CONTROL, MOD_CONTROL);
	l_btVirt |= KeyPressed(VK_MENU, MOD_ALT);
	l_btVirt |= KeyPressed(VK_SHIFT, MOD_SHIFT);

	if (a_iKey == VK_TAB || a_iKey == VK_ESCAPE)
	{
		return;
	}

	if (a_iKey == VK_BACK && l_btVirt == 0)
	{
		l_pData->ClearKey();
		::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
		return;
	}

	if ((l_btVirt == 0) || (l_btVirt == MOD_SHIFT && (a_iKey >= 'A' && a_iKey <= 'Z')))
	{
		return;
	}

	if (a_iKey != VK_CONTROL && a_iKey != VK_MENU && a_iKey != VK_SHIFT)
	{
		if ((l_pData->m_rkey.iKey != a_iKey) || (l_pData->m_rkey.iMod != l_btVirt))
		{
			RSHORTCUTKEY l_keyOld;
			CopyShortcutKey(&l_keyOld, &(l_pData->m_rkey));
			l_pData->m_rkey.iMod = l_btVirt;
			l_pData->m_rkey.iKey = static_cast<WORD>(a_iKey);
			::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			RKNMCHG l_nm;
			l_nm.nmhdr.code = RKN_CHANGE;
			CopyShortcutKey(&(l_nm.keyOld), &(l_keyOld));
			CopyShortcutKey(&(l_nm.keyNew), &(l_pData->m_rkey));
			Notify(a_hWnd, reinterpret_cast<LPNMHDR>(&l_nm));
		}
	}
}


void OnSetFocus(HWND a_hWnd)
{
	s_hook = ::SetWindowsHookEx(WH_MSGFILTER, HookMessageProc, NULL, ::GetCurrentThreadId());
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	NMHDR l_nmhdr;
	l_nmhdr.code = RKN_SETFOCUS;
	Notify(a_hWnd, &l_nmhdr);
}


void OnKillFocus(HWND a_hWnd)
{
	::UnhookWindowsHookEx(s_hook);
	::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	NMHDR l_nmhdr;
	l_nmhdr.code = RKN_KILLFOCUS;
	Notify(a_hWnd, &l_nmhdr);
}


void GetDrawText(HWND a_hWnd, tstring* a_psDraw)
{
	RKeyData* l_pData = GetRKeyData(a_hWnd);
	if (l_pData->m_rkey.iKey == 0 && l_pData->m_rkey.iMod == 0)
	{
		*a_psDraw = l_pData->m_sEmptyText;
		return;
	}
	GetKeyDesc(&(l_pData->m_rkey), a_psDraw);
}


void GetKeyDesc(LPRSHORTCUTKEY a_pKey, tstring* a_psDraw)
{
	*a_psDraw = _T("");
	if ((a_pKey->iMod & MOD_CONTROL) == MOD_CONTROL)
	{
		*a_psDraw = _T("Ctrl");
	}
	if ((a_pKey->iMod & MOD_ALT) == MOD_ALT)
	{
		if ((a_pKey->iMod & MOD_CONTROL) == MOD_CONTROL)
		{
			*a_psDraw += _T("-Alt");
		}
		else
		{
			*a_psDraw = _T("Alt");
		}
	}
	if ((a_pKey->iMod & MOD_SHIFT) == MOD_SHIFT)
	{
		if ((a_pKey->iMod & MOD_CONTROL) == MOD_CONTROL ||
			(a_pKey->iMod & MOD_ALT) == MOD_ALT
			)
		{
			*a_psDraw += _T("-Shift");
		}
		else
		{
			*a_psDraw = _T("Shift");
		}
	}

	TCHAR l_sName[100];
	::GetKeyNameText(::MapVirtualKey(a_pKey->iKey, 0) << 16, l_sName, sizeof(l_sName) / sizeof(l_sName[0]));
	tstring l_tsName(l_sName);
	*a_psDraw = *a_psDraw + _T("-") + l_tsName;
}


LRESULT CALLBACK HookMessageProc(int a_iCode, WPARAM a_wParam, LPARAM a_lParam)
{
	if (a_iCode != MSGF_DIALOGBOX) 
	{
		return ::CallNextHookEx(s_hook, a_iCode, a_wParam, a_lParam);
	}

	LPMSG l_lpMsg = reinterpret_cast<LPMSG>(a_lParam);
	if (l_lpMsg->message == WM_SYSCHAR) 
	{
		return 1;
	}
	if (
		(l_lpMsg->message == WM_KEYDOWN) && 
		(l_lpMsg->wParam >= _T('A') && l_lpMsg->wParam <= _T('Z')) && 
		IsKeyPressed(VK_SHIFT)
	   )
	{
		return 1;
	}

	if (l_lpMsg->message == WM_KEYDOWN && l_lpMsg->wParam == VK_RETURN)
	{
		::SendMessage(l_lpMsg->hwnd, l_lpMsg->message, l_lpMsg->wParam, l_lpMsg->lParam);
		return 1;
	}
	return ::CallNextHookEx(s_hook, a_iCode, a_wParam, a_lParam);
}

void OnSetEmptyText(HWND a_hWnd, LPCTSTR a_sText)
{
	RKeyData* l_pData = GetRKeyData(a_hWnd);
	l_pData->m_sEmptyText = a_sText;
}


void OnGetEmptyText(HWND a_hWnd, LPTSTR a_psText, UINT a_iLen)
{
	RKeyData* l_pData = GetRKeyData(a_hWnd);
	_tcsncpy_s(a_psText, a_iLen, l_pData->m_sEmptyText.c_str(), l_pData->m_sEmptyText.length());
	a_psText[l_pData->m_sEmptyText.length()] = _T('\0');

}

void OnSetKey(HWND a_hWnd, LPRSHORTCUTKEY a_pKey)
{
	RKeyData* l_pData = GetRKeyData(a_hWnd);
	CopyShortcutKey(&(l_pData->m_rkey), a_pKey);
}

void OnGetKey(HWND a_hWnd, LPRSHORTCUTKEY a_key)
{
	RKeyData* l_pData = GetRKeyData(a_hWnd);
	a_key->iKey = l_pData->m_rkey.iKey;
	a_key->iMod = l_pData->m_rkey.iMod;
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

void CopyShortcutKey(LPRSHORTCUTKEY a_pKeyDst, LPRSHORTCUTKEY a_pKeySrc)
{
	a_pKeyDst->iKey = a_pKeySrc->iKey;
	a_pKeyDst->iMod = a_pKeySrc->iMod;
}


LRESULT OnGetText(HWND a_hWnd, UINT a_iMaxLen, LPTSTR a_psStr)
{
	tstring l_sText;
	GetDrawText(a_hWnd, &l_sText);
	_tcsncpy_s(a_psStr, a_iMaxLen, l_sText.c_str(), l_sText.length());
	return min(l_sText.length(), a_iMaxLen);

}


LRESULT OnGetTextLength(HWND a_hWnd)
{
	tstring l_sText;
	GetDrawText(a_hWnd, &l_sText);
	return l_sText.length();
}


int RKeyDown_GetKeyDesc(LPRSHORTCUTKEY a_pKey, LPTSTR a_sDesc, UINT a_iDescLen)
{
	tstring l_sKeyDesc;
	GetKeyDesc(a_pKey, &l_sKeyDesc);
	if (l_sKeyDesc.length() > a_iDescLen)
	{
		return 0;
	}

	_tcsncpy_s(a_sDesc, a_iDescLen, l_sKeyDesc.c_str(), l_sKeyDesc.length());
	return 1;
}


UINT RKeyDown_ToFModifier(BYTE a_btMod)
{
	BYTE l_btMod = (a_btMod & MOD_CONTROL) ? FCONTROL : 0;
	l_btMod |= (a_btMod & MOD_ALT) ? FALT : 0;
	l_btMod |= (a_btMod & MOD_SHIFT) ? FSHIFT : 0;
	return l_btMod | FVIRTKEY;
}


UINT RKeyDown_ToMModifier(BYTE a_btMod)
{
	BYTE l_btMod = (a_btMod & FCONTROL) ? MOD_CONTROL : 0;
	l_btMod |= (a_btMod & FALT) ? MOD_ALT : 0;
	l_btMod |= (a_btMod & FSHIFT) ? MOD_SHIFT : 0;
	return l_btMod;
}