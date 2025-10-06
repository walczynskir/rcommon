#pragma once
#include <rcommon/rcommon.h>
#include <rcommon/rstring.h>



extern RCOMMON_API void ExceptionMessageBox(HWND a_hWnd, LPCTSTR a_sMessage);
extern RCOMMON_API BOOL DecisionMessageBox(HWND a_hWndParent, LPCTSTR a_sDecisionText);

class RCOMMON_API RCustomMessageBox
{
public:
    enum class Result {
        Yes = IDYES,
        No = IDNO,
        Cancel = IDCANCEL
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

    enum class ButtonIdx {
        First,      // returns IDYES
        Second,     // return IDNO
        Third       // returns IDCANCEL
    };

    enum class ButtonsCount {
        One,
        Two,
        Three
    };

    RCustomMessageBox(HINSTANCE a_hInst, HWND a_hParent);
    ~RCustomMessageBox(); // in header

    // delete copy and move constructors
    RCustomMessageBox(const RCustomMessageBox&) = delete;
    RCustomMessageBox& operator=(const RCustomMessageBox&) = delete;
    RCustomMessageBox(RCustomMessageBox&&) = delete;
    RCustomMessageBox& operator=(RCustomMessageBox&&) = delete;

    // Conversion operator to HWND
    operator HWND() const { return m_hDlg; }

    void SetCaption(LPCTSTR a_sCaption);
    void SetMessage(LPCTSTR a_sMsg);

    void SetIcon(IconType a_iconType) { m_iconType = a_iconType; }
    void SetCenteredParent() { m_enCenterType = CenterType::Parent; }
    void SetScreenCentered() { m_enCenterType = CenterType::Screen; }
    void SetNonCentered() { m_enCenterType = CenterType::None; }
    void SetButtonsCount(ButtonsCount a_count);
    void SetButtonText(ButtonIdx a_idx, LPCTSTR a_sText);
    void SetButtonDefault(ButtonIdx a_idx) ;
    void SetButtonGap(LONG a_dxGap) {a_dxGap = a_dxGap;};

    Result Show();



private:
    static INT_PTR CALLBACK DialogProc(HWND a_hDlg, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);
    BOOL OnInitDialog();
    void AdjustButtons();
    void SetDefaultButton();

    void AdjustDialogSize();
    void CenterDialogScreen();
    void CenterDialogParent();
    HICON LoadIcon();

private:
    class Impl; // Forward declaration (to keep tstrings out of the class for dll ABI compatibility)
    Impl* m_pImpl;

    HWND m_hDlg = nullptr;
    HINSTANCE m_hInstance;
    HWND m_hParent;
    IconType m_iconType = IconType::None;
    CenterType m_enCenterType = CenterType::None;
    ButtonsCount m_count = ButtonsCount::One;
    ButtonIdx m_idxDefault = ButtonIdx::First;
    LONG m_dxGap = 20;
};
