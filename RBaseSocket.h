#pragma once
#include <rcommon/rcommon.h>
#include <rcommon/RRawWnd.h>

enum SocketReceiveCmd { SocketStatusChanged = -100, NewSocketAccepted = -101, SocketError = -102 };


class RCOMMON_API RBaseSocket :	public RRawWnd
{
public:
	RBaseSocket(HWND a_hWnd = NULL, UINT a_iMsg = 0) : m_hWndReceive(a_hWnd), m_iMsgReceive(a_iMsg) {};
	virtual ~RBaseSocket(void) {};

	void SetMsgReceive(UINT a_iMsg) { m_iMsgReceive = a_iMsg; };
	void SetWndReceive(HWND a_hWnd) { m_hWndReceive = a_hWnd; };

protected:
	bool IsAsync() const { return (m_hWndReceive != NULL); };
	UINT GetMsgReceive() const { return m_iMsgReceive; };
	HWND GetWndReceive() const { return m_hWndReceive; };
	virtual BOOL SetReceiveTarget(HWND a_hWnd, UINT a_iMsg) { m_hWndReceive = a_hWnd; m_iMsgReceive = a_iMsg; return TRUE; };

	void PostReceiveMessage(WPARAM a_wParam, LPARAM a_lParam);
	LRESULT SendReceiveMessage(WPARAM a_wParam, LPARAM a_lParam);
	LRESULT SendErrorMessage(int a_iErr);

private:
	UINT m_iMsgReceive;
	HWND m_hWndReceive;
};
