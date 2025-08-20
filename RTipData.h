#pragma once

// RTipData.h: interface for the RTipData class.
//
//////////////////////////////////////////////////////////////////////

#include "RString.h"
#include "celltip.h"

#define TDC_TEXT 0
#define TDC_BK   1


class RTipData  
{
public:
	RTipData();
	virtual ~RTipData();

	void SetOwnerWnd(HWND a_hWnd) { m_hWndOwner = a_hWnd; };
	HWND GetOwnerWnd() const { return m_hWndOwner; };

	void SetText(LPCTSTR a_psText) { m_sText = a_psText; };
	LPCTSTR GetText() const { return m_sText.c_str(); };
	int GetTextLen() const { return static_cast<int>(m_sText.length()); };

	void SetColorText(COLORREF a_dwColor) { m_arColors[TDC_TEXT] = a_dwColor; };
	COLORREF GetColorText() const { return m_arColors[TDC_TEXT]; };

	void SetColorBk(COLORREF a_dwColor) { m_arColors[TDC_BK] = a_dwColor; };
	COLORREF GetColorBk() const { return m_arColors[TDC_BK]; };

	void SetSize(const SIZE& a_size) { m_size = a_size; };
	void SetDx(LONG a_dx) { m_size.cx = a_dx; };
	void SetDy(LONG a_dy) { m_size.cy = a_dy; };
	SIZE GetSize() const { return m_size; };

	void SetFont(HFONT a_hFont) { m_hFont = a_hFont; };
	HFONT GetFont() const { return m_hFont; };

	RTIPDELAY  m_tipDelay;
	POINT  m_pt;
private:
	HWND m_hWndOwner;
	tstring m_sText;
	COLORREF m_arColors[2];
	SIZE m_size;;
	HFONT m_hFont;
};

