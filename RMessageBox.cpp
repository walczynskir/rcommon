#include "stdafx.h"
#include "resource.h"
#include "RMessageBox.h"

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

BOOL RCenteredMessageBox::DecisionBox2(HWND a_hWndParent, LPCTSTR a_sDecisionText)
{
    TCHAR l_sDecisionTitle[256];
    ::LoadString(RCommon_GetInstance(), IDS_BOX_DECISION, l_sDecisionTitle, ArraySize(l_sDecisionTitle));

    RCustomMessageBox l_msgBox(RCommon_GetInstance(), a_hWndParent);
    l_msgBox.SetCaption(l_sDecisionTitle);
    l_msgBox.SetMessage(a_sDecisionText);
    l_msgBox.SetIcon(RCustomMessageBox::IconType::Question);
    l_msgBox.SetCenteredParent();

    return (l_msgBox.Show() == RCustomMessageBox::Result::Yes);
}


RCustomMessageBox::Result RCustomMessageBox::Show() {
    return static_cast<Result>(DialogBoxParam(
        m_hInstance,
        MAKEINTRESOURCE(IDD_CUSTOM_MSGBOX),
        m_hParent,
        DialogProc,
        reinterpret_cast<LPARAM>(this)
    ));
}

INT_PTR CALLBACK RCustomMessageBox::DialogProc(HWND a_hDlg, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam)
{
    static RCustomMessageBox* l_pThis = nullptr;

    switch (a_iMsg) {
    case WM_INITDIALOG:
        l_pThis = reinterpret_cast<RCustomMessageBox*>(a_lParam);
        l_pThis->m_hDlg = a_hDlg;
        return l_pThis->OnInitDialog();

    case WM_COMMAND:
        switch (LOWORD(a_wParam))
        {
        case IDYES:
        case IDNO:
            ::EndDialog(a_hDlg, LOWORD(a_wParam));
            return TRUE;
        }
        break;
    case WM_CLOSE:
        EndDialog(a_hDlg, IDNO); // or IDCANCEL if you prefer
        return TRUE;
    }
    return FALSE;
}


BOOL RCustomMessageBox::OnInitDialog()
{
    if (!m_sCaption.empty())
        ::SetWindowText(m_hDlg, m_sCaption.c_str());

    if (!m_sMsg.empty())
        ::SetDlgItemText(m_hDlg, IDC_STATIC, m_sMsg.c_str());

    if (!m_sYes.empty())
        SetDlgItemText(m_hDlg, IDYES, m_sYes.c_str());

    if (!m_sNo.empty())
        SetDlgItemText(m_hDlg, IDNO, m_sNo.c_str());

    // Load icon
    HICON l_hIcon = nullptr;
    switch (m_iconType) {
        case IconType::Information:
            l_hIcon = ::LoadIcon(nullptr, IDI_INFORMATION);
            break;
        case IconType::Warning:
            l_hIcon = ::LoadIcon(nullptr, IDI_WARNING);     
            break;
        case IconType::Error:
            l_hIcon = ::LoadIcon(nullptr, IDI_ERROR);       
            break;
        case IconType::Question:
            l_hIcon = ::LoadIcon(nullptr, IDI_QUESTION);    
            break;
        default: 
            break;
    }
    if (l_hIcon) {
        ::SendDlgItemMessage(m_hDlg, IDC_BOX_ICON, STM_SETICON, reinterpret_cast<WPARAM>(l_hIcon), 0);
    }

    AdjustDialogSize();
    switch (m_enCenterType)
    {
    case CenterType::Parent:
        CenterDialogParent();
        break;

    case CenterType::Screen:
        CenterDialogParent();
        break;

    default:
        break;
    }

    SetForegroundWindow(m_hDlg);

    return TRUE;
}


void RCustomMessageBox::AdjustDialogSize() 
{
    HWND l_hWndText = ::GetDlgItem(m_hDlg, IDC_STATIC);
    RECT l_rcText;
    ::GetWindowRect(l_hWndText, &l_rcText);
    ::MapWindowPoints(nullptr, m_hDlg, reinterpret_cast<POINT*>(&l_rcText), 2);

    HDC l_hDC = ::GetDC(l_hWndText);
    HFONT l_hFont = reinterpret_cast<HFONT>(::SendMessage(l_hWndText, WM_GETFONT, 0, 0));
   :: SelectObject(l_hDC, l_hFont);

    RECT l_rcCalc = { 0, 0, 300, 0 }; // max width
    ::DrawText(l_hDC, m_sMsg.c_str(), -1, &l_rcCalc, DT_CALCRECT | DT_WORDBREAK | DT_VCENTER);
    ::ReleaseDC(l_hWndText, l_hDC);

    ::SetWindowPos(l_hWndText, nullptr, l_rcText.left, l_rcText.top, l_rcText.right, l_rcText.bottom, SWP_NOZORDER);
}


void RCustomMessageBox::CenterDialogScreen()
{
    RECT l_rcDlg;
    GetWindowRect(m_hDlg, &l_rcDlg);

    int l_dxWidth = l_rcDlg.right - l_rcDlg.left;
    int l_dyHeight = l_rcDlg.bottom - l_rcDlg.top;

    HMONITOR l_hMonitor = ::MonitorFromWindow(m_hDlg, MONITOR_DEFAULTTONEAREST);
    MONITORINFO l_mi = { sizeof(l_mi) };
    ::GetMonitorInfo(l_hMonitor, &l_mi);

    int l_dxScreen = l_mi.rcWork.right - l_mi.rcWork.left;
    int l_dyScreen = l_mi.rcWork.bottom - l_mi.rcWork.top;

    int l_x = l_mi.rcWork.left + (l_dxScreen - l_dxWidth) / 2;
    int l_y = l_mi.rcWork.top + (l_dyScreen - l_dyHeight) / 2;

    ::SetWindowPos(m_hDlg, nullptr, l_x, l_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


void RCustomMessageBox::CenterDialogParent() 
{
    if (!m_hParent || !IsWindow(m_hParent))
        return;

    RECT l_rcParent;
    RECT l_rcDlg;
    ::GetWindowRect(m_hParent, &l_rcParent);
    ::GetWindowRect(m_hDlg, &l_rcDlg);


    int l_dxDlg = l_rcDlg.right - l_rcDlg.left;
    int l_dyDlg = l_rcDlg.bottom - l_rcDlg.top;

    int l_dxParent = l_rcParent.right - l_rcParent.left;
    int l_dyParent = l_rcParent.bottom - l_rcParent.top;

    int l_xDlg = l_rcParent.left + static_cast<int>((l_dxParent - l_dxDlg) / 2);
    int l_yDlg = l_rcParent.top + static_cast<int>((l_dyParent - l_dyDlg) / 2);

    ::SetWindowPos(m_hDlg, nullptr, l_xDlg, l_yDlg, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}