// FloatingHelper.h
#pragma once
#include <rcommon/rcommon.h>


#ifdef __cplusplus
extern "C" {
#endif

extern RCOMMON_API double StringToDouble(LPCTSTR a_sNumber);
extern RCOMMON_API BOOL StrToDateTime(LPSYSTEMTIME a_pDateTime, const TCHAR* a_psDate);
extern RCOMMON_API LPTSTR DateTimeYearToMinToStr(const SYSTEMTIME& a_date, TCHAR* a_sDate, UINT a_iSize);

#ifdef __cplusplus
} // extern "C"
#endif
