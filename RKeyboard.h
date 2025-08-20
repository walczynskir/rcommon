#pragma once
// RKeyboard.cpp: keyboard helpers
//
//////////////////////////////////////////////////////////////////////
#include "rcommon.h"

extern RCOMMON_API BOOL IsVKeyPressed(int a_iVKey);
extern RCOMMON_API BOOL WasKeyPressed(int a_iKey);
extern RCOMMON_API BOOL IsCtrlPressed();
extern RCOMMON_API BOOL IsShiftPressed();
extern RCOMMON_API BOOL LeftMouseButtonPressed();