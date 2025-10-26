#pragma once
#include "rcommon.h"
#include "RException.h"

class RCOMMON_API RNotEnoughMemExc : public RException
{
public:

	RNotEnoughMemExc()
	{
	}

	virtual ~RNotEnoughMemExc(void)
	{
	}

	virtual LPCTSTR GetFormattedMsg(void) { return _T("Not enough memory"); };

};
