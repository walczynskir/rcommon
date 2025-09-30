#pragma once

// normal and unicode strings
#include <string>
#include <sstream>
#include <format>
#include <type_traits>


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



#ifdef _UNICODE
#define FormatTextT(fmt, ...) std::format(fmt, __VA_ARGS__)
#else
#define FormatTextT(fmt, ...) std::format(fmt, __VA_ARGS__)
#endif