#pragma once
#include "rcommon.h"
#include "RGetSocket.h"

class RCOMMON_API RGetService : public RGetSocket
{
public:
	RGetService(HWND a_hWnd = NULL, UINT a_iMsg = 0);
	virtual ~RGetService(void) {};
	void GetServiceByName(const char* a_sName, const char* a_sProtocol);
	void GetServiceByPort(int a_iPort, const char* a_sProtocol);
	const servent* GetService() { return (servent*)GetBuf(); };
};
