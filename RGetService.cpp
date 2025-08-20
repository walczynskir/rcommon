#include "stdafx.h"
#include "RGetService.h"
#include "RSystemExc.h"

#define WM_GETSERVICE (WM_USER + 1)


RGetService::RGetService(
	HWND a_hWnd, 
	UINT a_iMsg
	) : RGetSocket(WM_GETSERVICE, a_hWnd, a_iMsg)
{
}

void 
RGetService::GetServiceByName(
	const char* a_sName, 
	const char* a_sProtocol
	)
{
	ASSERT(GetTask() == NULL);
	if (GetWnd() == NULL)
	{
		Create();
	}
	SetTask(::WSAAsyncGetServByName(
		GetWnd(), WM_GETSERVICE, a_sName, a_sProtocol,
		GetBuf(), GetBufSize()));
}


void 
RGetService::GetServiceByPort(
	int a_iPort, 
	const char* a_sProtocol
	)
{
	if (GetWnd() == NULL)
	{
		Create();
	}
	ASSERT(GetTask() == NULL);
	SetTask(::WSAAsyncGetServByPort(
		GetWnd(), WM_GETSERVICE, a_iPort, a_sProtocol,
		GetBuf(), GetBufSize()));
}


