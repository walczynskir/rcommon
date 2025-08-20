// RTipData.cpp: implementation of the RTipData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <tchar.h>
#include "RTipData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RTipData::RTipData()
{
	m_sText = _T("");
	m_arColors[TDC_TEXT] = ::GetSysColor(COLOR_INFOTEXT);
	m_arColors[TDC_BK] = ::GetSysColor(COLOR_INFOBK);
	m_hWndOwner = NULL;
	m_hFont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	m_tipDelay.iShow = 0;
	m_tipDelay.iHide = 0;
}

RTipData::~RTipData()
{

}
