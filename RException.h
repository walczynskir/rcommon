#pragma once
#include <rcommon/rcommon.h>

class RCOMMON_API RException
{
public:

	RException() {};
	~RException(void) {};

	virtual LPCTSTR GetFormattedMsg() const {
		return m_sMsg;
	};

protected:
	TCHAR m_sMsg[1024]{};

};
