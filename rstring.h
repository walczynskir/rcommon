#pragma once

// normal and unicode strings
#include <string>
#include <sstream>

#ifdef _UNICODE
	typedef std::wstring    tstring;
#else
	typedef std::string		tstring;
#endif

#ifdef _UNICODE
	typedef std::wstringstream tstringstream;	
#else
	typedef std::stringstream tstringstream;
#endif
