// RColorData.h: interface for the RColorData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RCOLORDATA_H__FBD57AB9_03B1_49F4_8939_587D23317A1E__INCLUDED_)
#define AFX_RCOLORCDATA_H__FBD57AB9_03B1_49F4_8939_587D23317A1E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <rcommon/rstring.h>

class RColorData
{
public:
	RColorData();
	virtual ~RColorData();

	COLORREF GetColor() const { return m_color; };
	void SetColor(COLORREF a_color) 
	{ 
		m_color = a_color; 
		::DeleteObject(m_hBrushColor);
		m_hBrushColor = ::CreateSolidBrush(m_color);
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
	HBITMAP m_hBitmap;
	UINT    m_iColumns;

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


#endif // !defined(AFX_RCOLORDATA_H__FBD57AB9_03B1_49F4_8939_587D23317A1E__INCLUDED_)
