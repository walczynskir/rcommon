// RColorData.cpp: implementation of the RColorData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RColorData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static const int s_iCols = 4;


RColorData::RColorData() 
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
	m_hBitmap = NULL;
	m_iColumns = s_iCols;
}


RColorData::~RColorData()
{
	::DeleteObject(m_hBrushWhite);
	::DeleteObject(m_hBrushBlack);
	::DeleteObject(m_hBrush3DShadow);
	::DeleteObject(m_hBrushBtnFace);
	::DeleteObject(m_hBrushColor);
}
