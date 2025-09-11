#pragma once
#include <rcommon/rcommon.h>



class RCOMMON_API RCenteredMessageBox
{
public:
    static int Show(HWND hParent, LPCWSTR text, LPCWSTR caption, UINT type);
    void WarningBox(HWND a_hDlg, LPCTSTR a_sWarningText);
    BOOL DecisionBox(HWND a_hDlg, LPCTSTR a_sDecisionText);

private:
    static HHOOK s_hHook;
    static HWND s_hParent;

    static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);

};

