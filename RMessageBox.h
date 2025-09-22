#pragma once
#include <windows.h>
#include <rcommon/rcommon.h>
#include <rcommon/rstring.h>



class RCOMMON_API RCenteredMessageBox
{
public:
    static int Show(HWND hParent, LPCWSTR text, LPCWSTR caption, UINT type);
    static void WarningBox(HWND a_hDlg, LPCTSTR a_sWarningText);
    static BOOL DecisionBox(HWND a_hDlg, LPCTSTR a_sDecisionText);
    static BOOL DecisionBox2(HWND a_hWndParent, LPCTSTR a_sDecisionText);


private:
    static HHOOK s_hHook;
    static HWND s_hParent;

    static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);

};



class RCOMMON_API RCustomMessageBox 
{
public:
    enum class Result {
        Yes = IDYES,
        No = IDNO
    };

    enum class IconType {
        None,
        Information,
        Warning,
        Error,
        Question
    };

    enum class CenterType {
        None,
        Parent,
        Screen
    };

    RCustomMessageBox(HINSTANCE hInstance, HWND hParent) : m_hInstance(hInstance), m_hParent(hParent)
    {
    }

    // Conversion operator to HWND
    operator HWND() const { return m_hDlg; }

    void SetCaption(const tstring& caption) { m_sCaption = caption; }
    void SetMessage(const tstring& message) { m_sMsg = message; }
    void SetYesLabel(const tstring& a_sYes) { m_sYes = a_sYes; }
    void SetNoLabel(const tstring& a_sNo) { m_sNo = a_sNo; }
    void SetIcon(IconType a_iconType) { m_iconType = a_iconType; }
    void SetCenteredParent() { m_enCenterType = CenterType::Parent; }
    void SetScreenCentered() { m_enCenterType = CenterType::Screen; }
    void SetNonCentered() { m_enCenterType = CenterType::None; }

    Result Show();

private:
    static INT_PTR CALLBACK DialogProc(HWND a_hDlg, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);
    BOOL OnInitDialog();
    void AdjustDialogSize();
    void CenterDialogScreen();
    void CenterDialogParent();


    HWND m_hDlg = nullptr;
    HINSTANCE m_hInstance;
    HWND m_hParent;
    tstring m_sCaption{};
    tstring m_sMsg;
    tstring m_sYes;
    tstring m_sNo;
    IconType m_iconType;
    CenterType m_enCenterType = CenterType::None;

};
