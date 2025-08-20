#pragma once
// socket.h : header file
//

#include <rcommon/rcommon.h>
#include <rcommon/RBaseSocket.h>
#include <winsock2.h>
#include <rcommon/RGetHost.h>
#include <rcommon/RDynamicBuf.h>
#include <rcommon/rstring.h>
#include <list>

typedef std::list<std::string> StringList;

#define WS_VERSION_REQUIRED 0x0101
#define WS_VERSION_MAJOR 1
#define WS_VERSION_MINOR 1
#define MIN_SOCKETS_REQUIRED 10



/////////////////////////////////////////////////////////////////////////////
// RSocket window       

enum SocketStatus {UNINITIALIZED, DISCONNECTED, CONNECTING, LISTENING, DISCONNECTING, CONNECTED, 
			ERRORSTATE, TIMEDOUT} ;


class RCOMMON_API RSocket : public RBaseSocket
{   

public:
	RSocket(unsigned int a_iTimeout = 5);
	void Create(int a_iType = SOCK_STREAM, SOCKET a_socket = INVALID_SOCKET, SocketStatus a_status = UNINITIALIZED);
	void RemoveGetHost(void);
	void RemoveRawSendData(void);
	operator SOCKET() const { return m_socket; }
	void SendTo(const std::string& a_sAddress, u_short a_nPort, LPCVOID a_pData, int a_iDataLen);
	void SendTo(u_long a_iAddress, u_short a_nPort, LPCVOID a_pData, int a_iDataLen);
	void SetSocketOpt(int a_iLevel, int a_iOptName, const char* a_pcOptVal, int a_iOptLen);

	void SetThroughProxy(bool a_bThroughProxy, bool a_bDetectProxy);
	void DetectProxySettings();


protected:
	enum SocketMessage
	{

		EMsg = (WM_USER + 1),
		EMsgGetHost, 
		EMsgGetPort
	};

	enum SocketTimer
	{
		ETimerConnect = 1,
		ETimerReceive
	};

	struct sockaddr_in m_saServer;
	SocketStatus m_status;
	SOCKET m_socket;

// Attributes
private:
	// connect attributes
	std::string m_sAddress;
	short       m_nPort;

	RGetHost* m_pGetHost;
	struct sockaddr_in* m_pSaConnected;
	
	StringList ReceiveLines;
	std::string RemainingReceive;
	StringList SendLines;
	char *RawSendData;
	int RawSendDataLength;
	RDynamicBuf<BYTE> m_dynbuf;
	
	UINT m_iTimeout;
	
// Operations
public:

// Implementation
public:
	virtual ~RSocket(void);
	
	virtual void Connect(const std::string& address, u_short port);
	virtual void Disconnect(void);
	                             
	virtual BOOL SetReceiveTarget(HWND a_hWnd, UINT a_iMsg);                             
	virtual void Send(const std::string& a_sData);
	virtual void Send(LPCVOID a_pData, int a_iSize);
	std::string GetLine(void);
	BOOL GetSockName(SOCKADDR *sock_addr, int *addr_len);
	void Bind(const SOCKADDR* a_pAddr, int a_iSize);
	void Listen(int back_log = 5);
	void Accept(RSocket* a_pNewSocket);
	void Linger(void);

	SocketStatus GetStatus(void) const { return m_status; }
	tstring GetConnectedAddress(void) const;
	tstring RSocket::GetServerAddress(void) const;

protected:
 	BOOL Start(SOCKET sock = INVALID_SOCKET);
 	BOOL End(void);
 	void ConnectStep2(void);
	void AddToReceive(const char *buffer, int amt);

	virtual void OnTimer(UINT a_idTimer, BOOL* a_pbDefProc);
 	virtual LRESULT OnSocketMessage(WPARAM wParam, LPARAM lParam);
 	LRESULT OnConnectGetHost(int a_iErr);
	virtual LRESULT OnWndMsg(UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL* a_pbDefProc);
	virtual LRESULT OnSocketRead(void);
	virtual LRESULT OnSocketWrite(void);
	virtual LRESULT OnSocketOutOfBand(void) { return 0; };
	virtual LRESULT OnSocketAccept(void);
	virtual LRESULT OnSocketConnect(void);
	virtual LRESULT OnSocketClose(void);

public:
	static void Initialize(void);
	static void Terminate(void);

private:
	void WaitUntilSendBufferEmpty(void);
	void HandleError(DWORD a_dwErr);
	static void YieldControl();

};

