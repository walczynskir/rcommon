// RTracker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "RTrackerData.h"
#include "RTrackerWnd.h"
#include "RMemDC.h"


static const long c_iWindowOfs = sizeof(RTrackerData*) - sizeof(int);
static inline RTrackerData* GetRData(HWND a_hWnd);

static inline LRESULT OnCreate(HWND a_hWnd, LPCREATESTRUCT a_pCreate);
static inline void OnNcDestroy(HWND a_hWnd);
static inline void OnPaint(HWND a_hWnd);

static inline void Draw(HWND a_hWnd, HDC a_hDC);

ATOM RTrackerWnd_RegisterClass()
{
	WNDCLASSEX l_wcex;
	l_wcex.cbSize = sizeof(WNDCLASSEX); 
	l_wcex.style		 = CS_DBLCLKS;
	l_wcex.lpfnWndProc	 = (WNDPROC)RTrackerWnd_WndProc;
	l_wcex.cbClsExtra	 = 0;
	l_wcex.cbWndExtra	 = sizeof(RTrackerData*);
	l_wcex.hInstance	 = RCommon_GetInstance();
	l_wcex.hIcon		 = NULL;
	l_wcex.hCursor		 = NULL;
	l_wcex.hbrBackground = NULL;
	l_wcex.lpszMenuName	 = NULL;
	l_wcex.lpszClassName = RTrackerWnd_ClassName;
	l_wcex.hIconSm		 = NULL;
	return RegisterClassEx(&l_wcex);
}


LRESULT CALLBACK RTrackerWnd_WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam)
{

	switch (a_iMsg) 
	{
	case WM_CREATE:
		return OnCreate(a_hWnd, reinterpret_cast<LPCREATESTRUCT>(a_lParam));

	case WM_PAINT:
		OnPaint(a_hWnd);
		break;

	case WM_NCDESTROY:
		OnNcDestroy(a_hWnd);
		break;

	default:
		return ::DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
	}
	return 0;
}


RTrackerData* GetRData(HWND a_hWnd)
{
	ASSERT(a_hWnd != NULL);
#pragma warning(disable: 4312)
	return (RTrackerData*)::GetWindowLongPtr(a_hWnd, c_iWindowOfs);
#pragma warning(default: 4312)
}


LRESULT OnCreate(HWND a_hWnd, LPCREATESTRUCT a_pCreate)
{
	RTrackerData* l_pData = new RTrackerData();
	if (l_pData == NULL)
	{
		return -1;
	}
	l_pData->m_hBrush = reinterpret_cast<HBRUSH>(a_pCreate->lpCreateParams);
#pragma warning(disable: 4244)
	::SetWindowLongPtr(a_hWnd, c_iWindowOfs, (LONG_PTR)l_pData);
#pragma warning(default: 4244)
	return 0;
}


void OnNcDestroy(HWND a_hWnd)
{
	RTrackerData* l_pData = GetRData(a_hWnd);
	delete l_pData;
}


void OnPaint(HWND a_hWnd)
{
	PAINTSTRUCT l_ps;
	HDC l_hdc;
	l_hdc = ::BeginPaint(a_hWnd, &l_ps);

	RECT l_rect;
	::GetClientRect(a_hWnd, &l_rect);
	{	// to allow destruction of RMemDC
#ifdef _DEBUG
	HDC l_dc = l_hdc;
#else
	RMemDC l_MemDC = RMemDC(l_hdc, &l_ps.rcPaint);
	HDC l_dc = l_MemDC;
#endif

	Draw(a_hWnd, l_dc);
	}

	::EndPaint(a_hWnd, &l_ps);
}


void Draw(HWND a_hWnd, HDC a_hDC)
{
	RTrackerData* l_pData = GetRData(a_hWnd);
	RECT l_rect;
	::GetClipBox(a_hDC, &l_rect);
	::FillRect(a_hDC, &l_rect, l_pData->m_hBrush);
}


//	---------------------------------------------------------------------------------------
//	Creation of RTrackerWnd window
//
HWND	// Handle of created window or NULL if failed
RTrackerWnd_Create(HBRUSH a_hBrush)
{
	return ::CreateWindowEx(WS_EX_NOACTIVATE, RTrackerWnd_ClassName, NULL, WS_POPUP,
		0, 0, 0, 0, NULL, NULL, RCommon_GetInstance(), reinterpret_cast<LPVOID>(a_hBrush));
}