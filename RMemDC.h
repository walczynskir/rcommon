#pragma once
#include "rcommon.h"

// inspired (and fragments of code used) by Keith Rule article from www.codeproject.com

class RCOMMON_API RMemDC
{
public:

	RMemDC(HDC a_hDC, const RECT* a_pRect)
	{
		m_hDC = a_hDC;
		
		if (a_pRect == NULL)
		{
			::GetClipBox(m_hDC, &m_rect);
		}
		else
		{
			m_rect = *a_pRect;
		}

		// create compatible offscreen bitmap
		m_hMemDC = ::CreateCompatibleDC(m_hDC);
		m_hBmp = ::CreateCompatibleBitmap(m_hDC, 
			m_rect.right - m_rect.left, m_rect.bottom - m_rect.top);
		m_hBmpOld = (HBITMAP)::SelectObject(m_hMemDC, m_hBmp);
		::SetWindowOrgEx(m_hMemDC, m_rect.left, m_rect.top, NULL);
		
		// do not know which solution is better
		//RDraw::FillSolidRect(m_hMemDC, m_rect, ::GetBkColor(m_hDC));
		// copy real dc to memdc
		// watchout! BitBlt is very slow on big screens (resolution higher than 1026*768)
		::BitBlt(m_hMemDC, m_rect.left, m_rect.top, 
			m_rect.right - m_rect.left, m_rect.bottom - m_rect.top,
			m_hDC, m_rect.left, m_rect.top, SRCCOPY);
	}

	virtual ~RMemDC(void)
	{
		// Copy the offscreen bitmap onto the screen.
		::BitBlt(m_hDC, m_rect.left, m_rect.top, 
			m_rect.right - m_rect.left, m_rect.bottom - m_rect.top,
			m_hMemDC, m_rect.left, m_rect.top, SRCCOPY);

		//Swap back the original bitmap.
		::SelectObject(m_hMemDC, m_hBmpOld);
		::DeleteObject(m_hBmp);
		::DeleteDC(m_hMemDC);

	}

	operator HDC() { return m_hMemDC; };

private:
	HDC m_hDC;
	HDC m_hMemDC;
	HBITMAP m_hBmp;
	HBITMAP m_hBmpOld;
	RECT m_rect;
};
