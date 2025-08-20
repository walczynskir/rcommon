// RListData.cpp: implementation of the RListData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RListData.h"
#include <algorithm>
#include <string>
#include <tchar.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static const short c_nDefRowHeight = 20;
static const short c_dxTrackLine = 1;
static const COLORREF c_clrTrackLine = RGB(0, 0, 0);
static const short c_nTextMarginDflt = 2;



/*	---------------------------------------------------------------------------------------
	Konstruktor
*/
RListData::RListData()	
{
	SetFlags(LMB_DRAWGRID);
	SetTextMargin(c_nTextMarginDflt);

	SetDefColors();

#ifdef _WIN32_WCE
	LOGFONT l_lf;
	l_lf.lfHeight = -11;
	l_lf.lfWidth = 0;
	l_lf.lfEscapement = 0;
	l_lf.lfOrientation = 0;
	l_lf.lfWeight = FW_LIGHT;
	l_lf.lfItalic = 0;
	l_lf.lfUnderline = 0;
	l_lf.lfStrikeOut = 0;
	l_lf.lfCharSet = DEFAULT_CHARSET;
	l_lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	l_lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	l_lf.lfQuality = DEFAULT_QUALITY;
	l_lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(l_lf.lfFaceName, ArraySize(l_lf.lfFaceName), _T("MS Sans Serif"));

	m_hFont = ::CreateFontIndirect(&l_lf);
#else
	m_hFont = ::CreateFont(-11, 0,	0, 0, FW_LIGHT, 0, 0, 0, DEFAULT_CHARSET, 
		0, 0, DEFAULT_QUALITY, 0, _T("MS Sans Serif"));
#endif
	m_bDeleteFont = TRUE;

	m_procData = NULL;
	m_pDataObj = NULL;
	m_procCount = NULL;
	m_pCountObj = NULL;
	m_psTooltip = NULL;
	m_hTooltipWnd = NULL;
	m_hWndCellTip = NULL;

	SetRectCalculated(FALSE);

	m_iDefRowHeight = c_nDefRowHeight;
	m_iFixedRows = 1;
	m_iFixedCols = 1;
	SetFirstVisRow(GetFixedRows());
	SetFirstVisCol(GetFixedCols());

	SetSelRow(GetFirstVisRow());
	m_iPrevSelRow = GetSelRow();

	SetTrackingCol(FALSE);
	SetTrackingRow(FALSE);
	SetTrackPos(0);
	SetTrackedColRow(-1);

	SetTrackLineWidth(c_dxTrackLine);
	SetTrackLineColor(c_clrTrackLine);
	SetTrackLineInverted(TRUE);
	SetWndEdit(NULL);
	SetMouseCellNull();
	SetTrackingMouse(false);
}


/*	---------------------------------------------------------------------------------------
	Destruktor
*/
RListData::~RListData()
{
	if (m_bDeleteFont)
	{
		::DeleteObject(m_hFont);
	}
	if (m_psTooltip != NULL)
	{
		delete[] m_psTooltip;
	}
}


/*	---------------------------------------------------------------------------------------
	Dodanie kolumny
*/
void
RListData::AddColumn(
	tstring a_sName, 
	long a_iLen
	)
{
	long l_iLeft;
	long l_iSize = static_cast<long>(m_vctColumns.size());

	if (l_iSize == 0)
	{
		l_iLeft = 0;
	}
	else
	{
		const RColumn& l_colPrev = m_vctColumns[l_iSize - 1];
		l_iLeft = l_colPrev.GetRight();
	}

	RColumn l_col(a_sName, l_iLeft, l_iLeft + a_iLen);

	m_vctColumns.push_back(l_col);
}


//	---------------------------------------------------------------------------------------
//	sets column width
//
void			
RListData::SetColumnWidth(
	long a_iCol, 
	long a_iWidth
	)
{
	RCOLUMNS_ITERATOR l_itStart = m_vctColumns.begin() + a_iCol;
	RCOLUMNS_ITERATOR l_itEnd = m_vctColumns.end();

	long l_iRightNew = (*l_itStart).GetLeft() + a_iWidth;
	(*l_itStart).SetRight(l_iRightNew);
	l_itStart++;

	for (; l_itStart != l_itEnd; ++l_itStart)
	{
		long l_iWidth = (*l_itStart).GetRight() - (*l_itStart).GetLeft();
		(*l_itStart).SetLeft(l_iRightNew);
		l_iRightNew = (*l_itStart).GetLeft() + l_iWidth;
		(*l_itStart).SetRight(l_iRightNew);
	}
}


//	---------------------------------------------------------------------------------------
//	sets color
//
COLORREF	//OUT previous color
RListData::SetColor(
	UINT a_iIdx, 
	COLORREF a_cr
	)
{
	ASSERT((a_iIdx > 0) && (a_iIdx <= (sizeof(m_arColors) / sizeof(m_arColors[0]))));

	COLORREF l_crPrev = m_arColors[a_iIdx];
	m_arColors[a_iIdx - 1] = a_cr;
	return l_crPrev;
}


//	---------------------------------------------------------------------------------------
//	returns color
//
COLORREF	//OUT previous color
RListData::GetColor(
	UINT a_iIdx
	) const
{
	ASSERT((a_iIdx > 0) && (a_iIdx <= (sizeof(m_arColors) / sizeof(m_arColors[0]))));
	return m_arColors[a_iIdx - 1];
}


//	---------------------------------------------------------------------------------------
//	Returns row height
//
long 
RListData::GetRowHeight(
	long a_iRow
	) const
{
	ROWINFO::const_iterator l_iter = m_mapRows.find(a_iRow);

	if (m_mapRows.empty() || (l_iter == m_mapRows.end()))
	{
		return m_iDefRowHeight;
	}
	return l_iter->second;
}


//	---------------------------------------------------------------------------------------
//	Sets row height
//
void  
RListData::SetRowHeight(
	long a_iRow, 
	long a_iHeight
	)
{
	m_mapRows[a_iRow] = a_iHeight;
}


//	---------------------------------------------------------------------------------------
//	Returns row height
//
void  
RListData::SetAllRowsHeight(
	long a_iHeight
	)
{
	// should be for_each algorithm, but didn't have time to check how to do this
	long l_iStart = GetFixedRows();
	long l_iRow;
	for (l_iRow = l_iStart; l_iRow < GetRowsCount(); l_iRow++)
	{
		SetRowHeight(l_iRow, a_iHeight);
	}
}


//	---------------------------------------------------------------------------------------
//	Returns fixed cols width
//
long  
RListData::GetFixedColsWidth() const
{
	long l_iCol;
	long l_iWidth = 0;
	for (l_iCol = 0; l_iCol < min(m_iFixedCols, static_cast<long>(m_vctColumns.size())); l_iCol++)
	{
		l_iWidth += m_vctColumns[l_iCol].GetWidth();
	}
	return l_iWidth;	
}


//	---------------------------------------------------------------------------------------
//	Returns fixed rows height
//
long  
RListData::GetFixedRowsHeight() const
{
	long l_iRow;
	long l_iHeight = 0;
	for (l_iRow = 0; l_iRow < m_iFixedRows; l_iRow++)
	{
		l_iHeight += GetRowHeight(l_iRow);
	}
	return l_iHeight;
}


//	---------------------------------------------------------------------------------------
//	Sets fixed cols
//
void  
RListData::SetFixedCols(
	long a_iCols
	)
{
	long l_iDiff = m_iFixedCols - a_iCols;
	m_iFixedCols = a_iCols; 
	if (l_iDiff > 0)
	{
		m_iFirstVisCol -= l_iDiff;
	}
	else if ((l_iDiff < 0) && (m_iFirstVisCol < m_iFixedCols))
	{
		m_iFirstVisCol = m_iFixedCols;
	}
}


//	---------------------------------------------------------------------------------------
//	Sets fixed rows
//
void  
RListData::SetFixedRows(
	long a_iRows
	)
{ 
	long l_iDiff = m_iFixedRows - a_iRows;
	m_iFixedRows = a_iRows; 
	if (l_iDiff > 0)
	{
		m_iFirstVisRow -= l_iDiff;
	}
	else if ((l_iDiff < 0) && (m_iFirstVisRow < m_iFixedRows))
	{
		m_iFirstVisRow = m_iFixedRows;
	}
	if (m_iSelRow < a_iRows)
	{
		m_iSelRow = a_iRows;
	}
}


//	---------------------------------------------------------------------------------------
//	Fills struct with List Colors
//
void 
RListData::GetColors(
	EColorType a_enColor,
	LPRLCELLCLR a_pColors
	) const
{
	switch (a_enColor)
	{
		case E_CT_CELLACT:
			a_pColors->clrBackground = GetColor(RLC_CELL_CELLACT);
			a_pColors->clrText = GetColor(RLC_TEXT_CELLACT);
			a_pColors->clrLBorder = GetColor(RLC_LBOR_CELLACT);
			a_pColors->clrRBorder = GetColor(RLC_RBOR_CELLACT);
			a_pColors->clrUBorder = GetColor(RLC_UBOR_CELLACT);
			a_pColors->clrDBorder = GetColor(RLC_DBOR_CELLACT);
			break;

		case E_CT_CELLACTSEL:
			a_pColors->clrBackground = GetColor(RLC_CELL_CELLACTSEL);
			a_pColors->clrText = GetColor(RLC_TEXT_CELLACTSEL);
			a_pColors->clrLBorder = GetColor(RLC_LBOR_CELLACTSEL);
			a_pColors->clrRBorder = GetColor(RLC_RBOR_CELLACTSEL);
			a_pColors->clrUBorder = GetColor(RLC_UBOR_CELLACTSEL);
			a_pColors->clrDBorder = GetColor(RLC_DBOR_CELLACTSEL);
			break;

		case E_CT_CELLNOACTNOSEL:
			a_pColors->clrBackground = GetColor(RLC_CELL_CELLNOACTNOSEL);
			a_pColors->clrText = GetColor(RLC_TEXT_CELLNOACTNOSEL);
			a_pColors->clrLBorder = GetColor(RLC_LBOR_CELLNOACTNOSEL);
			a_pColors->clrRBorder = GetColor(RLC_RBOR_CELLNOACTNOSEL);
			a_pColors->clrUBorder = GetColor(RLC_UBOR_CELLNOACTNOSEL);
			a_pColors->clrDBorder = GetColor(RLC_DBOR_CELLNOACTNOSEL);
			break;
	
		case E_CT_CELLNOACTSEL:
			a_pColors->clrBackground = GetColor(RLC_CELL_CELLNOACTSEL);
			a_pColors->clrText = GetColor(RLC_TEXT_CELLNOACTSEL);
			a_pColors->clrLBorder = GetColor(RLC_LBOR_CELLNOACTSEL);
			a_pColors->clrRBorder = GetColor(RLC_RBOR_CELLNOACTSEL);
			a_pColors->clrUBorder = GetColor(RLC_UBOR_CELLNOACTSEL);
			a_pColors->clrDBorder = GetColor(RLC_DBOR_CELLNOACTSEL);
			break;

		case E_CT_FIXEDACT:
			a_pColors->clrBackground = GetColor(RLC_CELL_FIXEDACT);
			a_pColors->clrText = GetColor(RLC_TEXT_FIXEDACT);
			a_pColors->clrLBorder = GetColor(RLC_LBOR_FIXEDACT);
			a_pColors->clrRBorder = GetColor(RLC_RBOR_FIXEDACT);
			a_pColors->clrUBorder = GetColor(RLC_UBOR_FIXEDACT);
			a_pColors->clrDBorder = GetColor(RLC_DBOR_FIXEDACT);
			break;

		case E_CT_FIXEDACTSEL:
			a_pColors->clrBackground = GetColor(RLC_CELL_FIXEDACTSEL);
			a_pColors->clrText = GetColor(RLC_TEXT_FIXEDACTSEL);
			a_pColors->clrLBorder = GetColor(RLC_LBOR_FIXEDACTSEL);
			a_pColors->clrRBorder = GetColor(RLC_RBOR_FIXEDACTSEL);
			a_pColors->clrUBorder = GetColor(RLC_UBOR_FIXEDACTSEL);
			a_pColors->clrDBorder = GetColor(RLC_DBOR_FIXEDACTSEL);
			break;

		case E_CT_FIXEDNOACTNOSEL:
			a_pColors->clrBackground = GetColor(RLC_CELL_FIXEDNOACTNOSEL);
			a_pColors->clrText = GetColor(RLC_TEXT_FIXEDNOACTNOSEL);
			a_pColors->clrLBorder = GetColor(RLC_LBOR_FIXEDNOACTNOSEL);
			a_pColors->clrRBorder = GetColor(RLC_RBOR_FIXEDNOACTNOSEL);
			a_pColors->clrUBorder = GetColor(RLC_UBOR_FIXEDNOACTNOSEL);
			a_pColors->clrDBorder = GetColor(RLC_DBOR_FIXEDNOACTNOSEL);
			break;
	
		case E_CT_FIXEDNOACTSEL:
			a_pColors->clrBackground = GetColor(RLC_CELL_FIXEDNOACTSEL);
			a_pColors->clrText = GetColor(RLC_TEXT_FIXEDNOACTSEL);
			a_pColors->clrLBorder = GetColor(RLC_LBOR_FIXEDNOACTSEL);
			a_pColors->clrRBorder = GetColor(RLC_RBOR_FIXEDNOACTSEL);
			a_pColors->clrUBorder = GetColor(RLC_UBOR_FIXEDNOACTSEL);
			a_pColors->clrDBorder = GetColor(RLC_DBOR_FIXEDNOACTSEL);
			break;
		default:
			ASSERT(FALSE);
	}
}


//	---------------------------------------------------------------------------------------
//	Sets default colors
//
void
RListData::SetDefColors()
{

	SetColor(RLC_CELL_CELLACT, ::GetSysColor(COLOR_WINDOW));
	SetColor(RLC_TEXT_CELLACT, ::GetSysColor(COLOR_BTNTEXT));
	SetColor(RLC_LBOR_CELLACT, ::GetSysColor(COLOR_WINDOW));
	SetColor(RLC_RBOR_CELLACT, ::GetSysColor(COLOR_WINDOW));
	SetColor(RLC_UBOR_CELLACT, ::GetSysColor(COLOR_WINDOW));
	SetColor(RLC_DBOR_CELLACT, ::GetSysColor(COLOR_WINDOW));

	SetColor(RLC_CELL_CELLACTSEL, ::GetSysColor(COLOR_HIGHLIGHT));
	SetColor(RLC_TEXT_CELLACTSEL, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
	SetColor(RLC_LBOR_CELLACTSEL, ::GetSysColor(COLOR_HIGHLIGHT));
	SetColor(RLC_RBOR_CELLACTSEL, ::GetSysColor(COLOR_HIGHLIGHT));
	SetColor(RLC_UBOR_CELLACTSEL, ::GetSysColor(COLOR_HIGHLIGHT));
	SetColor(RLC_DBOR_CELLACTSEL, ::GetSysColor(COLOR_HIGHLIGHT));

	SetColor(RLC_CELL_CELLNOACTNOSEL, ::GetSysColor(COLOR_WINDOW));
	SetColor(RLC_TEXT_CELLNOACTNOSEL, ::GetSysColor(COLOR_BTNTEXT));
	SetColor(RLC_LBOR_CELLNOACTNOSEL, ::GetSysColor(COLOR_WINDOW));
	SetColor(RLC_RBOR_CELLNOACTNOSEL, ::GetSysColor(COLOR_WINDOW));
	SetColor(RLC_UBOR_CELLNOACTNOSEL, ::GetSysColor(COLOR_WINDOW));
	SetColor(RLC_DBOR_CELLNOACTNOSEL, ::GetSysColor(COLOR_WINDOW));

	SetColor(RLC_CELL_CELLNOACTSEL, ::GetSysColor(COLOR_BTNFACE));
	SetColor(RLC_TEXT_CELLNOACTSEL, ::GetSysColor(COLOR_BTNTEXT));
	SetColor(RLC_LBOR_CELLNOACTSEL, ::GetSysColor(COLOR_BTNFACE));
	SetColor(RLC_RBOR_CELLNOACTSEL, ::GetSysColor(COLOR_BTNFACE));
	SetColor(RLC_UBOR_CELLNOACTSEL, ::GetSysColor(COLOR_BTNFACE));
	SetColor(RLC_DBOR_CELLNOACTSEL, ::GetSysColor(COLOR_BTNFACE));
 
	SetColor(RLC_CELL_FIXEDACT, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_TEXT_FIXEDACT, ::GetSysColor(COLOR_BTNTEXT));
	SetColor(RLC_LBOR_FIXEDACT, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_RBOR_FIXEDACT, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_UBOR_FIXEDACT, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_DBOR_FIXEDACT, ::GetSysColor(COLOR_3DSHADOW));

	SetColor(RLC_CELL_FIXEDACTSEL, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_TEXT_FIXEDACTSEL, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
	SetColor(RLC_LBOR_FIXEDACTSEL, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_RBOR_FIXEDACTSEL, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_UBOR_FIXEDACTSEL, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_DBOR_FIXEDACTSEL, ::GetSysColor(COLOR_3DSHADOW));

	SetColor(RLC_CELL_FIXEDNOACTNOSEL, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_TEXT_FIXEDNOACTNOSEL, ::GetSysColor(COLOR_BTNTEXT));
	SetColor(RLC_LBOR_FIXEDNOACTNOSEL, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_RBOR_FIXEDNOACTNOSEL, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_UBOR_FIXEDNOACTNOSEL, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_DBOR_FIXEDNOACTNOSEL, ::GetSysColor(COLOR_3DSHADOW));

	SetColor(RLC_CELL_FIXEDNOACTSEL, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_TEXT_FIXEDNOACTSEL, ::GetSysColor(COLOR_BTNTEXT));
	SetColor(RLC_LBOR_FIXEDNOACTSEL, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_RBOR_FIXEDNOACTSEL, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_UBOR_FIXEDNOACTSEL, ::GetSysColor(COLOR_3DSHADOW));
	SetColor(RLC_DBOR_FIXEDNOACTSEL, ::GetSysColor(COLOR_3DSHADOW));

	SetColor(RLC_BK_ACT, ::GetSysColor(COLOR_WINDOW));
	SetColor(RLC_BK_NOACT, ::GetSysColor(COLOR_WINDOW));

	SetColor(RLC_GRID_ACT, RGB(0, 0, 0));
	SetColor(RLC_GRID_NOACT, RGB(0, 0, 0));
}


//	---------------------------------------------------------------------------------------
//	Gets data for row col
//
BOOL	//OUT	returns if data from GetDataProc	
RListData::GetData(
	long a_iRow, 
	long a_iCol, 
	const TCHAR** a_ppData
	) const 
{ 
	if (m_procData == NULL) 
	{
		*a_ppData = _T("");
		return FALSE;
	}
	else
	{  
		if (m_procData(m_pDataObj, a_iRow, a_iCol, a_ppData))
		{
			return TRUE;
		}
		else
		{
			*a_ppData = _T("");
			return FALSE;
		}
	}
}


/*	---------------------------------------------------------------------------------------
	SetFont
*/
void RListData::SetFont(HFONT a_hFont)	
{
	if (m_hFont != NULL && m_bDeleteFont)
	{
		::DeleteObject(m_hFont);
		m_hFont = NULL;
	}
	if (a_hFont != NULL)
	{
		m_hFont = a_hFont;
		m_bDeleteFont = FALSE;
	}
}


void 
RListData::SetTooltip(LPCTSTR a_sTooltip)
{
	if (m_psTooltip != NULL)
	{
		delete[] m_psTooltip;
		m_psTooltip = NULL;
	}
	if (a_sTooltip == NULL)
	{
		return;
	}
	UINT l_iSize = static_cast<UINT>(_tcslen(a_sTooltip));
	m_psTooltip = new TCHAR[l_iSize + 1];
	_tcsncpy_s(m_psTooltip, l_iSize, a_sTooltip, l_iSize);
	m_psTooltip[l_iSize] = _T('\0');
}
