#include "stdafx.h"
#include "conversion.h"
#include <stdio.h>

inline static BOOL StrToSystemTimeStruct(LPSYSTEMTIME a_pDateTime, const TCHAR* a_psDate);

double StringToDouble(LPCTSTR a_sNumber)
{
	TCHAR l_sNumber[128];
	_tcscpy_s(l_sNumber, ArraySize(l_sNumber), a_sNumber);
	LPTSTR l_psComa = _tcschr(l_sNumber, _T(','));
	if (l_psComa != NULL)
	{
		*l_psComa = _T('.');
	}
	return _tstof(l_sNumber);

}


// from "yyyy-mm-dd hh:mm:ss"
BOOL StrToDateTime(LPSYSTEMTIME a_pDateTime, const TCHAR* a_psDate)
{
	if (!StrToSystemTimeStruct(a_pDateTime, a_psDate))
	{
		return FALSE;
	}
	// only to fulfill wDayOfWeek member
	FILETIME l_ft;
	if (!::SystemTimeToFileTime(a_pDateTime, &l_ft))
	{
		return FALSE;
	}
	return ::FileTimeToSystemTime(&l_ft, a_pDateTime);
}


// from "yyyy-mm-dd hh:mm:ss"
BOOL StrToSystemTimeStruct(LPSYSTEMTIME a_pDateTime, const TCHAR* a_psDate)
{
	a_pDateTime->wHour = 0;
	a_pDateTime->wMinute = 0;
	a_pDateTime->wSecond = 0;
	a_pDateTime->wMilliseconds = 0;
	a_pDateTime->wDayOfWeek = 1;

	TCHAR l_sDate[64];
	LPTSTR l_psPtr = l_sDate;
	LPTSTR l_psEnd = NULL;
	_tcscpy_s(l_sDate, ArraySize(l_sDate), a_psDate);

	// year
	l_psEnd = _tcschr(l_psPtr, _T('-'));
	if (l_psEnd == NULL)
	{
		return FALSE;
	}

	*l_psEnd = _T('\0');
	a_pDateTime->wYear = static_cast<WORD>(_tstoi(l_psPtr));
	if (a_pDateTime->wYear <= 0)
	{
		return FALSE;
	}

	// month
	l_psPtr = l_psEnd + 1;
	if (*l_psPtr == _T('\0'))
	{
		return FALSE;
	}
	l_psEnd = _tcschr(l_psPtr, _T('-'));
	if (l_psEnd == NULL)
	{
		return FALSE;
	}
	*l_psEnd = _T('\0');
	a_pDateTime->wMonth = static_cast<WORD>(_tstoi(l_psPtr));
	if ((a_pDateTime->wMonth < 1) || (a_pDateTime->wMonth > 12))
	{
		return FALSE;
	}

	// day
	l_psPtr = l_psEnd + 1;
	if (*l_psPtr == _T('\0'))
	{
		return FALSE;
	}
	l_psEnd = _tcschr(l_psPtr, _T(' '));
	if (l_psEnd == NULL)
	{
		a_pDateTime->wDay = static_cast<WORD>(_tstoi(l_psPtr));
		if ((a_pDateTime->wDay < 1) || (a_pDateTime->wDay > 31))
		{
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		*l_psEnd = _T('\0');
		a_pDateTime->wDay = static_cast<WORD>(_tstoi(l_psPtr));
		if ((a_pDateTime->wDay < 1) || (a_pDateTime->wDay > 31))
		{
			return FALSE;
		}
	}

	// hour
	l_psPtr = l_psEnd + 1;
	if (*l_psPtr == _T('\0'))
	{
		return TRUE;
	}
	l_psEnd = _tcschr(l_psPtr, _T(':'));
	if (l_psEnd == NULL)
	{
		a_pDateTime->wHour = static_cast<WORD>(_tstoi(l_psPtr));
		if ((a_pDateTime->wHour < 1) || (a_pDateTime->wHour > 24))
		{
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		*l_psEnd = _T('\0');
		a_pDateTime->wHour = static_cast<WORD>(_tstoi(l_psPtr));
		if ((a_pDateTime->wHour < 1) || (a_pDateTime->wHour > 24))
		{
			return FALSE;
		}
	}

	// minute
	l_psPtr = l_psEnd + 1;
	if (*l_psPtr == _T('\0'))
	{
		return TRUE;
	}
	l_psEnd = _tcschr(l_psPtr, _T(':'));
	if (l_psEnd == NULL)
	{
		a_pDateTime->wMinute = static_cast<WORD>(_tstoi(l_psPtr));
		if ((a_pDateTime->wMinute < 0) || (a_pDateTime->wMinute > 59))
		{
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		*l_psEnd = _T('\0');
		a_pDateTime->wMinute = static_cast<WORD>(_tstoi(l_psPtr));
		if ((a_pDateTime->wMinute < 1) || (a_pDateTime->wMinute > 59))
		{
			return FALSE;
		}
	}

	// second
	l_psPtr = l_psEnd + 1;
	if (*l_psPtr == _T('\0'))
	{
		return TRUE;
	}
	l_psEnd = _tcschr(l_psPtr, _T(':'));
	if (l_psEnd == NULL)
	{
		a_pDateTime->wSecond = static_cast<WORD>(_tstoi(l_psPtr));
		if ((a_pDateTime->wSecond < 0) || (a_pDateTime->wSecond > 59))
		{
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		*l_psEnd = _T('\0');
		a_pDateTime->wSecond = static_cast<WORD>(_tstoi(l_psPtr));
		if ((a_pDateTime->wSecond < 1) || (a_pDateTime->wSecond > 59))
		{
			return FALSE;
		}
	}
	return TRUE;
}


LPTSTR DateTimeYearToMinToStr(const SYSTEMTIME& a_date, LPTSTR a_sDate, UINT a_iSize)
{
	_stprintf_s(a_sDate, a_iSize, _T("%02d-%02d-%02d %02d:%02d"), 
		a_date.wYear, a_date.wMonth, a_date.wDay, 
		a_date.wHour, a_date.wMinute);
	return a_sDate;
}