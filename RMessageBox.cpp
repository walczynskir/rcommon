#include "stdafx.h"
#include "resource.h"
#include <rcommon/RMessageBox.h>


void WarningBox(HWND a_hDlg, LPCTSTR a_sWarningText)
{
	TCHAR l_sWarningTitle[256];
	::LoadString(RCommon_GetInstance(), IDS_BOX_WARNING, l_sWarningTitle, ArraySize(l_sWarningTitle));
	::MessageBox(a_hDlg, a_sWarningText, l_sWarningTitle, MB_OK | MB_ICONWARNING);
}


BOOL DecisionBox(HWND a_hDlg, LPCTSTR a_sDecisionText)
{
	TCHAR l_sDecisionTitle[256];
	::LoadString(RCommon_GetInstance(), IDS_BOX_DECISION, l_sDecisionTitle, ArraySize(l_sDecisionTitle));
	return (::MessageBox(a_hDlg, a_sDecisionText, l_sDecisionTitle, MB_YESNO | MB_ICONWARNING) == IDYES);
}
