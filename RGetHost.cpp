#include "stdafx.h"
#include "RGetHost.h"
#include "RSystemExc.h"

#define WM_GETHOST (WM_USER + 1)

RGetHost::RGetHost(
	HWND a_hWnd, 
	UINT a_iMsg
	) : RGetSocket(WM_GETHOST, a_hWnd, a_iMsg) 
{
}


void 
RGetHost::GetHostByName(
	const char* a_sName
	)
{
	ASSERT(GetTask() == NULL);
	if (GetWnd() == NULL)
	{
		Create();
	}

  	// treat this as a host name and get it resolved.
	SetTask(::WSAAsyncGetHostByName(GetWnd(), WM_GETHOST, a_sName, 
   		GetBuf(), MAXGETHOSTSTRUCT));
}


void
RGetHost::GetHostByAddr(
	const char* a_sAddr
	)
{
	ASSERT(GetTask() == NULL);
	if (GetWnd() == NULL)
	{
		Create();
	}

  	// treat this as a host name and get it resolved.
	SetTask(::WSAAsyncGetHostByAddr(GetWnd(), WM_GETHOST, a_sAddr, (int)strlen(a_sAddr), AF_INET,
   		GetBuf(), MAXGETHOSTSTRUCT));
}
