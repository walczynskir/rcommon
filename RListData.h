// RListData.h: interface for the RListData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RLISTDATA_H__528BBE2C_02A1_445C_A8C6_23417D1EE325__INCLUDED_)
#define AFX_RLISTDATA_H__528BBE2C_02A1_445C_A8C6_23417D1EE325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#pragma warning(disable: 4786)

#include <vector>
#include "RString.h"
#include <bitset>
#include <map>
#include "RListCtrl.h"


enum EColorType 
{
	E_CT_CELLACT,
	E_CT_CELLACTSEL,
	E_CT_CELLNOACTNOSEL,
	E_CT_CELLNOACTSEL,
	E_CT_FIXEDACT,
	E_CT_FIXEDACTSEL,
	E_CT_FIXEDNOACTNOSEL,
	E_CT_FIXEDNOACTSEL
};




class RColumn
{
public:
	RColumn(tstring a_sName, long a_iLeft, long a_iRight) { m_sName = a_sName, m_iLeft = a_iLeft ; m_iRight = a_iRight; };
	long GetLeft() const { return m_iLeft; };
	long GetRight() const { return m_iRight; };
	long GetWidth() const { return m_iRight - m_iLeft; };
	void SetLeft(long a_iLeft) { m_iLeft = a_iLeft; };
	void SetRight(long a_iRight) { m_iRight = a_iRight; };
	const tstring GetColName() const { return m_sName; };

private:
	long m_iLeft;
	long m_iRight;
	tstring m_sName;
};



class RCell : private S_CELL
{
public:
	RCell() { bNull = TRUE; iRow = -1; iCol = -1; };
	RCell(long a_iRow, long a_iCol) { SetCell(a_iRow, a_iCol); };
	RCell(const CELL& a_cell) { SetCell(a_cell.iRow, a_cell.iCol); };
	void SetNull() { bNull = TRUE; iRow = -1; iCol = -1; } ;
	BOOL IsNull() const { return bNull; };
	void SetCell(long a_iRow, long a_iCol) 
	{ 
		if ((a_iRow < 0) || (a_iCol < 0))
		{
			SetNull();
		}
		else
		{
			iRow = a_iRow; 
			iCol = a_iCol; 
			bNull = FALSE; 
		}
	};
	long GetRow() const { return iRow; };
	long GetCol() const { return iCol; };
	bool Equal(const RCell& a_cell) const 
	{ 
		if (IsNull() && a_cell.IsNull())
		{
			return true;
		}
		else if (IsNull() && !a_cell.IsNull())
		{
			return false;
		}
		else if (!IsNull() && a_cell.IsNull())
		{
			return false;
		}
		else
		{
			return ((this->iRow == a_cell.iRow) && (this->iCol == a_cell.iCol)); 
		}
	}

private:
	bool bNull;
};

typedef std::vector<RColumn> COLINFO;
typedef COLINFO::const_iterator RCOLUMNS_CONSTITERATOR;
typedef COLINFO::iterator RCOLUMNS_ITERATOR;
typedef std::map<long, long> ROWINFO;

// type for storing control modes (outer and inner)
typedef std::bitset<32> LISTMODE;
typedef std::bitset<3>  INNERMODE;


#define IM_RECTCALCULATED	0x01	
#define IM_TRACKINGCOL		0x02
#define IM_TRACKINGROW		0x04



class RListData  
{
public:
	RListData();
	virtual ~RListData();


	HWND GetCellTip() const { return m_hWndCellTip; };
	void SetCellTip(HWND a_hWnd) { m_hWndCellTip = a_hWnd; };
	long GetFixedColsWidth() const;
	long GetFixedRowsHeight() const;

	long GetRowHeight(long a_iRow) const;
	void SetRowHeight(long a_iRow, long a_iHeight);
	void SetAllRowsHeight(long a_iHeight);

	BOOL IsTrackingCol() const { return m_bitInnerMode.to_ulong() & IM_TRACKINGCOL; };
	void SetTrackingCol(BOOL a_bTracked) { a_bTracked ? m_bitInnerMode |= IM_TRACKINGCOL : m_bitInnerMode &= (IM_TRACKINGCOL ^ 0xFFFF); } ;

	BOOL IsTrackingRow() const { return m_bitInnerMode.to_ulong() & IM_TRACKINGROW; };
	void SetTrackingRow(BOOL a_bTracked) { a_bTracked ? m_bitInnerMode |= IM_TRACKINGROW : m_bitInnerMode &= (IM_TRACKINGROW ^ 0xFFFF); } ;

	long GetTrackedColRow() const { return m_iTrackedColRow; };
	void SetTrackedColRow(long a_iTrackedColRow) { m_iTrackedColRow = a_iTrackedColRow; } ;

	long GetTrackPos() const { return m_xTrackPos; };
	void SetTrackPos(long a_xTrackPos) { m_xTrackPos = a_xTrackPos; } ;

	long  GetFirstVisRow() const { return m_iFirstVisRow; };
	void  SetFirstVisRow(long a_iRow) { ASSERT(a_iRow >= 0); m_iFirstVisRow = a_iRow; } ;

	long  GetFirstVisCol() const { return m_iFirstVisCol; };
	void  SetFirstVisCol(long a_iCol) { m_iFirstVisCol = a_iCol; } ;

	long  GetFixedCols() const { return m_iFixedCols; };
	void  SetFixedCols(long a_iCols);
	long  GetFixedRows() const { return m_iFixedRows; };
	void  SetFixedRows(long a_iRows);

	long  GetSelRow() const { return m_iSelRow; };
	void  SetSelRow(long a_iRow) { m_iPrevSelRow = m_iSelRow; m_iSelRow = a_iRow; } ;

	long  GetPrevSelRow() const { return m_iPrevSelRow; };

	COLORREF SetColor(UINT a_iIdx, COLORREF a_cr);
	COLORREF GetColor(UINT a_iIdx) const;

	void GetColors(EColorType a_enColor, LPRLCELLCLR a_pColors) const;

	void	SetDataProc(RLISTDATAPROC a_proc, void* a_pObj) { m_procData = a_proc; m_pDataObj = a_pObj; };
	BOOL	GetData(long a_iRow, long a_iCol, const TCHAR** a_ppData) const;

	void	SetCountProc(RLISTCOUNTPROC a_proc, void* a_pObj) { m_procCount = a_proc; m_pCountObj = a_pObj; };
	long	GetRowsCount() const { long l_iRows = (m_procCount == NULL) ? 0 : m_procCount(m_pCountObj); ASSERT(l_iRows >= 0); return l_iRows; };

	void	SetDrawProc(RLISTDRAWPROC a_proc, void* a_pObj) { m_procDraw = a_proc; m_pDrawObj = a_pObj; };
	void	SetDrawBkProc(RLISTDRAWBKPROC a_proc, void* a_pObj) { m_procDrawBk = a_proc; m_pDrawBkObj = a_pObj; };
	void	SetGridProc(RLISTGRIDPROC a_proc, void* a_pObj) { m_procGrid = a_proc; m_pGridObj = a_pObj; };

	BOOL	DrawCell(HWND a_hWnd, HDC a_hDC, LPRLCELL a_pCell) 
		{ return (m_procDraw == NULL) ? FALSE : m_procDraw(a_hWnd, a_hDC, m_pDrawObj, a_pCell); };
	BOOL	DrawBk(HWND a_hWnd, HDC a_hDC, LPRECT a_pRect) 
		{ return (m_procDrawBk == NULL) ? FALSE : m_procDrawBk(a_hWnd, a_hDC, m_pDrawBkObj, a_pRect); };
	BOOL	DrawGrid(HWND a_hWnd, HDC a_hDC, LPRLGRID a_pGrid) 
		{ return (m_procGrid == NULL) ? FALSE : m_procGrid(a_hWnd, a_hDC, m_pDrawObj, a_pGrid); };

	BOOL  GetRectCalculated() const { return m_bitInnerMode.to_ulong() & IM_RECTCALCULATED; };
	void  SetRectCalculated(BOOL a_bCalc) { a_bCalc ? m_bitInnerMode |= IM_RECTCALCULATED : m_bitInnerMode &= (IM_RECTCALCULATED ^ 0xFFFF); } ;

	void			AddColumn(tstring a_sName, long a_iLen);
	long			GetColumnsCount() const { return static_cast<long>(m_vctColumns.size()); };
	const RColumn&  GetColumn(long a_iCol) const { return m_vctColumns[a_iCol]; };
	void			SetColumnWidth(long a_iCol, long a_iWidth);

	void	SetTrackLineWidth(short a_dx) { m_dxTrackLine = a_dx; };
	short	GetTrackLineWidth() const { return m_dxTrackLine; };

	void	SetTrackLineInverted(BOOL a_bInverted) { m_bTrackLineInverted = a_bInverted; };
	BOOL	GetTrackLineInverted() const{ return m_bTrackLineInverted; };

	void		SetTrackLineColor(COLORREF a_clr) { m_clrTrackLine = a_clr; };
	COLORREF	GetTrackLineColor() const { return m_clrTrackLine; };

	void SetFlags(long a_iFlags) { m_bitMode |= a_iFlags ; };
	void UnsetFlags(long a_iFlags) { m_bitMode &= ~a_iFlags; };
	unsigned long GetFlags() const { return m_bitMode.to_ulong(); };
	BOOL TestFlags(long a_iFlags) const { return (GetFlags() & a_iFlags); };

	HFONT	 GetFont() const { return m_hFont; };
	void	 SetFont(HFONT a_hFont);

	void SetMouseCell(long a_iRow, long a_iCol) { m_cellMouse.SetCell(a_iRow, a_iCol); };
	void SetMouseCellNull() { m_cellMouse.SetNull(); };
	const RCell& GetMouseCell() const { return m_cellMouse; };

	void SetCellPosEdit(const RLCELLPOS& a_cellposEdit) { m_cellposEdit = a_cellposEdit; };
	RLCELLPOS GetCellPosEdit() const { return m_cellposEdit; };

	void SetWndEdit(HWND a_hwndEdit) { m_hwndEdit = a_hwndEdit; };
	HWND GetWndEdit() const { return m_hwndEdit; };
	BOOL IsEditCell() const { return m_hwndEdit != NULL; };

	void SetTooltip(LPCTSTR a_sTooltip);
	LPTSTR GetTooltip() const { return m_psTooltip; };

	void SetTooltipWnd(HWND a_hTooltipWnd) { m_hTooltipWnd = a_hTooltipWnd; };
	HWND GetTooltipWnd() const { return m_hTooltipWnd; };

	bool IsTrackingMouse() const { return m_bTrackingMouse; };
	void SetTrackingMouse(bool a_bTracking) { m_bTrackingMouse = a_bTracking; };

	void SetTextMargin(short a_nMargin) { m_nMargin = a_nMargin ; };
	short GetTextMargin() const { return m_nMargin; };

	void SetDefRowHeight(long a_iHeight) { m_iDefRowHeight = a_iHeight ; };
	long GetDefRowHeight() const { return m_iDefRowHeight; };

private:
	COLORREF m_arColors[52];

private:

	void SetDefColors();
	// only for inner purposes
	long  m_iFirstVisRow;
	long  m_iFirstVisCol;

	INNERMODE m_bitInnerMode;

	long m_xTrackPos;
	long m_iTrackedColRow;

	// attributes of control (can be set)
	HWND m_hWndCellTip;

	long m_iFixedCols;
	long m_iFixedRows;

	long  m_iSelRow;
	long  m_iPrevSelRow;

	long  m_iDefHeaderHeight;
	long  m_iDefRowHeight;
	long  m_iDefLeftColWidth;

	COLINFO		m_vctColumns;
	ROWINFO		m_mapRows;

	LISTMODE m_bitMode;
	short    m_nMargin;

	RLISTDATAPROC	m_procData;
	void*			m_pDataObj;

	RLISTCOUNTPROC	m_procCount;
	void*			m_pCountObj;

	RLISTDRAWPROC	m_procDraw;
	void*			m_pDrawObj;
	RLISTDRAWBKPROC	m_procDrawBk;
	void*			m_pDrawBkObj;
	RLISTGRIDPROC	m_procGrid;
	void*			m_pGridObj;


	HFONT m_hFont;
	BOOL  m_bDeleteFont;

	short			m_dxTrackLine;
	COLORREF		m_clrTrackLine;
	BOOL			m_bTrackLineInverted;

	RCell			m_cellMouse;

	// edit mode 
	RLCELLPOS		m_cellposEdit;
	HWND			m_hwndEdit;

	// tooltip (for control not for single cell)
	LPTSTR			m_psTooltip;
	HWND			m_hTooltipWnd;

	bool			m_bTrackingMouse;
};

#endif // !defined(AFX_RLISTDATA_H__528BBE2C_02A1_445C_A8C6_23417D1EE325__INCLUDED_)
