// RColorDownData.cpp: implementation of the RColorDownData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "rcommon.h"
#include "RColorDownData.h"
#include "resource.h"
#include "RColorWnd.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
bool RColorDownData::s_bInititalized = Initialize();
COLORREF RColorDownData::s_crSavedCustom[16];



// positions should be calculated, not hardcoded!
static COLORREF s_PosColor[] = 
{
	RGB(255, 255, 255),
	RGB(0, 0, 0),
	RGB(192, 192, 192),
	RGB(128, 128, 128),

	RGB(255, 0, 0),
	RGB(128, 0, 0),
	RGB(255, 255, 0),
	RGB(128, 128, 0),

	RGB(0, 255, 0),
	RGB(0, 128, 0),
	RGB(0, 255, 255),
	RGB(0, 128, 128),

	RGB(0, 0, 255),
	RGB(0, 0, 128),
	RGB(255, 0, 255),
	RGB(128, 0, 128),

	RGB(192, 220, 192),
	RGB(166, 202, 240),
	RGB(255, 251, 240),
	RGB(160, 160, 164)
};




RColorDownData::RColorDownData(const RColorData* a_pData) : m_pData(a_pData)
{
	m_hWndOwner = NULL;
	m_stateButton = e_Normal;
	m_color = RGB(0, 0, 0);
	m_colorSel = m_color;
	m_bHovering = false;
	m_bShowPrefix = false;
	m_sBtnText[0] = L'\0';

	if (::LoadString(RCommon_GetInstance(), IDS_COLOR_BTN, m_sBtnText, ArraySize(m_sBtnText)))
	{
		LPCTSTR l_psSign = m_sBtnText;
		m_cAccell = _T('\0');
		while (l_psSign != _T('\0'))
		{
			if (*l_psSign == _T('&'))
			{
				if (*(l_psSign + 1) != _T('&'))
				{
					m_cAccell = static_cast<TCHAR>(_totupper(*(l_psSign + 1)));
					break;
				}
				else
				{
					l_psSign++;
				}
			}
			l_psSign++;
		}
	}
}


RColorDownData::~RColorDownData()
{

}


bool RColorDownData::Initialize()
{
	int l_iAt;
	for (l_iAt = 0; l_iAt < ArraySize(s_crSavedCustom); l_iAt++)
		s_crSavedCustom[l_iAt] = RGB(255, 255, 255);
	return true;
}


UINT RColorDownData::GetRows() const
{
	return (ArraySize(s_PosColor) / GetColumns());
}


COLORREF RColorDownData::GetColor(UINT a_iIdx) const
{
	ASSERT((a_iIdx >= 0) && (a_iIdx < ArraySize(s_PosColor)));
	return s_PosColor[a_iIdx];
}


COLORREF RColorDownData::GetColorsCount() const
{
	return ArraySize(s_PosColor);
}