#pragma once

#include "REOleCallback.h"


class ROleInPlaceFrame;

class RRichEditData
{
public:
    //
    // Constructors and Destructor
    //
	RRichEditData();
    RRichEditData(HWND a_hFrame);
	~RRichEditData();

    BOOL Initialize(HWND a_hRichEdit);

    HRESULT GetNewStorage(LPSTORAGE* ppStg);
    BOOL InsertObject();
	HRESULT DoVerb(UINT id);

    ROleInPlaceFrame* GetInPlaceFrame() { return m_pInPlaceFrame; }
    HWND GetFrameWindow() { return m_hFrame; }
    HACCEL GetInPlaceAccel() { return m_hAccelInPlace; }
    BOOL MoveEditControl(int left, int top, int width, int height, int style);

	//attributes
public:
	HWND m_hFrame;						
    ROleInPlaceFrame* m_pInPlaceFrame;                                       
    HACCEL m_hAccelInPlace;             
    UINT m_cStorages;                   
    RRichEditOleCallback* m_pRECallback;
    LPRICHEDITOLE m_pRichEditOle;       


};