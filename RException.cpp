// RException.cpp: implementation of the RException class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RException.h"


#ifndef _WIN32_WCE
std::ofstream* RException::s_pOfStream = NULL;
tstring		   RException::s_sLogFile = _T("");

void RException::LogException(const tstring& a_sFile, int a_iLine) const
{
	if (s_pOfStream == NULL)
	{
		return;
	}
	*s_pOfStream << a_sFile << _T(":") << a_iLine;
	s_pOfStream->flush();
}


void RException::StartLog()
{
	s_pOfStream = new std::ofstream();
	if (s_pOfStream != NULL)
	{
		TCHAR l_sTempDir[MAX_PATH];
		::GetTempPath(ArraySize(l_sTempDir), l_sTempDir);
		s_sLogFile = tstring(l_sTempDir) + tstring(_T("\\"));
		s_sLogFile = s_sLogFile + tstring(_T("rcomm.log"));
		s_pOfStream->open(s_sLogFile.c_str(), std::ios_base::out);
	}
}


void RException::EndLog()
{
	if (s_pOfStream != NULL)
	{
		delete s_pOfStream;
		s_pOfStream = NULL;
		::DeleteFile(s_sLogFile.c_str());
	}
}

#endif // _WIN32_WCE
