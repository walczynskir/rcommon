// REmoData.h: interface for the REmoData class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

class REmoData
{
public:
	REmoData() 
	{
		m_state = e_Normal;
		m_color = RGB(0, 0, 0);
		m_wndDown = NULL;
		m_bHovering = false;
		m_hBrushBlack = ::CreateSolidBrush(RGB(0, 0, 0));
		m_hBrushWhite = ::CreateSolidBrush(RGB(255, 255, 255));
		m_hBrush3DShadow = ::CreateSolidBrush(::GetSysColor(COLOR_3DSHADOW));
		m_hBrushBtnFace =  ::CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
		m_hBrushColor =  ::CreateSolidBrush(m_color);
		m_hTooltip = NULL;
		m_psTooltip = NULL;
		m_hSelBitmap = NULL;
		m_hBitmap = NULL;
		m_sel.iRow = 1; 
		m_sel.iCol = 1; 
		m_sizeEmo.cx = 22; 
		m_sizeEmo.cy = 22; 
		m_clrBackBitmap = RGB(192, 192, 192);
	}


	~REmoData()
	{
		::DeleteObject(m_hBrushWhite);
		::DeleteObject(m_hBrushBlack);
		::DeleteObject(m_hBrush3DShadow);
		::DeleteObject(m_hBrushBtnFace);
		::DeleteObject(m_hBrushColor);
		if (m_hSelBitmap != NULL)
		{
			::DeleteObject(m_hSelBitmap);
		}
	}
	
	HWND GetDownWnd() const { return m_wndDown; };
	void SetDownWnd(HWND a_hWnd) { m_wndDown = a_hWnd; };

	bool IsHovering() const { return m_bHovering; };
	void SetHovering(bool a_bHovering) { m_bHovering = a_bHovering; };

	bool IsClicked() const { return (m_state == e_Clicked); };
	bool IsPressed() const { return (m_state == e_Pressed); };
	bool IsNormal() const { return (m_state == e_Normal); };
	void SetClicked() { m_state = e_Clicked; };
	void SetPressed() { m_state = e_Pressed; };
	void SetNormal() { m_state = e_Normal; };

	HBRUSH m_hBrushWhite;
	HBRUSH m_hBrushBlack;
	HBRUSH m_hBrush3DShadow;
	HBRUSH m_hBrushBtnFace;
	HBRUSH m_hBrushColor;
	LPTSTR m_psTooltip;
	HWND   m_hTooltip;
	HBITMAP m_hSelBitmap;
	TSelect m_sel;

	SIZE    m_sizeEmo;
	UINT    m_iRows;
	UINT    m_iCols;
	HBITMAP m_hBitmap;
	COLORREF m_clrBackBitmap;

private:
	COLORREF m_color;
	HWND m_wndDown;
	bool m_bHovering;

	enum 
	{
		e_Normal,
		e_Clicked,	// clicked by mouse 
		e_Pressed	// pushed by key (space)
	} m_state;
};



