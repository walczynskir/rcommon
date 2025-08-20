#pragma once
#include "rstring.h"


class RKeyData
{
public:
	RKeyData(void) : m_clrText(::GetSysColor(COLOR_GRAYTEXT)), m_sEmptyText(_T(""))
	{
		ClearKey();
		m_hFont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	}

	~RKeyData(void)
	{
	}

	void ClearKey()
	{
		m_rkey.iMod = 0;
		m_rkey.iKey = 0;
	}

	HFONT    m_hFont;
	COLORREF m_clrText;
	RSHORTCUTKEY	 m_rkey;
	tstring	 m_sEmptyText;
};
