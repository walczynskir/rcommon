// RColorDownWnd.h: interface for the RColorDownWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RCOLORDOWNWND_H__B4F6F8A1_3A40_11D5_AA18_00104B1EB1A3__INCLUDED_)
#define AFX_RCOLORDOWNWND_H__B4F6F8A1_3A40_11D5_AA18_00104B1EB1A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class RColorData;

#define RColorDownWnd_ClassName _T("RCOLORDOWNWND")

LRESULT CALLBACK RColorDownWnd_WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);

ATOM RColorDownWnd_RegisterClass();
HWND RColorDownWnd_Create(HWND a_hWndParent, DWORD a_dwStyle, RColorData* a_pData);
void RColorDownWnd_DropDown(HWND a_hWnd, LPCRECT a_pRect, COLORREF a_color);
void RColorDownWnd_Hide(HWND a_hWnd);


#define CDMO_FIRST			WM_USER + 0x4000
#define CDM_COLORCHANGED	(CDMO_FIRST + 0)
#define CDM_HIDEDROP		(CDMO_FIRST + 1)



#endif // !defined(AFX_RCOLORDOWNWND_H__B4F6F8A1_3A40_11D5_AA18_00104B1EB1A3__INCLUDED_)
