// DrawUtl.cpp : Defines drawing helper functions
//
#include "stdafx.h"
#include "DrawUtl.h"
#include <math.h>
#include <vector>
#include <olectl.h>
#include <windows.h>
#include <wincodec.h>  // WIC
#include <wingdi.h>     // For BLENDFUNCTION

#pragma comment(lib, "Msimg32.lib") // Required for AlphaBlend
#pragma comment(lib, "gdiplus.lib")






const int cc_nHMInch = 2540;
const float cc_fPi = 3.1415926f;



namespace RDraw
{

	typedef struct S_ANIMATE
	{
		DWORD dwFlags;
		RECT  rectStart;
		SIZE  sizeLast;
		POINT ptLast;
		int   iStep;
		DWORD dwSWPFlag;
	} ANIMATE, *LPANIMATE;


	static void GetFirstAnimStep(LPANIMATE a_pAnimate);
	static bool GetNextAnimStep(LPANIMATE a_pAnimate);


	int DrawTextDPI(HDC a_hDC, LPCTSTR a_psText, int a_iLen, LPRECT a_pRect, UINT a_iFmt, const DpiContext& a_dpi)
	{
		RECT l_rectScaled = a_dpi.ScaleRect(*a_pRect);
		return ::DrawText(a_hDC, a_psText, a_iLen, &l_rectScaled, a_iFmt);
	}

	//	-----------------------------------------------------------------
	//	Draws 3D rectangle
	//
	void
		Draw3DRect(
			HDC a_hDC,
			const RECT& a_rect,
			COLORREF a_clrTopLeft,
			COLORREF a_clrBottomRight
		)
	{
		Draw3DRect(a_hDC, a_rect.left, a_rect.top,
			a_rect.right - a_rect.left, a_rect.bottom - a_rect.top,
			a_clrTopLeft, a_clrBottomRight);
	}


	//	-----------------------------------------------------------------
	//	Draws 3D rectangle
	//
	void
		Draw3DRect(
			HDC a_hDC,
			int a_x,
			int a_y,
			int a_cx,
			int a_cy,
			COLORREF a_clrTopLeft,
			COLORREF a_clrBottomRight
		)
	{
		DrawBorderRect(a_hDC, a_x, a_y, a_cx, a_cy, a_clrTopLeft, a_clrBottomRight,
			a_clrTopLeft, a_clrBottomRight);
	}


	//	-----------------------------------------------------------------
	//	Draws rect with borders
	//
	void
		DrawBorderRect(
			HDC a_hDC,
			const RECT& a_rect,
			COLORREF a_clrLeft,
			COLORREF a_clrRight,
			COLORREF a_clrTop,
			COLORREF a_clrBottom
		)
	{
		DrawBorderRect(a_hDC, a_rect.left, a_rect.top,
			a_rect.right - a_rect.left, a_rect.bottom - a_rect.top,
			a_clrLeft, a_clrRight, a_clrTop, a_clrBottom);
	}


	//	-----------------------------------------------------------------
	//	Draws rect with borders
	//
	void
		DrawBorderRect(
			HDC a_hDC,
			int a_x,
			int a_y,
			int a_cx,
			int a_cy,
			COLORREF a_clrLeft,
			COLORREF a_clrRight,
			COLORREF a_clrTop,
			COLORREF a_clrBottom
		)
	{
		FillSolidRect(a_hDC, a_x, a_y, a_cx, 1, a_clrTop);
		FillSolidRect(a_hDC, a_x, a_y, 1, a_cy, a_clrLeft);
		FillSolidRect(a_hDC, a_x + a_cx, a_y + 1, -1, a_cy - 1, a_clrRight);
		FillSolidRect(a_hDC, a_x + 1, a_y + a_cy, a_cx - 1, -1, a_clrBottom);
	}


	//	-----------------------------------------------------------------
	//	fills rect
	//
	void
		FillSolidRect(
			HDC a_hDC,
			int a_x,
			int a_y,
			int a_cx,
			int a_cy,
			COLORREF a_clr
		)
	{
		RECT l_rect;
		l_rect.left = a_x;
		l_rect.top = a_y;
		l_rect.right = a_x + a_cx;
		l_rect.bottom = a_y + a_cy;
		FillSolidRect(a_hDC, l_rect, a_clr);
	}


	//	-----------------------------------------------------------------
	//	fills rect
	//
	void
		FillSolidRect(
			HDC a_hDC,
			const RECT& a_rect,
			COLORREF a_clr
		)
	{
		COLORREF l_color = ::SetBkColor(a_hDC, a_clr);
		::ExtTextOut(a_hDC, 0, 0, ETO_OPAQUE, &a_rect, NULL, 0, NULL);
		::SetBkColor(a_hDC, l_color);
	}


	//	---------------------------------------------------------------------------------------
	//	Draws 3D button
	//
	void
		Draw3DButton(
			HDC a_hDC,
			const RECT& a_rect,
			LPCLR3DBUTTON a_pClrs,
			HFONT a_hFont,
			const tstring& a_sText /*= _T("")*/,
			BOOL /*a_bSelected = FALSE*/
		)
	{
		RECT l_rect = a_rect;

		Draw3DRect(a_hDC, l_rect, RGB(255, 255, 255), RGB(0, 0, 0));

		::InflateRect(&l_rect, -1, -1);

		Draw3DRect(a_hDC, l_rect, a_pClrs->clrBorderLeftUp, a_pClrs->clrBorderRightDown);
		::InflateRect(&l_rect, -1, -1);
		FillSolidRect(a_hDC, l_rect, a_pClrs->clrFace);
		if (a_sText.compare(_T("")))
		{
			return;
		}

		HFONT l_hFontOld = (HFONT)::SelectObject(a_hDC, a_hFont);

		::SetTextColor(a_hDC, a_pClrs->clrText);

		::DrawText(a_hDC, a_sText.data(), static_cast<int>(a_sText.length()), &l_rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		::SelectObject(a_hDC, l_hFontOld);
	}


	//	---------------------------------------------------------------------------------------
	//	Draws button
	//
	void
		DrawButton(
			HDC a_hDC,
			const RECT& a_rect,
			LPCLR3DBUTTON a_pClrs,
			HFONT a_hFont,
			const tstring& a_sText /*= _T("")*/,
			BOOL a_bTransparent /*= FALSE */
		)
	{
		CLRCELL l_clrs;
		l_clrs.clrBk = a_pClrs->clrFace;
		l_clrs.clrBorderBottom = a_pClrs->clrBorderRightDown;
		l_clrs.clrBorderLeft = a_pClrs->clrBorderLeftUp;
		l_clrs.clrBorderRight = a_pClrs->clrBorderRightDown;
		l_clrs.clrBorderTop = a_pClrs->clrBorderLeftUp;

		TEXTCELL l_text;
		l_text.clrText = a_pClrs->clrText;
		l_text.hFont = a_hFont;
		l_text.iFormat = 0;
		l_text.sText = a_sText;

		DrawCell(a_hDC, a_rect, l_clrs, l_text, a_bTransparent);
	}


	//	---------------------------------------------------------------------------------------
	//	Draws cell
	//
	void
		DrawCell(
			HDC a_hDC,
			const RECT& a_rect,
			const CLRCELL& a_clrs,
			const TEXTCELL& a_text,
			BOOL a_bTransparent,
			BOOL a_bBorder
		)
	{
		RECT l_rect = a_rect;

		if (a_bBorder)
		{
			DrawBorderRect(a_hDC, l_rect, a_clrs.clrBorderLeft, a_clrs.clrBorderRight,
				a_clrs.clrBorderTop, a_clrs.clrBorderBottom);
			::InflateRect(&l_rect, -1, -1);
		}

		if (!a_bTransparent)
			FillSolidRect(a_hDC, l_rect, a_clrs.clrBk);
		if (a_text.sText.compare(_T("")) == 0)
		{
			return;
		}
		HFONT l_hFontOld = (HFONT)::SelectObject(a_hDC, a_text.hFont);

		COLORREF l_clrOldText = ::SetTextColor(a_hDC, a_text.clrText);

		int l_iOldBkMode = 0;
		COLORREF l_clrOldBk = 0;
		if (a_bTransparent)
		{
			l_iOldBkMode = ::SetBkMode(a_hDC, TRANSPARENT);
		}
		else
		{
			l_iOldBkMode = ::SetBkMode(a_hDC, OPAQUE);
			l_clrOldBk = ::SetBkColor(a_hDC, a_clrs.clrBk);
		}

		UINT l_iFormat = a_text.iFormat;
		if (l_iFormat == 0)
		{
			l_iFormat = DT_CENTER | DT_SINGLELINE | DT_VCENTER;
			l_iFormat |= DT_END_ELLIPSIS;
		}

		::InflateRect(&l_rect, -a_text.nMargin, 0);
		::DrawText(a_hDC, a_text.sText.data(), static_cast<int>(a_text.sText.length()), &l_rect, l_iFormat);
		::SelectObject(a_hDC, l_hFontOld);
		::SetTextColor(a_hDC, l_clrOldText);

		::SetBkMode(a_hDC, l_iOldBkMode);
		if (!a_bTransparent)
		{
			::SetBkColor(a_hDC, l_clrOldBk);
		}

	}


	//	---------------------------------------------------------------------------------------
	//	Draws sunken rect
	//
	void
		DrawSunkenRect(
			HDC a_hDC,
			const RECT& a_rect
		)
	{
		Draw3DRect(a_hDC,
			a_rect.left, a_rect.top,
			a_rect.right - a_rect.left, a_rect.bottom - a_rect.top,
			::GetSysColor(COLOR_3DSHADOW), RGB(255, 255, 255));
		Draw3DRect(a_hDC,
			a_rect.left + 1, a_rect.top + 1,
			a_rect.right - a_rect.left - 2, a_rect.bottom - a_rect.top - 2,
			RGB(0, 0, 0), ::GetSysColor(COLOR_3DLIGHT));
	}


	//	---------------------------------------------------------------------------
	//	Draws sunken line
	//
	void
		DrawSunkenLine(
			HDC a_hDC,
			const POINT& a_ptStart,
			int	a_iWidth,
			BOOL a_bVertical
		)
	{

		RECT l_rect;
		if (a_bVertical)
		{
			l_rect.left = a_ptStart.x;
			l_rect.top = a_ptStart.y;
			l_rect.right = l_rect.left + 1;
			l_rect.bottom = l_rect.top + a_iWidth + 1;
		}
		else
		{
			l_rect.left = a_ptStart.x;
			l_rect.top = a_ptStart.y;
			l_rect.bottom = l_rect.top + 1;
			l_rect.right = l_rect.left + a_iWidth + 1;
		}

		FillSolidRect(a_hDC, l_rect, ::GetSysColor(COLOR_3DSHADOW));

		if (a_bVertical)
		{
			l_rect.left += 1;
			l_rect.right += 1;
		}
		else
		{
			l_rect.top += 1;
			l_rect.bottom += 1;
		}

		FillSolidRect(a_hDC, l_rect, RGB(255, 255, 255));

	}


	//	---------------------------------------------------------------------------
	//	Draws convex rect
	//
	void
		DrawConvexRect(
			HDC	a_hDC,
			const RECT& a_rect
		)
	{
		Draw3DRect(a_hDC,
			a_rect.left, a_rect.top,
			a_rect.right - a_rect.left, a_rect.bottom - a_rect.top,
			RGB(255, 255, 255), RGB(0, 0, 0));
		Draw3DRect(a_hDC,
			a_rect.left + 1, a_rect.top + 1,
			a_rect.right - a_rect.left - 2, a_rect.bottom - a_rect.top - 2,
			::GetSysColor(COLOR_3DLIGHT), ::GetSysColor(COLOR_3DSHADOW));
	}


	void DrawFrame(
		HDC a_hDC,
		const RECT& a_rect,
		COLORREF a_clr
	)
	{
		DrawBorderRect(a_hDC, a_rect, a_clr, a_clr, a_clr, a_clr);
	}


	//	---------------------------------------------------------------------------
	//	draws button border
	//
	void
		DrawButtonBorder(
			HDC  a_hDC,
			const RECT& a_rect,
			bool a_bPushed,
			bool a_bDisabled,
			bool a_bFlat,
			bool a_bHot
		)
	{

		RECT l_rect = a_rect;
		UINT l_iState = DFCS_BUTTONPUSH;
		if (a_bFlat)
		{
			l_iState |= DFCS_FLAT;
		}

		if (a_bDisabled)
		{
			l_iState |= DFCS_INACTIVE;
		}
		else if (a_bPushed)
		{
			l_iState |= DFCS_PUSHED;
		}
		else if (a_bHot & a_bFlat)
		{
			l_iState ^= DFCS_FLAT;
		}

		::DrawFrameControl(a_hDC, &l_rect, DFC_BUTTON, l_iState);
	}


	//	---------------------------------------------------------------------------
	//	Copies bitmap fragment
	//  returned bitmap must be deleted with DeleteObject after use
	//
	HBITMAP
		GetBitmap(
			HBITMAP a_hBmp,
			UINT a_x,
			UINT a_y,
			UINT a_dx,
			UINT a_dy
		)
	{
		if (a_hBmp == NULL)
		{
			return  NULL;
		}

		HDC l_hdc = ::GetDC(NULL);
		HDC l_hdcMem1 = ::CreateCompatibleDC(l_hdc);
		HDC l_hdcMem2 = ::CreateCompatibleDC(l_hdc);
		HBITMAP l_hBmpDst = ::CreateCompatibleBitmap(l_hdc, a_dx, a_dy);
		::SelectObject(l_hdcMem2, a_hBmp);

		HBITMAP l_hBmpOld = (HBITMAP)::SelectObject(l_hdcMem1, l_hBmpDst);
		::BitBlt(l_hdcMem1, 0, 0, a_dx, a_dy, l_hdcMem2, a_x, a_y, SRCCOPY);
		::SelectObject(l_hdcMem1, l_hBmpOld);

		::DeleteDC(l_hdcMem2);
		::DeleteDC(l_hdcMem1);
		::ReleaseDC(NULL, l_hdc);
		return l_hBmpDst;
	}


	BOOL AnimateWindow(HWND a_hWnd, DWORD a_dwTime, DWORD a_dwFlags, int a_iStep)
	{
		RECT l_rect;
		::GetWindowRect(a_hWnd, &l_rect);
		SIZE l_size;
		l_size.cx = RectWidth(l_rect);
		l_size.cy = RectHeight(l_rect);
		return AnimateWindowEx(a_hWnd, a_dwTime, a_dwFlags, NULL, a_iStep);
	}


	BOOL AnimateWindowEx(HWND a_hWnd, DWORD a_dwTime, DWORD a_dwFlags, LPSIZE /*a_pSize*/, int a_iStep)
	{
		RECT l_rect;
		::GetWindowRect(a_hWnd, &l_rect);
		UINT l_iWidth = RectWidth(l_rect);
		UINT l_iHeight = RectHeight(l_rect);
		bool l_bHorz = (a_dwFlags & RAW_HOR_POSITIVE) || (a_dwFlags & RAW_HOR_NEGATIVE);

		DWORD l_iSleep = a_dwTime / (l_bHorz ? l_iWidth : l_iHeight) * a_iStep;

		l_iSleep = (l_iSleep == 0) ? 1 : l_iSleep;
		bool l_bHide = ((a_dwFlags & RAW_HIDE) == RAW_HIDE);
		int l_iSleepReal;
		ANIMATE l_animate;
		l_animate.dwFlags = a_dwFlags;
		l_animate.iStep = a_iStep;
		l_animate.rectStart = l_rect;
		GetFirstAnimStep(&l_animate);
		while (GetNextAnimStep(&l_animate))
		{
			DWORD l_iTickStart = ::GetTickCount();
			::SetWindowPos(a_hWnd, NULL, l_animate.ptLast.x, l_animate.ptLast.y, l_animate.sizeLast.cx, l_animate.sizeLast.cy, l_animate.dwSWPFlag | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
			::RedrawWindow(a_hWnd, NULL, NULL, RDW_NOFRAME | RDW_UPDATENOW);
			l_iSleepReal = l_iSleep - (GetTickCount() - l_iTickStart);
			if (l_iSleepReal > 0)
			{
				::SleepEx(l_iSleepReal, FALSE);
				if (l_iSleep == 1)
				{
					l_animate.iStep += 1;
				}
				else
				{
					l_iSleep -= 1;
				}
			}
			else // have to increase step
			{
				l_animate.iStep *= 2;
			}
		}
		if (l_bHide)
		{
			::ShowWindow(a_hWnd, SW_HIDE);
		}
		else
		{
			::SetWindowPos(a_hWnd, NULL, l_rect.left, l_rect.top, l_iWidth, l_iHeight, SWP_NOREDRAW | SWP_NOZORDER | SWP_SHOWWINDOW | (((a_dwFlags & RAW_ACTIVATE) == RAW_ACTIVATE) ? 0 : SWP_NOACTIVATE));
			::RedrawWindow(a_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME);
		}
		return TRUE;
	}


#define FlagSet(a_dwVal, a_dwFlag) ((a_dwVal & a_dwFlag) == a_dwFlag)

	void GetFirstAnimStep(LPANIMATE a_pAnimate)
	{
		bool l_bHide = (a_pAnimate->dwFlags & RAW_HIDE) == RAW_HIDE;
		if (FlagSet(a_pAnimate->dwFlags, RAW_HOR_POSITIVE) || FlagSet(a_pAnimate->dwFlags, RAW_VER_POSITIVE))
		{
			a_pAnimate->ptLast.x = 0;
			a_pAnimate->ptLast.y = 0;
			a_pAnimate->dwSWPFlag = SWP_NOMOVE;
		}
		else if (FlagSet(a_pAnimate->dwFlags, RAW_HOR_NEGATIVE))
		{
			a_pAnimate->ptLast.x = l_bHide ? a_pAnimate->rectStart.left : a_pAnimate->rectStart.right;
			a_pAnimate->ptLast.y = a_pAnimate->rectStart.top;
			a_pAnimate->dwSWPFlag = 0;
		}
		else if (FlagSet(a_pAnimate->dwFlags, RAW_VER_NEGATIVE))
		{
			a_pAnimate->ptLast.x = a_pAnimate->rectStart.left;
			a_pAnimate->ptLast.y = l_bHide ? a_pAnimate->rectStart.top : a_pAnimate->rectStart.bottom;
			a_pAnimate->dwSWPFlag = 0;
		}
		else
		{
			ASSERT(FALSE);
		}

		if (FlagSet(a_pAnimate->dwFlags, RAW_HOR_POSITIVE) || FlagSet(a_pAnimate->dwFlags, RAW_HOR_NEGATIVE))
		{
			a_pAnimate->sizeLast.cx = (l_bHide ? (RectWidth(a_pAnimate->rectStart)) : 0);
			a_pAnimate->sizeLast.cy = RectHeight(a_pAnimate->rectStart);
		}
		else if (FlagSet(a_pAnimate->dwFlags, RAW_VER_POSITIVE) || FlagSet(a_pAnimate->dwFlags, RAW_VER_NEGATIVE))
		{
			a_pAnimate->sizeLast.cx = RectWidth(a_pAnimate->rectStart);
			a_pAnimate->sizeLast.cy = (l_bHide ? (RectHeight(a_pAnimate->rectStart)) : 0);
		}
		else
		{
			ASSERT(FALSE);
		}
	}


	bool GetNextAnimStep(LPANIMATE a_pAnimate)
	{
		bool l_bHide = ((a_pAnimate->dwFlags & RAW_HIDE) == RAW_HIDE);
		bool l_bFinish = false;
		if (FlagSet(a_pAnimate->dwFlags, RAW_HOR_POSITIVE))
		{
			a_pAnimate->sizeLast.cx = (l_bHide ? (a_pAnimate->sizeLast.cx - a_pAnimate->iStep) : (a_pAnimate->sizeLast.cx + a_pAnimate->iStep));
		}
		else if (FlagSet(a_pAnimate->dwFlags, RAW_HOR_NEGATIVE))
		{
			a_pAnimate->ptLast.x = (l_bHide ? (a_pAnimate->ptLast.x + a_pAnimate->iStep) : (a_pAnimate->ptLast.x - a_pAnimate->iStep));
			a_pAnimate->sizeLast.cx = (l_bHide ? (a_pAnimate->sizeLast.cx - a_pAnimate->iStep) : (a_pAnimate->sizeLast.cx + a_pAnimate->iStep));
		}
		l_bFinish = l_bHide ?
			(a_pAnimate->sizeLast.cx < 0) :
			(a_pAnimate->sizeLast.cx > RectWidth(a_pAnimate->rectStart));
		if (FlagSet(a_pAnimate->dwFlags, RAW_VER_POSITIVE))
		{
			a_pAnimate->sizeLast.cy = (l_bHide ? (a_pAnimate->sizeLast.cy - a_pAnimate->iStep) : (a_pAnimate->sizeLast.cy + a_pAnimate->iStep));
		}
		else if (FlagSet(a_pAnimate->dwFlags, RAW_VER_NEGATIVE))
		{
			a_pAnimate->ptLast.y = (l_bHide ? (a_pAnimate->ptLast.y + a_pAnimate->iStep) : (a_pAnimate->ptLast.y - a_pAnimate->iStep));
			a_pAnimate->sizeLast.cy = (l_bHide ? (a_pAnimate->sizeLast.cy - a_pAnimate->iStep) : (a_pAnimate->sizeLast.cy + a_pAnimate->iStep));
		}
		return !((l_bHide ?
			(a_pAnimate->sizeLast.cy < 0) :
			(a_pAnimate->sizeLast.cy > RectHeight(a_pAnimate->rectStart))) || l_bFinish);
	}

#undef FlagSet


	//	---------------------------------------------------------------------------
	//	Replace bitmap color with given color
	//  returned bitmap must be deleted with DeleteObject after use
	//
	HBITMAP
		ReplaceColor(
			HBITMAP a_hBmp,
			COLORREF a_clrBackOld,
			COLORREF a_clrBackNew
		)
	{
		HDC l_hdc = ::GetDC(NULL);
		HDC l_hdcMem1 = ::CreateCompatibleDC(l_hdc);
		HDC l_hdcMem2 = ::CreateCompatibleDC(l_hdc);
		BITMAP l_bmp;
		::GetObject(a_hBmp, sizeof(l_bmp), &l_bmp);

		HBITMAP l_hBmpDst = ::CreateCompatibleBitmap(l_hdc, l_bmp.bmWidth, l_bmp.bmHeight);
		::SelectObject(l_hdcMem2, a_hBmp);

		HBITMAP l_hBmpOld = (HBITMAP)::SelectObject(l_hdcMem1, l_hBmpDst);
		FillSolidRect(l_hdcMem1, 0, 0, l_bmp.bmWidth, l_bmp.bmHeight, a_clrBackNew);
		::TransparentBlt(l_hdcMem1, 0, 0, l_bmp.bmWidth, l_bmp.bmHeight, l_hdcMem2, 0, 0, l_bmp.bmWidth, l_bmp.bmHeight, a_clrBackOld);
		::SelectObject(l_hdcMem1, l_hBmpOld);

		::DeleteDC(l_hdcMem2);
		::DeleteDC(l_hdcMem1);
		::ReleaseDC(NULL, l_hdc);
		return l_hBmpDst;
	}


	//	---------------------------------------------------------------------------
	//	Loads picture to bitmap. 
	//  Returned bitmap must be deleted with DeleteObject after use
	//  Uses OleLoadPicture, so you must call OleInitialize before calling (at least 
	//  I think so - didn't check.
	//
	HBITMAP LoadPicture(
		LPCTSTR a_sPictureFile,
		COLORREF a_clrBack,
		LPSIZE a_pSize
	)
	{
		IPicture* l_pPicture = NULL;
		// LPCTSTR to OLESTR
		long l_iLen = static_cast<long>(_tcslen(a_sPictureFile));
		LPTSTR l_sPictureFile = new TCHAR[l_iLen + 1];

		_tcsncpy_s(l_sPictureFile, l_iLen + 1, a_sPictureFile, l_iLen + 1);


		HRESULT l_hRes = ::OleLoadPicturePath(l_sPictureFile, NULL, 0, a_clrBack, IID_IPicture, (LPVOID*)&l_pPicture);
		delete[] l_sPictureFile;
		if (l_hRes != S_OK)
		{
			return NULL;
		}

		long l_iHmWidth;
		long l_iHmHeight;

		l_hRes = l_pPicture->get_Width(&l_iHmWidth);
		if (l_hRes != S_OK)
		{
			l_pPicture->Release();
			return NULL;
		}

		l_hRes = l_pPicture->get_Height(&l_iHmHeight);
		if (l_hRes != S_OK)
		{
			l_pPicture->Release();
			return NULL;
		}

		HDC l_hdc = ::GetDC(NULL);
		int l_iWidth = MulDiv(l_iHmWidth, ::GetDeviceCaps(l_hdc, LOGPIXELSX), cc_nHMInch);
		int l_iHeight = MulDiv(l_iHmHeight, ::GetDeviceCaps(l_hdc, LOGPIXELSY), cc_nHMInch);

		HDC l_hdcMem = ::CreateCompatibleDC(l_hdc);
		HBITMAP l_hBmp = ::CreateCompatibleBitmap(l_hdc, l_iWidth, l_iHeight);
		HBITMAP l_hBmpOld = static_cast<HBITMAP>(::SelectObject(l_hdcMem, l_hBmp));
		FillSolidRect(l_hdcMem, 0, 0, l_iWidth, l_iHeight, a_clrBack);
		RECT l_rect;
		l_hRes = l_pPicture->Render(l_hdcMem, 0, 0, l_iWidth, l_iHeight,
			0, l_iHmHeight, l_iHmWidth, -l_iHmHeight, &l_rect);
		// done 
		::SelectObject(l_hdcMem, l_hBmpOld);
		::DeleteDC(l_hdcMem);
		::ReleaseDC(NULL, l_hdc);
		l_pPicture->Release();
		if (l_hRes != S_OK)
		{
			// failed
			::DeleteObject(l_hBmp);
			return NULL;
		}
		if (a_pSize != NULL)
		{
			a_pSize->cx = l_iWidth;
			a_pSize->cy = l_iHeight;
		}
		return l_hBmp;
	}


	void DrawBitmap(HDC a_hDC, const POINT& a_pt, HBITMAP a_hBmp, DWORD a_dwRop)
	{
		BITMAP l_bmp;
		::GetObject(a_hBmp, sizeof(l_bmp), &l_bmp);
		HDC l_hdc = ::CreateCompatibleDC(a_hDC);
		HBITMAP l_hBmpOld = static_cast<HBITMAP>(::SelectObject(l_hdc, a_hBmp));
		::BitBlt(a_hDC, a_pt.x, a_pt.y, l_bmp.bmWidth, l_bmp.bmHeight, l_hdc, 0, 0, a_dwRop);
		::SelectObject(l_hdc, l_hBmpOld);
		::DeleteDC(l_hdc);
	}


	BOOL DrawBitmapTransparent(HDC a_hDC, int a_x, int a_y, HBITMAP a_hBmp, COLORREF a_clrTransparent)
	{
		ASSERT(a_hDC != NULL);
		ASSERT(a_hBmp != NULL);

		HDC l_hdc = ::CreateCompatibleDC(a_hDC);

		HBITMAP l_hBmpOld = static_cast<HBITMAP>(::SelectObject(l_hdc, a_hBmp));
		BITMAP l_bmp;
		::GetObject(a_hBmp, sizeof(BITMAP), &l_bmp);

		BOOL l_bOk = ::TransparentBlt(a_hDC, a_x, a_y, l_bmp.bmWidth, l_bmp.bmHeight,
			l_hdc, 0, 0, l_bmp.bmWidth, l_bmp.bmHeight, a_clrTransparent);
		::SelectObject(l_hdc, l_hBmpOld);
		::DeleteDC(l_hdc);
		return l_bOk;
	}


	// Draw with transparency using AlphaBlend
	void DrawTransparentAlphaBlend(HDC a_hDC, int a_x, int a_y, HBITMAP a_hBmp, BYTE a_btSrcAlpha)
	{
		BITMAP l_bmp;
		::GetObject(a_hBmp, sizeof(l_bmp), &l_bmp);

		HDC l_hDCComp = ::CreateCompatibleDC(a_hDC);
		HBITMAP l_hBmpOld = (HBITMAP)::SelectObject(l_hDCComp, a_hBmp);

		BLENDFUNCTION l_blend = {};
		l_blend.BlendOp = AC_SRC_OVER;
		l_blend.SourceConstantAlpha = a_btSrcAlpha;
		l_blend.AlphaFormat = AC_SRC_ALPHA;

		::AlphaBlend(a_hDC, a_x, a_y, l_bmp.bmWidth, l_bmp.bmHeight,
			l_hDCComp, 0, 0, l_bmp.bmWidth, l_bmp.bmHeight, l_blend);

		::SelectObject(l_hDCComp, l_hBmpOld);
		::DeleteDC(l_hDCComp);
	}

	BOOL DrawBitmapTransparent(HDC a_hDC, const POINT& a_pt, HBITMAP a_hBmp, COLORREF a_clrTransparent)
	{
		return DrawBitmapTransparent(a_hDC, a_pt.x, a_pt.y, a_hBmp, a_clrTransparent);
	}


	void DrawStretchedBitmap(HDC a_hDC, const RECT& a_rect, HBITMAP a_hBmp, DWORD a_dwRop)
	{
		BITMAP l_bmp;
		::GetObject(a_hBmp, sizeof(BITMAP), &l_bmp);
		HDC l_hdc = ::CreateCompatibleDC(a_hDC);
		HBITMAP l_hBmpOld = static_cast<HBITMAP>(::SelectObject(l_hdc, a_hBmp));
		::StretchBlt(a_hDC, a_rect.left, a_rect.top, RectWidth(a_rect), RectHeight(a_rect), l_hdc, 0, 0, l_bmp.bmWidth, l_bmp.bmHeight, a_dwRop);
		::SelectObject(l_hdc, l_hBmpOld);
		::DeleteDC(l_hdc);
	}


	void DrawStretchedBitmapTransparent(HDC a_hDC, const RECT& a_rect, HBITMAP a_hBmp, COLORREF a_clrTransparent)
	{
		HDC l_hdc = ::CreateCompatibleDC(a_hDC);

		HBITMAP l_hBmpOld = static_cast<HBITMAP>(::SelectObject(l_hdc, a_hBmp));
		BITMAP l_bmp;
		::GetObject(a_hBmp, sizeof(BITMAP), &l_bmp);

		::TransparentBlt(a_hDC, a_rect.left, a_rect.top, RectWidth(a_rect), RectHeight(a_rect),
			l_hdc, 0, 0, l_bmp.bmWidth, l_bmp.bmHeight, a_clrTransparent);
		::SelectObject(l_hdc, l_hBmpOld);
		::DeleteDC(l_hdc);
	}




	void DrawGradientRect(HDC a_hDC, const RECT& a_rect, COLORREF a_clr1, COLORREF a_clr2)
	{
		TRIVERTEX l_vertex[2];
		l_vertex[0].x = a_rect.left;
		l_vertex[0].y = a_rect.top;
		l_vertex[0].Red = GetRValue(a_clr1) << 8;
		l_vertex[0].Green = GetGValue(a_clr1) << 8;
		l_vertex[0].Blue = GetBValue(a_clr1) << 8;
		l_vertex[0].Alpha = 0x0000;

		l_vertex[1].x = a_rect.right;
		l_vertex[1].y = a_rect.bottom;
		l_vertex[1].Red = GetRValue(a_clr2) << 8;
		l_vertex[1].Green = GetGValue(a_clr2) << 8;
		l_vertex[1].Blue = GetBValue(a_clr2) << 8;
		l_vertex[1].Alpha = 0x0000;


		GRADIENT_RECT l_gRect;
		l_gRect.UpperLeft = 0;
		l_gRect.LowerRight = 1;

		// Draw a shaded rectangle. 
		::GradientFill(a_hDC, l_vertex, ArraySize(l_vertex), &l_gRect, 1, GRADIENT_FILL_RECT_V);
	}


	// Loads  image from resources and converts it to HBITMAP - generated by CoPilot, slightly modified
	HBITMAP LoadImageFromResource(HINSTANCE a_hInstance, UINT a_idResource, LPCTSTR a_sResourceType)
	{
		// Step 1: Load resource
		HRSRC l_hRes = ::FindResource(a_hInstance, MAKEINTRESOURCE(a_idResource), a_sResourceType); // doesn't need to be released
		if (!l_hRes)
			return nullptr;

		HGLOBAL l_hMem = ::LoadResource(a_hInstance, l_hRes);  // doesn't need to be released
		if (!l_hMem)
			return nullptr;

		void* l_pResData = ::LockResource(l_hMem);	// doesn't need to be released
		DWORD l_resSize = SizeofResource(a_hInstance, l_hRes);

		// Step 2: Create WIC factory
		IWICImagingFactory* l_pFactory = nullptr;
		HRESULT l_hr = ::CoInitialize(nullptr);
		if (FAILED(l_hr))
			return nullptr;

		l_hr = ::CoCreateInstance(
			CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&l_pFactory)
		);
		if (FAILED(l_hr))
		{
			::CoUninitialize();
			return nullptr;
		}

		// Step 3: Create WIC stream from memory
		IWICStream* l_pStream = nullptr;
		l_hr = l_pFactory->CreateStream(&l_pStream);
		if (FAILED(l_hr))
		{
			::CoUninitialize();
			return nullptr;
		}

		l_hr = l_pStream->InitializeFromMemory((BYTE*)l_pResData, l_resSize);
		if (FAILED(l_hr))
		{
			l_pStream->Release();
			l_pFactory->Release();
			::CoUninitialize();
			return nullptr;
		}

		// Step 4: Decode PNG
		IWICBitmapDecoder* l_pDecoder = nullptr;
		l_hr = l_pFactory->CreateDecoderFromStream(l_pStream, nullptr, WICDecodeMetadataCacheOnLoad, &l_pDecoder);
		if (FAILED(l_hr))
		{
			l_pStream->Release();
			l_pFactory->Release();
			::CoUninitialize();
			return nullptr;
		}

		IWICBitmapFrameDecode* l_pFrame = nullptr;
		l_hr = l_pDecoder->GetFrame(0, &l_pFrame);
		if (FAILED(l_hr))
		{
			l_pDecoder->Release();
			l_pStream->Release();
			l_pFactory->Release();
			::CoUninitialize();
			return nullptr;
		}

		// Step 5: Convert to 32bpp BGRA
		IWICFormatConverter* l_pConverter = nullptr;
		l_hr = l_pFactory->CreateFormatConverter(&l_pConverter);
		if (FAILED(l_hr))
		{
			l_pFrame->Release();
			l_pDecoder->Release();
			l_pStream->Release();
			l_pFactory->Release();
			::CoUninitialize();
			return nullptr;
		}

		l_hr = l_pConverter->Initialize(
			l_pFrame, GUID_WICPixelFormat32bppBGRA,
			WICBitmapDitherTypeNone, nullptr, 0.f,
			WICBitmapPaletteTypeCustom
		);
		if (FAILED(l_hr))
		{
			l_pConverter->Release();
			l_pFrame->Release();
			l_pDecoder->Release();
			l_pStream->Release();
			l_pFactory->Release();
			::CoUninitialize();
			return nullptr;
		}


		// Step 6: Create HBITMAP
		SIZE l_size;
		l_pConverter->GetSize(reinterpret_cast<UINT*>(&l_size.cx), reinterpret_cast<UINT*>(&l_size.cy));

		BITMAPINFO l_bmi = {};
		l_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		l_bmi.bmiHeader.biWidth = l_size.cx;
		l_bmi.bmiHeader.biHeight = -((LONG)l_size.cy); // top-down
		l_bmi.bmiHeader.biPlanes = 1;
		l_bmi.bmiHeader.biBitCount = 32;
		l_bmi.bmiHeader.biCompression = BI_RGB;

		void* l_pBits = nullptr;
		HDC l_hdc = ::GetDC(nullptr);
		HBITMAP l_hBitmap = CreateDIBSection(l_hdc, &l_bmi, DIB_RGB_COLORS, &l_pBits, nullptr, 0);
		::ReleaseDC(nullptr, l_hdc);

		l_hr = l_pConverter->CopyPixels(nullptr, l_size.cx * 4, l_size.cx * l_size.cy * 4, (BYTE*)l_pBits);
		if (FAILED(l_hr))
		{
			::DeleteObject(l_hBitmap);
			l_hBitmap = nullptr;
		}

		// Cleanup
		l_pConverter->Release();
		l_pFrame->Release();
		l_pDecoder->Release();
		l_pStream->Release();
		l_pFactory->Release();
		::CoUninitialize();

		return l_hBitmap;
	}

	int DrawTextRotated(HDC a_hDC, LPCTSTR a_psText, int a_iLen, const LPPOINT a_pPt, const LPSIZE a_pSize, UINT a_iFmt, int a_iAngle)
	{
		int l_iGMOrg = ::SetGraphicsMode(a_hDC, GM_ADVANCED);
		XFORM l_xformOrg;
		::GetWorldTransform(a_hDC, &l_xformOrg);
		XFORM l_xform;
		ZeroMemory(&l_xform, sizeof(l_xform));
		double l_fAngle = static_cast<double>(a_iAngle) / 180.0 * cc_fPi;
		l_xform.eM11 = static_cast<float>(cos(l_fAngle));
		l_xform.eM12 = static_cast<float>(sin(l_fAngle));
		l_xform.eM21 = static_cast<float>(-sin(l_fAngle));
		l_xform.eM22 = static_cast<float>(cos(l_fAngle));
		l_xform.eDx = static_cast<FLOAT>(a_pPt->x);
		l_xform.eDy = static_cast<FLOAT>(a_pPt->y);

		RECT l_rectDraw;
		l_rectDraw.left = 0;
		l_rectDraw.top = 0;
		l_rectDraw.right = a_pSize->cx;
		l_rectDraw.bottom = a_pSize->cy;
		::SetWorldTransform(a_hDC, &l_xform);
		int l_iSts = ::DrawText(a_hDC, a_psText, a_iLen, &l_rectDraw, a_iFmt);

		::SetWorldTransform(a_hDC, &l_xformOrg);
		::SetGraphicsMode(a_hDC, l_iGMOrg);
		return l_iSts;
	}


	// region functions

	static inline HRGN InternalCreateRegion(HDC a_hdcMem, COLORREF a_clrTransp, const SIZE& a_sizeBmp);
	static inline HRGN CreateRegionFromBitmapPoint(HBITMAP a_hBmp, COLORREF a_clrTransp, const POINT& a_ptTransp, bool a_bColor);

	HRGN CreateRegionFromBitmapPoint(HBITMAP a_hBmp, COLORREF a_clrTransp, const POINT& a_ptTransp, bool a_bColor)
	{
		ASSERT(a_hBmp != NULL);
		SIZE l_size;
		BITMAP l_bmp;
		if (::GetObject(a_hBmp, sizeof(l_bmp), &l_bmp) == 0)
		{
			return NULL;
		}
		l_size.cx = l_bmp.bmWidth;
		l_size.cy = l_bmp.bmHeight;

		HRGN l_hRgnBitmap = NULL;

		// Insert the bitmap into a temporary memory dc
		HDC l_hdcMem = ::CreateCompatibleDC(NULL);
		::SelectObject(l_hdcMem, a_hBmp);
		if (!a_bColor)
		{
			a_clrTransp = ::GetPixel(l_hdcMem, a_ptTransp.x, a_ptTransp.y);
		}

		l_hRgnBitmap = InternalCreateRegion(l_hdcMem, a_clrTransp, l_size);

		::DeleteDC(l_hdcMem);
		return l_hRgnBitmap;
	}

	HRGN CreateRegionFromBitmap(HBITMAP a_hBmp, COLORREF a_clrTransp)
	{
		ASSERT(a_hBmp != NULL);
		POINT l_pt = { 0, 0 };
		return CreateRegionFromBitmapPoint(a_hBmp, a_clrTransp, l_pt, true);
	}


	HRGN CreateRegionFromBitmap(HBITMAP a_hBmp, const POINT& a_ptTransp)
	{
		ASSERT(a_hBmp != NULL);
		return CreateRegionFromBitmapPoint(a_hBmp, RGB(0, 0, 0), a_ptTransp, false);
	}


	HRGN InternalCreateRegion(HDC a_hdcMem, COLORREF a_clrTransp, const SIZE& a_sizeBmp)
	{
		// We start with the full rectangular region
		HRGN l_hrgnBitmap = ::CreateRectRgn(0, 0, a_sizeBmp.cx, a_sizeBmp.cy);

		BOOL l_bInTransparency = FALSE;  // Already inside a transparent part?
		int l_xStart = -1;			   // Start of transparent part

		// For all rows of the bitmap ...
		for (int l_y = 0; l_y < a_sizeBmp.cy; l_y++)
		{
			// For all pixels of the current row ...
			// (To close any transparent region, we go one pixel beyond the
			// right boundary)
			for (int l_x = 0; l_x <= a_sizeBmp.cx; l_x++)
			{
				BOOL l_bTransparent = FALSE; // Current pixel transparent?

				// Check for positive transparency within image boundaries

				if ((l_x < a_sizeBmp.cx) && (a_clrTransp == ::GetPixel(a_hdcMem, l_x, l_y)))
				{
					l_bTransparent = TRUE;
				}

				// Does transparency change?
				if (l_bInTransparency != l_bTransparent)
				{
					if (l_bTransparent)
					{
						// Transparency starts. Remember x position.
						l_bInTransparency = TRUE;
						l_xStart = l_x;
					}
					else
					{
						// Transparency ends (at least beyond image boundaries).
						// Create a region for the transparency, one pixel high,
						// beginning at start_x and ending at the current x, and
						// subtract that region from the current bitmap region.
						// The remainding region becomes the current bitmap region.
						HRGN l_hrgnTransp = ::CreateRectRgn(l_xStart, l_y, l_x, l_y + 1);
						::CombineRgn(l_hrgnBitmap, l_hrgnBitmap, l_hrgnTransp, RGN_DIFF);
						::DeleteObject(l_hrgnTransp);

						l_bInTransparency = FALSE;
					}
				}
			}
		}

		return l_hrgnBitmap;
	}



	void BlendOverlay(HDC a_hDC, RECT a_rect, COLORREF a_clr, BYTE a_btAlpha)
	{
		// Create a memory DC and compatible bitmap
		HDC l_hdcOverlay = ::CreateCompatibleDC(a_hDC);
		HBITMAP l_hBmpOverlay = ::CreateCompatibleBitmap(a_hDC, a_rect.right - a_rect.left, a_rect.bottom - a_rect.top);
		HBITMAP l_hBmpOld = (HBITMAP)::SelectObject(l_hdcOverlay, l_hBmpOverlay);

		// Fill the overlay with the desired color
		HBRUSH l_hBrush = ::CreateSolidBrush(a_clr);
		::FillRect(l_hdcOverlay, &a_rect, l_hBrush);
		::DeleteObject(l_hBrush);

		// Set up blending function
		BLENDFUNCTION l_blend = { AC_SRC_OVER, 0, a_btAlpha, 0 };

		// Apply the overlay
		::AlphaBlend(a_hDC, a_rect.left, a_rect.top, a_rect.right - a_rect.left, a_rect.bottom - a_rect.top,
			l_hdcOverlay, a_rect.left, a_rect.top, a_rect.right - a_rect.left, a_rect.bottom - a_rect.top, l_blend);

		// Cleanup
		::SelectObject(l_hdcOverlay, l_hBmpOld);
		::DeleteObject(l_hBmpOverlay);
		::DeleteDC(l_hdcOverlay);
	}



void DrawSketchLine(Gdiplus::Graphics* a_graphics, POINT a_ptStart, POINT a_ptEnd, int a_iWobble, int a_iSegments, float a_fThickness, Gdiplus::Color a_color)
{
	Gdiplus::Pen l_pen(a_color, a_fThickness);
	l_pen.SetStartCap(Gdiplus::LineCapRound);
	l_pen.SetEndCap(Gdiplus::LineCapRound);
	l_pen.SetLineJoin(Gdiplus::LineJoinRound);

	std::vector<Gdiplus::PointF> l_vectPoints;
	float l_dx = (float)(a_ptEnd.x - a_ptStart.x) / a_iSegments;
	float l_dy = (float)(a_ptEnd.y - a_ptStart.y) / a_iSegments;

	float l_xPoint = (float)a_ptStart.x;
	float l_yPoint = (float)a_ptStart.y;

	l_vectPoints.emplace_back(l_xPoint, l_yPoint);

	for (int l_iAt = 1; l_iAt <= a_iSegments; ++l_iAt)
	{
		float l_fAngle = (float)l_iAt / a_iSegments * cc_fPi * 2.0f;
		float l_xOffset = std::sin(l_fAngle * 2.0f) * a_iWobble;
		float l_yOffset = std::cos(l_fAngle * 1.5f) * a_iWobble;

		l_xPoint += l_dx;
		l_yPoint += l_dy;

		l_vectPoints.emplace_back(l_xPoint + l_xOffset, l_yPoint + l_yOffset);
	}

	a_graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	a_graphics->DrawLines(&l_pen, l_vectPoints.data(), static_cast<INT>(l_vectPoints.size()));
}


void DrawSketchLine(HDC a_hDC, POINT a_ptStart, POINT a_ptEnd,
	int a_iWobble, int a_iSegments, float a_fThickness, COLORREF a_color, BYTE a_btAlpha)
{
	Gdiplus::Graphics l_graphics(a_hDC);

	DrawSketchLine(&l_graphics, a_ptStart, a_ptEnd, a_iWobble, a_iSegments, a_fThickness, Gdiplus::Color(a_btAlpha, GetRValue(a_color), GetGValue(a_color), GetBValue(a_color)));
}




void DrawSmartText(Gdiplus::Graphics* a_graphics, const Gdiplus::Font& a_font, const Gdiplus::RectF& a_rectLayout, LPCTSTR a_sText,
	const Gdiplus::Color& a_colorText,		// main text
	const Gdiplus::Color& a_colorOutline)	// outline (black)
{
	Gdiplus::StringFormat l_format;
	l_format.SetAlignment(Gdiplus::StringAlignmentCenter);
	l_format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

#ifdef _UNICODE
	std::wstring l_wsText(a_sText);
#else
	int l_iLen = ::MultiByteToWideChar(CP_UTF8, 0, a_sText, -1, nullptr, 0);
	std::wstring l_wsText(l_iLen, L'\0');
	::MultiByteToWideChar(CP_UTF8, 0, a_sText, -1, l_wsText.data(), l_iLen);
#endif

	// Simulate outline by drawing text offset in 8 directions
	const float l_fOffset = 1.0f;
	for (int l_dx = -1; l_dx <= 1; ++l_dx)
	{
		for (int l_dy = -1; l_dy <= 1; ++l_dy)
		{
			if (l_dx == 0 && l_dy == 0) continue; // skip center
			Gdiplus::RectF l_rectOutline = a_rectLayout;
			l_rectOutline.X += l_dx * l_fOffset;
			l_rectOutline.Y += l_dy * l_fOffset;
			Gdiplus::SolidBrush l_brushOutline(a_colorOutline);
			a_graphics->DrawString(l_wsText.c_str(), -1, &a_font, l_rectOutline, &l_format, &l_brushOutline);
		}
	}

	// Draw main text
	Gdiplus::SolidBrush l_brushText(a_colorText);
	a_graphics->DrawString(l_wsText.c_str(), -1, &a_font, a_rectLayout, &l_format, &l_brushText);
}

void DrawSmartText(HDC a_hDC, LPCTSTR a_sFont, int a_iFontSize, const RECT& a_rectLayout, LPCTSTR a_sText,
	COLORREF a_colorText, BYTE a_btAlphaText, COLORREF a_colorOutline, BYTE a_btAlphaOutline)
{
	Gdiplus::Graphics l_graphics(a_hDC);

#ifdef _UNICODE
	std::wstring l_wsFont(a_sFont);
#else
	int l_iLen = ::MultiByteToWideChar(CP_UTF8, 0, a_sFont, -1, nullptr, 0);
	std::wstring l_wsFont(l_iLen, L'\0');
	::MultiByteToWideChar(CP_UTF8, 0, a_sFont, -1, l_wsFont.data(), l_iLen);
#endif

	Gdiplus::Font l_font(l_wsFont.c_str(), static_cast<Gdiplus::REAL>(a_iFontSize));
	Gdiplus::RectF l_rectLayout(static_cast<Gdiplus::REAL>(a_rectLayout.left), static_cast<Gdiplus::REAL>(a_rectLayout.top), static_cast<Gdiplus::REAL>(RectWidth(a_rectLayout)), static_cast<Gdiplus::REAL>(RectHeight(a_rectLayout)));

	Gdiplus::Color l_colorText(a_btAlphaText, GetRValue(a_colorText), GetGValue(a_colorText), GetBValue(a_colorText));
	Gdiplus::Color l_colorOutline(a_btAlphaOutline, GetRValue(a_colorOutline), GetGValue(a_colorOutline), GetBValue(a_colorOutline));


	DrawSmartText(&l_graphics, l_font, l_rectLayout, a_sText, l_colorText, l_colorOutline);

}

} // end of namespace