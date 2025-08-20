#pragma once
#include "rstring.h"


class RHyperlinkData
{
public:
	RHyperlinkData(void) : m_sUrl(_T("")), m_sText(_T(""))
	{
		m_bFollowed = false;
		m_bHover = false;
		m_hFont = NULL;
		m_hCursor = NULL;
		m_hTip = NULL;
		m_hDefCursor = ::LoadCursor(NULL, IDC_HAND);
		SetFont(static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT)));
		m_colors.clrLink = RGB(0, 0, 255);	// Blue
		m_colors.clrActive = RGB(0, 128, 128);	// Dark cyan
		m_colors.clrVisited = RGB(128, 0, 128);	// Purple
		m_colors.clrHover = RGB(255, 0, 0);	// Red
	}

	~RHyperlinkData(void)
	{
		if (m_hDefCursor != NULL)
		{
			::DestroyCursor(m_hDefCursor);
		}
		DeleteFontObj();
		if (m_hTip != NULL)
		{
			::DestroyWindow(m_hTip);
		}
	}

	// make font underlined
	void SetFont(HFONT a_hFont)
	{
		LOGFONT l_lf;
		if (::GetObject(a_hFont, sizeof(l_lf), &l_lf) == 0)
		{
			return;
		}
		l_lf.lfUnderline = TRUE;
		DeleteFontObj();
		m_hFont = ::CreateFontIndirect(&l_lf);
	}

	HFONT GetFont() const { return m_hFont; }
	void DeleteFontObj() 
	{ 
		if (m_hFont != NULL)
		{
			::DeleteObject(m_hFont);
			m_hFont = NULL;
		}
	}

	RHYPERLINKCOLORS m_colors;
	tstring  m_sText;
	tstring  m_sUrl;
	bool     m_bFollowed;
	bool	 m_bHover;
	HCURSOR  m_hCursor;
	HCURSOR  m_hDefCursor;
	HWND	 m_hTip;
private:
	HFONT    m_hFont;
};
