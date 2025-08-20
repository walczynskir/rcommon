// RKeyboard.cpp: funkcje obs�ugi klawiatury
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RKeyboard.h"


//	---------------------------------------------------------------------------
//	Checks if given virtual key is pressed
//
BOOL IsVKeyPressed(int a_iVKey)
{
	SHORT l_nKeyState = ::GetAsyncKeyState(a_iVKey);
	return ((l_nKeyState & 0x8000) == 0x8000);
}


//	---------------------------------------------------------------------------
//	Czy klawisz by� wci�ni�ty
//
BOOL 
WasKeyPressed(
	int a_iKey
	)
{
	return (::GetAsyncKeyState(a_iKey) & 0x000F);
}


//	---------------------------------------------------------------------------
//	Czy Ctrl wci�ni�ty
//
BOOL 
IsCtrlPressed()
{
	return IsKeyPressed(VK_CONTROL);
}


//	---------------------------------------------------------------------------
//	Czy Shift wci�ni�ty
//
BOOL 
IsShiftPressed()
{
	return IsKeyPressed(VK_SHIFT);
}


//	---------------------------------------------------------------------------
//	Checks if left mouse button is pressed
//
BOOL 
LeftMouseButtonPressed()
{
	int l_iKey = (GetSystemMetrics(SM_SWAPBUTTON)) ? VK_RBUTTON : VK_LBUTTON;
	return ::GetAsyncKeyState(l_iKey) & 0x8000;
}