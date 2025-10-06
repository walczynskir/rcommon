#pragma once

#include <rcommon/RMessageBox.h>
#include <rcommon/RSystemExc.h>
#include <rcommon/ROwnExc.h>


template <LRESULT(*WndProc)(HWND, UINT, WPARAM, LPARAM)>
LRESULT CALLBACK SafeWndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam) noexcept
{
    try {
        return WndProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
    }
    catch (const RSystemExc& l_exc) {
        ExceptionMessageBox(a_hWnd, l_exc.GetFormattedMsg().c_str());
        return 0;
    }
    catch (const ROwnExc& l_exc) {
        ExceptionMessageBox(a_hWnd, l_exc.GetFormattedMsg().c_str());
        return 0;
    }
    catch (...) {
        ExceptionMessageBox(a_hWnd, _T("Unknown exception"));
        return 0;
    }
}


template <INT_PTR(*DialogProc)(HWND, UINT, WPARAM, LPARAM)>
INT_PTR CALLBACK SafeDialogProc(HWND a_hDlg, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam) noexcept
{
    try {
        return DialogProc(a_hDlg, a_iMsg, a_wParam, a_lParam);
    }
    catch (const RSystemExc& l_exc) {
        ExceptionMessageBox(a_hDlg, l_exc.GetFormattedMsg().c_str());
        return FALSE; // abort safely
    }
    catch (const ROwnExc& l_exc) {
        ExceptionMessageBox(a_hDlg, l_exc.GetFormattedMsg().c_str());
        return FALSE;
    }
    catch (...) {
        ExceptionMessageBox(a_hDlg, _T("Unknown exception"));
        return FALSE;
    }
}
