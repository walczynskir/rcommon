#pragma once
#include "rcommon.h"
#include "RException.h"


#ifdef _UNICODE
#define THROW_ROWN_EXC(place) \
    throw ROwnExc((place) + std::wstring(L" [") + std::wstring(__FILEW__) + std::wstring(L":") + std::to_wstring(__LINE__) + std::wstring(L"]"))
#else
#define THROW_ROWN_EXC(place) \
	throw ROwnExc(std::string(place) + std::string(" [") + std::string(__FILE__) + std::string(":") + std::to_string(__LINE__) + std::string("]"))
#endif 



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
