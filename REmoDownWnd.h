// REmoDownWnd.h: interface for the REmoDownWnd class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
class REmoData;

#define REmoDownWnd_ClassName _T("REMODOWNWND")

LRESULT CALLBACK REmoDownWnd_WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);

ATOM REmoDownWnd_RegisterClass();
HWND REmoDownWnd_Create(HWND a_hWndParent, const REmoData* a_pEmoData);
void REmoDownWnd_DropDown(HWND a_hWnd, LPCRECT a_pRect);
void REmoDownWnd_Hide(HWND a_hWnd);


#define EDMO_FIRST			WM_USER + 0x4000
#define EDM_EMOCHANGED	    (EDMO_FIRST + 0)
#define EDM_HIDEDROP		(EDMO_FIRST + 1)


