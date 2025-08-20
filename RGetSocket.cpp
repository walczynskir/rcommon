#include "stdafx.h"
#include "RGetSocket.h"
#include "RSystemExc.h"


RGetSocket::~RGetSocket(void)
{
	try
	{
		Cancel();
	}
	catch (RSystemExc&)
	{
	}
}


void 
RGetSocket::Cancel(void)
{
	if (m_hTask == NULL)
	{
		return;
	}

	if (::WSACancelAsyncRequest(m_hTask) == SOCKET_ERROR)
	{
		DWORD l_dwErr = ::WSAGetLastError();
		if (l_dwErr != WSAEALREADY)
		{
			throw RSystemExc(l_dwErr);
		}
	}
}


LRESULT 
RGetSocket::OnWndMsg(
	UINT a_iMsg, 
	WPARAM a_wParam, 
	LPARAM a_lParam, 
	BOOL* a_pbDefProc
	)
{
	*a_pbDefProc = TRUE;
	if (a_iMsg != m_iSocketGetMsg)
	{
		return RBaseSocket::OnWndMsg(a_iMsg, a_wParam, a_lParam, a_pbDefProc);
	}

	return OnSocketGet((HANDLE)a_wParam, HIWORD(a_lParam));
}


LRESULT 
RGetSocket::OnSocketGet(
	HANDLE a_hTask, 
	int a_iErr
	)
{
	ASSERT(m_hTask == a_hTask);
	UNUSED(a_hTask);
	m_hTask = NULL;
	PostReceiveMessage(0, (LPARAM)a_iErr);
	return 1;
}


void 
RGetSocket::SetTask(
	HANDLE a_hTask
	)
{
#pragma warning(disable: 4311)
	if ((int)a_hTask == SOCKET_ERROR)
#pragma warning(default: 4311)
	{
		throw RSystemExc(::WSAGetLastError());
	}
	m_hTask = a_hTask;
}
