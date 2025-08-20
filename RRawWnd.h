// RWnd.h: interface for the RWnd class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include "rcommon.h"

#define RC_HOLLOW			1
#define RC_WINDOW			2

class RCOMMON_API RRawWnd  
{
public:
	RRawWnd();
	virtual ~RRawWnd(void);

	BOOL Attach(HWND a_hWnd);
	HWND Detach(void);

	HWND GetWnd() const { return m_hWnd; };
	void SetWnd(HWND a_hWnd) { m_hWnd = a_hWnd; };

	virtual void Create(void);

 	BOOL DestroyWindow();

	BOOL PostMessage(UINT a_iMsg, WPARAM a_wParam = 0, LPARAM a_lParam = 0) { ASSERT(m_hWnd != NULL) ; return ::PostMessage(m_hWnd, a_iMsg, a_wParam, a_lParam); };
	LRESULT SendMessage(UINT a_iMsg, WPARAM a_wParam = 0, LPARAM a_lParam = 0) { ASSERT(m_hWnd != NULL) ; return ::SendMessage(m_hWnd, a_iMsg, a_wParam, a_lParam); };

	BOOL IsWindow() const { return ::IsWindow(m_hWnd); };
	LONG GetWindowLong(int a_iIdx) const { ASSERT(m_hWnd != NULL) ; return ::GetWindowLong(m_hWnd, a_iIdx); };
	LONG SetWindowLong(int a_iIdx, LONG a_iVal) { ASSERT(m_hWnd != NULL) ; return ::SetWindowLong(m_hWnd, a_iIdx, a_iVal); };
	LONG GetStyle() const { return GetWindowLong(GWL_STYLE);};

	LRESULT DefaultProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);

	static LRESULT CALLBACK WndProc(HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);

	operator HWND() const { return GetWnd(); };
	static const TCHAR s_sWindowClass[];
	static const TCHAR s_sHollowClass[];
	static BOOL RegClass(void);

protected:
	static void HookWindowCreate(RRawWnd* a_pWnd);
	static void UnhookWindowCreate();

	//{MESSAGES}
	virtual BOOL OnCreate(LPCREATESTRUCT, BOOL*) { return TRUE; };
	virtual void OnTimer(UINT, BOOL*) {};
	virtual void OnDestroy(BOOL*) {};
	virtual void OnNcDestroy(BOOL* a_pbDefProc);

	virtual LRESULT OnWndMsg(UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL* a_pbDefProc);

	//{END MESSAGES}

	static long s_lRegClasses;

private:
	static LRESULT CALLBACK HookCreateProc(int a_nCode, WPARAM a_wParam, LPARAM a_lParam);

	static RRawWnd* s_pWndInit;
	static HHOOK s_hook;
	static BOOL s_bRegistered;
	HWND m_hWnd;
public:
	BOOL KillTimer(UINT_PTR a_idEvent);
	UINT_PTR SetTimer(UINT_PTR a_idTimer, UINT a_iElapse, TIMERPROC a_pfTimer = NULL);
};

