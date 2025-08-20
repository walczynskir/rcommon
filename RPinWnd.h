#pragma once
#include "rcommon.h"
// RPinWnd.h: interface for the RPinWnd control
//
//////////////////////////////////////////////////////////////////////

#define RPinWnd_ClassName _T("RPINWND")



#define PS_TITLETOP	   0x0000
#define PS_TITLELEFT   0x0001
#define PS_TITLERIGHT  0x0002
#define PS_TITLEBOTTOM 0x0004
#define PS_TITLEMASK   (PS_TITLETOP | PS_TITLELEFT | PS_TITLERIGHT | PS_TITLEBOTTOM)

extern RCOMMON_API ATOM RPinWnd_RegisterClass(void);
extern RCOMMON_API HWND RPinWnd_CreateEx(DWORD a_iStyle, DWORD a_iStyleEx, LPCTSTR a_sTitle, int a_x, int a_y, int a_iWidth,
	int a_iHeight, HWND	a_hWndParent, LPVOID a_lpParam);
extern RCOMMON_API LRESULT CALLBACK RPinWnd_WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);

// control messages
// sets WND WPARAM = unused LPARAM = HWND to set
#define RPWM_SETWND (WM_USER + 1001)
// gets WND WPARAM = unused LPARAM = pointer to HWND 
#define RPWM_GETWND (WM_USER + 1002)

#define RPinWnd_SetWnd(a_hWnd, a_hInnerWnd) ::SendMessage(a_hWnd, RPWM_SETWND, 0, reinterpret_cast<LPARAM>(a_hInnerWnd))
#define RPinWnd_GetWnd(a_hWnd, a_phInnerWnd) ::SendMessage(a_hWnd, RPWM_GETWND, 0, reinterpret_cast<LPARAM>(a_phInnerWnd))

// set caption color WPARAM = array count LPARAM = COLORREF[4]
#define RPWM_SETCAPTIONCOLOR (WM_USER + 1003)
// get caption color WPARAM = array count LPARAM = COLORREF[4]
#define RPWM_GETCAPTIONCOLOR (WM_USER + 1004)
#define RPinWnd_SetCaptionColor(a_hWnd, a_iCnt, a_pColors) ::SendMessage(a_hWnd, RPWM_SETCAPTIONCOLOR, a_iCnt, reinterpret_cast<LPARAM>(a_pColors))
#define RPinWnd_GetCaptionColor(a_hWnd, a_iCnt, a_pColors) ::SendMessage(a_hWnd, RPWM_GETCAPTIONCOLOR, a_iCnt, reinterpret_cast<LPARAM>(a_pColors))


// animate show WND WPARAM = short (animate style) LPARAM = LPRECT (size and pos of window)
#define RPWM_SHOW (WM_USER + 1005)
#define RPinWnd_Show(a_hWnd, a_nStyle, a_pRect) ::SendMessage(a_hWnd, RPWM_SHOW, a_nStyle, reinterpret_cast<LRESULT>(a_pRect))

// animate hide WND WPARAM = short (animate style) LPARAM = unused
#define RPWM_HIDE (WM_USER + 1006)
#define RPinWnd_Hide(a_hWnd, a_nStyle) ::SendMessage(a_hWnd, RPWM_HIDE, a_nStyle, 0L)

typedef struct S_ADDCAPTIONICON
{
	HINSTANCE hInst;
	COLORREF  clrMask;
	int       iCnt;
	BOOL	  bCaption;
	union
	{
		UINT	 nID;
		HBITMAP  hBmp; 
	} bmp;
	union
	{
		UINT*    arrIdTip;
		LPCTSTR* arrStrTip;
	} tip;
}  RADDCICON, *LPRADDCICON;


// add caption icons WPARAM = unused LPARAM = LPRADDCICON
#define RPWM_ADDCAPTIONICON (WM_USER + 1007)
#define RPinWnd_AddCaptionIcon(a_hWnd, a_pBmp) ::SendMessage(a_hWnd, RPWM_ADDCAPTIONICON, 0, reinterpret_cast<LPARAM>(a_pBmp))

// sets delay (ms) WPARAM = unused LPARAM = delay int
#define RPWM_SETDELAY (WM_USER + 1008)
// gets delay (ms) WPARAM = unused LPARAM = unused LRESULT - delay 
#define RPWM_GETDELAY (WM_USER + 1009)

#define RPinWnd_SetDelay(a_hWnd, a_iDelay) ::SendMessage(a_hWnd, RPWM_SETDELAY, 0, a_iDelay)
#define RPinWnd_GetDelay(a_hWnd)		   static_Cast<int>(::SendMessage(a_hWnd, RPWM_GETDELAY, 0, 0L))


// control notifications
#define RPN_FIRST			(WM_USER + 0x4020)
#define RPN_CLICKED			(RPN_FIRST + 0)
#define RPN_CLOSE		    (RPN_FIRST + 1)
#define RPN_PIN				(RPN_FIRST + 2)
#define RPN_CAPTIONICON		(RPN_FIRST + 3)
#define RPN_SETFOCUS		(RPN_FIRST + 4)

typedef struct S_NMPWCLK
{
	NMHDR nmhdr;
	int   iIcon;
} RCCNMPWCLK, *LPRCCNMPWCLK;

