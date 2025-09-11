#pragma once
#include <rcommon/rcommon.h>
#include <rcommon/rstring.h>

#define RListCtrl_ClassName _T("RLISTCTRL")


#ifdef _WIN32_WCE
#define RLCF_NOTOOLTIP
#endif

// column definition for adding column
typedef struct S_RLCCOLDEF
{
	LPCTSTR psColName;
	long iLength;
} RLCCOLDEF, *LPRLCCOLDEF;


// cel definition
typedef struct S_CELL
{
	long iRow;
	long iCol;
} CELL, *LPCELL;


typedef struct S_CELLPOS
{
	CELL cell;
	RECT rect;
} RLCELLPOS, *LPRLCELLPOS;


typedef struct S_CELLCLR
{
	COLORREF		clrBackground;
	COLORREF		clrText;
	COLORREF		clrLBorder;
	COLORREF		clrRBorder;
	COLORREF		clrUBorder;
	COLORREF		clrDBorder;
} RLCELLCLR, *LPRLCELLCLR;



// cell definition - used in DrawGridProc
typedef struct S_RLCELL
{
	RLCELLPOS		pos;
	RLCELLCLR		clrs;
	HFONT			hFont;
	unsigned long	iGridMode;	// LMB_* flags
	unsigned int	iState;		// RLS_* states
	tstring			sText;
	UINT			iFormat;	// DT_ (DrawText flags)
	short			nMargin;

} RLCELL, *LPRLCELL;

typedef const LPRLCELL LPCLRCELL;

// flags 
#define LMB_SAMEROWSHEIGHT	(0x001)	// all rows have the same height
#define LMB_FILLLASTCOL		(0x002)	// last column expanded to match window length
#define LMB_DRAWVLINES		(0x004)	// draw vertical lines 
#define LMB_DRAWHLINES		(0x008)	// draw horizontal lines 
#define LMB_DRAWGRID		(LMB_DRAWHLINES | LMB_DRAWVLINES)	// draw all grid 
#define LMB_DRAWTRANSP		(0x010)	// draw transparent cell grid (ignore row color)
#define LMB_SMOOTHSCROLL	(0x020)	// scrolling by pixel (not column)
#define LMB_CONSTROWS		(0x040)	// cannot change row size
#define LMB_CONSTCOLS		(0x080)	// cannot change column size
#define LMB_CONSTGRIDSIZE	(LMB_CONSTROWS | LMB_CONSTCOLS)	// cannot change column and row size
#define LMB_NOTOOLTIPS		(0x100)	// not showing tooltipis
#define LMB_LEFTALIGN		(0x200)	// left align of cell text (default - center)
#define LMB_RIGHTALIGN		(0x400)	// right align of cell text (default - center)
#define LMB_DRAWCELLBORDER	(0x800)	// draws cell border
#define LMB_MOVEONUP		(0x1000)	// moves selection on LeftButtonUp (not down)
#define LMB_EDITABLE		(0x2000)	// editable grid

// grid states
#define RLS_FOCUSED		(0x01)		// cell focused
#define	RLS_SELECTED	(0x02)		// cell selected
#define	RLS_DRAWBK		(0x04)		// is drawing bk for cell necessery
#define	RLS_FIXED		(0x08)		// cell fixed



typedef struct S_RLGRID
{
	BOOL			bVert;			// TRUE - vertical
	long			iRowCol;		// for row or column
	RECT			rect;			// rect for grid
	HPEN			hPen;			// pen for drawing lines
} RLGRID, *LPRLGRID;


typedef BOOL (*RLISTDATAPROC)(void* a_pObj, long a_iRow, long a_iCol, const TCHAR** a_ppData);
typedef long (*RLISTCOUNTPROC)(void* a_pObj);
typedef BOOL (*RLISTDRAWPROC)(HWND a_hWnd, HDC a_hDC, void* a_pObj, LPRLCELL a_pCell);
typedef BOOL (*RLISTGRIDPROC)(HWND a_hWnd, HDC a_hDC, void* a_pObj, LPRLGRID a_pGrid);
typedef BOOL (*RLISTDRAWBKPROC)(HWND a_hWnd, HDC a_hDC, void* a_pObj, LPRECT a_pRect);


// control messages
// adding column, WPARAM not used, LPARAM = LPRLCCOLDEF
#define RLCM_SETCOL		(WM_USER + 1001)	

// mode setting, WPARAM = 0 - unset flags, 1 - set flags, LPARAM - flags
#define RLCM_SETMODE	(WM_USER + 1002)

// data function setting, WPARAM - any pointer to data, LPARAM - RLISTDATAPROC
#define RLCM_SETDATAPROC	(WM_USER + 1003)

// count function setting, WPARAM - any pointer to data, LPARAM - RLISTCOUNTPROC
#define RLCM_SETCOUNTPROC	(WM_USER + 1004)

// sets function for drawing cell, WPARAM - any pointer to data, LPARAM - RLISTDRAWCELL
#define RLCM_SETDRAWPROC	(WM_USER + 1005)

// sets function for drawing grid, WPARAM - any pointer to data, LPARAM - RLISTDRAWCELL
#define RLCM_SETGRIDPROC	(WM_USER + 1006)

// sets grid colors, WPARAM - color index (defined below), LPARAM - RGB, returns RGB
#define RLCM_SETCOLOR	(WM_USER + 1007)

// gets grid colors, WPARAM - color index (defined below), LPARAM - ignored, returns RGB
#define RLCM_GETCOLOR	(WM_USER + 1008)

// sets number of fixed columns, WPARAM = 1 or rows WPARAM = 0, LPARAM - number of fixed columns
#define RLCM_SETFIXED	(WM_USER + 1009)

// gets number of fixed columns WPARAM = 1 or rows WPARAM = 0, LPARAM - unused
#define RLCM_GETFIXED	(WM_USER + 1010)

// sets row height WPARAM = row number (-1 - default row height), LPARAM - height
#define RLCM_SETROWHEIGHT	(WM_USER + 1011)

// sets column width WPARAM = col number, LPARAM - width
#define RLCM_SETCOLWIDTH	(WM_USER + 1012)

// gets row height WPARAM = row number (-1 - default row height)
#define RLCM_GETROWHEIGHT	(WM_USER + 1013)

// gets column width WPARAM = col number
#define RLCM_GETCOLWIDTH	(WM_USER + 1014)

// returns rows count
#define RLCM_ROWSCOUNT		(WM_USER + 1015)

// returns columns count
#define RLCM_COLSCOUNT		(WM_USER + 1016)

// ends cell editing WPARAM = 0 cancel, not 0 accept value, LPARAM not used
#define RLCM_EDITEND		(WM_USER + 1017)

// sets tooltip WPARAM not used, LPARAM LPCTSTR - tooltip
#define RLCM_SETTOOLTIP		(WM_USER + 1018)

// gets tooltip WPARAM UINT size of buffer, LPARAM LPTSTR - tooltip. Returns TCHAR's copied
#define RLCM_GETTOOLTIP		(WM_USER + 1019)

// refreshes control - use after changing data WPARAM unused, LPARAM unused.
#define RLCM_REFRESH		(WM_USER + 1020)

// sets std font WPARAM not used, LPARAM HFONT - font
#define RLCM_SETFONT		(WM_USER + 1021)

// gets font WPARAM not used, LPARAM not used. Returns HFONT
#define RLCM_GETFONT		(WM_USER + 1022)

// sets cell text margin WPARAM not used, LPARAM short - margin
#define RLCM_SETTEXTMARGIN	(WM_USER + 1023)

// gets cell text margin WPARAM not used, LPARAM not used. Returns short
#define RLCM_GETTEXTMARGIN	(WM_USER + 1024)

// gets selected row WPARAM = not used, LPARAM = not used, Returns selected row
#define RLCM_GETSELECTED	(WM_USER + 1025)

// sets selected row WPARAM = not used, LPARAM = row, 
#define RLCM_SETSELECTED	(WM_USER + 1026)

// invalidates row WPARAM = unused, LPARAM = long row
#define RLCM_INVALIDATEROW	(WM_USER + 1027)

// invalidates cell WPARAM = long row, LPARAM = long col
#define RLCM_INVALIDATECELL	(WM_USER + 1028)

// starts edit cell WPARAM = long row, LPARAM = long col
#define RLCM_EDITCELL		(WM_USER + 1029)

// gets cursor position WPARAM = not used, LPARAM = CELL* cell under cursor
#define RLCM_GETHOVERCELL	(WM_USER + 1030)

// invalidates col WPARAM = unused, LPARAM = long col
#define RLCM_INVALIDATECOL	(WM_USER + 1031)

// sets function for drawing background, WPARAM - any pointer to data, LPARAM - pointer to RLISTDRAWBKPROC
#define RLCM_SETDRAWBKPROC	(WM_USER + 1032)

// colors indexes
// text
#define RLC_TEXT_CELLACT			1	// normal cell, focused ctrl, not selected cell
#define RLC_TEXT_CELLACTSEL			2	// normal cell, focused ctrl, selected cell
#define RLC_TEXT_CELLNOACTNOSEL		3	// normal cell, not focused ctrl, not selected cell
#define RLC_TEXT_CELLNOACTSEL		4	// normal cell, not focused ctrl, selected cell
#define RLC_TEXT_FIXEDACT			5	// fixed cell, focused ctrl, not selected cell
#define RLC_TEXT_FIXEDACTSEL		6	// fixed cell, focused ctrl, selected cell
#define RLC_TEXT_FIXEDNOACTNOSEL	7	// fixed cell, not focused ctrl, not selected cell
#define RLC_TEXT_FIXEDNOACTSEL		8	// fixed cell, not focused ctrl, selected cell

// cell
#define RLC_CELL_CELLACT			9	// normal cell, focused ctrl, not selected cell
#define RLC_CELL_CELLACTSEL			10	// normal cell, focused ctrl, selected cell
#define RLC_CELL_CELLNOACTNOSEL		11	// normal cell, not focused ctrl, not selected cell
#define RLC_CELL_CELLNOACTSEL		12	// normal cell, not focused ctrl, selected cell
#define RLC_CELL_FIXEDACT			13	// fixed cell, focused ctrl, not selected cell
#define RLC_CELL_FIXEDACTSEL		14	// fixed cell, focused ctrl, selected cell
#define RLC_CELL_FIXEDNOACTNOSEL	15	// fixed cell, not focused ctrl, not selected cell
#define RLC_CELL_FIXEDNOACTSEL		16	// fixed cell, not focused ctrl, selected cell

// cell borders left
#define RLC_LBOR_CELLACT			17	// normal cell, focused ctrl, not selected cell
#define RLC_LBOR_CELLACTSEL			18	// normal cell, focused ctrl, selected cell
#define RLC_LBOR_CELLNOACTNOSEL		19	// normal cell, not focused ctrl, not selected cell
#define RLC_LBOR_CELLNOACTSEL		20	// normal cell, not focused ctrl, selected cell
#define RLC_LBOR_FIXEDACT			21	// fixed cell, focused ctrl, not selected cell
#define RLC_LBOR_FIXEDACTSEL		22	// fixed cell, focused ctrl, selected cell
#define RLC_LBOR_FIXEDNOACTNOSEL	23	// fixed cell, not focused ctrl, not selected cell
#define RLC_LBOR_FIXEDNOACTSEL		24	// fixed cell, not focused ctrl, selected cell

// cell borders rigth
#define RLC_RBOR_CELLACT			25	// normal cell, focused ctrl, not selected cell
#define RLC_RBOR_CELLACTSEL			26	// normal cell, focused ctrl, selected cell
#define RLC_RBOR_CELLNOACTNOSEL		27	// normal cell, not focused ctrl, not selected cell
#define RLC_RBOR_CELLNOACTSEL		28	// normal cell, not focused ctrl, selected cell
#define RLC_RBOR_FIXEDACT			29	// fixed cell, focused ctrl, not selected cell
#define RLC_RBOR_FIXEDACTSEL		30	// fixed cell, focused ctrl, selected cell
#define RLC_RBOR_FIXEDNOACTNOSEL	31	// fixed cell, not focused ctrl, not selected cell
#define RLC_RBOR_FIXEDNOACTSEL		32	// fixed cell, not focused ctrl, selected cell

// cell borders up
#define RLC_UBOR_CELLACT			33	// normal cell, focused ctrl, not selected cell
#define RLC_UBOR_CELLACTSEL			34	// normal cell, focused ctrl, selected cell
#define RLC_UBOR_CELLNOACTNOSEL		35	// normal cell, not focused ctrl, not selected cell
#define RLC_UBOR_CELLNOACTSEL		36	// normal cell, not focused ctrl, selected cell
#define RLC_UBOR_FIXEDACT			37	// fixed cell, focused ctrl, not selected cell
#define RLC_UBOR_FIXEDACTSEL		38	// fixed cell, focused ctrl, selected cell
#define RLC_UBOR_FIXEDNOACTNOSEL	39	// fixed cell, not focused ctrl, not selected cell
#define RLC_UBOR_FIXEDNOACTSEL		40	// fixed cell, not focused ctrl, selected cell

// cell borders down
#define RLC_DBOR_CELLACT			41	// normal cell, focused ctrl, not selected cell
#define RLC_DBOR_CELLACTSEL			42	// normal cell, focused ctrl, selected cell
#define RLC_DBOR_CELLNOACTNOSEL		43	// normal cell, not focused ctrl, not selected cell
#define RLC_DBOR_CELLNOACTSEL		44	// normal cell, not focused ctrl, selected cell
#define RLC_DBOR_FIXEDACT			45	// fixed cell, focused ctrl, not selected cell
#define RLC_DBOR_FIXEDACTSEL		46	// fixed cell, focused ctrl, selected cell
#define RLC_DBOR_FIXEDNOACTNOSEL	47	// fixed cell, not focused ctrl, not selected cell
#define RLC_DBOR_FIXEDNOACTSEL		48	// fixed cell, not focused ctrl, selected cell

// background
#define RLC_BK_ACT					49	// focused ctrl
#define RLC_BK_NOACT				50	// not focused ctrl

// grid
#define RLC_GRID_ACT				51	// focused ctrl
#define RLC_GRID_NOACT				52	// not focused ctrl



// notification messages
// can change row
#define RLCN_SELCHANGING	(WM_USER + 1001)

// after row has changed
#define RLCN_SELCHANGED	(WM_USER + 1002)


// selection notification message structure
typedef struct S_NMSEL
{
	NMHDR nmhdr;
	long  iCurrRow;
	long  iNewRow;
} RLCNMSEL, *LPRLCNMSEL;


// mouse notification
#define RLCN_LBUTTONDOWN (WM_USER + 1003)
#define RLCN_LBUTTONUP   (WM_USER + 1004)
#define RLCN_LDBLCLICK	 (WM_USER + 1005)
#define RLCN_MOUSECELLENTER	 (WM_USER + 1006)
#define RLCN_MOUSECELLLEAVE	 (WM_USER + 1007)

// mouse notification message structure
typedef struct S_NMMOUSE
{
	NMHDR nmhdr;
	POINT pt;
	CELL  cell;
} RLCNMMOUSE, *LPRLCNMMOUSE;


#define RLCN_STARTEDIT	(WM_USER + 1008)
#define RLCN_ENDEDIT	(WM_USER + 1009)
// edit cell notification message structure
typedef struct S_NMEDIT
{
	NMHDR nmhdr;
	HWND  hwndEdit;
	CELL  cell;
	BOOL  bCommit;
	RECT  rectEdit;
} RLCNMEDIT, *LPRLCNMEDIT;

// focus notification (NMCHAR)
#define RLCN_SETFOCUS	(WM_USER + 1010)
#define RLCN_KILLFOCUS	(WM_USER + 1011)

// keyboard notification (NMKEY & NMCHAR)
#define RLCN_KEYUP		(WM_USER + 1012)

// selection notification (RLCNMSELECTED)
#define RLCN_SELECTED	(WM_USER + 1013)
typedef struct S_NMSELECTED
{
	NMHDR nmhdr;
	long  iRow;
} RLCNMSELECTED, *LPRLCNMSELECTED;


// exported functions and macros
extern RCOMMON_API BOOL RListCtrl_DrawCell(HWND a_hWnd, HDC a_hDC, void* a_pObj, LPRLCELL a_pCell);
extern RCOMMON_API BOOL RListCtrl_DrawBk(HWND a_hWnd, HDC a_hDC, void* a_pObj, LPRECT a_pRectInv);
extern RCOMMON_API BOOL RListCtrl_DrawGrid(HWND a_hWnd, HDC a_hDC, void* a_pObj, LPRLGRID a_pGrid);

extern RCOMMON_API ATOM RListCtrl_Register();
extern RCOMMON_API LRESULT CALLBACK RListCtrl_WndProc(HWND, UINT, WPARAM, LPARAM);
extern RCOMMON_API HWND RListCtrl_Create(LPTSTR a_sTitle, DWORD a_iStyle, int a_x, int	a_y, int a_iWidth, int a_iHeight, 
	HWND a_hWndParent, HMENU a_hMenu, LPVOID a_lpParam);
extern RCOMMON_API LRESULT CALLBACK RListCtrl_EditFilter(WNDPROC a_wndprocDef, HWND a_hWnd, UINT a_iMsg, WPARAM a_wParam, LPARAM a_lParam);


#define RListCtrl_AddCol(a_hWnd, a_pCol) (::SendMessage(a_hWnd, RLCM_SETCOL, 0, (LPARAM)a_pCol))
#define RListCtrl_SetCol(a_hWnd, a_pCol) (::SendMessage(a_hWnd, RLCM_SETCOL, 0, (LPARAM)a_pCol))
#define RListCtrl_SetDataProc(a_hWnd, a_pVoid, a_pProc) (::SendMessage(a_hWnd, RLCM_SETDATAPROC, (WPARAM)a_pVoid, (LPARAM)a_pProc))
#define RListCtrl_SetCountProc(a_hWnd, a_pVoid, a_pProc) (::SendMessage(a_hWnd, RLCM_SETCOUNTPROC, (WPARAM)a_pVoid, (LPARAM)a_pProc))
#define RListCtrl_SetDrawProc(a_hWnd, a_pVoid, a_pProc) (::SendMessage(a_hWnd, RLCM_SETDRAWPROC, (WPARAM)a_pVoid, (LPARAM)a_pProc))
#define RListCtrl_SetDrawBkProc(a_hWnd, a_pVoid, a_pProc) (::SendMessage(a_hWnd, RLCM_SETDRAWBKPROC, (WPARAM)a_pVoid, (LPARAM)a_pProc))
#define RListCtrl_SetGridProc(a_hWnd, a_pVoid, a_pProc) (::SendMessage(a_hWnd, RLCM_SETGRIDPROC, (WPARAM)a_pVoid, (LPARAM)a_pProc))
#define RListCtrl_GetFixedRows(a_hWnd) (::SendMessage(a_hWnd, RLCM_GETFIXED, 0, 0))
#define RListCtrl_GetFixedCols(a_hWnd) (::SendMessage(a_hWnd, RLCM_GETFIXED, 1, 0))
#define RListCtrl_SetFixedRows(a_hWnd, a_iRows) (::SendMessage(a_hWnd, RLCM_SETFIXED, 1, a_iRows))
#define RListCtrl_SetFixedCols(a_hWnd, a_iCols) (::SendMessage(a_hWnd, RLCM_SETFIXED, 0, a_iCols))
#define RListCtrl_GetColor(a_hWnd, a_iIdx) ( ::SendMessage(a_hWnd, RLCM_GETCOLOR, a_iIdx, 0))
#define RListCtrl_SetColor(a_hWnd, a_iIdx, a_cr) ( ::SendMessage(a_hWnd, RLCM_SETCOLOR, a_iIdx, a_cr))
#define RListCtrl_SetMode(a_hWnd, a_iMode, a_bSet) ( ::SendMessage(a_hWnd, RLCM_SETMODE, a_bSet, a_iMode))
#define RListCtrl_SetRowHeight(a_hWnd, a_iRow, a_iHeight) (::SendMessage(a_hWnd, RLCM_SETROWHEIGHT, a_iRow, a_iHeight))
#define RListCtrl_SetColWidth(a_hWnd, a_iCol, a_iWidth) (::SendMessage(a_hWnd, RLCM_SETCOLWIDTH, a_iCol, a_iWidth))
#define RListCtrl_GetRowHeight(a_hWnd, a_iRow) (::SendMessage(a_hWnd, RLCM_GETROWHEIGHT, a_iRow, 0))
#define RListCtrl_GetColWidth(a_hWnd, a_iCol) (::SendMessage(a_hWnd, RLCM_GETCOLWIDTH, a_iCol, 0))
#define RListCtrl_GetRowsCount(a_hWnd) (::SendMessage(a_hWnd, RLCM_ROWSCOUNT, 0, 0))
#define RListCtrl_GetColsCount(a_hWnd) (::SendMessage(a_hWnd, RLCM_COLSCOUNT, 0, 0))
#define RListCtrl_SetTooltip(a_hWnd, a_sTooltip) (::SendMessage(a_hWnd, RLCM_SETTOOLTIP, 0, (LPARAM)a_sTooltip))
#define RListCtrl_GetTooltip(a_hWnd, a_sTooltip, a_iSize) (::SendMessage(a_hWnd, RLCM_GETTOOLTIP, (WPARAM)a_iSize, (LPARAM)a_sTooltip))
#define RListCtrl_Refresh(a_hWnd) (::SendMessage(a_hWnd, RLCM_REFRESH, 0, 0L))
#define RListCtrl_SetFont(a_hWnd, a_hFont) (::SendMessage(a_hWnd, RLCM_SETFONT, 0, (LPARAM)(a_hFont)))
#define RListCtrl_GetFont(a_hWnd) ((HFONT)(::SendMessage(a_hWnd, RLCM_GETFONT, 0, 0L)))

#define RListCtrl_SetTextMargin(a_hWnd, a_nMargin) (::SendMessage(a_hWnd, RLCM_SETTEXTMARGIN, 0, (LPARAM)(a_nMargin)))
#define RListCtrl_GetTextMargin(a_hWnd) ((short)(::SendMessage(a_hWnd, RLCM_GETTEXTMARGIN, 0, 0L)))

#define RListCtrl_SetSelected(a_hWnd, a_iRow) (::SendMessage(a_hWnd, RLCM_SETSELECTED, 0, (LPARAM)(a_iRow)))
#define RListCtrl_GetSelected(a_hWnd) (::SendMessage(a_hWnd, RLCM_GETSELECTED, 0, 0L))

#define RListCtrl_InvalidateRow(a_hWnd, a_iRow) (::SendMessage(a_hWnd, RLCM_INVALIDATEROW, 0, (LPARAM)(a_iRow)))
#define RListCtrl_InvalidateCol(a_hWnd, a_iCol) (::SendMessage(a_hWnd, RLCM_INVALIDATECOL, 0, (LPARAM)(a_iCol)))
#define RListCtrl_InvalidateCell(a_hWnd, a_iRow, a_iCol) (::SendMessage(a_hWnd, RLCM_INVALIDATECELL, a_iRow, a_iCol))

#define RListCtrl_EditCell(a_hWnd, a_iRow, a_iCol) (::SendMessage(a_hWnd, RLCM_EDITCELL, a_iRow, a_iCol))

#define RListCtrl_GetHoverCell(a_hWnd, a_pCell) (::SendMessage(a_hWnd, RLCM_GETHOVERCELL, 0, (LPARAM)(a_pCell)))

