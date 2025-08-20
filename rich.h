#pragma once

// rich.h : helpers for RichEdit
#include <rcommon/rcommon.h>
#include <rcommon/RDataBuf.h>


extern RCOMMON_API void RichEdit_GetRtf(HWND a_hWndRich, RDataBuf<BYTE>* a_pBuf, long* a_piSize);
extern RCOMMON_API void RichEdit_SetRtf(HWND a_hWndRich, RDataBuf<BYTE>* a_pBuf, long a_iSize, bool a_bRtf);

#define RERR_NOMEM	(1)