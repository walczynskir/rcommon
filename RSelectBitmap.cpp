#include "stdafx.h"
#include "RSelectBitmap.h"
#include <rcommon/RMemDC.h>
#include <rcommon/IndexedMapVector.hpp>
#include <rcommon/rstddef.h>
#include <vector>
#include <algorithm>



constexpr UINT_PTR constTimerId = 1;
constexpr int constAnimStep = 10;
constexpr int constAnimSpeed = 20;


class SelectBitmapControl {
public:
    IndexedMapVector<UINT, HBITMAP> m_mapBitmaps;

    int m_idxCurrent = 0;

    // animation data
    int m_iAnimOffset = 0;
    int m_idxTarget = 0;

    // animation settings
    int m_iAnimStep = constAnimStep;
    int m_iAnimSpeed = constAnimSpeed;

    UINT_PTR m_idTimer = 0;


    // animation 
    enum EAnimDirection
    {
        AnimDirection_Null,
        AnimDirection_Right,
        AnimDirection_Left
    };

    EAnimDirection m_enumAnimDirection = AnimDirection_Null;

    UINT_PTR timerId = 0;

    void MoveLeft() 
    {
        if (m_mapBitmaps.empty()) 
            return;
        m_idxCurrent = (m_idxCurrent == 0) ? m_mapBitmaps.size() - 1 : m_idxCurrent - 1;
    }

    void MoveRight() 
    {
        if (m_mapBitmaps.empty())
            return;
        m_idxCurrent = (m_idxCurrent + 1) % m_mapBitmaps.size();
    }

    int GetSelectedBitmapID() const 
    {
        return m_mapBitmaps.empty() ? 0 : m_mapBitmaps[m_idxCurrent].first;
    }

    bool SetSelectedBitmapID(UINT a_idRes) 
    {
        m_idxCurrent = m_mapBitmaps.indexOf(a_idRes);
        return m_idxCurrent != -1;
    }

    void ClearBitmaps() {
        m_mapBitmaps.clear();
        m_idxCurrent = -1;
    }
};


static inline bool OnCreate(HWND a_hWnd);
static inline void OnDestroy(HWND a_hWnd);
static inline void OnPaint(HWND a_hWnd);
static inline void OnTimer(HWND a_hWnd, UINT_PTR a_idTimer);

static inline LRESULT OnAddBitmap(HWND a_hWnd, HINSTANCE a_hInst, UINT a_idRes);
static inline int OnGetSelectedBitmap(HWND a_hWnd);
BOOL OnSetSelectedBitmap(HWND a_hWnd, UINT a_idRes);
void OnClearBitmaps(HWND a_hWnd);
void OnMoveLeft(HWND a_hWnd);
void OnMoveRight(HWND a_hWnd);
void OnSetAnimationSpeed(HWND a_hWnd, UINT a_iSpeed);
UINT OnGetAnimationSpeed(HWND a_hWnd);
void OnPauseAnimation(HWND a_hWnd);

static inline void Draw(HWND a_hWnd, HDC a_hDC);
static inline void DrawAnimated(HWND a_hWnd, HDC a_hDC);
static inline void DrawStatic(HWND a_hWnd, HDC a_hDC);

void DrawBitmap(HDC a_hDC, HBITMAP a_hBmp, int a_x, int a_y, const RECT& a_rectDest);
void DrawBitmapCropped(HDC a_hDC, HBITMAP a_hBmp, int a_x, int a_y, const RECT& a_rectDest);
void DrawBitmap(HDC a_hDC, HBITMAP a_hBmp, int a_xStart, int a_yStart);


static void StartAnimation(HWND a_hWnd, SelectBitmapControl::EAnimDirection a_enumDirection);
static void EndAnimation(HWND a_hWnd);




static SelectBitmapControl* GetControl(HWND a_hWnd) {
    return reinterpret_cast<SelectBitmapControl*>(GetWindowLongPtr(a_hWnd, GWLP_USERDATA));
}

static void SetControl(HWND hWnd, SelectBitmapControl* a_pCtrl) {
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(a_pCtrl));
}

LRESULT CALLBACK SelectBitmapProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam) {

    switch (a_iMsg) 
    {
    case WM_CREATE:
        return OnCreate(a_hWnd) ? 0 : -1;

    case WM_DESTROY:
        OnDestroy(a_hWnd);
        return 0;

    case WM_PAINT: 
        OnPaint(a_hWnd);
        return 0;

    case WM_TIMER:
        OnTimer(a_hWnd, static_cast<UINT_PTR>(a_wParam));
        return 0;

    case WM_ADDBITMAP:
        return OnAddBitmap(a_hWnd, reinterpret_cast<HINSTANCE>(a_lParam), static_cast<UINT>(a_wParam));

    case WM_GETSELECTEDBITMAP:
        return OnGetSelectedBitmap(a_hWnd); 

    case WM_SETSELECTEDBITMAP:
        return OnSetSelectedBitmap(a_hWnd, static_cast<UINT>(a_lParam));

    case WM_CLEARBITMAPS:
        OnClearBitmaps(a_hWnd);
        return 0;

    case WM_MOVELEFT:
        OnMoveLeft(a_hWnd);
        return 0;

    case WM_MOVERIGHT:
        OnMoveRight(a_hWnd);
        return 0;

    case WM_SETANIMATIONSPEED:
        OnSetAnimationSpeed(a_hWnd, static_cast<int>(a_wParam));
        return 0;

    case WM_GETANIMATIONSPEED:
        return OnGetAnimationSpeed(a_hWnd);

    }

    return ::DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
}

ATOM RSelectBitmap_RegisterClass(HINSTANCE a_hInst) 
{
    WNDCLASS l_wc = {};
    l_wc.lpfnWndProc = SelectBitmapProc;
    l_wc.hInstance = a_hInst;
    l_wc.lpszClassName = SelectBitmap_ClassName;
    l_wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    l_wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    l_wc.style = CS_HREDRAW | CS_VREDRAW;

    return ::RegisterClass(&l_wc);
}


HWND RSelectBitmap_Create(HWND a_hParent, int a_x, int a_y, int a_dx, int a_dy, int a_idCtrl, DWORD a_iStyle)
{
    return ::CreateWindowEx(0, SelectBitmap_ClassName, nullptr, WS_CHILD | WS_VISIBLE | a_iStyle,
        a_x, a_y, a_dx, a_dy, a_hParent, (HMENU)(INT_PTR)a_idCtrl,
        reinterpret_cast<HINSTANCE>(::GetWindowLongPtr(a_hParent, GWLP_HINSTANCE)),
        nullptr
    );
}


bool OnCreate(HWND a_hWnd)
{
    SelectBitmapControl* l_pCtrl = new SelectBitmapControl();
    SetControl(a_hWnd, l_pCtrl);
    return l_pCtrl != nullptr;
}


void OnDestroy(HWND a_hWnd)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl) {
        ::KillTimer(a_hWnd, constTimerId);
        delete l_pCtrl;
        SetControl(a_hWnd, nullptr);
    }
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


void OnTimer(HWND a_hWnd, UINT_PTR a_idTimer)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return; // no data

    if (a_idTimer != constTimerId)
        return;    // not our timer


    if (l_pCtrl->m_enumAnimDirection == SelectBitmapControl::AnimDirection_Null)
        return;

    RECT l_rect;
    ::GetClientRect(a_hWnd, &l_rect);
    int l_dx = l_rect.right - l_rect.left;

    l_pCtrl->m_iAnimOffset += l_pCtrl->m_iAnimStep;

    if (l_pCtrl->m_iAnimOffset >= l_dx)
    {
        l_pCtrl->m_idxCurrent = l_pCtrl->m_idxTarget;
        l_pCtrl->m_enumAnimDirection = SelectBitmapControl::AnimDirection_Null;
        EndAnimation(a_hWnd);
    }
    ::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

}

void Draw(HWND a_hWnd, HDC a_hDC)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return; // no data

    if (l_pCtrl->m_mapBitmaps.empty())
        return; // nothing to draw

    if (HasWindowStyle(a_hWnd, SBM_ANIMATE))
        DrawAnimated(a_hWnd, a_hDC);
    else
        DrawStatic(a_hWnd, a_hDC);

 }


LRESULT OnAddBitmap(HWND a_hWnd, HINSTANCE a_hInst, UINT a_idRes)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return -1; // no data

    HBITMAP l_hBmp = ::LoadBitmap(a_hInst, MAKEINTRESOURCE(a_idRes));
    if (l_hBmp == NULL)
        return -1; // bitmap not found

    l_pCtrl->m_mapBitmaps.add(a_idRes, l_hBmp);
    return l_pCtrl->m_mapBitmaps.size() - 1;
}


int OnGetSelectedBitmap(HWND a_hWnd)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return -1; // no data
    return l_pCtrl->GetSelectedBitmapID();
}


BOOL OnSetSelectedBitmap(HWND a_hWnd, UINT a_idRes)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return -1; // no data

    if (l_pCtrl->SetSelectedBitmapID(a_idRes)) 
    {
        ::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
        return TRUE;
    }
    return FALSE;
}


void OnClearBitmaps(HWND a_hWnd)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return; // no data
    l_pCtrl->ClearBitmaps();
    ::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}


void OnMoveLeft(HWND a_hWnd)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return; // no data

    if (HasWindowStyle(a_hWnd, SBM_ANIMATE))
        StartAnimation(a_hWnd, SelectBitmapControl::AnimDirection_Left);
    else
    {
        l_pCtrl->MoveLeft();
        ::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
    }
}

void OnMoveRight(HWND a_hWnd)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return; // no data

    if (HasWindowStyle(a_hWnd, SBM_ANIMATE))
        StartAnimation(a_hWnd, SelectBitmapControl::AnimDirection_Right);
    else
    {
        l_pCtrl->MoveRight();
        ::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
    }
}


void OnSetAnimationSpeed(HWND a_hWnd, UINT a_iSpeed)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return; // no data
    
    l_pCtrl->m_iAnimSpeed = a_iSpeed;
}

UINT OnGetAnimationSpeed(HWND a_hWnd)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return 0; // no data

    return l_pCtrl->m_iAnimSpeed;
}


void OnSetAnimationStep(HWND a_hWnd, UINT a_iStep)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return; // no data

    l_pCtrl->m_iAnimStep = a_iStep;
}

UINT OnGetAnimationStep(HWND a_hWnd)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return 0; // no data

    return l_pCtrl->m_iAnimStep;
}


void StartAnimation(HWND a_hWnd, SelectBitmapControl::EAnimDirection a_enumDirection)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return; // no data

    if (l_pCtrl->m_enumAnimDirection != SelectBitmapControl::AnimDirection_Null)
        return; // already started

    l_pCtrl->m_enumAnimDirection = a_enumDirection;
    l_pCtrl->m_idxTarget = (l_pCtrl->m_idxCurrent + (l_pCtrl->m_enumAnimDirection == SelectBitmapControl::AnimDirection_Right ? 1 : -1) + l_pCtrl->m_mapBitmaps.size()) % l_pCtrl->m_mapBitmaps.size();
    l_pCtrl->m_iAnimOffset = 0;

    // Create timer and store mapping
    l_pCtrl->m_idTimer = ::SetTimer(a_hWnd, constTimerId, l_pCtrl->m_iAnimSpeed, nullptr);
}


void EndAnimation(HWND a_hWnd)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return; // no data

    ::KillTimer(NULL, l_pCtrl->m_idTimer);
    l_pCtrl->m_idTimer = 0;
}


void DrawAnimated(HWND a_hWnd, HDC a_hDC)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return; // no data

    RECT l_rect{};
    ::GetClientRect(a_hWnd, &l_rect);

    if (l_pCtrl->m_enumAnimDirection == SelectBitmapControl::AnimDirection_Null)
    {
        if (HasWindowStyle(a_hWnd, SBM_STRETCH))
            DrawBitmap(a_hDC, l_pCtrl->m_mapBitmaps[l_pCtrl->m_idxCurrent].second, 0, 0, l_rect);
        else
            DrawBitmapCropped(a_hDC, l_pCtrl->m_mapBitmaps[l_pCtrl->m_idxCurrent].second, 0, 0, l_rect);
    }
    else
    {
        int l_iOffset = l_pCtrl->m_iAnimOffset * (l_pCtrl->m_enumAnimDirection == SelectBitmapControl::AnimDirection_Left ? 1 : -1);
        if (HasWindowStyle(a_hWnd, SBM_STRETCH))
        {
            DrawBitmap(a_hDC, l_pCtrl->m_mapBitmaps[l_pCtrl->m_idxTarget].second, 0, l_iOffset, l_rect);
            DrawBitmap(a_hDC, l_pCtrl->m_mapBitmaps[l_pCtrl->m_idxCurrent].second, l_iOffset, RectWidth(l_rect), l_rect);
        }
        else
        {
            DrawBitmapCropped(a_hDC, l_pCtrl->m_mapBitmaps[l_pCtrl->m_idxTarget].second, 0, l_iOffset, l_rect);
            DrawBitmapCropped(a_hDC, l_pCtrl->m_mapBitmaps[l_pCtrl->m_idxCurrent].second, l_iOffset, RectWidth(l_rect), l_rect);

        }
    }

}


void DrawStatic(HWND a_hWnd, HDC a_hDC)
{
    SelectBitmapControl* l_pCtrl = GetControl(a_hWnd);
    if (l_pCtrl == nullptr)
        return; // no data

    HBITMAP l_hBmp = l_pCtrl->m_mapBitmaps[l_pCtrl->m_idxCurrent].second;
    HDC l_dcMem = ::CreateCompatibleDC(a_hDC);
    HBITMAP l_hBmpOld = (HBITMAP)::SelectObject(l_dcMem, l_hBmp);

    BITMAP l_bmp;
    ::GetObject(l_hBmp, sizeof(l_bmp), &l_bmp);

    RECT l_rc;
    ::GetClientRect(a_hWnd, &l_rc);

    if (HasWindowStyle(a_hWnd, SBM_STRETCH))
        ::StretchBlt(a_hDC, 0, 0, l_rc.right, l_rc.bottom, l_dcMem, 0, 0, l_bmp.bmWidth, l_bmp.bmHeight, SRCCOPY);
    else
        ::BitBlt(a_hDC, 0, 0, l_bmp.bmWidth, l_bmp.bmHeight, l_dcMem, 0, 0, SRCCOPY);

    ::SelectObject(l_dcMem, l_hBmpOld);
    ::DeleteDC(l_dcMem);
}



// with keeping aspect and fitting into the static 
void DrawBitmap(HDC a_hDC, HBITMAP a_hBmp, int a_x, int a_y, const RECT& a_rectDest)
{
    if (!a_hBmp)
        return;

    HDC l_hDCMem = ::CreateCompatibleDC(a_hDC);
    HBITMAP l_hBmpOld = (HBITMAP)::SelectObject(l_hDCMem, a_hBmp);

    BITMAP l_bitmap;
    ::GetObject(a_hBmp, sizeof(l_bitmap), &l_bitmap);

    // destination size
    int l_dxDest = a_rectDest.right - a_rectDest.left;
    int l_dyDest = a_rectDest.bottom - a_rectDest.top;

    // calculate scales
    double l_dScaleX = (double)l_dxDest / l_bitmap.bmWidth;
    double l_dScaleY = (double)l_dyDest / l_bitmap.bmHeight;
    double l_dScale = min(l_dScaleX, l_dScaleY);  // dopasuj tak, by ca³a bitmapa siê zmieœci³a

    int l_dxNew = (int)(l_bitmap.bmWidth * l_dScale);
    int l_dyNew = (int)(l_bitmap.bmHeight * l_dScale);

    // Wyœrodkuj obraz w polu STATIC
    int l_iOffsetX = a_rectDest.left + (l_dxDest - l_dxNew) / 2;
    int l_iOffsetY = a_rectDest.top + (l_dyDest - l_dyNew) / 2;


    ::StretchBlt(a_hDC, l_iOffsetX, l_iOffsetY, l_dxNew, l_dyNew, l_hDCMem, a_x, 0, l_bitmap.bmWidth, l_bitmap.bmHeight, SRCCOPY);

    ::SelectObject(l_hDCMem, l_hBmpOld);
    ::DeleteDC(l_hDCMem);
}


void DrawBitmapCropped(HDC a_hDC, HBITMAP a_hBmp, int a_x, int a_y, const RECT& a_rectDest)
{
    if (!a_hBmp)
        return;

    HDC l_hDCMem = ::CreateCompatibleDC(a_hDC);
    HBITMAP l_hBmpOld = (HBITMAP)::SelectObject(l_hDCMem, a_hBmp);

    BITMAP l_bitmap;
    ::GetObject(a_hBmp, sizeof(l_bitmap), &l_bitmap);

    int bmpWidth = l_bitmap.bmWidth;
    int bmpHeight = l_bitmap.bmHeight;

    int destWidth = a_rectDest.right - a_rectDest.left;
    int destHeight = a_rectDest.bottom - a_rectDest.top;

    // Calculate scale to fit while preserving aspect ratio
    double scaleX = (double)destWidth / bmpWidth;
    double scaleY = (double)destHeight / bmpHeight;
    double scale = min(scaleX, scaleY);

    // Calculate visible bitmap area (source)
    int scaledWidth = (int)(bmpWidth * scale);
    int scaledHeight = (int)(bmpHeight * scale);

    // Center the scaled bitmap in the destination
    int offsetX = a_rectDest.left + (destWidth - scaledWidth) / 2;
    int offsetY = a_rectDest.top + (destHeight - scaledHeight) / 2;

    // Calculate source rectangle to copy (crop if needed)
    int srcWidth = min(scaledWidth, destWidth);
    int srcHeight = min(scaledHeight, destHeight);

    // Calculate source origin (top-left of bitmap)
    int srcX = a_x + (int)(bmpWidth - srcWidth / scale) / 2;
    int srcY = a_y + (int)(bmpHeight - srcHeight / scale) / 2;

    // Calculate destination rectangle
    int dstX = offsetX;
    int dstY = offsetY;

    // Copy visible portion without stretching
    ::BitBlt(a_hDC, dstX, dstY, srcWidth, srcHeight, l_hDCMem, srcX, srcY, SRCCOPY);

    ::SelectObject(l_hDCMem, l_hBmpOld);
    ::DeleteDC(l_hDCMem);
}


void DrawBitmap(HDC a_hDC, HBITMAP a_hBmp, int a_xStart, int a_yStart)
{
    if (!a_hBmp)
        return;

    HDC l_hDCMem = CreateCompatibleDC(a_hDC);
    HBITMAP l_hBmpOld = (HBITMAP)SelectObject(l_hDCMem, a_hBmp);

    BITMAP l_bitmap;
    GetObject(a_hBmp, sizeof(l_bitmap), &l_bitmap);

    BitBlt(a_hDC, a_xStart, a_yStart, l_bitmap.bmWidth, l_bitmap.bmHeight, l_hDCMem, 0, 0, SRCCOPY);

    SelectObject(l_hDCMem, l_hBmpOld);
    DeleteDC(l_hDCMem);
}

