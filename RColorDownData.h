// RColorDownData.h: interface for the RColorDownData class.
//
//////////////////////////////////////////////////////////////////////
#include "RColorData.h"

#if !defined(AFX_RCOLORDOWNDATA_H__FBD57AB9_03B1_49F4_8939_587D23317A1E__INCLUDED_)
#define AFX_RCOLORDOWNDATA_H__FBD57AB9_03B1_49F4_8939_587D23317A1E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class RColorDownData
{
public:
	typedef enum 
	{
		e_Normal = 1,
		e_Selected = 2, 
		e_Pushed = 4,		// buuton is sunken because was clicked or pressed
		e_Clicked = 8,		// clicked by mouse, so pushed
		e_Pressed = 16		// pressed by key (space)
	} T_STATE;

	RColorDownData(const RColorData* a_pData);
	virtual ~RColorDownData();

	COLORREF GetColor() const { return m_color; };
	void SetColor(COLORREF a_color) { m_color = a_color; };

	COLORREF GetSelColor() const { return m_colorSel; };
	void SetSelColor(COLORREF a_color) { m_colorSel = a_color; };

	bool IsHovering() const { return m_bHovering; };
	void SetHovering(bool a_bHovering) { m_bHovering = a_bHovering; };

	BOOL IsButtonNormal() const { return (m_stateButton & e_Normal); };
	BOOL IsButtonSelected() const { return (m_stateButton & e_Selected); };
	BOOL IsButtonPushed() const { return (m_stateButton & e_Pushed); };
	BOOL IsButtonClicked() const { return (m_stateButton & e_Clicked); };
	BOOL IsButtonPressed() const { return (m_stateButton & e_Pressed); };

	void SetButtonState(BYTE a_state) { m_stateButton = a_state; };
	COLORREF* GetSavedColors() { return s_crSavedCustom; };

	HWND GetOwner() const { return m_hWndOwner; };
	void SetOwner(HWND a_hWndOwner) { m_hWndOwner = a_hWndOwner; };
	
	UINT GetColumns() const { return m_pData->m_iColumns; };

	UINT GetRows() const;
	COLORREF GetColor(UINT a_iIdx) const;
	COLORREF GetColorsCount() const;

	bool m_bShowPrefix;
	TCHAR m_sBtnText[100];
	TCHAR m_cAccell;
private:

	COLORREF m_color;
	COLORREF m_colorSel;
	HWND m_hWndOwner;
	bool m_bHovering;
    const RColorData* m_pData;

	void LoadDefColors();
	static bool Initialize();
	static bool s_bInititalized;
	static COLORREF s_crSavedCustom[16];


	BYTE m_stateButton;
	
};

#endif // !defined(AFX_RCOLORDOWNDATA_H__FBD57AB9_03B1_49F4_8939_587D23317A1E__INCLUDED_)
