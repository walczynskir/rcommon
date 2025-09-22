#pragma once
#include "rcommon.h"
#include "RException.h"

#pragma warning(disable: 4251)
class RCOMMON_API ROwnExc :	public RException
{
public:

	ROwnExc(const tstring& a_sMsg) : m_sMsg(a_sMsg)	{}
	ROwnExc(HINSTANCE a_hInst, UINT a_idStr) 
	{
		TCHAR l_sBuf[1024]; // hope it's big enough
		::LoadString(a_hInst, a_idStr, l_sBuf, ArraySize(l_sBuf));
		m_sMsg = l_sBuf;
	}

	~ROwnExc(void) {}

	virtual tstring GetFormattedMsg() const { return m_sMsg; }

private:
	tstring m_sMsg;
};
#pragma warning(default: 4251)
