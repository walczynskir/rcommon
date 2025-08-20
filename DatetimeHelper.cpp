#include "stdafx.h"
#include "DatetimeHelper.h"


LONG CompareSystemTime(const SYSTEMTIME* a_pStime1, const SYSTEMTIME* a_pStime2)
{
	FILETIME l_ftime1;
	FILETIME l_ftime2;

	SystemTimeToFileTime(a_pStime1, &l_ftime1);
	SystemTimeToFileTime(a_pStime2, &l_ftime2);
	return CompareFileTime(&l_ftime1, &l_ftime2);
}
