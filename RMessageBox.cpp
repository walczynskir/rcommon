#include "stdafx.h"
#include "resource.h"
#include "RMessageBox.h"
#include <map>


class RCustomMessageBox::Impl 
{
public:
    tstring m_sCaption;
    tstring m_sMsg;
    std::map<ButtonIdx, tstring> m_mapButtons;
    };


RCustomMessageBox::RCustomMessageBox(HINSTANCE a_hInst, HWND a_hParent) : m_hInstance(a_hInst), m_hParent(a_hParent), m_pImpl(new Impl())
{
}

// in .cpp
RCustomMessageBox::~RCustomMessageBox() 
{
    delete m_pImpl;
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
        case IDCANCEL:
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
    if (!m_pImpl->m_sCaption.empty())
        ::SetWindowText(m_hDlg, m_pImpl->m_sCaption.c_str());

    if (!m_pImpl->m_sMsg.empty())
        ::SetDlgItemText(m_hDlg, IDC_STATIC, m_pImpl->m_sMsg.c_str());


    auto SetButtonText = [&](ButtonIdx a_idx, int a_idCtrl)
        {
            auto l_it = m_pImpl->m_mapButtons.find(a_idx);
            if (l_it != m_pImpl->m_mapButtons.end() && !l_it->second.empty())
            {
                ::SetDlgItemText(m_hDlg, a_idCtrl, l_it->second.c_str());
            }
        };

    SetButtonText(ButtonIdx::First, IDYES);
    SetButtonText(ButtonIdx::Second, IDNO);
    SetButtonText(ButtonIdx::Third, IDCANCEL);    
      
    HICON l_hIcon = LoadIcon();

    if (l_hIcon) {
        ::SendDlgItemMessage(m_hDlg, IDC_BOX_ICON, STM_SETICON, reinterpret_cast<WPARAM>(l_hIcon), 0);
    }

    //AdjustDialogSize();
    AdjustButtons();
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


HICON RCustomMessageBox::LoadIcon()
{
    int l_dpi = GetDpiForWindow(m_hDlg);
    int l_size = MulDiv(32, l_dpi, 96); // scale 32px to current DPI

    // lambda for mapping
    auto IconTypeToId = [](IconType a_idx) {
        switch (a_idx) {
        case IconType::Information:  return IDI_INFORMATION;
        case IconType::Warning: return IDI_WARNING;
        case IconType::Error:  return IDI_ERROR;
        case IconType::Question:  return IDI_QUESTION;
        }
        return MAKEINTRESOURCE(0); // fallback
        };

    return static_cast<HICON>(::LoadImage(
        nullptr,
        IconTypeToId(m_iconType),
        IMAGE_ICON,
        l_size, l_size,
        LR_SHARED));
}


void RCustomMessageBox::SetCaption(LPCTSTR a_sCaption)
{
    m_pImpl->m_sCaption = a_sCaption;
}


void RCustomMessageBox::SetMessage(LPCTSTR a_sMsg)
{
    m_pImpl->m_sMsg = a_sMsg;
}

void RCustomMessageBox::SetButtonsCount(ButtonsCount a_count)
{
    m_count = a_count;
}

void RCustomMessageBox::SetButtonText(ButtonIdx a_idx, LPCTSTR a_sText)
{
    m_pImpl->m_mapButtons[a_idx] = a_sText;
}


void RCustomMessageBox::SetButtonDefault(ButtonIdx a_idx) 
{
    m_idxDefault = a_idx;
}


void RCustomMessageBox::AdjustButtons()
{
    RECT l_rectDlg{};
    ::GetWindowRect(m_hDlg, &l_rectDlg);

    LONG l_dxHalf = RectWidth(l_rectDlg) / 2;

    HWND l_hWndYes = ::GetDlgItem(m_hDlg, IDYES);
    HWND l_hWndNo = ::GetDlgItem(m_hDlg, IDNO);
    HWND l_hWndCancel = ::GetDlgItem(m_hDlg, IDCANCEL);
    RECT l_rectButton{};
    ::GetWindowRect(l_hWndYes, &l_rectButton);

    ::MapWindowPoints(HWND_DESKTOP, m_hDlg, reinterpret_cast<LPPOINT>(&l_rectButton), 2);
    LONG l_dxButton = RectWidth(l_rectButton);
    LONG l_yButton = l_rectButton.top;

    switch (m_count)
    {
    case ButtonsCount::One: // only IDYES button in the centre
        ::SetWindowPos(l_hWndYes, nullptr, l_dxHalf - l_dxButton / 2, l_yButton, 0, 0, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOSIZE);

        ::ShowWindow(l_hWndNo, FALSE);
        ::ShowWindow(l_hWndCancel, FALSE);
        break;
    case ButtonsCount::Two: // IDYES and IDNO buttons 
        ::SetWindowPos(l_hWndYes, nullptr, l_dxHalf - m_dxGap / 2 - l_dxButton, l_yButton, 0, 0, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOSIZE);
        ::SetWindowPos(l_hWndNo, nullptr, l_dxHalf + m_dxGap / 2, l_yButton, 0, 0, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOSIZE);

        ::ShowWindow(l_hWndCancel, FALSE);
        break;
    case ButtonsCount::Three: // IDYES and IDNO buttons 
        ::SetWindowPos(l_hWndYes, nullptr, l_dxHalf - l_dxButton / 2 - m_dxGap - l_dxButton, l_yButton, 0, 0, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOSIZE);
        ::SetWindowPos(l_hWndNo, nullptr, l_dxHalf - l_dxButton / 2, l_yButton, 0, 0, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOSIZE);
        ::SetWindowPos(l_hWndCancel, nullptr, l_dxHalf + l_dxButton / 2 + m_dxGap, l_yButton, 0, 0, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOSIZE);
        break;
    default:
        break;// do nothing
    }

}

void RCustomMessageBox::SetDefaultButton()
{
    // Remove default style from old button
    SendDlgItemMessage(m_hDlg, IDYES, BM_SETSTYLE, BS_PUSHBUTTON, TRUE);

    // lambda for mapping
    auto IdxToCtrlId = [](ButtonIdx a_idx) {
        switch (a_idx) {
        case ButtonIdx::First:  return IDYES;
        case ButtonIdx::Second: return IDNO;
        case ButtonIdx::Third:  return IDCANCEL;
        }
        return 0; // fallback
        };

    // Add default style to new button
    SendDlgItemMessage(m_hDlg, IdxToCtrlId(m_idxDefault), BM_SETSTYLE, BS_DEFPUSHBUTTON, TRUE);

    // Tell dialog manager which is default
    SendMessage(m_hDlg, DM_SETDEFID, IdxToCtrlId(m_idxDefault), 0);
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
    ::DrawText(l_hDC, m_pImpl->m_sMsg.c_str(), -1, &l_rcCalc, DT_CALCRECT | DT_WORDBREAK | DT_VCENTER | DT_END_ELLIPSIS);
    ::ReleaseDC(l_hWndText, l_hDC);

    ::SetWindowPos(l_hWndText, nullptr, l_rcText.left, l_rcText.top, RectWidth(l_rcCalc) - 150, RectHeight(l_rcCalc), SWP_NOZORDER);
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


void ExceptionMessageBox(HWND a_hWnd, LPCTSTR a_sMessage)
{
    RCustomMessageBox l_msgBox(RCommon_GetInstance(), a_hWnd);
    TCHAR l_sCaption[128];
    ::LoadString(RCommon_GetInstance(), IDS_BOX_EXCEPTIONCAPTION, l_sCaption, ArraySize(l_sCaption));

    l_msgBox.SetCaption(l_sCaption);
    l_msgBox.SetCenteredParent();

    TCHAR l_sMessageDefault[256];
    ::LoadString(RCommon_GetInstance(), IDS_BOX_EXCEPTIONMESSAGE, l_sMessageDefault, ArraySize(l_sMessageDefault));

    tstring l_sWholeMessage = a_sMessage;
    if (_tcslen(a_sMessage) == 0)
        l_sWholeMessage = l_sMessageDefault;
    else
        l_sWholeMessage = a_sMessage + tstring(_T("\n")) + l_sMessageDefault;

    TCHAR l_sButtonText[64];
    ::LoadString(RCommon_GetInstance(), IDS_BOX_BUTTONOK, l_sButtonText, ArraySize(l_sButtonText));

    l_msgBox.SetMessage(l_sWholeMessage.c_str());
    l_msgBox.SetIcon(RCustomMessageBox::IconType::Error);
    l_msgBox.SetButtonsCount(RCustomMessageBox::ButtonsCount::One);
    l_msgBox.SetButtonText(RCustomMessageBox::ButtonIdx::First, l_sButtonText);
    l_msgBox.Show();
}

BOOL DecisionMessageBox(HWND a_hWndParent, LPCTSTR a_sDecisionText)
{
    TCHAR l_sDecisionTitle[256];
    ::LoadString(RCommon_GetInstance(), IDS_BOX_DECISION, l_sDecisionTitle, ArraySize(l_sDecisionTitle));

    RCustomMessageBox l_msgBox(RCommon_GetInstance(), a_hWndParent);
    l_msgBox.SetCaption(l_sDecisionTitle);
    l_msgBox.SetMessage(a_sDecisionText);
    l_msgBox.SetIcon(RCustomMessageBox::IconType::Question);
    l_msgBox.SetCenteredParent();
    l_msgBox.SetButtonsCount(RCustomMessageBox::ButtonsCount::Two);

    return (l_msgBox.Show() == RCustomMessageBox::Result::Yes);
}


