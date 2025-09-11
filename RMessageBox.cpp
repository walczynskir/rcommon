#include "stdafx.h"
#include "resource.h"
#include <rcommon/RMessageBox.h>

// Static member definitions
HHOOK RCenteredMessageBox::s_hHook = nullptr;
HWND RCenteredMessageBox::s_hParent = nullptr;




int RCenteredMessageBox::Show(HWND hParent, LPCWSTR text, LPCWSTR caption, UINT type)
{
    s_hParent = hParent;
    s_hHook = SetWindowsHookEx(WH_CBT, HookProc, nullptr, GetCurrentThreadId());
    return MessageBox(hParent, text, caption, type);
}

LRESULT CALLBACK RCenteredMessageBox::HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HCBT_ACTIVATE)
    {
        HWND hMsgBox = reinterpret_cast<HWND>(wParam);

        RECT rcParent{}, rcMsgBox{};
        GetWindowRect(s_hParent, &rcParent);
        GetWindowRect(hMsgBox, &rcMsgBox);

        int width = rcMsgBox.right - rcMsgBox.left;
        int height = rcMsgBox.bottom - rcMsgBox.top;

        int x = rcParent.left + ((rcParent.right - rcParent.left) - width) / 2;
        int y = rcParent.top + ((rcParent.bottom - rcParent.top) - height) / 2;

        MoveWindow(hMsgBox, x, y, width, height, FALSE);

        UnhookWindowsHookEx(s_hHook);
    }

    return CallNextHookEx(s_hHook, nCode, wParam, lParam);
}


void RCenteredMessageBox::WarningBox(HWND a_hDlg, LPCTSTR a_sWarningText)
{
	TCHAR l_sWarningTitle[256];
	::LoadString(RCommon_GetInstance(), IDS_BOX_WARNING, l_sWarningTitle, ArraySize(l_sWarningTitle));
    RCenteredMessageBox::Show(a_hDlg, a_sWarningText, l_sWarningTitle, MB_OK | MB_ICONWARNING);
}


BOOL RCenteredMessageBox::DecisionBox(HWND a_hDlg, LPCTSTR a_sDecisionText)
{
	TCHAR l_sDecisionTitle[256];
	::LoadString(RCommon_GetInstance(), IDS_BOX_DECISION, l_sDecisionTitle, ArraySize(l_sDecisionTitle));
	return (RCenteredMessageBox::Show(a_hDlg, a_sDecisionText, l_sDecisionTitle, MB_YESNO | MB_ICONWARNING) == IDYES);
}
