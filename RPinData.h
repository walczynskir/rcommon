#pragma once
#include "rstring.h"
#include <commctrl.h>
#include <vector>


typedef std::vector<tstring> StringVector;

class RPinData
{
public:
	RPinData(void)
	{
		m_hWnd = NULL;
		m_bOnTop = false;
		HBITMAP l_hBmpPin = ::LoadBitmap(RCommon_GetInstance(), MAKEINTRESOURCE(IDB_PIN));
		ASSERT(l_hBmpPin != NULL);
		BITMAP l_bmp;
		::GetObject(l_hBmpPin, sizeof(l_bmp), &l_bmp);
		m_sizePin.cx = l_bmp.bmWidth / 3;
		m_sizePin.cy = l_bmp.bmHeight;
		// must have same width and height
		ASSERT(m_sizePin.cx == m_sizePin.cy);

		m_hImagePin = ImageList_Create(m_sizePin.cx, m_sizePin.cy, ILC_COLOR32 | ILC_MASK,
			3, 3);
		ImageList_AddMasked(m_hImagePin, l_hBmpPin, RGB(255, 0, 0));
		::DeleteObject(l_hBmpPin);
		m_clrCaption[0] = m_clrCaption[1] = ::GetSysColor(COLOR_ACTIVECAPTION);
		m_clrCaption[2] = ::GetSysColor(COLOR_3DFACE);
		m_hImageAddit = NULL;
		m_hImageCaption = NULL;
		m_nAniStyle = 0;
		m_bHovering = false;
		m_iHoverAt = 0;
		m_iPushedAt = 0;
		m_iDelay = 2000;
		m_hTip = NULL;
	}

	~RPinData(void)
	{
		ImageList_Destroy(m_hImagePin);
		if (m_hImageAddit != NULL)
		{
			ImageList_Destroy(m_hImageAddit);
		}
		if (m_hImageCaption != NULL)
		{
			ImageList_Destroy(m_hImageCaption);
		}
	}

	HWND	      m_hWnd;
	bool          m_bOnTop;
	SIZE          m_sizePin;
	SIZE		  m_sizeAddit;
	HIMAGELIST    m_hImagePin;
	HIMAGELIST    m_hImageAddit;
	StringVector  m_sTipsAddit;
	HIMAGELIST    m_hImageCaption;
	SIZE		  m_sizeCaption;
	tstring		  m_sTipCaption;
	int			  m_iCntAddit;
	bool          m_bHovering;
	int			  m_iHoverAt;
	int			  m_iPushedAt;
	COLORREF      m_clrCaption[4];
	short		  m_nAniStyle;
	int           m_iDelay;
	HWND		  m_hTip;
	tstring		  m_sPinOn;
	tstring		  m_sPinOff;
	SIZE		  m_sizeWnd;
};
