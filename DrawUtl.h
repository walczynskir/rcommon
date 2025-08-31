#pragma once
#include <rcommon/rcommon.h>
#include <tchar.h>
#include <rcommon/rstring.h>


namespace RDraw
{

typedef struct S_CLR3DBUTTON
{
	COLORREF clrFace;
	COLORREF clrBorderLeftUp;
	COLORREF clrBorderRightDown;
	COLORREF clrText;
} CLR3DBUTTON, *LPCLR3DBUTTON;

typedef struct S_CLRCELL
{
	COLORREF clrBk;
	COLORREF clrBorderLeft;
	COLORREF clrBorderRight;
	COLORREF clrBorderTop;
	COLORREF clrBorderBottom;
} CLRCELL, *LPCLRCELL;

typedef struct S_TEXTCELL
{
	COLORREF	clrText;
	HFONT		hFont;
	tstring		sText;
	UINT		iFormat;	// like DrawText formats
	short		nMargin;
} TEXTCELL, *LPTEXTCELL;


extern RCOMMON_API void Draw3DRect(HDC a_hDC, const RECT& a_rect, COLORREF a_clrTopLeft, 
	COLORREF a_clrBottomRight);
extern RCOMMON_API void Draw3DRect(HDC a_hDC, int a_x, int a_y, int a_cx, int a_cy, COLORREF a_clrTopLeft, 
	COLORREF a_clrBottomRight);
extern RCOMMON_API void DrawBorderRect(HDC a_hDC, int a_x, int a_y, int a_cx, int a_cy, COLORREF a_clrLeft, 
	COLORREF a_clrRight, COLORREF a_clrTop, COLORREF a_clrBottom);
extern RCOMMON_API void DrawBorderRect(HDC a_hDC, const RECT& a_rect, COLORREF a_clrLeft, 
	COLORREF a_clrRight, COLORREF a_clrTop, COLORREF a_clrBottom);
extern RCOMMON_API void DrawFrame(HDC a_hDC, const RECT& a_rect, COLORREF a_clr);
extern RCOMMON_API void DrawConvexRect(HDC	a_hDC, const RECT& a_rect);

extern RCOMMON_API void FillSolidRect(HDC a_hDC, int a_x, int a_y, int a_cx, int a_cy, COLORREF a_clr);
extern RCOMMON_API void FillSolidRect(HDC a_hDC, const RECT& a_rect, COLORREF a_clr);

extern RCOMMON_API void DrawSunkenRect(HDC a_hDC, const RECT& a_rect);
extern RCOMMON_API void DrawSunkenLine(HDC a_hDC, const POINT& a_ptStart, int a_iWidth, BOOL a_bVertical);

extern RCOMMON_API void Draw3DButton(HDC a_hDC, const RECT& a_rect, LPCLR3DBUTTON a_pClrs,
	HFONT a_hFont, const tstring& a_sText = _T(""), BOOL a_bSelected = FALSE);
extern RCOMMON_API void DrawButton(HDC a_hDC, const RECT& a_rect, LPCLR3DBUTTON a_pClrs,	
	HFONT a_hFont, const tstring& a_sText = _T(""), BOOL a_bTransparent = FALSE);

extern RCOMMON_API void DrawCell(HDC a_hDC, const RECT& a_rect, const CLRCELL& a_clrs, const TEXTCELL& a_text, 
	BOOL a_bTransparent = FALSE, BOOL a_bBorder = TRUE);

extern RCOMMON_API void DrawButtonBorder(HDC a_hDC, const RECT& a_rect, bool a_bPushed, bool a_bDisabled, bool a_bFlat, bool a_bHot);

extern RCOMMON_API HBITMAP GetBitmap(HBITMAP a_hBmp, UINT a_x, UINT a_y, UINT a_dx, UINT a_dy);
extern RCOMMON_API HBITMAP LoadImageFromResource(HINSTANCE a_hInstance, LPCWSTR a_sResourceName, LPCWSTR a_sResourceType);


// like in Windows SDK AnimateWindow
#define RAW_HOR_POSITIVE             0x00000001
#define RAW_HOR_NEGATIVE             0x00000002
#define RAW_VER_POSITIVE             0x00000004
#define RAW_VER_NEGATIVE             0x00000008
#define RAW_CENTER                   0x00000010
#define RAW_HIDE                     0x00020020
#define RAW_ACTIVATE                 0x00000040

#ifndef _WIN32_WCE
extern RCOMMON_API BOOL AnimateWindow(HWND a_hWnd, DWORD a_dwTime, DWORD a_dwFlags, int a_iStep = 5);
extern RCOMMON_API BOOL AnimateWindowEx(HWND a_hWnd, DWORD a_dwTime, DWORD a_dwFlags, LPSIZE a_pSize, int a_iStep = 5);
#endif // of  !_WIN32_WCE


extern RCOMMON_API HBITMAP ReplaceColor(HBITMAP a_hBmp, COLORREF a_clrBackOld, COLORREF a_clrBackNew);
extern RCOMMON_API HBITMAP LoadPicture(LPCTSTR a_sPictureFile, COLORREF a_clrBack, LPSIZE a_pSize);

extern RCOMMON_API void DrawBitmap(HDC a_hDC, const POINT& a_pt, HBITMAP a_hBmp, DWORD a_dwRop);
extern RCOMMON_API BOOL DrawBitmapTransparent(HDC a_hDC, const POINT& a_pt, HBITMAP a_hBmp, COLORREF a_clrTransparent);
extern RCOMMON_API BOOL DrawBitmapTransparent(HDC a_hDC, int a_x, int a_y, HBITMAP a_hBmp, COLORREF a_clrTransparent);
extern RCOMMON_API void DrawStretchedBitmap(HDC a_hDC, const RECT& a_rect, HBITMAP a_hBmp, DWORD a_dwRop);
extern RCOMMON_API void DrawStretchedBitmapTransparent(HDC a_hDC, const RECT& a_rect, HBITMAP a_hBmp, COLORREF a_clrTransparent);
extern RCOMMON_API void DrawGradientRect(HDC a_hDC, const RECT& a_rect, COLORREF a_clr1, COLORREF a_clr2);

#ifndef _WIN32_WCE
extern RCOMMON_API int DrawTextRotated(HDC a_hDC, LPTSTR a_psText, int a_iLen, const LPPOINT a_pPt, const LPSIZE a_pSize, UINT a_iFmt, int a_iAngle);
#endif // of  !_WIN32_WCE

// region functions
#ifndef _WIN32_WCE
HRGN CreateRegionFromBitmap(HBITMAP a_hBmp, COLORREF a_clrTransp);
HRGN CreateRegionFromBitmap(HBITMAP a_hBmp, const POINT& a_ptTransp);
#endif // of  !_WIN32_WCE

} // end of namespace