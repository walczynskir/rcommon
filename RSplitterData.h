#pragma once
#include "resource.h"

class RSplitterData
{
public:

	RSplitterData(BYTE a_btBarThickness = 5) 
	{
		memset(&m_sizing, 0, sizeof(m_sizing));
		m_bDragging = false;
		m_btBarThickness = a_btBarThickness;
		m_hWndTop = NULL;
		m_hWndBottom = NULL;
		m_iDrag = 0;
		m_ptDragTip.x = 0;
		m_ptDragTip.y = 0;
		m_hTip = NULL;
		m_btBias = 0;
		SetSimpleBrush();
		m_hBrushBorder = reinterpret_cast<HBRUSH>(COLOR_BTNSHADOW + 1);

		m_hTracker = NULL;
		m_hBrushTracker = NULL;
		m_bDeleteTrackerBrush = false;
		m_bRedraw = true;
	}

	~RSplitterData(void)
	{
		if (m_hTip != NULL)
		{
			::DestroyWindow(m_hTip);
		}
		if (m_hTracker != NULL)
		{
			::DestroyWindow(m_hTracker);
		}
		if (m_hBrushBorder != NULL)
		{
			::DeleteObject(m_hBrushBorder);
		}
		ReleaseBrush();
		if (m_bDeleteTrackerBrush)
		{
			::DeleteObject(m_hBrushTracker);
		}
	}

	void SetSimpleBrush() { m_hBrush = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1); }

	typedef union U_SIZING
	{
		long iSize;
		double fPerc;
	} SIZING, *LPSIZING;

	void SetSize(long a_iSize) { m_sizing.iSize = max(0,  a_iSize); }
	void SetPSize(double a_fPerc) 
	{ 
		if (a_fPerc < 0) 
		{
			a_fPerc = 0;
		}
		else if (a_fPerc > 100)
		{
			a_fPerc = 100;
		}
		m_sizing.fPerc = a_fPerc; 
	}

	SIZING GetSize() const { return m_sizing; }
	void ReleaseBrush() 
	{
		if (m_hBrush != NULL)
		{
			::DeleteObject(m_hBrush);
		}
	}

	void CreateTrackerBrush(bool a_bHalftone)
	{
		ASSERT(m_hBrushTracker == NULL);
		if (a_bHalftone)
		{
			WORD l_arrPattern[8];
			for (int i = 0; i < 8; i++)
			{
				l_arrPattern[i] = (WORD)(0x5555 << (i & 1));
			}
			HBITMAP l_bmpGray = ::CreateBitmap(8, 8, 1, 1, &l_arrPattern);
			if (l_bmpGray != NULL)
			{
				m_hBrushTracker = ::CreatePatternBrush(l_bmpGray);
				m_bDeleteTrackerBrush = true;
				::DeleteObject(l_bmpGray);
			}
		}
		else
		{
			m_hBrushTracker = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
		}
	}

	POINT  m_ptDragTip;
	bool   m_bDragging;
	BYTE   m_btBarThickness;
	HWND   m_hWndTop;
	HWND   m_hWndBottom;
	HBRUSH m_hBrush;
	HBRUSH m_hBrushBorder;

	HWND   m_hTip;

	long   m_iDrag;
	BYTE   m_btBias;

	HWND   m_hTracker;
	HBRUSH m_hBrushTracker;
	bool   m_bDeleteTrackerBrush;
	bool   m_bRedraw;

private:
	SIZING m_sizing;
};
