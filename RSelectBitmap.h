#pragma once
#include <rcommon/rcommon.h>

// window class
#define SelectBitmap_ClassName  _T("SELECTBITMAP")

// Styles
#define SBM_ANIMATE 0x0001  // Animate through bitmap list
#define SBM_STRETCH 0x0002  // Stretch bitmap to fit control

// Messages
#define WM_ADDBITMAP           (WM_USER + 1)  // lParam = resource ID
#define WM_GETSELECTEDBITMAP   (WM_USER + 2)  // returns current resource ID
#define WM_SETSELECTEDBITMAP   (WM_USER + 3)  // lParam = resource ID
#define WM_CLEARBITMAPS        (WM_USER + 4)  // clears bitmap list
#define WM_MOVELEFT            (WM_USER + 5)  // select previous bitmap
#define WM_MOVERIGHT           (WM_USER + 6)  // select next bitmap
#define WM_SETANIMATIONSPEED   (WM_USER + 7)  // wParam = milliseconds
#define WM_GETANIMATIONSPEED   (WM_USER + 8)  // returns current speed
#define WM_SETANIMATIONSTEP    (WM_USER + 9)  // wParam = step of animation
#define WM_GETANIMATIONSTEP    (WM_USER + 10)  // returns current step of animation

// Add a bitmap resource ID to the control (can be from dll), returns index of added element
#define SelectBitmap_AddBitmap(hWnd, hInst, resId) \
    ((int)SendMessage((hWnd), WM_ADDBITMAP, (WPARAM)(resId), (LPARAM)(hInst)))

// Get the currently selected bitmap resource ID
#define SelectBitmap_GetSelected(hWnd) \
    ((int)SendMessage((hWnd), WM_GETSELECTEDBITMAP, 0, 0))

// Set the selected bitmap by resource ID
#define SelectBitmap_SetSelected(hWnd, resId) \
    SendMessage((hWnd), WM_SETSELECTEDBITMAP, 0, (LPARAM)(resId))

// Clear all bitmaps from the control
#define SelectBitmap_Clear(hWnd) \
    SendMessage((hWnd), WM_CLEARBITMAPS, 0, 0)

// Move selection to the previous bitmap
#define SelectBitmap_MoveLeft(hWnd) \
    SendMessage((hWnd), WM_MOVELEFT, 0, 0)

// Move selection to the next bitmap
#define SelectBitmap_MoveRight(hWnd) \
    SendMessage((hWnd), WM_MOVERIGHT, 0, 0)

// Set animation speed in milliseconds
#define SelectBitmap_SetAnimationSpeed(hWnd, ms) \
    SendMessage((hWnd), WM_SETANIMATIONSPEED, (WPARAM)(ms), 0)

// Get current animation speed in milliseconds
#define SelectBitmap_GetAnimationSpeed(hWnd) \
    ((int)SendMessage((hWnd), WM_GETANIMATIONSPEED, 0, 0))

// Set animation step 
#define SelectBitmap_SetAnimationStep(hWnd, step) \
    SendMessage((hWnd), WM_SETANIMATIONSTEP, (WPARAM)(step), 0)

// Get current animation step
#define SelectBitmap_GetAnimationStep(hWnd) \
    ((int)SendMessage((hWnd), WM_GETANIMATIONSTEP, 0, 0))

extern RCOMMON_API ATOM RSelectBitmap_RegisterClass(HINSTANCE a_hInst);
extern RCOMMON_API HWND RSelectBitmap_Create(HWND a_hParent, int a_x, int a_y, int a_dx, int a_dy, int a_idCtrl, DWORD a_iStyle);