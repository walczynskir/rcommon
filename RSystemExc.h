#pragma once
#include <rcommon/RException.h>
#include <strsafe.h>


#ifdef _UNICODE
#define THROW_RSYSTEM_EXC(place) \
{ \
    TCHAR l_sBuf[1024]; \
    StringCchPrintf(l_sBuf, 1024, L"%s [%s:%d]", place, __FILEW__, __LINE__); \
    throw RSystemExc(l_sBuf); \
}
#else
#define THROW_RSYSTEM_EXC(place) \
{ \
    char l_sBuf[1024]; \
    StringCchPrintfA(l_sBuf, 1024, "%s [%s:%d]", place, __FILE__, __LINE__); \
    throw RSystemExc(l_sBuf); \
}
#endif


class RCOMMON_API RSystemExc : public RException
{
public:
    RSystemExc() { Init(_T(""), 0); SetFormattedMsg();}

    RSystemExc(DWORD a_dwCode) { Init(_T(""), a_dwCode); SetFormattedMsg();}

    RSystemExc(DWORD a_dwCode, LPCTSTR a_sMsg) { Init(a_sMsg, a_dwCode); SetFormattedMsg();}

    RSystemExc(LPCTSTR a_sMsg) { Init(a_sMsg, ::GetLastError()); SetFormattedMsg();  }

    virtual ~RSystemExc() {}

/*    RSystemExc& operator=(DWORD a_dwCode) {
        m_dwCode = a_dwCode;
        return *this;
    }
    */
    void SetCode(DWORD a_dwCode) { m_dwCode = a_dwCode; SetFormattedMsg();}

    DWORD GetCode() const { return m_dwCode; }

    operator DWORD() const { return m_dwCode; }

    void SetModule(LPCTSTR a_sModule) {
        StringCchCopy(m_sModule, MAX_PATH, a_sModule);
        SetFormattedMsg();
    }

private:
    void SetFormattedMsg()  {
        TCHAR l_sSysMsg[1024] = _T("");
        LPTSTR l_lpMsgBuf = nullptr;
        HMODULE l_hLookupMod = nullptr;
        DWORD l_dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;

        if (m_sModule[0] != _T('\0')) {
            l_hLookupMod = ::LoadLibraryEx(m_sModule, NULL, LOAD_LIBRARY_AS_DATAFILE);
            if (l_hLookupMod != NULL) {
                l_dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
            }
        }

        DWORD l_dwFmtRt = ::FormatMessage(
            l_dwFlags,
            (LPCVOID)l_hLookupMod,
            m_dwCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPTSTR>(&l_lpMsgBuf),
            0,
            NULL
        );

        if (l_dwFmtRt > 0 && l_lpMsgBuf != nullptr) {
            StringCchCopy(l_sSysMsg, ArraySize(l_sSysMsg), l_lpMsgBuf);
            ::LocalFree(l_lpMsgBuf);
        }

        if (l_hLookupMod != NULL) {
            ::FreeLibrary(l_hLookupMod);
        }

        TCHAR l_sBuf[1024];
        StringCchCopy(l_sBuf, ArraySize(l_sBuf), m_sMsg);
        StringCchPrintf(m_sMsg, ArraySize(m_sMsg), _T("%s - %lu: %s"), l_sBuf, m_dwCode, l_sSysMsg);
    }

    void Init(LPCTSTR a_sMsg, DWORD a_dwCode) {
        StringCchCopy(m_sMsg, 1024, a_sMsg);
        m_dwCode = a_dwCode;
        m_sModule[0] = _T('\0');
        SetFormattedMsg();
    }

    DWORD m_dwCode;
    TCHAR m_sModule[MAX_PATH];
};
