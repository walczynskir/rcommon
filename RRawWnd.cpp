// RRawWnd.cpp: implementation of the RRawWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RRawWnd.h"
#include "ROwnExc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static const long c_iWindowOfs = sizeof(RRawWnd*) - 4;
const TCHAR c_sSimpleClass[] = _T("RSIMPLE");
HHOOK RRawWnd::s_hook = NULL;
RRawWnd* RRawWnd::s_pWndInit = NULL;
BOOL RRawWnd::s_bRegistered = FALSE;

RRawWnd::RRawWnd(void) : m_hWnd(NULL)
{
	if (!s_bRegistered)
	{
		s_bRegistered = RegClass();
	}
}


RRawWnd::~RRawWnd(void)
{
	if (m_hWnd != NULL)
	{
		DestroyWindow();
		ASSERT(m_hWnd == NULL);
	}
}


//	---------------------------------------------------------------------------
//	Utworzenie okna
//
void
RRawWnd::Create(void)
{
	ASSERT(s_bRegistered);
	RRawWnd::HookWindowCreate(this);
	HWND l_hWnd = ::CreateWindowEx(0, c_sSimpleClass, _T(""), 
		0, 0, 0, 0, 0, NULL, NULL, RCommon_GetInstance(), NULL);
	RRawWnd::UnhookWindowCreate();
	ASSERT(m_hWnd == l_hWnd);
	UNUSED(l_hWnd);
	if (m_hWnd == NULL) // filled in hook procedure
	{
		throw ROwnExc(_T("Nie mo¿na utworzyæ okna."));
	}
}


//	---------------------------------------------------------------------------
//	Hook na utworzenie okna (dla zapamiêtania dostatecznie wczeœnie - przed 
//  WM_CREATE) uchwytu okna
//
void 
RRawWnd::HookWindowCreate(
	RRawWnd* a_pWnd
	)
{
	ASSERT(s_pWndInit == NULL);
	s_pWndInit = a_pWnd;

	s_hook = ::SetWindowsHookEx(WH_CBT, HookCreateProc, RCommon_GetInstance(), 
		GetCurrentThreadId());
}


//	---------------------------------------------------------------------------
//	Odhookowanie okna po jego utworzeniu
//
void 
RRawWnd::UnhookWindowCreate()
{
	// jeœli HOOK jest NULL to nakprawdopodobniej Create wo³ane z OnCreate
	if (s_hook != NULL)
	{
		::UnhookWindowsHookEx(s_hook);
		s_hook = NULL;
	}
}


//	---------------------------------------------------------------------------
//	Zniszczenie okna
//
BOOL 
RRawWnd::DestroyWindow()
{
	ASSERT(m_hWnd != NULL);
	return ::DestroyWindow(m_hWnd);
}


//	---------------------------------------------------------------------------
//	Ustalenie wskaŸnika na obiekt dla okna
//  Dziêki tej metodzie ju¿ w zdarzenie WM_CREATE mamy
//  wskaŸnik na obiekt
//
LRESULT CALLBACK 
RRawWnd::HookCreateProc(
	int a_nCode, 
	WPARAM a_wParam, 
	LPARAM a_lParam
	)
{
	if ((a_nCode != HCBT_CREATEWND) || (s_pWndInit == NULL))
	{
		return ::CallNextHookEx(s_hook, a_nCode, a_wParam, a_lParam);
	}

	RRawWnd* l_pWndInit = s_pWndInit;
	l_pWndInit->SetWnd((HWND)a_wParam);
#pragma warning(disable: 4244)	// I do not understand why compiler issues this warning!
	::SetWindowLongPtr(l_pWndInit->m_hWnd, c_iWindowOfs, (LONG_PTR)s_pWndInit);
#pragma warning(default: 4244)
	s_pWndInit = NULL;
	return ::CallNextHookEx(s_hook, a_nCode, a_wParam, a_lParam);
}


//	---------------------------------------------------------------------------
// Obs³uga zdarzeñ
//
LRESULT 
CALLBACK RRawWnd::WndProc(
	HWND a_hWnd, 
	UINT   a_iMsg, 
	WPARAM a_wParam, 
	LPARAM a_lParam
	)
{
#pragma warning(disable: 4312)
	RRawWnd* l_pWnd = (RRawWnd*)::GetWindowLongPtr(a_hWnd, c_iWindowOfs);
#pragma warning(default: 4312)
	ASSERT(l_pWnd->GetWnd() == a_hWnd);
	BOOL l_bDefProc = TRUE;

	switch (a_iMsg) 
	{
		case WM_CREATE:
			if (!l_pWnd->OnCreate((LPCREATESTRUCT) a_lParam, &l_bDefProc))
			{
				return -1;
			}
			break;

		case WM_TIMER:
			l_pWnd->OnTimer((UINT)a_wParam, &l_bDefProc);
			break;

		case WM_DESTROY:
			l_bDefProc = 0;
			l_pWnd->OnDestroy(&l_bDefProc);
			break;

		case WM_NCDESTROY:
			l_pWnd->OnNcDestroy(&l_bDefProc);
			break;

		default:
			{
				LRESULT l_lRes = l_pWnd->OnWndMsg(a_iMsg, a_wParam, a_lParam, &l_bDefProc);
				if (!l_bDefProc)
				{
					return l_lRes;
				}
			}
	}
	if (l_bDefProc)
	{
		return l_pWnd->DefaultProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
	}
	return 0;
}


//	---------------------------------------------------------------------------
//	Default processing
//
LRESULT
RRawWnd::OnWndMsg(
	UINT a_iMsg, 
	WPARAM a_wParam, 
	LPARAM a_lParam, 
	BOOL* a_pbDefProc
	)
{
	*a_pbDefProc = FALSE;
	return DefaultProc(m_hWnd, a_iMsg, a_wParam, a_lParam);
}


//	---------------------------------------------------------------------------
//	Default processing
//
LRESULT 
RRawWnd::DefaultProc(
	HWND a_hWnd, 
	UINT a_iMsg, 
	WPARAM a_wParam, 
	LPARAM a_lParam
	)
{
	ASSERT(a_hWnd == GetWnd());
	return ::DefWindowProc(a_hWnd, a_iMsg, a_wParam, a_lParam);
}


//	---------------------------------------------------------------------------
//	Utworzonej kontrolce przydzielamy okno
//
BOOL 
RRawWnd::Attach(
	HWND a_hWndNew
	)
{
	ASSERT(m_hWnd == NULL);
	ASSERT(a_hWndNew != NULL);

	// trzeba subclassowaæ
	SetWnd(a_hWndNew);
	return TRUE;
}


//	---------------------------------------------------------------------------
//	Od³¹czenie klasy od okna
//
HWND
RRawWnd::Detach(void)
{
	HWND l_hWnd = m_hWnd;
	m_hWnd = NULL;

	::SetWindowLongPtr(l_hWnd, c_iWindowOfs, (LONG_PTR)NULL);
	return l_hWnd;
}


//	---------------------------------------------------------------------------
//	Zarejestrowanie standardowych klas okien
//
BOOL 
RRawWnd::RegClass(void)
{
	WNDCLASSEX l_wcex;
	memset(&l_wcex, 0, sizeof(WNDCLASSEX));   // start with NULL defaults

	l_wcex.cbSize			= sizeof(WNDCLASSEX); 
	l_wcex.style			= CS_GLOBALCLASS;
	l_wcex.lpfnWndProc		= RRawWnd::WndProc;
	l_wcex.cbWndExtra		= sizeof(RRawWnd*);
	l_wcex.hInstance		= RCommon_GetInstance();
	l_wcex.lpszClassName	= c_sSimpleClass;

	return (::RegisterClassEx(&l_wcex) != NULL);
}



BOOL 
RRawWnd::KillTimer(
	UINT_PTR a_idEvent
	)
{
	ASSERT(m_hWnd != NULL); 
	return ::KillTimer(m_hWnd, a_idEvent);
}


UINT_PTR 
RRawWnd::SetTimer(
	UINT_PTR a_idTimer, 
	UINT  a_iElapse, 
	TIMERPROC a_pfTimer
	)
{
	ASSERT(m_hWnd != NULL); 
	return ::SetTimer(m_hWnd, a_idTimer, a_iElapse, a_pfTimer);
}


void 
RRawWnd::OnNcDestroy(BOOL* a_pbDefProc)
{
	// call default, unsubclass, and detach from the map
	*a_pbDefProc = FALSE;
	DefaultProc(m_hWnd, WM_NCDESTROY, 0, 0);
	Detach();
	ASSERT(m_hWnd == NULL);
}