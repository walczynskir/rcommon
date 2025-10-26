#pragma once
#include "rcommon.h"
#include "RException.h"
#include <strsafe.h>


#ifdef _UNICODE
#define THROW_ROWN_EXC(place) \
{ \
    TCHAR l_sBuf[1024]; \
    StringCchPrintf(l_sBuf, ArraySize(l_sBuf), L"%s [%s:%d]", place, __FILEW__, __LINE__); \
    throw ROwnExc(l_sBuf); \
}
#else
#define THROW_ROWN_EXC(place) \
{ \
    char l_sBuf[1024]; \
    StringCchPrintfA(l_sBuf, ArraySize(l_sBuf), "%s [%s:%d]", place, __FILE__, __LINE__); \
    throw ROwnExc(l_sBuf); \
}
#endif



class RCOMMON_API ROwnExc : public RException
{
public:
    ROwnExc(LPCTSTR a_sMsg)
    {
        StringCchCopy(m_sMsg, ArraySize(m_sMsg), a_sMsg);
    }

    ROwnExc(HINSTANCE a_hInst, UINT a_idStr)
    {
        ::LoadString(a_hInst, a_idStr, m_sMsg, ArraySize(m_sMsg));
    }


};
