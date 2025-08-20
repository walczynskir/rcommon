#include "stdafx.h"
#include <windows.h>     // required for all Windows applications
#include <windowsx.h>
#include "resource.h"
#include <oledlg.h>
#include "OleHelper.h"



/*
 * OleUIMetafilePictIconFree    [stolen from OLE2UI]
 *
 * Purpose:
 *  Deletes the metafile contained in a METAFILEPICT structure and
 *  frees the memory for the structure itself.
 *
 * Parameters:
 *  hMetaPict       HGLOBAL metafilepict structure created in
 *                  OleUIMetafilePictFromIconAndLabel
 *
 * Return Value:
 *  None
 */

STDAPI_(void) OleUIMetafilePictIconFree(HGLOBAL hMetaPict)
{
    LPMETAFILEPICT pMF;
    
    if (NULL == hMetaPict)
        return;
    
    pMF = (LPMETAFILEPICT)GlobalLock(hMetaPict);
    
    if (NULL != pMF)
    {
        if (NULL != pMF->hMF)
            DeleteMetaFile(pMF->hMF);
    }
    
    GlobalUnlock(hMetaPict);
    GlobalFree(hMetaPict);
    return;
}

/* OleStdSwitchDisplayAspect    [stolen from OLE2UI]
** -------------------------
**    Switch the currently cached display aspect between DVASPECT_ICON
**    and DVASPECT_CONTENT.
**
**    NOTE: when setting up icon aspect, any currently cached content
**    cache is discarded and any advise connections for content aspect
**    are broken.
**
**    RETURNS:
**      S_OK -- new display aspect setup successfully
**      E_INVALIDARG -- IOleCache interface is NOT supported (this is
**                  required).
**      <other SCODE> -- any SCODE that can be returned by
**                  IOleCache::Cache method.
**      NOTE: if an error occurs then the current display aspect and
**            cache contents unchanged.
*/

STDAPI OleStdSwitchDisplayAspect(LPOLEOBJECT  lpOleObj,
                                 LPDWORD      lpdwCurAspect,
                                 DWORD        dwNewAspect,
                                 HGLOBAL      hMetaPict,
                                 BOOL         fDeleteOldAspect,
                                 BOOL         fSetupViewAdvise,
                                 LPADVISESINK lpAdviseSink,
                                 LPBOOL       lpfMustUpdate)
{
    LPOLECACHE      lpOleCache = NULL;
    LPVIEWOBJECT    lpViewObj = NULL;
    LPENUMSTATDATA  lpEnumStatData = NULL;
    STATDATA        StatData;
    FORMATETC       FmtEtc;
    STGMEDIUM       Medium;
    DWORD           dwAdvf;
    DWORD           dwNewConnection;
    DWORD           dwOldAspect = *lpdwCurAspect;
    HRESULT         hrErr;

    if (lpfMustUpdate)
        *lpfMustUpdate = FALSE;

    lpOleObj->QueryInterface(IID_IOleCache, (LPVOID*)&lpOleCache);

    // if IOleCache* is NOT available, do nothing
    if (!lpOleCache)
        return E_INVALIDARG;

    // Setup new cache with the new aspect
    FmtEtc.cfFormat = 0;     // whatever is needed to draw
    FmtEtc.ptd      = NULL;
    FmtEtc.dwAspect = dwNewAspect;
    FmtEtc.lindex   = -1;
    FmtEtc.tymed    = TYMED_NULL;

    /* OLE2NOTE: if we are setting up Icon aspect with a custom icon
    **    then we do not want DataAdvise notifications to ever change
    **    the contents of the data cache. thus we set up a NODATA
    **    advise connection. otherwise we set up a standard DataAdvise
    **    connection.
    */
    if (dwNewAspect == DVASPECT_ICON && hMetaPict)
        dwAdvf = ADVF_NODATA;
    else
        dwAdvf = ADVF_PRIMEFIRST;

    hrErr = lpOleCache->Cache(&FmtEtc, dwAdvf, &dwNewConnection);
    if (!SUCCEEDED(hrErr))
    {
        lpOleCache->Release();
        return hrErr;
    }

    *lpdwCurAspect = dwNewAspect;

    /* OLE2NOTE: if we are setting up Icon aspect with a custom icon,
    **    then stuff the icon into the cache. otherwise the cache must
    **    be forced to be updated. set the *lpfMustUpdate flag to tell
    **    caller to force the object to Run so that the cache will be
    **    updated.
    */
    if (dwNewAspect == DVASPECT_ICON && hMetaPict)
    {
       FmtEtc.cfFormat = CF_METAFILEPICT;
       FmtEtc.ptd      = NULL;
       FmtEtc.dwAspect = DVASPECT_ICON;
       FmtEtc.lindex   = -1;
       FmtEtc.tymed    = TYMED_MFPICT;

       Medium.tymed          = TYMED_MFPICT;
       Medium.hGlobal        = hMetaPict;
       Medium.pUnkForRelease = NULL;

       hrErr = lpOleCache->SetData(&FmtEtc, &Medium, FALSE /* fRelease */);
    }
    else
    {
        if (lpfMustUpdate)
            *lpfMustUpdate = TRUE;
    }

    if (fSetupViewAdvise && lpAdviseSink)
    {
        /* OLE2NOTE: re-establish the ViewAdvise connection */
        lpOleObj->QueryInterface(IID_IViewObject, (LPVOID*)&lpViewObj);

        if (lpViewObj)
        {
            lpViewObj->SetAdvise(dwNewAspect, 0, lpAdviseSink);
            lpViewObj->Release();
        }
    }

    /* OLE2NOTE: remove any existing caches that are set up for the old
    **    display aspect. It WOULD be possible to retain the caches set
    **    up for the old aspect, but this would increase the storage
    **    space required for the object and possibly require additional
    **    overhead to maintain the unused cachaes. For these reasons the
    **    strategy to delete the previous caches is prefered. If it is a
    **    requirement to quickly switch between Icon and Content
    **    display, then it would be better to keep both aspect caches.
    */

    if (fDeleteOldAspect)
    {
        hrErr = lpOleCache->EnumCache(&lpEnumStatData);

        while (hrErr == NOERROR)
        {
            hrErr = lpEnumStatData->Next(1, &StatData, NULL);
            if (hrErr != NOERROR)
                break;              // DONE! no more caches.

            if (StatData.formatetc.dwAspect == dwOldAspect)
            {
                // Remove previous cache with old aspect
                lpOleCache->Uncache(StatData. dwConnection);
            }
        }

        if (lpEnumStatData)
        {
            if (lpEnumStatData->Release() != 0)
                MessageBox(NULL,
                           TEXT("OleStdSwitchDisplayAspect: Cache enumerator NOT released"),
                           NULL,
                           MB_ICONEXCLAMATION | MB_OK);
        }
    }

    if (lpOleCache)
        lpOleCache->Release();

    return NOERROR;
}

