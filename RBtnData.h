#pragma once
#include <commctrl.h>
#include "RoundIter.h"

class RBtnData
{
public:
	RBtnData(void)
	{
		m_hImageList = NULL;
		m_bHovering = false;
		m_state = e_Normal;
		m_bOverImg = true;
	}

	~RBtnData(void)
	{
		if (m_hImageList != NULL)
		{
			ImageList_Destroy(m_hImageList);
			m_hImageList = NULL;
		}
	}
	bool IsHovering() const { return m_bHovering; };
	void SetHovering(bool a_bHovering) { m_bHovering = a_bHovering; };

	bool IsClicked() const { return (m_state == e_Clicked); };
	bool IsPressed() const { return (m_state == e_Pressed); };
	bool IsNormal() const { return (m_state == e_Normal); };
	void SetClicked() { m_state = e_Clicked; };
	void SetPressed() { m_state = e_Pressed; };
	void SetNormal() { m_state = e_Normal; };

	HIMAGELIST m_hImageList;
	bool m_bOverImg;
	RRoundIter<int> m_iterCurrent;

private:
	bool m_bHovering;

	enum 
	{
		e_Normal,
		e_Clicked,	// clicked by mouse 
		e_Pressed	// pushed by key (space)
	} m_state;
};
