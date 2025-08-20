#pragma once
#include "rcommon.h"
#include <winsock.h>
#include "rbasesocket.h"


class RCOMMON_API RGetSocket : public RBaseSocket
{
public:
	RGetSocket(UINT a_iSocketGetMsg, HWND a_hWnd = NULL, UINT a_iMsg = 0) : RBaseSocket(a_hWnd, a_iMsg), m_hTask(NULL), m_iSocketGetMsg(a_iSocketGetMsg) {};
	virtual ~RGetSocket(void);
protected:
	void Cancel(void);

	virtual LRESULT OnWndMsg(UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL* a_pbDefProc);
	virtual LRESULT OnSocketGet(HANDLE a_hTask, int a_iErr);
	HANDLE  GetTask() const { return m_hTask; };
	void SetTask(HANDLE a_hTask);
	char* GetBuf() { return m_sBuf; };
	UINT GetBufSize() const { return sizeof(m_sBuf) / sizeof(m_sBuf[0]); };

private:
	char m_sBuf[MAXGETHOSTSTRUCT];
	HANDLE m_hTask;
	UINT m_iSocketGetMsg;
};
