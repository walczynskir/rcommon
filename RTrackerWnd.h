#pragma once

#define RTrackerWnd_ClassName _T("RTRACKERWND")

ATOM RTrackerWnd_RegisterClass();
LRESULT CALLBACK RTrackerWnd_WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);
HWND RTrackerWnd_Create(HBRUSH a_hBrush);

