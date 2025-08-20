#include "stdafx.h"
#include "RBaseSocket.h"




LRESULT
RBaseSocket::SendReceiveMessage(
	WPARAM a_wParam,
	LPARAM a_lParam
	)
{
	if (m_hWndReceive != NULL)
	{
		return ::SendMessage(m_hWndReceive, m_iMsgReceive, a_wParam, a_lParam);
	}
	return 0;
}


void
RBaseSocket::PostReceiveMessage(
	WPARAM a_wParam,
	LPARAM a_lParam
	)
{
	if (m_hWndReceive != NULL)
	{
		::PostMessage(m_hWndReceive, m_iMsgReceive, a_wParam, a_lParam);
	}
}


LRESULT
RBaseSocket::SendErrorMessage(
	int a_iErr
	)
{
	ASSERT(a_iErr != 0);
	return SendReceiveMessage((WPARAM)SocketError, (LPARAM)a_iErr);
}


