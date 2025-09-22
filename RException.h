#pragma once
#include <rcommon/rcommon.h>
#include <rcommon/rstring.h>

class RCOMMON_API RException
{
public:

	RException() {};
	~RException(void) {};

	virtual tstring GetFormattedMsg(void) const { return _T(""); };
};
