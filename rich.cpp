// rich.cpp : helpers for RichEdit
//
#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <RichEdit.h>
#include "rich.h"
#include "resource.h"
#include "RNotEnoughMemExc.h"
#include "ROwnExc.h"


static DWORD CALLBACK RichEditWriteCallback(DWORD_PTR a_pdwObj, LPBYTE a_pbBuff, LONG a_iSize, LONG* a_piUsed);
static DWORD CALLBACK RichEditReadCallback(DWORD_PTR a_pdwObj, LPBYTE a_pbBuff, LONG a_iSize, LONG* a_piUsed);



typedef struct _S_RichWrite
{
	long iSize;
	RDataBuf<BYTE>* pBuf;
} RICHWRITE, *LPRICHWRITE;


typedef struct _S_RichRead
{
	long iSize;
	RDataBuf<BYTE>* pBuf;
	long iReaded;
} RICHREAD, *LPRICHREAD;



DWORD CALLBACK RichEditWriteCallback(
	DWORD_PTR a_pdwCookie,
    LPBYTE a_pbBuff,
    LONG a_iSize,
    LONG* a_piUsed
)
{
	LPRICHWRITE l_prw = (LPRICHWRITE)a_pdwCookie;
	l_prw->iSize += a_iSize;
	RDataBuf<BYTE> l_data;
	try
	{
		l_data.AllocBuffer(a_iSize);
	}
	catch (RNotEnoughMemExc&)
	{
		return RERR_NOMEM;
	}
	memcpy(l_data, a_pbBuff, a_iSize);
	l_data.SetDataSize(a_iSize);
	*(l_prw->pBuf) += l_data;
	*a_piUsed = a_iSize;

	return 0;	
}


DWORD CALLBACK RichEditReadCallback(
	DWORD_PTR a_pdwObj, 
	LPBYTE a_pbBuff, 
	LONG a_iSize, 
	LONG* a_piUsed
	)
{
	LPRICHREAD l_prr = reinterpret_cast<LPRICHREAD>(a_pdwObj);
	
	long l_iStepRead = min(a_iSize, l_prr->iSize - l_prr->iReaded);
	RDataBuf<BYTE>& l_dynbuf = *(l_prr->pBuf);
	LPBYTE l_pData = l_dynbuf;
	memcpy(a_pbBuff, l_pData + l_prr->iReaded, l_iStepRead);
	l_prr->iReaded += l_iStepRead;
	*a_piUsed = l_iStepRead;

	return 0;	
}


//
//
//
void RichEdit_GetRtf(HWND a_hWndRich, RDataBuf<BYTE>* a_pBuf, long* a_piSize)
{
	ASSERT(a_pBuf != NULL);
	ASSERT(a_piSize != NULL);
	RICHWRITE l_rw;
	l_rw.pBuf = a_pBuf;
	l_rw.iSize = 0;
	EDITSTREAM l_es;
	l_es.dwCookie = reinterpret_cast<DWORD_PTR>(&l_rw);
	l_es.dwError = 0;
	l_es.pfnCallback = RichEditWriteCallback;

	::SendMessage(a_hWndRich, EM_STREAMOUT, SF_RTF, (LPARAM)&l_es);
	if (l_es.dwError != 0)
	{
		if (l_es.dwError == RERR_NOMEM)
		{
			throw RNotEnoughMemExc();
		}
		else
		{
			throw ROwnExc(GetModuleHandle(NULL), IDS_ERR_UNKNOWN);
		}
	}

	*a_piSize = l_rw.iSize;
}


void RichEdit_SetRtf(HWND a_hWndRich, RDataBuf<BYTE>* a_pBuf, long a_iSize, bool a_bRtf)
{
	RICHREAD l_rr;
	l_rr.iSize = a_iSize;
	l_rr.pBuf = a_pBuf;
	l_rr.iReaded = 0;

	EDITSTREAM l_es;
	l_es.dwCookie = reinterpret_cast<DWORD_PTR>(&l_rr);
	l_es.dwError = 0;
	l_es.pfnCallback = RichEditReadCallback;
	
	::SendMessage(a_hWndRich, EM_STREAMIN, (a_bRtf ? SF_RTF : SF_TEXT) | SFF_SELECTION, (LPARAM)&l_es);
}


