#pragma once
#include <rcommon/RException.h>
#include <format>  // C++20 header

class RSystemExc : public RException
{
public:
	RSystemExc() : m_sMsg(_T("")) { SetCode(0); };
	RSystemExc(DWORD a_dwCode) : m_sMsg(_T(""))  { SetCode(a_dwCode); };
	RSystemExc(DWORD a_dwCode, LPCTSTR a_sMsg) : m_sMsg(a_sMsg) { SetCode(a_dwCode); };
	RSystemExc(LPCTSTR a_sMsg) : m_sMsg(a_sMsg) { SetCode(::GetLastError()); };
	virtual ~RSystemExc(void) {};
	RSystemExc& operator = (DWORD a_dwCode ) { m_dwCode = a_dwCode;	return *this; };

	void SetCode(DWORD a_dwCode) { m_dwCode = a_dwCode;	};
	DWORD GetCode() const {	return m_dwCode; };
	operator DWORD()const {	return m_dwCode; };
	void SetModule(const tstring& a_sModule) { m_sModule = a_sModule; };

	virtual tstring GetFormattedMsg(void)  const
	{
		DWORD	l_dwFmtRt		= 0;
		DWORD	l_dwFlags		= FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;
		LPVOID	l_lpMsgBuf		= NULL;
		HMODULE l_hLookupMod	= NULL;
		tstring	l_sMsg			= _T("");

		if(!m_sModule.empty())
		{
			l_hLookupMod = ::LoadLibraryEx(m_sModule.data(), NULL, LOAD_LIBRARY_AS_DATAFILE);
			if(l_hLookupMod != NULL) 
			{
				l_dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
			}				
		}

		l_dwFmtRt = ::FormatMessage(l_dwFlags, (LPCVOID)l_hLookupMod,
			m_dwCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPTSTR>(&l_lpMsgBuf), 0,	NULL);
		
		if (l_dwFmtRt > 0) 
		{	
			l_sMsg = static_cast<TCHAR*>(l_lpMsgBuf);
		}
		if (l_lpMsgBuf != NULL) 
		{
			::LocalFree(l_lpMsgBuf);
		}
		if (l_hLookupMod != NULL) 
		{
			::FreeLibrary(l_hLookupMod);
		}
		return std::format(_T("{} - {}: {}"), m_sMsg, m_dwCode, l_sMsg);
	};

private:
	DWORD m_dwCode;
	tstring m_sModule;
	tstring m_sMsg;
};
