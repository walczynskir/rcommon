#pragma once
// bunch of usefull macros


#ifdef _DEBUG

#ifndef TRACE0
#define TRACE0(a_sText) 	_RPT0(_CRT_WARN, a_sText)
#endif 

#ifndef TRACE1
#define TRACE1(a_sText, a_arg1) _RPT1(_CRT_WARN, a_sText, a_arg1)
#endif 

#ifndef TRACE2
#define TRACE2(a_sText, a_arg1, a_arg2) _RPT2(_CRT_WARN, a_sText, a_arg1, a_arg2)
#endif 

#ifndef TRACE3
#define TRACE3(a_sText, a_arg1, a_arg2, a_arg3) _RPT3(_CRT_WARN, a_sText, a_arg1, a_arg2, a_arg3)
#endif 

#ifndef TRACE4
#define TRACE4(a_sText, a_arg1, a_arg2, a_arg3, a_arg4) _RPT4(_CRT_WARN, a_sText, a_arg1, a_arg2, a_arg3, a_arg4)
#endif 

#else //_DEBUG

#ifndef TRACE0
#define TRACE0(a_sText)
#endif 

#ifndef TRACE1
#define TRACE1(a_sText, a_arg1)
#endif 

#ifndef TRACE2
#define TRACE2(a_sText, a_arg1, a_arg2)
#endif 

#ifndef TRACE3
#define TRACE3(a_sText, a_arg1, a_arg2, a_arg3)
#endif 

#ifndef TRACE4
#define TRACE4(a_sText, a_arg1, a_arg2, a_arg3, a_arg4)
#endif 


#endif //! _DEBUG

#ifndef ArraySize
#define ArraySize(a_arr) (sizeof(a_arr) / sizeof(a_arr[0]))
#endif

#ifndef ASSERT
#ifdef _DEBUG
#include <crtdbg.h>
#define ASSERT(expr) _ASSERTE(expr)
#else
#define ASSERT(expr) ((void)0)
#endif
#endif

#ifdef _DEBUG
#define VERIFY(a) ASSERT(a)
#else
#define VERIFY(a) ((void)(a))
#endif

#ifdef _DEBUG
#define UNUSED(x)
#else
#define UNUSED(x) x
#endif

// useful macros from C++ Users Journal macros
#define chSTR(x) #x
#define chSTR2(x) chSTR(x)

#define LINE_STR chSTR2(__LINE__)
#define HERE __FILE__ "(" LINE_STR ") : "

#define todo(desc) message(__FILE__ "("chSTR2(__LINE__)"): TODO: "#desc" ")
#define err(desc) message(__FILE__ "("chSTR2(__LINE__)"): error: "#desc" ")
#define warn(desc) message(__FILE__ "("chSTR2(__LINE__)"): warning: "#desc" ")
#define changed(desc) message(__FILE__ "("chSTR2(__LINE__)"): changed: "#desc" ")

#define RectWidth(a_rect) ((a_rect).right - (a_rect).left)
#define RectHeight(a_rect) ((a_rect).bottom - (a_rect).top)

#define HasVertScrollBar(a_hWnd) (::GetWindowLong(a_hWnd, GWL_STYLE) & WS_VSCROLL)
#define HasHorzScrollBar(a_hWnd) (::GetWindowLong(a_hWnd, GWL_STYLE) & WS_HSCROLL)
#define IsDisabled(a_hWnd) ((::GetWindowLong(a_hWnd, GWL_STYLE) & WS_DISABLED) != 0)
#define HasStyle(a_hWnd, a_iStyle) ((::GetWindowLong(a_hWnd, GWL_STYLE) & a_iStyle) == a_iStyle)


#define IsFocused(a_hWnd) (::GetFocus() == a_hWnd)

#define IsKeyPressed(vkey) ((::GetKeyState(vkey) & 0xF000) == 0xF000)


#define ModifyStyle(a_hWnd, a_dwRemove, a_dwAdd) (::SetWindowLong(a_hWnd, GWL_STYLE, (::GetWindowLong(a_hWnd, GWL_STYLE) & ~a_dwRemove) | a_dwAdd));
