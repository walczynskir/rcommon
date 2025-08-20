#pragma once
#include <rcommon/rcommon.h>
#include <rcommon/RGetSocket.h>


class RCOMMON_API RGetHost : public RGetSocket
{
public:
	RGetHost(HWND a_hWnd = NULL, UINT a_iMsg = 0);
	virtual ~RGetHost(void) {};
	void GetHostByName(const char* a_sName);
	void GetHostByAddr(const char* a_sAddr);
	const hostent* GetHost() { return (struct hostent *)GetBuf(); };

};
