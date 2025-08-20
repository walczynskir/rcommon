// RSocket.cpp : implementation file
//

#include "stdafx.h"
#include <rcommon/RSocket.h>
#include <rcommon/ROwnExc.h>
#include <rcommon/RSystemExc.h>
#include <wininet.h>


const int c_iSockBlockSize = 1024;

/////////////////////////////////////////////////////////////////////////////
// RSocket


RSocket::RSocket(unsigned int a_iTimeout)
  :m_iTimeout(a_iTimeout), m_status(UNINITIALIZED), 
   RawSendData(NULL),
   RawSendDataLength(0),
   m_pGetHost(NULL),
   m_pSaConnected(NULL)
{
}


RSocket::~RSocket(void)
{
	// we must succeed
	try
	{
		Disconnect();
	}
	catch (RSystemExc&)
	{
	}
	RemoveRawSendData();
	RemoveGetHost();
	if (m_pSaConnected != NULL)
	{
		delete m_pSaConnected;
		m_pSaConnected = NULL;
	}

}


void 
RSocket::OnTimer(
	UINT a_idTimer,
	BOOL* a_pbDefProc
	)
{
	*a_pbDefProc = TRUE;
	switch (a_idTimer)
	{
	 	case ETimerConnect:
			KillTimer(a_idTimer);
    		if (m_status == CONNECTING)
    		{
    			// Connection timed out.
    			m_status = TIMEDOUT;
				SendReceiveMessage((WPARAM)SocketStatusChanged,	0);
    		}
 			break;
	 	
	 	case ETimerReceive:
			KillTimer(a_idTimer);
 	    	if (m_status == CONNECTED)
 	    	{
	   			// Receive timed out
				SendReceiveMessage((WPARAM)SocketStatusChanged,	0);
 	    		m_status = TIMEDOUT;
 	    	}
 	    	break;
	}                                                  
}


void 
RSocket::Create(
	int          a_iType,
	SOCKET		 a_socket,
	SocketStatus a_status
	)
{
	RRawWnd::Create();

	if (a_socket == INVALID_SOCKET)
	{
	    m_socket = socket(PF_INET, a_iType, 0);
    	if (m_socket == INVALID_SOCKET)
	    {
			throw(RSystemExc(::WSAGetLastError()));
	    }
		m_status = DISCONNECTED;
	}
	else
	{
		m_socket = a_socket;
		m_status = a_status;
	}
}               


// ensure the address is correct        
void
RSocket::Connect(
	const std::string& a_sAddress, 
	u_short a_nPort
	)
{               
	ASSERT(m_status == DISCONNECTED);
	                    
	m_status = CONNECTING;
	
	// set timer to go off  
	SetTimer(ETimerConnect, m_iTimeout * 1000);
	
	m_saServer.sin_family = AF_INET;
	m_sAddress = a_sAddress;
	m_nPort = a_nPort;
	m_saServer.sin_addr.s_addr = inet_addr(a_sAddress.c_str());
	m_saServer.sin_port = htons(a_nPort);
	
    if (m_saServer.sin_addr.s_addr == INADDR_NONE)
    {
   		// treat this as a host name and get it resolved.
		m_pGetHost = new RGetHost(GetWnd(), EMsgGetHost);
		m_pGetHost->GetHostByName(a_sAddress.c_str());
    }
    else
    {
    	ConnectStep2();
    }
    
}


// we have the address resolved now so do the connect    
void 
RSocket::ConnectStep2(void)
{
	int l_iErr;
	
	if (::WSAAsyncSelect(m_socket, GetWnd(), EMsg, 
    		FD_CONNECT) == SOCKET_ERROR)
    {       
		throw RSystemExc(::WSAGetLastError()); 	
    }

	if (::connect(m_socket, (LPSOCKADDR)&m_saServer, sizeof(m_saServer)) == SOCKET_ERROR)
	{
		l_iErr = WSAGetLastError();
		if (l_iErr != WSAEWOULDBLOCK)
		{
			throw RSystemExc(l_iErr);
		}
	}
	else
	{
		if (::WSAAsyncSelect(m_socket, GetWnd(), EMsg, 
    		FD_READ | FD_WRITE | FD_CLOSE ) == SOCKET_ERROR)
    	{       
			throw RSystemExc(::WSAGetLastError()); 	
	    }

		m_status =  CONNECTED;
		// our status just changed
		// send message if we are sending to another window
		KillTimer(ETimerConnect);
		SendReceiveMessage(static_cast<WPARAM>(SocketStatusChanged),	0);
 	}
    
}


void 
RSocket::Disconnect(void)
{  
	unsigned long nbIO = 0;

	if (WSAAsyncSelect(m_socket, GetWnd(), 0, 0) == SOCKET_ERROR)
	{
		throw RSystemExc(::WSAGetLastError());
	}

 	// disable non-blocking mode
	if (ioctlsocket(m_socket, FIONBIO, &nbIO) == SOCKET_ERROR)
	{
		throw RSystemExc(::WSAGetLastError());
	}

	if(closesocket(m_socket) == SOCKET_ERROR)
	{
		throw RSystemExc(::WSAGetLastError());
	}

}

void RSocket::Linger(void)
{
	int what = 0;
	struct linger sL;
	sL.l_onoff = 1;
	sL.l_linger = 30;

	what = setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (char *)&sL, sizeof(sL));

	if (what == SOCKET_ERROR)
	{
		//throw();
	}
}


// sets receiving window and message
//
BOOL 
RSocket::SetReceiveTarget(
	HWND a_hWnd, 
	UINT a_iMsg
	)
{                           
	RBaseSocket::SetReceiveTarget(a_hWnd, a_iMsg);

	// clear receive buffer as the data must be left over
	ReceiveLines.clear();
	RemainingReceive = "";

	// setup AsyncSelect
	switch (m_status)
	{
		case CONNECTED:
		    if (WSAAsyncSelect(m_socket, GetWnd(), EMsg, 
    			FD_READ | FD_WRITE | FD_CLOSE ) == SOCKET_ERROR)
    		{       
	   			return FALSE; 	
		    }
			break;

		case LISTENING:
		    if (WSAAsyncSelect(m_socket, GetWnd(), EMsg, 
    			FD_ACCEPT ) == SOCKET_ERROR)
    		{       
	   			return FALSE; 	
		    }
			break;

		default:
		    if (WSAAsyncSelect(m_socket, GetWnd(), EMsg, 
    			FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE ) == SOCKET_ERROR)
    		{       
	   			return FALSE; 	
		    }
			break;
	}

	return TRUE;
}


//
//
void 
RSocket::Send(
	const std::string& a_sData
	)
{
	if (a_sData.length() == 0)
	{
		return;
	}

	bool l_bEmpty = SendLines.empty();

	SendLines.push_back(a_sData);
	
	if (l_bEmpty)
	{
		StringList::iterator l_iterFirst = SendLines.begin();
		int l_iLen = (int)(*l_iterFirst).length();
		
 		int l_iAmount = send(m_socket, reinterpret_cast<const char*>((*l_iterFirst).c_str()), l_iLen, 0);	
		if (l_iAmount == SOCKET_ERROR)
		{
			int l_iErr = WSAGetLastError();
			if (l_iErr != WSAEWOULDBLOCK && l_iErr != WSAEINPROGRESS)
			{
				throw(RSystemExc(l_iErr));
			}
		}                      
		if (l_iAmount == l_iLen)
		{
			SendLines.pop_front();
		}
		else
		{
			// only part of line was sent leave rest for later
			(*l_iterFirst) = (*l_iterFirst).substr(l_iAmount);
		}
	}
}


void 
RSocket::WaitUntilSendBufferEmpty(void)
{
	// Pump messages until all lines sent
	while (!SendLines.empty() && RawSendDataLength)
	{
		MSG l_msg;
		if (::PeekMessage(&l_msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&l_msg);
			::DispatchMessage(&l_msg);
		}                    
		else
		{
			break;
		}
	}
}


void 
RSocket::Send(
	LPCVOID a_pData, 
	int a_iDataLen
	)
{
	char* l_psData = new char[RawSendDataLength + a_iDataLen];
	if (RawSendData != NULL)
	{
		memcpy(l_psData, RawSendData, RawSendDataLength);
	}
	memcpy(l_psData + RawSendDataLength, a_pData, a_iDataLen);
	if (RawSendData != NULL)
	{
		delete[] RawSendData;
	}
	RawSendData = l_psData;

	RawSendDataLength += a_iDataLen;

	while(RawSendDataLength > 0)
	{
		int l_iAmount = send(m_socket, (char *)RawSendData, RawSendDataLength, 0);
		if (l_iAmount == SOCKET_ERROR)
		{
			DWORD l_dwErr = WSAGetLastError();
			if (l_dwErr != WSAEWOULDBLOCK && l_dwErr != WSAEINPROGRESS)
			{
				RSystemExc l_exc(l_dwErr);
				throw(l_exc);
			}
			break;
		}
		else
		{
			RawSendDataLength -= l_iAmount;
			memcpy(RawSendData, RawSendData + l_iAmount, RawSendDataLength);
		}
	}

}


void 
RSocket::AddToReceive(
	const char* a_sData, 
	int a_iAmount
	)
{
	ASSERT(FALSE); // poni¿sza linia do zmiany, bo blok mo¿e byæ wiêkszy od c_iSock...
	char l_sBuf[c_iSockBlockSize + 1];
	
	int l_iPos = 0;
	while (l_iPos < a_iAmount)
	{
		const char* l_pNewLine = strchr(a_sData + l_iPos, '\n');
		int l_iLen;
		
		// found new line chcracter
		if (l_pNewLine != NULL)
		{
			l_iLen = (int)(l_pNewLine - a_sData - l_iPos + 1);
		}
		else
		{
			l_iLen = a_iAmount - l_iPos;
		}
					
		strncpy_s(l_sBuf, ArraySize(l_sBuf), a_sData + l_iPos, l_iLen);
		l_sBuf[l_iLen] = 0;	// add 0 to the end
		
		if (l_pNewLine != NULL)
		{				                  
			// we have a linefeed so add string
			if (RemainingReceive.empty())
			{
				ReceiveLines.push_back(l_sBuf);
			}                            
			else
			{
				ReceiveLines.push_back(RemainingReceive + l_sBuf);
				RemainingReceive.clear();
			}
 		}
 		else
 		{
 			// we don't have a linefeed so add to RemainingReceive
 			// for future use
 			RemainingReceive += l_sBuf;
 		}
 		l_iPos += l_iLen;
	}
}
				

std::string 
RSocket::GetLine(void)
{                                            
	if (!IsAsync())
	{
		// we are in the wrong mode to use GetLine()
		return "";
	}

	// ensure we yield control on each line gotten
	YieldControl();
	
	if (m_status == CONNECTED && ReceiveLines.empty())
	{
		SetTimer(ETimerReceive, m_iTimeout * 1000, NULL);
	
		while(m_status == CONNECTED && ReceiveLines.empty())
		{
			YieldControl();
		}
		
		KillTimer(ETimerReceive);
	}
	        
	if (!ReceiveLines.empty())
	{   
		std::string l_sHead = *(ReceiveLines.begin());
		ReceiveLines.pop_front();
		return l_sHead;
			
	}
	return "";
} 


BOOL 
RSocket::GetSockName(
	LPSOCKADDR sock_addr, 
	LPINT addr_len
	)
{
	if (getsockname(m_socket, sock_addr, addr_len) == SOCKET_ERROR)
	{                                          
		return FALSE;
	}

	return TRUE;
}


void
RSocket::Bind(
	const SOCKADDR* a_pAddr, 
	int a_iSize
	)
{
	if (::bind(m_socket, a_pAddr, a_iSize) == SOCKET_ERROR)
	{                                          
		throw (RSystemExc(::WSAGetLastError()));
	}
}

void
RSocket::Listen(int back_log /* = 5 */)
{
	if (IsAsync())
	{
		// receiving callbacks when status changes so setup AsyncSelect
		if (::WSAAsyncSelect(m_socket, GetWnd(), EMsg, 
    		FD_ACCEPT) == SOCKET_ERROR)
    	{       
			throw (RSystemExc(::WSAGetLastError()));
    	}
	}

	if (listen(m_socket, back_log) == SOCKET_ERROR)
	{                                          
		throw (RSystemExc(::WSAGetLastError()));
	}
	
	m_status = LISTENING;
}


void
RSocket::Accept(
	RSocket* a_pNewSocket
	)
{
	ASSERT(a_pNewSocket != NULL);

	if (m_pSaConnected == NULL)
	{
		m_pSaConnected = new (struct sockaddr_in);
	}
	int l_iLen = sizeof(struct sockaddr_in);

	SOCKET sock = ::accept(m_socket, reinterpret_cast<struct sockaddr*>(m_pSaConnected), &l_iLen);

	if (sock == INVALID_SOCKET)
	{
		int l_dwErr = WSAGetLastError();
		if (l_dwErr != WSAEWOULDBLOCK)
		{
			throw(RSystemExc(l_dwErr));
		}
	}
	else
	{
		// the new socket will only have AsyncSelect of FD_ACCEPT on
		// a call to SetReceiveTarget will reset this
		a_pNewSocket->Create(SOCK_STREAM, sock, CONNECTED);
	}

}


LRESULT RSocket::OnSocketMessage(WPARAM /*a_wParam*/, LPARAM a_lParam)
{
 	LRESULT l_iRes = 0;
 	
	if (WSAGETSELECTERROR(a_lParam) != 0)
	{
		m_status = ERRORSTATE;
		return SendErrorMessage(WSAGETSELECTERROR(a_lParam));
	}

 	switch (WSAGETSELECTEVENT(a_lParam)) 
 	{
 		case FD_READ:		return OnSocketRead();
 		case FD_WRITE:		return OnSocketWrite();
 		case FD_OOB:		return OnSocketOutOfBand();   
 		case FD_ACCEPT:		return OnSocketAccept();           
 		case FD_CONNECT:	return OnSocketConnect();
 		case FD_CLOSE:		return OnSocketClose();

 		default:
 			break;
 	}	                               
 	return l_iRes;
}
             

LRESULT 
RSocket::OnConnectGetHost(
	int a_iErr
	)
{        
	if (a_iErr != 0)
	{             
		m_status = ERRORSTATE;
		SendErrorMessage(a_iErr);
		return 0;
	}            
	
	memcpy(&(m_saServer.sin_addr.s_addr), m_pGetHost->GetHost()->h_addr, sizeof(IN_ADDR));
	delete m_pGetHost;
	m_pGetHost = NULL;
	ConnectStep2();
	
	return 0;
}

LRESULT 
RSocket::OnWndMsg(
	UINT a_iMsg, 
	WPARAM a_wParam, 
	LPARAM a_lParam, 
	BOOL* a_pbDefProc
	)
{
	*a_pbDefProc = FALSE;
	switch (a_iMsg)
	{
		case EMsg:
			return OnSocketMessage(a_wParam, a_lParam);

		case EMsgGetHost:
			return OnConnectGetHost((int)a_lParam);

	}
	return RRawWnd::OnWndMsg(a_iMsg, a_wParam, a_lParam, a_pbDefProc);
}


void
RSocket::Initialize(void)
{
	WSADATA l_wsaData;
    DWORD l_dwErr;
   
	l_dwErr = ::WSAStartup(WS_VERSION_REQUIRED, &l_wsaData);
	if (l_dwErr != 0)
	{
		throw(RSystemExc(l_dwErr));
	}
    
    if (l_wsaData.wVersion != WS_VERSION_REQUIRED)
    { 
		throw(ROwnExc(_T("Niew³aœciwa wersja gniazdek")));
    }

	if (l_wsaData.iMaxSockets < MIN_SOCKETS_REQUIRED)
    {
		throw(ROwnExc(_T("Niewystarczaj¹ca iloœæ gniazdek")));
    }

}


void
RSocket::Terminate(void)
{
	if (WSACleanup() != 0)
	{
		throw(RSystemExc(WSAGetLastError())); 
	}                      
}


LRESULT
RSocket::OnSocketRead(void)
{
	u_long l_iAmount;

	if (::ioctlsocket(m_socket, FIONREAD, &l_iAmount) == SOCKET_ERROR)
	{
		return SendErrorMessage(::WSAGetLastError());
	}

	char* l_psBuf = (char*)m_dynbuf.GetBuffer(l_iAmount);

	int l_iFlags = 0;
	int l_iSize = ::recv(m_socket, l_psBuf, l_iAmount, l_iFlags);
	
	// an error - let's get out of here
 	if ((l_iAmount == 0) || (l_iSize != static_cast<int>(l_iAmount)))
 	{
		int l_iErr = ::WSAGetLastError();
 		if (l_iErr != WSAEWOULDBLOCK && l_iErr != WSAEINPROGRESS)
 		{
 			return SendErrorMessage(l_iErr);
 		}
		return 0;
 	}

 	if (IsAsync())
 	{                                               
 		// send to window
		LRESULT l_res = SendReceiveMessage((WPARAM)l_iAmount, (LPARAM)(LPSTR)l_psBuf);
		return l_res;
 	}

	// add to buffer - we are using GetLine() mode
	AddToReceive(l_psBuf, l_iAmount);
	return 0;
}


LRESULT 
RSocket::OnSocketWrite(void)
{
 	// Ready to write
	// if raw data available send it before lines
	if (RawSendDataLength != 0)
	{
		while(RawSendDataLength > 0)
		{
			int l_iAmount = send(m_socket, (char *)RawSendData, RawSendDataLength, 0);
			if (l_iAmount == SOCKET_ERROR)
			{
				int l_iErr = WSAGetLastError();
				if (l_iErr != WSAEWOULDBLOCK && l_iErr != WSAEINPROGRESS)
				{
					return SendErrorMessage(l_iErr);
				}
				return 0;
			}
			else
			{
				RawSendDataLength -= l_iAmount;
				memcpy(RawSendData, RawSendData + l_iAmount, RawSendDataLength);
			}
		}
		return 0;
	} 

 	if (!SendLines.empty())
 	{
		std::string& l_sHead = *(SendLines.begin());
		int l_iAmount;

		l_iAmount = ::send(m_socket, l_sHead.c_str(), (int)l_sHead.length(), 0);	
		if (l_iAmount == SOCKET_ERROR)
		{
			int l_iErr = WSAGetLastError();
			if (l_iErr != WSAEWOULDBLOCK && l_iErr != WSAEINPROGRESS)
			{
				return SendErrorMessage(l_iErr);
			}
			return 0;
		}
		SendLines.pop_front();
		return 0;
 	}
 	return 0;
}


LRESULT 
RSocket::OnSocketAccept(void)
{
 	ASSERT(m_status == LISTENING); // FD_RACCEPT when not in status LISTENING
	ASSERT(IsAsync());	// Received Async FD_ACCEPT without a target to get new RSocket

	SendReceiveMessage((WPARAM)NewSocketAccepted, 0);
	return 0;
}


LRESULT 
RSocket::OnSocketConnect(void)
{
	KillTimer(ETimerConnect);
	if (m_status != TIMEDOUT)		// can eventually happen, especially during debugging
	{
	 	ASSERT(m_status == CONNECTING); // FD_CONNECT received when not in status CONNECTING
	}
	else
	{
		return 0;
	}

	if (WSAAsyncSelect(m_socket, GetWnd(), EMsg, 
    		FD_READ | FD_WRITE | FD_CLOSE ) == SOCKET_ERROR)
    {       
   		return SendErrorMessage(WSAGetLastError());
    }

	 m_status = CONNECTED;
	SendReceiveMessage((WPARAM)SocketStatusChanged, 0);

	return 0;
}


LRESULT 
RSocket::OnSocketClose(void)
{
 	m_status = DISCONNECTED;
	// our status just changed
	// send message if we are sending to another window
	SendReceiveMessage((WPARAM)SocketStatusChanged, 0);
	return 0;
}


void RSocket::RemoveGetHost(void)
{
	if (m_pGetHost != NULL)
	{
		delete m_pGetHost;
	}
}


void RSocket::RemoveRawSendData(void)
{
	if (RawSendData != NULL)
	{
		delete[] RawSendData;
	}
}


tstring RSocket::GetConnectedAddress(void) const
{
	if (m_pSaConnected == NULL)
	{
		return _T("");
	}
	TCHAR l_sAddress[16];
	_sntprintf_s(l_sAddress, ArraySize(l_sAddress), ArraySize(l_sAddress), _T("%d.%d.%d.%d"), 
		m_pSaConnected->sin_addr.S_un.S_un_b.s_b1,
		m_pSaConnected->sin_addr.S_un.S_un_b.s_b2,
		m_pSaConnected->sin_addr.S_un.S_un_b.s_b3,
		m_pSaConnected->sin_addr.S_un.S_un_b.s_b4);
	l_sAddress[15] = _T('\0');
	return l_sAddress;
}


tstring 
RSocket::GetServerAddress(void) const
{
	if (GetStatus() != CONNECTED)
	{
		return _T("");
	}
	TCHAR l_sAddress[16];
	_sntprintf_s(l_sAddress, ArraySize(l_sAddress), ArraySize(l_sAddress), _T("%d.%d.%d.%d"), 
		m_saServer.sin_addr.S_un.S_un_b.s_b1,
		m_saServer.sin_addr.S_un.S_un_b.s_b2,
		m_saServer.sin_addr.S_un.S_un_b.s_b3,
		m_saServer.sin_addr.S_un.S_un_b.s_b4);
	l_sAddress[15] = _T('\0');
	return tstring(l_sAddress);
}


void 
RSocket::SetSocketOpt(int a_iLevel, int a_iOptName, const char* a_pcOptVal, int a_iOptLen)
{
	int l_iErr = ::setsockopt(m_socket, a_iLevel, a_iOptName, a_pcOptVal, a_iOptLen);
	if (l_iErr == SOCKET_ERROR)
	{
		throw RSystemExc(::WSAGetLastError());
	}
}


void 
RSocket::SendTo(
	const std::string& a_sAddress, 
	u_short a_nPort,
	LPCVOID a_pData, 
	int a_iDataLen
	)
{
	u_long l_iAddress = inet_addr(a_sAddress.c_str());
	if (l_iAddress == htonl(INADDR_NONE))
	{
		hostent* l_pHostEnt = ::gethostbyname(a_sAddress.c_str());
		memcpy(&(l_iAddress), l_pHostEnt->h_addr, sizeof(IN_ADDR));
	}
	SendTo(l_iAddress, a_nPort, a_pData, a_iDataLen);
}


void 
RSocket::SendTo(u_long a_iAddress, u_short a_nPort, LPCVOID a_pData, int a_iDataLen)
{
	SOCKADDR_IN l_sockaddr;
	memset(&l_sockaddr, 0, sizeof(l_sockaddr));
	l_sockaddr.sin_family = AF_INET;
	l_sockaddr.sin_port = htons(a_nPort); 
	l_sockaddr.sin_addr.s_addr = a_iAddress;

	int l_iErr = ::sendto(m_socket, (const char*)a_pData, a_iDataLen, 0, (const sockaddr*)&l_sockaddr, sizeof(l_sockaddr));
	if (l_iErr == SOCKET_ERROR)
	{
		throw RSystemExc(::WSAGetLastError());
	}
}


void RSocket::YieldControl()
{
	BOOL l_bContinue = TRUE;
	while (l_bContinue)
	{
		MSG l_msg;
		l_bContinue = ::PeekMessage(&l_msg, NULL, 0, 0, PM_REMOVE);
		if (l_bContinue)
		{
			::TranslateMessage(&l_msg);
			::DispatchMessage(&l_msg);
		}                         
	}            
}