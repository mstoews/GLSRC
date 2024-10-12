// vtss.h
//
// C/C++ header file for Formula One 2.0
//
// Copyright (c) 1992-1994, VisualTools, Inc. All rights reserved
//

#ifndef __VTSS_H__
#define __VTSS_H__

#ifndef _INC_WINDOWS
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

#define SS_VERSION 0x0200

#define SSM_BASE WM_USER

#define SSM_CHANGE (SSM_BASE + 0)
#define SSM_SELCHANGE (SSM_BASE + 1)
#define SSM_STARTEDIT (SSM_BASE + 2)
#define SSM_ENDEDIT (SSM_BASE + 3)
#define SSM_STARTRECALC (SSM_BASE + 4)
#define SSM_ENDRECALC (SSM_BASE + 5)
#define SSM_CLICK (SSM_BASE + 6)
#define SSM_DBLCLICK (SSM_BASE + 7)
#define SSM_DELETEROW (SSM_BASE + 8)
#define SSM_REFRESHROW (SSM_BASE + 9)
#define SSM_CELLCHANGED (SSM_BASE + 10)
#define SSM_SETFOCUS (SSM_BASE + 11)
#define SSM_MODIFIED (SSM_BASE + 12)
#define SSM_DELETETABLE (SSM_BASE + 13)
#define SSM_HELP (SSM_BASE + 14)
#define SSM_DATAMOVE (SSM_BASE + 15)
#define SSM_CANCELEDIT (SSM_BASE + 16)
#define SSM_DIALOGBOXPARAM (SSM_BASE + 17)
#define SSM_FETCH (SSM_BASE + 18)
#define SSM_GETDATAROWS (SSM_BASE + 19)
#define SSM_TOPLEFTCHANGED (SSM_BASE + 20)
#define SSM_OBJCLICK (SSM_BASE + 21)
#define SSM_OBJDBLCLICK (SSM_BASE + 22)
#define SSM_RCLICK (SSM_BASE + 23)
#define SSM_RDBLCLICK (SSM_BASE + 24)

#define SSM_LAST SSM_RDBLCLICK
#define SSM_NREVENTS (SSM_LAST - SSM_BASE + 1)

#define DM_FIRST 0
#define DM_LAST 1
#define DM_NEXT 2
#define DM_PREV 3
#define DM_ROW 4
#define DM_DELETE 5

#define GR_FROMSTART 0
#define GR_FROMEND 1
#define GR_FROMMIDDLE 2
#define GR_MOVECURRENT 3
#define GR_SCROLLFIRST 4
#define GR_SCROLLLAST 5

#ifndef _WIN32
#ifndef EXPORTSEG
#define EXPORTSEG
#endif
#define SSEXPORTAPI FAR PASCAL __export EXPORTSEG
#else
#ifdef VTSSDLL
#define SSEXPORTAPI __declspec(dllexport)
#else
#define SSEXPORTAPI __declspec(dllimport)
#endif
#endif

typedef int FAR *LPINT;
typedef BOOL FAR *LPBOOL;
typedef COLORREF FAR *LPCOLORREF;

#ifndef IOFUNC_DEFINED
typedef DWORD (FAR PASCAL *IOFUNC)(DWORD dwUserData, LPVOID pRWBuf, UINT nBytesToRW);
#endif

#define kMaxEditLength 1024

#define kFormatRCNrBufSize 16

#define kModeNormal 0
#define kModeLine 1
#define kModeRectangle 2
#define kModeOval 3
#define kModeArc 4
#define kModeChart 5
#define kModeField 6
#define kModeButton 7
#define kModePolygon 8
#define kModeCheckBox 9
#define kModeDropDown 10
#define kModeLast 10

#define kPolyEditModeNormal 0
#define kPolyEditModePoints 1

#define kShiftHorizontal 1
#define kShiftVertical 2
#define kShiftRows 3
#define kShiftColumns 4

#define kHAlignGeneral 1
#define kHAlignLeft 2
#define kHAlignCenter 3
#define kHAlignRight 4
#define kHAlignFill 5
#define kHAlignJustify 6
#define kHAlignCenterAcrossCells 7

#define kVAlignTop 1
#define kVAlignCenter 2
#define kVAlignBottom 3

#ifndef kFileFormulaOne
#define kFileFormulaOne 1
#define kFileExcel4 2
#define kFileTabbedText 3	// Only SSRead() supports this type.
#endif

#ifndef kClearDlg
#define kClearDlg 0
#define kClearAll 1
#define kClearFormats 2
#define kClearValues 3
#endif

typedef int RC;
typedef RC *PRC, FAR *LPRC;

typedef struct
{
	RC nRow;
	RC nCol;
} SSCELL, FAR *LPSSCELL;

#ifndef SSERROR_NONE
#include "sserror.h"
#endif

#define SSDECLARE_HANDLE(name) \
		struct name##__ { int unused; }; \
        typedef const struct name##__ FAR * name


#ifndef HSSEDIT
SSDECLARE_HANDLE(HSSEDIT);
#endif

#ifndef HSS
SSDECLARE_HANDLE(HSS);
#endif

#define kSSObjLine 1
#define kSSObjRectangle 2
#define kSSObjOval 3
#define kSSObjArc 4
#define kSSObjButton 7
#define kSSObjPolygon 8
#define kSSObjCheckBox 9
#define kSSObjDropDown 10

typedef DWORD OBJID;
typedef OBJID FAR *LPOBJID;
typedef float OBJPOS;
typedef OBJPOS FAR *LPOBJPOS;

SSERROR SSEXPORTAPI SSAddColPageBreak(HSS hSS, RC nCol);
SSERROR SSEXPORTAPI SSAddPageBreak(HSS hSS);
SSERROR SSEXPORTAPI SSAddRowPageBreak(HSS hSS, RC nRow);
SSERROR SSEXPORTAPI SSAddSelection(HSS hSS, RC nR1, RC nC1, RC nR2, RC nC2);
SSERROR SSEXPORTAPI SSAttach(HSS hSS, LPCSTR pTitle);
SSERROR SSEXPORTAPI SSAttachToSS(HSS hDstSS, HSS hSrcSS);
SSERROR SSEXPORTAPI SSCalculationDlg(HSS hSS);
LRESULT SSEXPORTAPI SSCallWindowProc(HSS hSS, UINT nMessage, WPARAM wParam, LPARAM lParam);
SSERROR SSEXPORTAPI SSCancelEdit(HSS hSS);
SSERROR SSEXPORTAPI SSCanEditPaste(HSS hSS, LPBOOL pCanEditPaste);
SSERROR SSEXPORTAPI SSCheckModified(HSS hSS);
SSERROR SSEXPORTAPI SSCheckRecalc(HSS hSS);
SSERROR SSEXPORTAPI SSClearClipboard(void);
SSERROR SSEXPORTAPI SSClearRange(HSS hSS, RC nR1, RC nC1, RC nR2, RC nC2, int nClearType);
SSERROR SSEXPORTAPI SSColorPaletteDlg(HSS hSS);
SSERROR SSEXPORTAPI SSColWidthDlg(HSS hSS);
SSERROR SSEXPORTAPI SSCopyAll(HSS hDstSS, HSS hSrcSS);
SSERROR SSEXPORTAPI SSCopyRange(HSS hDstSS, RC nDstR1, RC nDstC1, RC nDstR2, RC nDstC2, HSS hSrcSS, RC nSrcR1, RC nSrcC1, RC nSrcR2, RC nSrcC2);
SSERROR SSEXPORTAPI SSDefinedNameDlg(HSS hSS);
SSERROR SSEXPORTAPI SSDelete(HSS hSS, BOOL bSendDeleteTableMsg);
SSERROR SSEXPORTAPI SSDeleteDefinedName(HSS hSS, LPCSTR pName);
SSERROR SSEXPORTAPI SSDeleteRange(HSS hSS, RC nR1, RC nC1, RC nR2, RC nC2, int nShiftType);
SSERROR SSEXPORTAPI SSDeleteTable(HSS hSS);
SSERROR SSEXPORTAPI SSEditBarDelete(HSSEDIT hSSEdit);
int SSEXPORTAPI SSEditBarHeight(void);
SSERROR SSEXPORTAPI SSEditBarMove(HSSEDIT hSSEdit, int x, int y, int cx, int cy);
SSERROR SSEXPORTAPI SSEditBarNew(HWND hWndParent, HSSEDIT FAR *pSSEdit);
SSERROR SSEXPORTAPI SSEditClear(HSS hSS, int nClearType);
SSERROR SSEXPORTAPI SSEditCopy(HSS hSS);
SSERROR SSEXPORTAPI SSEditCopyDown(HSS hSS);
SSERROR SSEXPORTAPI SSEditCopyRight(HSS hSS);
SSERROR SSEXPORTAPI SSEditCut(HSS hSS);
SSERROR SSEXPORTAPI SSEditDelete(HSS hSS, int nShiftType);
SSERROR SSEXPORTAPI SSEditInsert(HSS hSS, int nShiftType);
SSERROR SSEXPORTAPI SSEditPaste(HSS hSS);
SSERROR SSEXPORTAPI SSEndEdit(HSS hSS);
SSERROR SSEXPORTAPI SSErrorNumberToText(SSERROR nError, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSFilePageSetupDlg(HSS hSS);
SSERROR SSEXPORTAPI SSFilePrint(HSS hSS, BOOL bShowPrintDlg);
SSERROR SSEXPORTAPI SSFilePrintSetupDlg(HSS hSS);
SSERROR SSEXPORTAPI SSFormatAlignmentDlg(HSS hSS);
SSERROR SSEXPORTAPI SSFormatBorderDlg(HSS hSS);
SSERROR SSEXPORTAPI SSFormatCurrency0(HSS hSS);
SSERROR SSEXPORTAPI SSFormatCurrency2(HSS hSS);
SSERROR SSEXPORTAPI SSFormatFixed(HSS hSS);
SSERROR SSEXPORTAPI SSFormatFixed2(HSS hSS);
SSERROR SSEXPORTAPI SSFormatFontDlg(HSS hSS);
SSERROR SSEXPORTAPI SSFormatFraction(HSS hSS);
SSERROR SSEXPORTAPI SSFormatGeneral(HSS hSS);
SSERROR SSEXPORTAPI SSFormatHmmampm(HSS hSS);
SSERROR SSEXPORTAPI SSFormatMdyy(HSS hSS);
SSERROR SSEXPORTAPI SSFormatNumberDlg(HSS hSS);
SSERROR SSEXPORTAPI SSFormatPatternDlg(HSS hSS);
SSERROR SSEXPORTAPI SSFormatPercent(HSS hSS);
SSERROR SSEXPORTAPI SSFormatRCNr(HSS hSS, RC nRow, RC nCol, BOOL bDoAbsolute, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSFormatScientific(HSS hSS);

SSERROR SSEXPORTAPI SSEditPasteValues(HSS hSS);
SSERROR SSEXPORTAPI SSGetAlignment(HSS hSS, LPINT pHorizontal, LPBOOL pWordWrap, LPINT pVertical, LPINT pOrientation);
SSERROR SSEXPORTAPI SSGetAppName(HSS hSS, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetBorder(HSS hSS, LPINT pLeft, LPINT pRight, LPINT pTop, LPINT pBottom, LPINT pShade, LPCOLORREF pcrLeft, LPCOLORREF pcrRight, LPCOLORREF pcrTop, LPCOLORREF pcrBottom);
SSERROR SSEXPORTAPI SSGetDoSetCursor(HSS hSS, LPBOOL pDoSetCursor);
SSERROR SSEXPORTAPI SSGetFont(HSS hSS, LPSTR pBuf, int nBufSize, LPINT pSize, LPBOOL pBold, LPBOOL pItalic, LPBOOL pUnderline, LPBOOL pStrikeout, LPCOLORREF pcrColor, LPBOOL pOutline, LPBOOL pShadow);
SSERROR SSEXPORTAPI SSGetHdrHeight(HSS hSS, LPINT pHeight);
SSERROR SSEXPORTAPI SSGetHdrWidth(HSS hSS, LPINT pWidth);
SSERROR SSEXPORTAPI SSGetLineStyle(HSS hSS, LPINT pStyle, LPCOLORREF pcrColor, LPINT pWeight);
SSERROR SSEXPORTAPI SSGetNumberFormat(HSS hSS, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetPattern(HSS hSS, LPINT pPattern, LPCOLORREF pcrFG, LPCOLORREF pcrBG);
SSERROR SSEXPORTAPI SSGetProtection(HSS hSS, LPBOOL pLocked, LPBOOL pHidden);
SSERROR SSEXPORTAPI SSGetTabbedText(HSS hSS, RC nR1, RC nC1, RC nR2, RC nC2, BOOL bValuesOnly, HGLOBAL FAR *phText);
SSERROR SSEXPORTAPI SSGetTopLeftText(HSS hSS, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSSetTabbedText(HSS hSS, RC nStartRow, RC nStartCol, LPRC pRows, LPRC pCols, BOOL bValuesOnly, LPCSTR pText);

SSERROR SSEXPORTAPI SSGetActiveCell(HSS hSS, LPRC pRow, LPRC pCol);
SSERROR SSEXPORTAPI SSGetAllowArrows(HSS hSS, LPBOOL pAllowArrows);
SSERROR SSEXPORTAPI SSGetAllowDelete(HSS hSS, LPBOOL pAllowDelete);
SSERROR SSEXPORTAPI SSGetAllowEditHeaders(HSS hSS, LPBOOL pAllowEditHeaders);
SSERROR SSEXPORTAPI SSGetAllowFillRange(HSS hSS, LPBOOL pAllowFillRange);
SSERROR SSEXPORTAPI SSGetAllowInCellEditing(HSS hSS, LPBOOL pAllowInCellEditing);
SSERROR SSEXPORTAPI SSGetAllowMoveRange(HSS hSS, LPBOOL pAllowMoveRange);
SSERROR SSEXPORTAPI SSGetAllowObjSelections(HSS hSS, LPBOOL pAllowObjSelections);
SSERROR SSEXPORTAPI SSGetAllowResize(HSS hSS, LPBOOL pAllowResize);
SSERROR SSEXPORTAPI SSGetAllowSelections(HSS hSS, LPBOOL pAllowSelections);
SSERROR SSEXPORTAPI SSGetAllowTabs(HSS hSS, LPBOOL pAllowTabs);
SSERROR SSEXPORTAPI SSGetAllowFormulas(HSS hSS, LPBOOL pAllowFormulas);
SSERROR SSEXPORTAPI SSGetAutoRecalc(HSS hSS, LPBOOL pAutoRecalc);
SSERROR SSEXPORTAPI SSGetBackColor(HSS hSS, LPCOLORREF pBackColor);
SSERROR SSEXPORTAPI SSGetColText(HSS hSS, RC nCol, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetColWidth(HSS hSS, RC nCol, LPINT pWidth);
SSERROR SSEXPORTAPI SSGetDefinedName(HSS hSS, LPCSTR pName, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetEnableProtection(HSS hSS, LPBOOL pEnableProtection);
SSERROR SSEXPORTAPI SSGetEnterMovesDown(HSS hSS, LPBOOL pEnterMovesDown);
SSERROR SSEXPORTAPI SSGetEntry(HSS hSS, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetEntryRC(HSS hSS, RC nRow, RC nCol, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetExtraColor(HSS hSS, LPCOLORREF pExtraColor);
SSERROR SSEXPORTAPI SSGetFireEvent(HSS hSS, UINT nEvent, LPBOOL pFireIt);
SSERROR SSEXPORTAPI SSGetFixedCols(HSS hSS, LPRC pC1, LPRC pCols);
SSERROR SSEXPORTAPI SSGetFixedRows(HSS hSS, LPRC pR1, LPRC pRows);
SSERROR SSEXPORTAPI SSGetFormattedText(HSS hSS, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetFormattedTextRC(HSS hSS, RC nRow, RC nCol, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetFormula(HSS hSS, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetFormulaRC(HSS hSS, RC nRow, RC nCol, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetHdrSelection(HSS hSS, LPBOOL pTopLeftHdr, LPBOOL pRowHdr, LPBOOL pColHdr);
SSERROR SSEXPORTAPI SSGetIteration(HSS hSS, LPBOOL pIteration, LPINT pMaxIterations, double FAR *pMaxChange);
SSERROR SSEXPORTAPI SSGetLastCol(HSS hSS, LPRC pLastCol);
SSERROR SSEXPORTAPI SSGetLastColForRow(HSS hSS, RC nRow, LPRC pLastColForRow);
SSERROR SSEXPORTAPI SSGetLastRow(HSS hSS, LPRC pLastRow);
SSERROR SSEXPORTAPI SSGetLeftCol(HSS hSS, LPRC pLeftCol);
SSERROR SSEXPORTAPI SSGetLogicalRC(HSS hSS, RC nRow, RC nCol, LPBOOL pIsTrue);
SSERROR SSEXPORTAPI SSGetMinCol(HSS hSS, LPRC pMinCol);
SSERROR SSEXPORTAPI SSGetMinRow(HSS hSS, LPRC pMinRow);
SSERROR SSEXPORTAPI SSGetMaxCol(HSS hSS, LPRC pMaxCol);
SSERROR SSEXPORTAPI SSGetMaxRow(HSS hSS, LPRC pMaxRow);
SSERROR SSEXPORTAPI SSGetMode(HSS hSS, LPINT pMode);
SSERROR SSEXPORTAPI SSGetNumber(HSS hSS, double FAR *pNumber);
SSERROR SSEXPORTAPI SSGetNumberRC(HSS hSS, RC nRow, RC nCol, double FAR *pNumber);
SSERROR SSEXPORTAPI SSGetPolyEditMode(HSS hSS, LPINT pPolyEditMode);
SSERROR SSEXPORTAPI SSGetPrintArea(HSS hSS, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetPrintBottomMargin(HSS hSS, double FAR *pPrintBottomMargin);
SSERROR SSEXPORTAPI SSGetPrintColHeading(HSS hSS, LPBOOL pPrintColHeading);
SSERROR SSEXPORTAPI SSGetPrintFooter(HSS hSS, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetPrintGridLines(HSS hSS, LPBOOL pPrintGridLines);
SSERROR SSEXPORTAPI SSGetPrintHCenter(HSS hSS, LPBOOL pPrintHCenter);
SSERROR SSEXPORTAPI SSGetPrintHeader(HSS hSS, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetPrintLeftMargin(HSS hSS, double FAR *pPrintLeftMargin);
SSERROR SSEXPORTAPI SSGetPrintLeftToRight(HSS hSS, LPBOOL pPrintLeftToRight);
SSERROR SSEXPORTAPI SSGetPrintNoColor(HSS hSS, LPBOOL pNoColor);
SSERROR SSEXPORTAPI SSGetPrintRightMargin(HSS hSS, double FAR *pPrintRightMargin);
SSERROR SSEXPORTAPI SSGetPrintRowHeading(HSS hSS, LPBOOL pPrintRowHeading);
SSERROR SSEXPORTAPI SSGetPrintScale(HSS hSS, LPINT pScale, LPBOOL pFitToPage, LPINT pVPages, LPINT pHPages);
SSERROR SSEXPORTAPI SSGetPrintTitles(HSS hSS, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetPrintTopMargin(HSS hSS, double FAR *pPrintTopMargin);
SSERROR SSEXPORTAPI SSGetPrintVCenter(HSS hSS, LPBOOL pPrintVCenter);
SSERROR SSEXPORTAPI SSGetRepaint(HSS hSS, LPBOOL pRepaint);
SSERROR SSEXPORTAPI SSGetRowHeight(HSS hSS, RC nRow, LPINT pHeight);
SSERROR SSEXPORTAPI SSGetRowMode(HSS hSS, LPBOOL pRowMode);
SSERROR SSEXPORTAPI SSGetRowText(HSS hSS, RC nRow, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetScale(HSS hSS, LPINT pScale);
SSERROR SSEXPORTAPI SSGetSelection(HSS hSS, int nSelection, LPRC pR1, LPRC pC1, LPRC pR2, LPRC pC2);
SSERROR SSEXPORTAPI SSGetSelectionCount(HSS hSS, LPINT pCount);
SSERROR SSEXPORTAPI SSGetSelectionRef(HSS hSS, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetShowColHeading(HSS hSS, LPBOOL pShowColHeading);
SSERROR SSEXPORTAPI SSGetShowFormulas(HSS hSS, LPBOOL pShowFormulas);
SSERROR SSEXPORTAPI SSGetShowGridLines(HSS hSS, LPBOOL pShowGridLines);
SSERROR SSEXPORTAPI SSGetShowHScrollBar(HSS hSS, LPINT pShowHScrollBar);
SSERROR SSEXPORTAPI SSGetShowRowHeading(HSS hSS, LPBOOL pShowRowHeading);
SSERROR SSEXPORTAPI SSGetShowSelections(HSS hSS, LPINT pShowSelections);
SSERROR SSEXPORTAPI SSGetShowVScrollBar(HSS hSS, LPINT pShowVScrollBar);
SSERROR SSEXPORTAPI SSGetShowZeroValues(HSS hSS, LPBOOL pShowZeroValues);
SSERROR SSEXPORTAPI SSGetSSEdit(HSS hSS, HSSEDIT *pSSEdit);
SSERROR SSEXPORTAPI SSGetText(HSS hSS, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetTextRC(HSS hSS, RC nRow, RC nCol, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetTitle(HSS hSS, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSGetTopRow(HSS hSS, LPRC pTopRow);
SSERROR SSEXPORTAPI SSGetTypeRC(HSS hSS, RC nRow, RC nCol, LPINT pType);
SSERROR SSEXPORTAPI SSGotoDlg(HSS hSS);
SSERROR SSEXPORTAPI SSInitTable(HSS hSS);
SSERROR SSEXPORTAPI SSInsertRange(HSS hSS, RC nR1, RC nC1, RC nR2, RC nC2, int nShiftType);
SSERROR SSEXPORTAPI SSLaunchAppDesigner(HSS hSS);
SSERROR SSEXPORTAPI SSLineStyleDlg(HSS hSS);
RC SSEXPORTAPI SSMaxCol(void);
RC SSEXPORTAPI SSMaxRow(void);
SSERROR SSEXPORTAPI SSMoveRange(HSS hSS, RC nR1, RC nC1, RC nR2, RC nC2, RC nRowOffset, RC nColOffset);
SSERROR SSEXPORTAPI SSNew(HWND hWnd, HSS FAR *pSS);
SSERROR SSEXPORTAPI SSNextColPageBreak(HSS hSS, RC nCol, LPRC pNextCol);
SSERROR SSEXPORTAPI SSNextRowPageBreak(HSS hSS, RC nRow, LPRC pNextRow);

SSERROR SSEXPORTAPI SSObjAddItem(HSS hSS, OBJID ID, LPCSTR pItem);
SSERROR SSEXPORTAPI SSObjAddSelection(HSS hSS, OBJID ID);
SSERROR SSEXPORTAPI SSObjBringToFront(HSS hSS);
SSERROR SSEXPORTAPI SSObjDeleteItem(HSS hSS, OBJID ID, int nItem);
SSERROR SSEXPORTAPI SSObjFirstID(HSS hSS, LPOBJID pFirstID);
SSERROR SSEXPORTAPI SSObjGetCell(HSS hSS, OBJID ID, LPBOOL pHasCell, LPRC pRow, LPRC pCol);
SSERROR SSEXPORTAPI SSObjGetItem(HSS hSS, OBJID ID, int nItem, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSObjGetItemCount(HSS hSS, OBJID ID, LPINT pItems);
SSERROR SSEXPORTAPI SSObjGetItems(HSS hSS, OBJID ID, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSObjGetName(HSS hSS, OBJID ID, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSObjGetPos(HSS hSS, OBJID ID, LPOBJPOS pX1, LPOBJPOS pY1, LPOBJPOS pX2, LPOBJPOS pY2);
SSERROR SSEXPORTAPI SSObjGetSelection(HSS hSS, int nSelection, LPOBJID pID);
SSERROR SSEXPORTAPI SSObjGetSelectionCount(HSS hSS, LPINT pCount);
SSERROR SSEXPORTAPI SSObjGetText(HSS hSS, OBJID ID, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSObjGetType(HSS hSS, OBJID ID, LPINT pType);
SSERROR SSEXPORTAPI SSObjGetValue(HSS hSS, OBJID ID, LPINT pValue);
SSERROR SSEXPORTAPI SSObjGetVisible(HSS hSS, OBJID ID, LPBOOL pVisible);
SSERROR SSEXPORTAPI SSObjInsertItem(HSS hSS, OBJID ID, int nItem, LPCSTR pItem);
SSERROR SSEXPORTAPI SSObjNameDlg(HSS hSS);
SSERROR SSEXPORTAPI SSObjNameToID(HSS hSS, LPCSTR pName, LPOBJID pID);
SSERROR SSEXPORTAPI SSObjNew(HSS hSS, int nType, OBJPOS nX1, OBJPOS nY1, OBJPOS nX2, OBJPOS nY2, LPOBJID pID);
SSERROR SSEXPORTAPI SSObjNewPicture(HSS hSS, OBJPOS nX1, OBJPOS nY1, OBJPOS nX2, OBJPOS nY2, LPOBJID pID, HMETAFILE hMF, int nMapMode, int nWndExtentX, int nWndExtentY);
SSERROR SSEXPORTAPI SSObjNewPolygon(HSS hSS, OBJPOS nX1, OBJPOS nY1, OBJPOS nX2, OBJPOS nY2, LPOBJID pID, LPPOINT pPoints, int nPoints, BOOL bClosed);
SSERROR SSEXPORTAPI SSObjNextID(HSS hSS, OBJID ID, LPOBJID pNextID);
SSERROR SSEXPORTAPI SSObjOptionsDlg(HSS hSS);
SSERROR SSEXPORTAPI SSObjPosToTwips(HSS hSS, OBJPOS nX1, OBJPOS nY1, OBJPOS nX2, OBJPOS nY2, LONG FAR *pX, LONG FAR *pY, LONG FAR *pCX, LONG FAR *pCY, LPINT pShown);
SSERROR SSEXPORTAPI SSObjSendToBack(HSS hSS);
SSERROR SSEXPORTAPI SSObjSetCell(HSS hSS, OBJID ID, BOOL bHasCell, RC nRow, RC nCol);
SSERROR SSEXPORTAPI SSObjSetItem(HSS hSS, OBJID ID, int nItem, LPCSTR pItem);
SSERROR SSEXPORTAPI SSObjSetItems(HSS hSS, OBJID ID, LPCSTR pItems);
SSERROR SSEXPORTAPI SSObjSetName(HSS hSS, OBJID ID, LPCSTR pName);
SSERROR SSEXPORTAPI SSObjSetPicture(HSS hSS, OBJID ID, HMETAFILE hMF, int nMapMode, int nWndExtentX, int nWndExtentY);
SSERROR SSEXPORTAPI SSObjSetPolygonPoints(HSS hSS, OBJID ID, LPPOINT pPoints, int nPoints, BOOL bClosed);
SSERROR SSEXPORTAPI SSObjSetPos(HSS hSS, OBJID ID, OBJPOS nX1, OBJPOS nY1, OBJPOS nX2, OBJPOS nY2);
SSERROR SSEXPORTAPI SSObjSetSelection(HSS hSS, OBJID ID);
SSERROR SSEXPORTAPI SSObjSetText(HSS hSS, OBJID ID, LPCSTR pText);
SSERROR SSEXPORTAPI SSObjSetValue(HSS hSS, OBJID ID, int nValue);
SSERROR SSEXPORTAPI SSObjSetVisible(HSS hSS, OBJID ID, BOOL bVisible);

SSERROR SSEXPORTAPI SSOpenFileDlg(LPCSTR pTitle, HWND hWndParent, LPSTR pBuf, int nBufSize);
SSERROR SSEXPORTAPI SSProtectionDlg(HSS hSS);
SSERROR SSEXPORTAPI SSRangeToTwips(HSS hSS, RC nRow1, RC nCol1, RC nRow2, RC nCol2, LONG FAR *pX, LONG FAR *pY, LONG FAR *pCX, LONG FAR *pCY, LPINT pShown);
SSERROR SSEXPORTAPI SSRead(HSS hSS, LPCSTR pPathName, LPINT pFileType);
SSERROR SSEXPORTAPI SSReadIO(HSS hSS, DWORD dwUserData, IOFUNC ioFunc, DWORD FAR *pUserRet);
SSERROR SSEXPORTAPI SSRecalc(HSS hSS);
SSERROR SSEXPORTAPI SSRemoveColPageBreak(HSS hSS, RC nCol);
SSERROR SSEXPORTAPI SSRemovePageBreak(HSS hSS);
SSERROR SSEXPORTAPI SSRemoveRowPageBreak(HSS hSS, RC nRow);
SSERROR SSEXPORTAPI SSRowHeightDlg(HSS hSS);
SSERROR SSEXPORTAPI SSSaveFileDlg(HSS hSS, LPCSTR pTitle, LPSTR pBuf, int nBufSize, LPINT pFileType);
SSERROR SSEXPORTAPI SSSaveWindowInfo(HSS hSS);
SSERROR SSEXPORTAPI SSSetActiveCell(HSS hSS, RC nRow, RC nCol);
SSERROR SSEXPORTAPI SSSetAlignment(HSS hSS, int nHorizontal, BOOL bWordWrap, int nVertical, int nOrientation);
SSERROR SSEXPORTAPI SSSetAllowArrows(HSS hSS, BOOL bAllowArrows);
SSERROR SSEXPORTAPI SSSetAllowDelete(HSS hSS, BOOL bAllowDelete);
SSERROR SSEXPORTAPI SSSetAllowEditHeaders(HSS hSS, BOOL bAllowEditHeaders);
SSERROR SSEXPORTAPI SSSetAllowFillRange(HSS hSS, BOOL bAllowFillRange);
SSERROR SSEXPORTAPI SSSetAllowInCellEditing(HSS hSS, BOOL bAllowInCellEditing);
SSERROR SSEXPORTAPI SSSetAllowMoveRange(HSS hSS, BOOL bAllowMoveRange);
SSERROR SSEXPORTAPI SSSetAllowObjSelections(HSS hSS, BOOL bAllowObjSelections);
SSERROR SSEXPORTAPI SSSetAllowResize(HSS hSS, BOOL bAllowResize);
SSERROR SSEXPORTAPI SSSetAllowSelections(HSS hSS, BOOL bAllowSelections);
SSERROR SSEXPORTAPI SSSetAllowTabs(HSS hSS, BOOL bAllowTabs);
SSERROR SSEXPORTAPI SSSetAllowFormulas(HSS hSS, BOOL bAllowFormulas);
SSERROR SSEXPORTAPI SSSetAppName(HSS hSS, LPCSTR pAppName);
SSERROR SSEXPORTAPI SSSetAutoRecalc(HSS hSS, BOOL bAutoRecalc);
SSERROR SSEXPORTAPI SSSetBackColor(HSS hSS, COLORREF crBackColor);
SSERROR SSEXPORTAPI SSSetBorder(HSS hSS, int nOutline, int nLeft, int nRight, int nTop, int nBottom, int nShade, COLORREF crOutline, COLORREF crLeft, COLORREF crRight, COLORREF crTop, COLORREF crBottom);
SSERROR SSEXPORTAPI SSSetColText(HSS hSS, RC nCol, LPCSTR pColText);
SSERROR SSEXPORTAPI SSSetColWidth(HSS hSS, RC nC1, RC nC2, int nWidth, BOOL bDefColWidth);
SSERROR SSEXPORTAPI SSSetColWidthAuto(HSS hSS, RC nR1, RC nC1, RC nR2, RC nC2, BOOL bSetDefaults);
SSERROR SSEXPORTAPI SSSetDefinedName(HSS hSS, LPCSTR pName, LPCSTR pFormula);
SSERROR SSEXPORTAPI SSSetDefWindowProc(HSS hSS, WNDPROC pWindowProc);
SSERROR SSEXPORTAPI SSSetDoSetCursor(HSS hSS, BOOL bDoSetCursor);
SSERROR SSEXPORTAPI SSSetEnableProtection(HSS hSS, BOOL bEnableProtection);
SSERROR SSEXPORTAPI SSSetEnterMovesDown(HSS hSS, BOOL bEnterMovesDown);
SSERROR SSEXPORTAPI SSSetEntry(HSS hSS, LPCSTR pEntry);
SSERROR SSEXPORTAPI SSSetEntryRC(HSS hSS, RC nRow, RC nCol, LPCSTR pEntry);
SSERROR SSEXPORTAPI SSSetExtraColor(HSS hSS, COLORREF crExtraColor);
SSERROR SSEXPORTAPI SSSetFireEvent(HSS hSS, UINT nEvent, BOOL bFireIt);
SSERROR SSEXPORTAPI SSSetFixedCols(HSS hSS, RC nC1, RC nCols);
SSERROR SSEXPORTAPI SSSetFixedRows(HSS hSS, RC nR1, RC nRows);
SSERROR SSEXPORTAPI SSSetFont(HSS hSS, LPCSTR pName, int nSize, BOOL bBold, BOOL bItalic, BOOL bUnderline, BOOL bStrikeout, COLORREF crColor, BOOL bOutline, BOOL bShadow);
SSERROR SSEXPORTAPI SSSetFormula(HSS hSS, LPCSTR pFormula);
SSERROR SSEXPORTAPI SSSetFormulaRC(HSS hSS, RC nRow, RC nCol, LPCSTR pFormula);
SSERROR SSEXPORTAPI SSSetHdrHeight(HSS hSS, int nHeight);
SSERROR SSEXPORTAPI SSSetHdrSelection(HSS hSS, BOOL bTopLeftHdr, BOOL bRowHdr, BOOL bColHdr);
SSERROR SSEXPORTAPI SSSetHdrWidth(HSS hSS, int nWidth);
SSERROR SSEXPORTAPI SSSetIteration(HSS hSS, BOOL bIteration, int nMaxIterations, double nMaxChange);
SSERROR SSEXPORTAPI SSSetLeftCol(HSS hSS, RC nLeftCol);
SSERROR SSEXPORTAPI SSSetLineStyle(HSS hSS, int nStyle, COLORREF crColor, int nWeight);
SSERROR SSEXPORTAPI SSSetLogicalRC(HSS hSS, RC nRow, RC nCol, BOOL bIsTrue);
SSERROR SSEXPORTAPI SSSetMinCol(HSS hSS, RC nMinCol);
SSERROR SSEXPORTAPI SSSetMinRow(HSS hSS, RC nMinRow);
SSERROR SSEXPORTAPI SSSetMaxCol(HSS hSS, RC nMaxCol);
SSERROR SSEXPORTAPI SSSetMaxRow(HSS hSS, RC nMaxRow);
SSERROR SSEXPORTAPI SSSetMode(HSS hSS, int nMode);
SSERROR SSEXPORTAPI SSSetNumber(HSS hSS, double nNumber);
SSERROR SSEXPORTAPI SSSetNumberFormat(HSS hSS, LPCSTR pNumberFormat);
SSERROR SSEXPORTAPI SSSetNumberRC(HSS hSS, RC nRow, RC nCol, double nNumber);
SSERROR SSEXPORTAPI SSSetPattern(HSS hSS, int nPattern, COLORREF crFG, COLORREF crBG);
SSERROR SSEXPORTAPI SSSetPolyEditMode(HSS hSS, int nPolyEditMode);
SSERROR SSEXPORTAPI SSSetPrintArea(HSS hSS, LPCSTR pFormula);
SSERROR SSEXPORTAPI SSSetPrintAreaFromSelection(HSS hSS);
SSERROR SSEXPORTAPI SSSetPrintBottomMargin(HSS hSS, double nMargin);
SSERROR SSEXPORTAPI SSSetPrintColHeading(HSS hSS, BOOL bColHeading);
SSERROR SSEXPORTAPI SSSetPrintFooter(HSS hSS, LPCSTR pPrintFooter);
SSERROR SSEXPORTAPI SSSetPrintGridLines(HSS hSS, BOOL bGridLines);
SSERROR SSEXPORTAPI SSSetPrintHCenter(HSS hSS, BOOL bHCenter);
SSERROR SSEXPORTAPI SSSetPrintHeader(HSS hSS, LPCSTR pPrintHeader);
SSERROR SSEXPORTAPI SSSetPrintLeftMargin(HSS hSS, double nMargin);
SSERROR SSEXPORTAPI SSSetPrintLeftToRight(HSS hSS, BOOL bLeftToRight);
SSERROR SSEXPORTAPI SSSetPrintNoColor(HSS hSS, BOOL bNoColor);
SSERROR SSEXPORTAPI SSSetPrintRightMargin(HSS hSS, double nMargin);
SSERROR SSEXPORTAPI SSSetPrintRowHeading(HSS hSS, BOOL bRowHeading);
SSERROR SSEXPORTAPI SSSetPrintScale(HSS hSS, int nScale, BOOL bFitToPage, int nVPages, int nHPages);
SSERROR SSEXPORTAPI SSSetPrintTitles(HSS hSS, LPCSTR pFormula);
SSERROR SSEXPORTAPI SSSetPrintTitlesFromSelection(HSS hSS);
SSERROR SSEXPORTAPI SSSetPrintTopMargin(HSS hSS, double nMargin);
SSERROR SSEXPORTAPI SSSetPrintVCenter(HSS hSS, BOOL bVCenter);
SSERROR SSEXPORTAPI SSSetProtection(HSS hSS, BOOL bLocked, BOOL bHidden);
SSERROR SSEXPORTAPI SSSetRepaint(HSS hSS, BOOL bRepaint);
SSERROR SSEXPORTAPI SSSetRowHeight(HSS hSS, RC nR1, RC nR2, int nHeight, BOOL bDefRowHeight);
SSERROR SSEXPORTAPI SSSetRowHeightAuto(HSS hSS, RC nR1, RC nC1, RC nR2, RC nC2, BOOL bSetDefaults);
SSERROR SSEXPORTAPI SSSetRowMode(HSS hSS, BOOL bRowMode);
SSERROR SSEXPORTAPI SSSetRowText(HSS hSS, RC nRow, LPCSTR pRowText);
SSERROR SSEXPORTAPI SSSetScale(HSS hSS, int nScale);
SSERROR SSEXPORTAPI SSSetSelection(HSS hSS, RC nR1, RC nC1, RC nR2, RC nC2);
SSERROR SSEXPORTAPI SSSetSelectionRef(HSS hSS, LPCSTR pFormula);
SSERROR SSEXPORTAPI SSSetShowColHeading(HSS hSS, BOOL bColHeading);
SSERROR SSEXPORTAPI SSSetShowFormulas(HSS hSS, BOOL bFormulas);
SSERROR SSEXPORTAPI SSSetShowGridLines(HSS hSS, BOOL bGridLines);
SSERROR SSEXPORTAPI SSSetShowHScrollBar(HSS hSS, int nShowHScrollBar);
SSERROR SSEXPORTAPI SSSetShowRowHeading(HSS hSS, BOOL bRowHeading);
SSERROR SSEXPORTAPI SSSetShowSelections(HSS hSS, int nSelections);
SSERROR SSEXPORTAPI SSSetShowVScrollBar(HSS hSS, int nShowVScrollBar);
SSERROR SSEXPORTAPI SSSetShowZeroValues(HSS hSS, BOOL bZeroValues);
SSERROR SSEXPORTAPI SSSetSSEdit(HSS hSS, HSSEDIT hSSEdit);
SSERROR SSEXPORTAPI SSSetText(HSS hSS, LPCSTR pText);
SSERROR SSEXPORTAPI SSSetTextRC(HSS hSS, RC nRow, RC nCol, LPCSTR pText);
SSERROR SSEXPORTAPI SSSetTitle(HSS hSS, LPCSTR pTitle);
SSERROR SSEXPORTAPI SSSetTopLeftText(HSS hSS, LPCSTR pTopLeftText);
SSERROR SSEXPORTAPI SSSetTopRow(HSS hSS, RC nTopRow);
SSERROR SSEXPORTAPI SSShowActiveCell(HSS hSS);
SSERROR SSEXPORTAPI SSSort(HSS hSS, RC nR1, RC nC1, RC nR2, RC nC2, BOOL bSortByRows, LPINT pKeys, int nKeys);
SSERROR SSEXPORTAPI SSSort3(HSS hSS, RC nR1, RC nC1, RC nR2, RC nC2, BOOL bSortByRows, int nKey1, int nKey2, int nKey3);
SSERROR SSEXPORTAPI SSSortDlg(HSS hSS);
SSERROR SSEXPORTAPI SSStartEdit(HSS hSS, BOOL bClear, BOOL bInCellEditFocus, BOOL bArrowsExitEditMode);
SSERROR SSEXPORTAPI SSSwapTables(HSS hSS1, HSS hSS2);
SSERROR SSEXPORTAPI SSTransactCommit(HSS hSS);
SSERROR SSEXPORTAPI SSTransactRollback(HSS hSS);
SSERROR SSEXPORTAPI SSTransactStart(HSS hSS);
SSERROR SSEXPORTAPI SSTwipsToRC(HSS hSS, LONG x, LONG y, LPRC pRow, LPRC pCol);
void SSEXPORTAPI SSUpdate(void);
WORD SSEXPORTAPI SSVersion(void);
SSERROR SSEXPORTAPI SSWrite(HSS hSS, LPCSTR pPathName, int nFileType);
SSERROR SSEXPORTAPI SSWriteIO(HSS hSS, DWORD dwUserData, IOFUNC ioFunc, DWORD FAR *pUserRet);

typedef struct _tagEDITPARAMS
{
	LPSTR pEditString;
	BOOL bCancel;
} EDITPARAMS;

typedef struct _tagCLICKPARAMS
{
	RC nRow;
	RC nCol;
} CLICKPARAMS;

typedef struct _tagDIALOGBOXPARAMS
{
	HANDLE hInst;
	LPSTR pszTemplateName;
	FARPROC lpDialogFunc;
	LONG lp;
} DIALOGBOXPARAMS;

typedef struct _tagFETCHPARAMS
{
	LPSTR pszName;
	RC nRow;
	int nType;
	union
	{
		double nDouble;
		BOOL bBool;
		LPSTR pStr;
	} Val;
} FETCHPARAMS;

#ifdef __cplusplus

		// Close 'extern "C"' from above and declare C++ interface
}

class CSSEdit
{
public:
	CSSEdit(HSSEDIT hSSEdit)
	{
		m_hSSEdit = hSSEdit;
		m_bDeleteInDestructor = FALSE;
	}

	CSSEdit(HWND hWndParent)
	{
		m_bDeleteInDestructor = (New(hWndParent) == SSERROR_NONE);
	}

	~CSSEdit()
	{
		if (m_bDeleteInDestructor)
			Delete();
	}

	operator HSSEDIT()
	{
		return m_hSSEdit;
	}

	SSERROR Delete()
	{
		SSERROR er = SSEditBarDelete(m_hSSEdit);
		m_hSSEdit = 0;
		return er;
	}

	static int Height()
	{
		return SSEditBarHeight();
	}

	SSERROR Move(int x, int y, int cx, int cy)
	{
		return SSEditBarMove(m_hSSEdit, x, y, cx, cy);
	}

	SSERROR New(HWND hWndParent)
	{
		return SSEditBarNew(hWndParent, &m_hSSEdit);
	}

private:
	BOOL m_bDeleteInDestructor;
	HSSEDIT m_hSSEdit;
};

typedef CSSEdit FAR *LPSSEDIT;

class CSSCell
{
public:
	RC m_nRow;
	RC m_nCol;
};

typedef CSSCell FAR *LPSSCell;

class CSSRange
{
public:
	RC Rows()
	{
		return m_nRow2 - m_nRow1 + 1;
	}

	RC Cols()
	{
		return m_nCol2 - m_nCol1 + 1;
	}

	RC m_nRow1;
	RC m_nCol1;
	RC m_nRow2;
	RC m_nCol2;
};

typedef CSSRange FAR *LPSSRange;

class CSS
{
public:
	CSS()
	{
		m_bDeleteInDestructor = FALSE;
		m_hSS = 0;
	}

	CSS(HSS hSS)
	{
		m_bDeleteInDestructor = FALSE;
		m_hSS = hSS;
	}

#ifdef __AFXEXT_H__	// CVBControl constructor
#ifdef DT_HSZ
	CSS(CVBControl *pVB)
	{
		m_bDeleteInDestructor = FALSE;
		m_hSS = 0;
		m_hSS = HSS(pVB->GetNumProperty("SS"));
	}
#endif
#endif	// __AFXEXT_H__

	~CSS()
	{
		if (m_bDeleteInDestructor)
			Delete(FALSE);
	}

	operator HSS()
	{
		return m_hSS;
	}

	SSERROR EditPasteValues()
	{
		return SSEditPasteValues(m_hSS);
	}

	SSERROR GetAlignment(LPINT pHorizontal, LPBOOL pWordWrap, LPINT pVertical, LPINT pOrientation)
	{
		return SSGetAlignment(m_hSS, pHorizontal, pWordWrap, pVertical, pOrientation);
	}

	SSERROR GetAppName(LPSTR pBuf, int nBufSize)
	{
		return SSGetAppName(m_hSS, pBuf, nBufSize);
	}

	SSERROR GetBorder(LPINT pLeft, LPINT pRight, LPINT pTop, LPINT pBottom, LPINT pShade, LPCOLORREF pcrLeft, LPCOLORREF pcrRight, LPCOLORREF pcrTop, LPCOLORREF pcrBottom)
	{
		return SSGetBorder(m_hSS, pLeft, pRight, pTop, pBottom, pShade, pcrLeft, pcrRight, pcrTop, pcrBottom);
	}

	SSERROR GetDoSetCursor(LPBOOL pDoSetCursor)
	{
		return SSGetDoSetCursor(m_hSS, pDoSetCursor);
	}

	SSERROR GetFont(LPSTR pBuf, int nBufSize, LPINT pSize, LPBOOL pBold, LPBOOL pItalic, LPBOOL pUnderline, LPBOOL pStrikeout, LPCOLORREF pcrColor, LPBOOL pOutline, LPBOOL pShadow)
	{
		return SSGetFont(m_hSS, pBuf, nBufSize, pSize, pBold, pItalic, pUnderline, pStrikeout, pcrColor, pOutline, pShadow);
	}

	SSERROR GetHdrHeight(LPINT pHeight)
	{
		return SSGetHdrHeight(m_hSS, pHeight);
	}

	SSERROR GetHdrWidth(LPINT pWidth)
	{
		return SSGetHdrWidth(m_hSS, pWidth);
	}

	SSERROR GetLineStyle(LPINT pStyle, LPCOLORREF pcrColor, LPINT pWeight)
	{
		return SSGetLineStyle(m_hSS, pStyle, pcrColor, pWeight);
	}

	SSERROR GetNumberFormat(LPSTR pBuf, int nBufSize)
	{
		return SSGetNumberFormat(m_hSS, pBuf, nBufSize);
	}

	SSERROR GetPattern(LPINT pPattern, LPCOLORREF pcrFG, LPCOLORREF pcrBG)
	{
		return SSGetPattern(m_hSS, pPattern, pcrFG, pcrBG);
	}

	SSERROR GetProtection(LPBOOL pLocked, LPBOOL pHidden)
	{
		return SSGetProtection(m_hSS, pLocked, pHidden);
	}

	SSERROR GetTabbedText(RC nR1, RC nC1, RC nR2, RC nC2, BOOL bValuesOnly, HGLOBAL FAR *phText)
	{
		return SSGetTabbedText(m_hSS, nR1, nC1, nR2, nC2, bValuesOnly, phText);
	}

	SSERROR GetTopLeftText(LPSTR pBuf, int nBufSize)
	{
		return SSGetTopLeftText(m_hSS, pBuf, nBufSize);
	}

	SSERROR SetTabbedText(RC nStartRow, RC nStartCol, LPRC pRows, LPRC pCols, BOOL bValuesOnly, LPCSTR pText)
	{
		return SSSetTabbedText(m_hSS, nStartRow, nStartCol, pRows, pCols, bValuesOnly, pText);
	}

	SSERROR CheckNew()
	{
		return m_hSS ? SSERROR_NONE : New(0);
	}

	SSERROR AddColPageBreak(RC nCol)
	{
		return SSAddColPageBreak(m_hSS, nCol);
	}

	SSERROR AddPageBreak()
	{
		return SSAddPageBreak(m_hSS);
	}

	SSERROR AddRowPageBreak(RC nRow)
	{
		return SSAddRowPageBreak(m_hSS, nRow);
	}

	SSERROR AddSelection(RC nR1, RC nC1, RC nR2, RC nC2)
	{
		return SSAddSelection(m_hSS, nR1, nC1, nR2, nC2);
	}

	SSERROR Attach(LPCSTR pTitle)
	{
		SSERROR er = CheckNew();
		return er ? er : SSAttach(m_hSS, pTitle);
	}

	SSERROR AttachToSS(HSS hSrcSS)
	{
		SSERROR er = CheckNew();
		return er ? er : SSAttachToSS(m_hSS, hSrcSS);
	}

	SSERROR CalculationDlg()
	{
		return SSCalculationDlg(m_hSS);
	}

	LRESULT CallWindowProc(UINT nMessage, WPARAM wParam, LPARAM lParam)
	{
		return SSCallWindowProc(m_hSS, nMessage, wParam, lParam);
	}

	SSERROR CancelEdit()
	{
		return SSCancelEdit(m_hSS);
	}

	SSERROR CanEditPaste(LPBOOL pCanEditPaste)
	{
		return SSCanEditPaste(m_hSS, pCanEditPaste);
	}

	SSERROR CheckModified()
	{
		return SSCheckModified(m_hSS);
	}

	SSERROR CheckRecalc()
	{
		return SSCheckRecalc(m_hSS);
	}

	static SSERROR ClearClipboard()
	{
		return SSClearClipboard();
	}

	SSERROR ClearRange(RC nR1, RC nC1, RC nR2, RC nC2, int nClearType)
	{
		return SSClearRange(m_hSS, nR1, nC1, nR2, nC2, nClearType);
	}

	SSERROR ColorPaletteDlg()
	{
		return SSColorPaletteDlg(m_hSS);
	}

	SSERROR ColWidthDlg()
	{
		return SSColWidthDlg(m_hSS);
	}

	SSERROR CopyAll(HSS hSrcSS)
	{
		return SSCopyAll(m_hSS, hSrcSS);
	}

	SSERROR CopyRange(RC nDstR1, RC nDstC1, RC nDstR2, RC nDstC2, HSS hSrcSS, RC nSrcR1, RC nSrcC1, RC nSrcR2, RC nSrcC2)
	{
		return SSCopyRange(m_hSS, nDstR1, nDstC1, nDstR2, nDstC2, hSrcSS, nSrcR1, nSrcC1, nSrcR2, nSrcC2);
	}

	SSERROR DefinedNameDlg()
	{
		return SSDefinedNameDlg(m_hSS);
	}

	SSERROR Delete(BOOL bSendDeleteTableMsg = FALSE)
	{
		HSS hSS = m_hSS;
		m_hSS = 0;
		return SSDelete(hSS, bSendDeleteTableMsg);
	}

	SSERROR DeleteDefinedName(LPCSTR pName)
	{
		return SSDeleteDefinedName(m_hSS, pName);
	}

	SSERROR DeleteRange(RC nR1, RC nC1, RC nR2, RC nC2, int nShiftType)
	{
		return SSDeleteRange(m_hSS, nR1, nC1, nR2, nC2, nShiftType);
	}

	SSERROR DeleteTable()
	{
		return SSDeleteTable(m_hSS);
	}

	SSERROR EditClear(int nClearType)
	{
		return SSEditClear(m_hSS, nClearType);
	}

	SSERROR EditCopy()
	{
		return SSEditCopy(m_hSS);
	}

	SSERROR EditCopyDown()
	{
		return SSEditCopyDown(m_hSS);
	}

	SSERROR EditCopyRight()
	{
		return SSEditCopyRight(m_hSS);
	}

	SSERROR EditCut()
	{
		return SSEditCut(m_hSS);
	}

	SSERROR EditDelete(int nShiftType)
	{
		return SSEditDelete(m_hSS, nShiftType);
	}

	SSERROR EditInsert(int nShiftType)
	{
		return SSEditInsert(m_hSS, nShiftType);
	}

	SSERROR EditPaste()
	{
		return SSEditPaste(m_hSS);
	}

	SSERROR EndEdit()
	{
		return SSEndEdit(m_hSS);
	}

	static SSERROR ErrorNumberToText(SSERROR nError, LPSTR pBuf, int nBufSize)
	{
		return SSErrorNumberToText(nError, pBuf, nBufSize);
	}

	SSERROR FilePageSetupDlg()
	{
		return SSFilePageSetupDlg(m_hSS);
	}

	SSERROR FilePrint(BOOL bShowPrintDlg)
	{
		return SSFilePrint(m_hSS, bShowPrintDlg);
	}

	SSERROR FilePrintSetupDlg()
	{
		return SSFilePrintSetupDlg(m_hSS);
	}

	SSERROR FormatAlignmentDlg()
	{
		return SSFormatAlignmentDlg(m_hSS);
	}

	SSERROR FormatBorderDlg()
	{
		return SSFormatBorderDlg(m_hSS);
	}

	SSERROR FormatCurrency0()
	{
		return SSFormatCurrency0(m_hSS);
	}

	SSERROR FormatCurrency2()
	{
		return SSFormatCurrency2(m_hSS);
	}

	SSERROR FormatFixed()
	{
		return SSFormatFixed(m_hSS);
	}

	SSERROR FormatFixed2()
	{
		return SSFormatFixed2(m_hSS);
	}

	SSERROR FormatFontDlg()
	{
		return SSFormatFontDlg(m_hSS);
	}

	SSERROR FormatFraction()
	{
		return SSFormatFraction(m_hSS);
	}

	SSERROR FormatGeneral()
	{
		return SSFormatGeneral(m_hSS);
	}

	SSERROR FormatHmmampm()
	{
		return SSFormatHmmampm(m_hSS);
	}

	SSERROR FormatMdyy()
	{
		return SSFormatMdyy(m_hSS);
	}

	SSERROR FormatNumberDlg()
	{
		return SSFormatNumberDlg(m_hSS);
	}

	SSERROR FormatPatternDlg()
	{
		return SSFormatPatternDlg(m_hSS);
	}

	SSERROR FormatPercent()
	{
		return SSFormatPercent(m_hSS);
	}

	SSERROR FormatRCNr(RC nRow, RC nCol, BOOL bDoAbsolute, LPSTR pBuf, int nBufSize)
	{
		return SSFormatRCNr(m_hSS, nRow, nCol, bDoAbsolute, pBuf, nBufSize);
	}

	SSERROR FormatScientific()
	{
		return SSFormatScientific(m_hSS);
	}

	SSERROR GetActiveCell(LPRC pRow, LPRC pCol)
	{
		return SSGetActiveCell(m_hSS, pRow, pCol);
	}

	SSERROR GetAllowArrows(LPBOOL pAllowArrows)
	{
		return SSGetAllowArrows(m_hSS, pAllowArrows);
	}

	SSERROR GetAllowDelete(LPBOOL pAllowDelete)
	{
		return SSGetAllowDelete(m_hSS, pAllowDelete);
	}

	SSERROR GetAllowEditHeaders(LPBOOL pAllowEditHeaders)
	{
		return SSGetAllowEditHeaders(m_hSS, pAllowEditHeaders);
	}

	SSERROR GetAllowFillRange(LPBOOL pAllowFillRange)
	{
		return SSGetAllowFillRange(m_hSS, pAllowFillRange);
	}

	SSERROR GetAllowInCellEditing(LPBOOL pAllowInCellEditing)
	{
		return SSGetAllowInCellEditing(m_hSS, pAllowInCellEditing);
	}

	SSERROR GetAllowMoveRange(LPBOOL pAllowMoveRange)
	{
		return SSGetAllowMoveRange(m_hSS, pAllowMoveRange);
	}

	SSERROR GetAllowObjSelections(LPBOOL lpAllowObjSelections)
	{
		return SSGetAllowObjSelections(m_hSS, lpAllowObjSelections);
	}

	SSERROR GetAllowResize(LPBOOL pAllowResize)
	{
		return SSGetAllowResize(m_hSS, pAllowResize);
	}

	SSERROR GetAllowSelections(LPBOOL pAllowSelections)
	{
		return SSGetAllowSelections(m_hSS, pAllowSelections);
	}

	SSERROR GetAllowTabs(LPBOOL pAllowTabs)
	{
		return SSGetAllowTabs(m_hSS, pAllowTabs);
	}

	SSERROR GetAllowFormulas(LPBOOL pAllowFormulas)
	{
		return SSGetAllowFormulas(m_hSS, pAllowFormulas);
	}

	SSERROR GetAutoRecalc(LPBOOL pAutoRecalc)
	{
		return SSGetAutoRecalc(m_hSS, pAutoRecalc);
	}

	SSERROR GetBackColor(LPCOLORREF pBackColor)
	{
		return SSGetBackColor(m_hSS, pBackColor);
	}

	SSERROR GetColText(RC nCol, LPSTR pBuf, int nBufSize)
	{
		return SSGetColText(m_hSS, nCol, pBuf, nBufSize);
	}

	SSERROR GetColWidth(RC nCol, LPINT pWidth)
	{
		return SSGetColWidth(m_hSS, nCol, pWidth);
	}

	SSERROR GetDefinedName(LPCSTR pName, LPSTR pBuf, int nBufSize)
	{
		return SSGetDefinedName(m_hSS, pName, pBuf, nBufSize);
	}

	SSERROR GetEnableProtection(LPBOOL pEnableProtection)
	{
		return SSGetEnableProtection(m_hSS, pEnableProtection);
	}

	SSERROR GetEnterMovesDown(LPBOOL pEnterMovesDown)
	{
		return SSGetEnterMovesDown(m_hSS, pEnterMovesDown);
	}

	SSERROR GetEntry(LPSTR pBuf, int nBufSize)
	{
		return SSGetEntry(m_hSS, pBuf, nBufSize);
	}

	SSERROR GetEntryRC(RC nRow, RC nCol, LPSTR pBuf, int nBufSize)
	{
		return SSGetEntryRC(m_hSS, nRow, nCol, pBuf, nBufSize);
	}

	SSERROR GetExtraColor(LPCOLORREF pExtraColor)
	{
		return SSGetExtraColor(m_hSS, pExtraColor);
	}

	SSERROR GetFireEvent(UINT nEvent, LPBOOL pFireIt)
	{
		return SSGetFireEvent(m_hSS, nEvent, pFireIt);
	}

	SSERROR GetFixedCols(LPRC pC1, LPRC pCols)
	{
		return SSGetFixedCols(m_hSS, pC1, pCols);
	}

	SSERROR GetFixedRows(LPRC pR1, LPRC pRows)
	{
		return SSGetFixedRows(m_hSS, pR1, pRows);
	}

	SSERROR GetFormattedText(LPSTR pBuf, int nBufSize)
	{
		return SSGetFormattedText(m_hSS, pBuf, nBufSize);
	}

	SSERROR GetFormattedTextRC(RC nRow, RC nCol, LPSTR pBuf, int nBufSize)
	{
		return SSGetFormattedTextRC(m_hSS, nRow, nCol, pBuf, nBufSize);
	}

	SSERROR GetFormula(LPSTR pBuf, int nBufSize)
	{
		return SSGetFormula(m_hSS, pBuf, nBufSize);
	}

	SSERROR GetFormulaRC(RC nRow, RC nCol, LPSTR pBuf, int nBufSize)
	{
		return SSGetFormulaRC(m_hSS, nRow, nCol, pBuf, nBufSize);
	}

	SSERROR GetHdrSelection(LPBOOL pTopLeftHdr, LPBOOL pRowHdr, LPBOOL pColHdr)
	{
		return SSGetHdrSelection(m_hSS, pTopLeftHdr, pRowHdr, pColHdr);
	}

	SSERROR GetIteration(LPBOOL pIteration, LPINT pMaxIterations, double FAR *pMaxChange)
	{
		return SSGetIteration(m_hSS, pIteration, pMaxIterations, pMaxChange);
	}

	SSERROR GetLastCol(LPRC pLastCol)
	{
		return SSGetLastCol(m_hSS, pLastCol);
	}

	SSERROR GetLastColForRow(RC nRow, LPRC pLastColForRow)
	{
		return SSGetLastColForRow(m_hSS, nRow, pLastColForRow);
	}

	SSERROR GetLastRow(LPRC pLastRow)
	{
		return SSGetLastRow(m_hSS, pLastRow);
	}

	SSERROR GetLeftCol(LPRC pLeftCol)
	{
		return SSGetLeftCol(m_hSS, pLeftCol);
	}

	SSERROR GetLogicalRC(RC nRow, RC nCol, LPBOOL pIsTrue)
	{
		return SSGetLogicalRC(m_hSS, nRow, nCol, pIsTrue);
	}

	SSERROR GetMinCol(LPRC pMinCol)
	{
		return SSGetMinCol(m_hSS, pMinCol);
	}

	SSERROR GetMinRow(LPRC pMinRow)
	{
		return SSGetMinRow(m_hSS, pMinRow);
	}

	SSERROR GetMaxCol(LPRC pMaxCol)
	{
		return SSGetMaxCol(m_hSS, pMaxCol);
	}

	SSERROR GetMaxRow(LPRC pMaxRow)
	{
		return SSGetMaxRow(m_hSS, pMaxRow);
	}

	SSERROR GetMode(LPINT pMode)
	{
		return SSGetMode(m_hSS, pMode);
	}

	SSERROR GetNumber(double FAR *pNumber)
	{
		return SSGetNumber(m_hSS, pNumber);
	}

	SSERROR GetNumberRC(RC nRow, RC nCol, double FAR *pNumber)
	{
		return SSGetNumberRC(m_hSS, nRow, nCol, pNumber);
	}

	SSERROR GetPolyEditMode(LPINT pPolyEditMode)
	{
		return SSGetPolyEditMode(m_hSS, pPolyEditMode);
	}

	SSERROR GetPrintArea(LPSTR pBuf, int nBufSize)
	{
		return SSGetPrintArea(m_hSS, pBuf, nBufSize);
	}

	SSERROR GetPrintBottomMargin(double FAR *pPrintBottomMargin)
	{
		return SSGetPrintBottomMargin(m_hSS, pPrintBottomMargin);
	}

	SSERROR GetPrintColHeading(LPBOOL pPrintColHeading)
	{
		return SSGetPrintColHeading(m_hSS, pPrintColHeading);
	}

	SSERROR GetPrintFooter(LPSTR pBuf, int nBufSize)
	{
		return SSGetPrintFooter(m_hSS, pBuf, nBufSize);
	}

	SSERROR GetPrintGridLines(LPBOOL pPrintGridLines)
	{
		return SSGetPrintGridLines(m_hSS, pPrintGridLines);
	}

	SSERROR GetPrintHCenter(LPBOOL pPrintHCenter)
	{
		return SSGetPrintHCenter(m_hSS, pPrintHCenter);
	}

	SSERROR GetPrintHeader(LPSTR pBuf, int nBufSize)
	{
		return SSGetPrintHeader(m_hSS, pBuf, nBufSize);
	}

	SSERROR GetPrintLeftMargin(double FAR *pPrintLeftMargin)
	{
		return SSGetPrintLeftMargin(m_hSS, pPrintLeftMargin);
	}

	SSERROR GetPrintLeftToRight(LPBOOL pPrintLeftToRight)
	{
		return SSGetPrintLeftToRight(m_hSS, pPrintLeftToRight);
	}

	SSERROR GetPrintNoColor(LPBOOL pNoColor)
	{
		return SSGetPrintNoColor(m_hSS, pNoColor);
	}

	SSERROR GetPrintRightMargin(double FAR *pPrintRightMargin)
	{
		return SSGetPrintRightMargin(m_hSS, pPrintRightMargin);
	}

	SSERROR GetPrintRowHeading(LPBOOL pPrintRowHeading)
	{
		return SSGetPrintRowHeading(m_hSS, pPrintRowHeading);
	}

	SSERROR GetPrintScale(LPINT pScale, LPBOOL pFitToPage, LPINT pVPages, LPINT pHPages)
	{
		return SSGetPrintScale(m_hSS, pScale, pFitToPage, pVPages, pHPages);
	}

	SSERROR GetPrintTitles(LPSTR pBuf, int nBufSize)
	{
		return SSGetPrintTitles(m_hSS, pBuf, nBufSize);
	}

	SSERROR GetPrintTopMargin(double FAR *pPrintTopMargin)
	{
		return SSGetPrintTopMargin(m_hSS, pPrintTopMargin);
	}

	SSERROR GetPrintVCenter(LPBOOL pPrintVCenter)
	{
		return SSGetPrintVCenter(m_hSS, pPrintVCenter);
	}

	SSERROR GetRepaint(LPBOOL pRepaint)
	{
		return SSGetRepaint(m_hSS, pRepaint);
	}

	SSERROR GetRowHeight(RC nRow, LPINT pHeight)
	{
		return SSGetRowHeight(m_hSS, nRow, pHeight);
	}

	SSERROR GetRowMode(LPBOOL pRowMode)
	{
		return SSGetRowMode(m_hSS, pRowMode);
	}

	SSERROR GetRowText(RC nRow, LPSTR pBuf, int nBufSize)
	{
		return SSGetRowText(m_hSS, nRow, pBuf, nBufSize);
	}

	SSERROR GetScale(LPINT pScale)
	{
		return SSGetScale(m_hSS, pScale);
	}

	SSERROR GetSelection(int nSelection, LPRC pR1, LPRC pC1, LPRC pR2, LPRC pC2)
	{
		return SSGetSelection(m_hSS, nSelection, pR1, pC1, pR2, pC2);
	}

	SSERROR GetSelection(int nSelection, LPSSRange pRange)
	{
		return SSGetSelection(m_hSS, nSelection, &pRange->m_nRow1, &pRange->m_nCol1, &pRange->m_nRow2, &pRange->m_nCol2);
	}

	SSERROR GetSelectionCount(LPINT pCount)
	{
		return SSGetSelectionCount(m_hSS, pCount);
	}

	SSERROR GetSelectionRef(LPSTR pBuf, int nBufSize)
	{
		return SSGetSelectionRef(m_hSS, pBuf, nBufSize);
	}

	SSERROR GetShowColHeading(LPBOOL pShowColHeading)
	{
		return SSGetShowColHeading(m_hSS, pShowColHeading);
	}

	SSERROR GetShowFormulas(LPBOOL pShowFormulas)
	{
		return SSGetShowFormulas(m_hSS, pShowFormulas);
	}

	SSERROR GetShowGridLines(LPBOOL pShowGridLines)
	{
		return SSGetShowGridLines(m_hSS, pShowGridLines);
	}

	SSERROR GetShowHScrollBar(LPINT pShowHScrollBar)
	{
		return SSGetShowHScrollBar(m_hSS, pShowHScrollBar);
	}

	SSERROR GetShowRowHeading(LPBOOL pShowRowHeading)
	{
		return SSGetShowRowHeading(m_hSS, pShowRowHeading);
	}

	SSERROR GetShowSelections(LPINT pShowSelections)
	{
		return SSGetShowSelections(m_hSS, pShowSelections);
	}

	SSERROR GetShowVScrollBar(LPINT pShowVScrollBar)
	{
		return SSGetShowVScrollBar(m_hSS, pShowVScrollBar);
	}

	SSERROR GetShowZeroValues(LPBOOL pShowZeroValues)
	{
		return SSGetShowZeroValues(m_hSS, pShowZeroValues);
	}

	SSERROR GetSSEdit(HSSEDIT *pSSEdit)
	{
		return SSGetSSEdit(m_hSS, pSSEdit);
	}

	SSERROR GetText(LPSTR pBuf, int nBufSize)
	{
		return SSGetText(m_hSS, pBuf, nBufSize);
	}

	SSERROR GetTextRC(RC nRow, RC nCol, LPSTR pBuf, int nBufSize)
	{
		return SSGetTextRC(m_hSS, nRow, nCol, pBuf, nBufSize);
	}

	SSERROR GetTitle(LPSTR pBuf, int nBufSize)
	{
		return SSGetTitle(m_hSS, pBuf, nBufSize);
	}

	SSERROR GetTopRow(LPRC pTopRow)
	{
		return SSGetTopRow(m_hSS, pTopRow);
	}

	SSERROR GetTypeRC(RC nRow, RC nCol, LPINT pType)
	{
		return SSGetTypeRC(m_hSS, nRow, nCol, pType);
	}

	SSERROR GotoDlg()
	{
		return SSGotoDlg(m_hSS);
	}

	SSERROR InitTable()
	{
		SSERROR er = CheckNew();
		return er ? er : SSInitTable(m_hSS);
	}

	SSERROR InsertRange(RC nR1, RC nC1, RC nR2, RC nC2, int nShiftType)
	{
		return SSInsertRange(m_hSS, nR1, nC1, nR2, nC2, nShiftType);
	}

	SSERROR LaunchAppDesigner()
	{
		return SSLaunchAppDesigner(m_hSS);
	}

	SSERROR LineStyleDlg()
	{
		return SSLineStyleDlg(m_hSS);
	}

	static RC MaxCol()
	{
		return SSMaxCol();
	}

	static RC MaxRow()
	{
		return SSMaxRow();
	}

	SSERROR MoveRange(RC nR1, RC nC1, RC nR2, RC nC2, RC nRowOffset, RC nColOffset)
	{
		return SSMoveRange(m_hSS, nR1, nC1, nR2, nC2, nRowOffset, nColOffset);
	}


	SSERROR New(HWND hWnd)
	{
		SSERROR er = SSNew(hWnd, &m_hSS);
		m_bDeleteInDestructor = (er == SSERROR_NONE);
		return er;
	}

	SSERROR NextColPageBreak(RC nCol, LPRC pNextCol)
	{
		return SSNextColPageBreak(m_hSS, nCol, pNextCol);
	}

	SSERROR NextRowPageBreak(RC nRow, LPRC pNextRow)
	{
		return SSNextRowPageBreak(m_hSS, nRow, pNextRow);
	}

	SSERROR ObjAddItem(OBJID ID, LPCSTR pItem)
	{
		return SSObjAddItem(m_hSS, ID, pItem);
	}

	SSERROR ObjAddSelection(OBJID ID)
	{
		return SSObjAddSelection(m_hSS, ID);
	}

	SSERROR ObjBringToFront()
	{
		return SSObjBringToFront(m_hSS);
	}

	SSERROR ObjDeleteItem(OBJID ID, int nItem)
	{
		return SSObjDeleteItem(m_hSS, ID, nItem);
	}

	SSERROR ObjFirstID(LPOBJID pFirstID)
	{
		return SSObjFirstID(m_hSS, pFirstID);
	}

	SSERROR ObjGetCell(OBJID ID, LPBOOL pHasCell, LPRC pRow, LPRC pCol)
	{
		return SSObjGetCell(m_hSS, ID, pHasCell, pRow, pCol);
	}

	SSERROR ObjGetItem(OBJID ID, int nItem, LPSTR pBuf, int nBufSize)
	{
		return SSObjGetItem(m_hSS, ID, nItem, pBuf, nBufSize);
	}

	SSERROR ObjGetItemCount(OBJID ID, LPINT pItems)
	{
		return SSObjGetItemCount(m_hSS, ID, pItems);
	}

	SSERROR ObjGetItems(OBJID ID, LPSTR pBuf, int nBufSize)
	{
		return SSObjGetItems(m_hSS, ID, pBuf, nBufSize);
	}

	SSERROR ObjGetName(OBJID ID, LPSTR pBuf, int nBufSize)
	{
		return SSObjGetName(m_hSS, ID, pBuf, nBufSize);
	}

	SSERROR ObjGetPos(OBJID ID, LPOBJPOS pX1, LPOBJPOS pY1, LPOBJPOS pX2, LPOBJPOS pY2)
	{
		return SSObjGetPos(m_hSS, ID, pX1, pY1, pX2, pY2);
	}

	SSERROR ObjGetSelection(int nSelection, LPOBJID pID)
	{
		return SSObjGetSelection(m_hSS, nSelection, pID);
	}

	SSERROR ObjGetSelectionCount(LPINT pCount)
	{
		return SSObjGetSelectionCount(m_hSS, pCount);
	}

	SSERROR ObjGetText(OBJID ID, LPSTR pBuf, int nBufSize)
	{
		return SSObjGetText(m_hSS, ID, pBuf, nBufSize);
	}

	SSERROR ObjGetType(OBJID ID, LPINT pType)
	{
		return SSObjGetType(m_hSS, ID, pType);
	}

	SSERROR ObjGetValue(OBJID ID, LPINT pValue)
	{
		return SSObjGetValue(m_hSS, ID, pValue);
	}

	SSERROR ObjGetVisible(OBJID ID, LPBOOL pVisible)
	{
		return SSObjGetVisible(m_hSS, ID, pVisible);
	}

	SSERROR ObjInsertItem(OBJID ID, int nItem, LPCSTR pItem)
	{
		return SSObjInsertItem(m_hSS, ID, nItem, pItem);
	}

	SSERROR ObjNameDlg()
	{
		return SSObjNameDlg(m_hSS);
	}

	SSERROR ObjNameToID(LPCSTR pName, LPOBJID pID)
	{
		return SSObjNameToID(m_hSS, pName, pID);
	}

	SSERROR ObjNew(int nType, OBJPOS nX1, OBJPOS nY1, OBJPOS nX2, OBJPOS nY2, LPOBJID pID)
	{
		return SSObjNew(m_hSS, nType, nX1, nY1, nX2, nY2, pID);
	}

	SSERROR ObjNewPicture(OBJPOS nX1, OBJPOS nY1, OBJPOS nX2, OBJPOS nY2, LPOBJID pID, HMETAFILE hMF, int nMapMode, int nWndExtentX, int nWndExtentY)
	{
		return SSObjNewPicture(m_hSS, nX1, nY1, nX2, nY2, pID, hMF, nMapMode, nWndExtentX, nWndExtentY);
	}

	SSERROR ObjNewPolygon(OBJPOS nTop, OBJPOS nLeft, OBJPOS nBottom, OBJPOS nRight, LPOBJID pID, LPPOINT pPoints, int nPoints, BOOL bClosed)
	{
		return SSObjNewPolygon(m_hSS, nTop, nLeft, nBottom, nRight, pID, pPoints, nPoints, bClosed);
	}

	SSERROR ObjNextID(OBJID ID, LPOBJID pNextID)
	{
		return SSObjNextID(m_hSS, ID, pNextID);
	}

	SSERROR ObjOptionsDlg()
	{
		return SSObjOptionsDlg(m_hSS);
	}

	SSERROR ObjPosToTwips(OBJPOS nX1, OBJPOS nY1, OBJPOS nX2, OBJPOS nY2, LONG FAR *pX, LONG FAR *pY, LONG FAR *pCX, LONG FAR *pCY, LPINT pShown)
	{
		return SSObjPosToTwips(m_hSS, nX1, nY1, nX2, nY2, pX, pY, pCX, pCY, pShown);
	}

	SSERROR ObjSendToBack()
	{
		return SSObjSendToBack(m_hSS);
	}

	SSERROR ObjSetCell(OBJID ID, BOOL bHasCell, RC nRow, RC nCol)
	{
		return SSObjSetCell(m_hSS, ID, bHasCell, nRow, nCol);
	}

	SSERROR ObjSetItem(OBJID ID, int nItem, LPCSTR pItem)
	{
		return SSObjSetItem(m_hSS, ID, nItem, pItem);
	}

	SSERROR ObjSetItems(OBJID ID, LPCSTR pItems)
	{
		return SSObjSetItems(m_hSS, ID, pItems);
	}

	SSERROR ObjSetName(OBJID ID, LPCSTR pName)
	{
		return SSObjSetName(m_hSS, ID, pName);
	}

	SSERROR ObjSetPicture(OBJID ID, HMETAFILE hMF, int nMapMode, int nWndExtentX, int nWndExtentY)
	{
		return SSObjSetPicture(m_hSS, ID, hMF, nMapMode, nWndExtentX, nWndExtentY);
	}

	SSERROR ObjSetPolygonPoints(OBJID ID, LPPOINT pPoints, int nPoints, BOOL bClosed)
	{
		return SSObjSetPolygonPoints(m_hSS, ID, pPoints, nPoints, bClosed);
	}

	SSERROR ObjSetPos(OBJID ID, OBJPOS nX1, OBJPOS nY1, OBJPOS nX2, OBJPOS nY2)
	{
		return SSObjSetPos(m_hSS, ID, nX1, nY1, nX2, nY2);
	}

	SSERROR ObjSetSelection(OBJID ID)
	{
		return SSObjSetSelection(m_hSS, ID);
	}

	SSERROR ObjSetText(OBJID ID, LPCSTR pText)
	{
		return SSObjSetText(m_hSS, ID, pText);
	}

	SSERROR ObjSetValue(OBJID ID, int nValue)
	{
		return SSObjSetValue(m_hSS, ID, nValue);
	}

	SSERROR ObjSetVisible(OBJID ID, BOOL bVisible)
	{
		return SSObjSetVisible(m_hSS, ID, bVisible);
	}

	static SSERROR OpenFileDlg(LPCSTR pTitle, HWND hWndParent, LPSTR pBuf, int nBufSize)
	{
		return SSOpenFileDlg(pTitle, hWndParent, pBuf, nBufSize);
	}

	SSERROR ProtectionDlg()
	{
		return SSProtectionDlg(m_hSS);
	}

	SSERROR RangeToTwips(RC nRow1, RC nCol1, RC nRow2, RC nCol2, LONG FAR *pX, LONG FAR *pY, LONG FAR *pCX, LONG FAR *pCY, LPINT pShown)
	{
		return SSRangeToTwips(m_hSS, nRow1, nCol1, nRow2, nCol2, pX, pY, pCX, pCY, pShown);
	}

	SSERROR Read(LPCSTR pPathName, LPINT pFileType)
	{
		SSERROR er = CheckNew();
		return er ? er : SSRead(m_hSS, pPathName, pFileType);
	}

	SSERROR ReadIO(DWORD dwUserData, IOFUNC ioFunc, DWORD FAR *pUserRet)
	{
		SSERROR er = CheckNew();
		return er ? er : SSReadIO(m_hSS, dwUserData, ioFunc, pUserRet);
	}

	SSERROR Recalc()
	{
		return SSRecalc(m_hSS);
	}

	SSERROR RemoveColPageBreak(RC nCol)
	{
		return SSRemoveColPageBreak(m_hSS, nCol);
	}

	SSERROR RemovePageBreak()
	{
		return SSRemovePageBreak(m_hSS);
	}

	SSERROR RemoveRowPageBreak(RC nRow)
	{
		return SSRemoveRowPageBreak(m_hSS, nRow);
	}

	SSERROR RowHeightDlg()
	{
		return SSRowHeightDlg(m_hSS);
	}

	SSERROR SaveFileDlg(LPCSTR pTitle, LPSTR pBuf, int nBufSize, LPINT pFileType)
	{
		return SSSaveFileDlg(m_hSS, pTitle, pBuf, nBufSize, pFileType);
	}

	SSERROR SaveWindowInfo()
	{
		return SSSaveWindowInfo(m_hSS);
	}

	SSERROR SetActiveCell(RC nRow, RC nCol)
	{
		return SSSetActiveCell(m_hSS, nRow, nCol);
	}

	SSERROR SetAlignment(int nHorizontal, BOOL bWordWrap, int nVertical, int nOrientation)
	{
		return SSSetAlignment(m_hSS, nHorizontal, bWordWrap, nVertical, nOrientation);
	}

	SSERROR SetAllowArrows(BOOL bAllowArrows)
	{
		return SSSetAllowArrows(m_hSS, bAllowArrows);
	}

	SSERROR SetAllowDelete(BOOL bAllowDelete)
	{
		return SSSetAllowDelete(m_hSS, bAllowDelete);
	}

	SSERROR SetAllowEditHeaders(BOOL bAllowEditHeaders)
	{
		return SSSetAllowEditHeaders(m_hSS, bAllowEditHeaders);
	}

	SSERROR SetAllowFillRange(BOOL bAllowFillRange)
	{
		return SSSetAllowFillRange(m_hSS, bAllowFillRange);
	}

	SSERROR SetAllowInCellEditing(BOOL bAllowInCellEditing)
	{
		return SSSetAllowInCellEditing(m_hSS, bAllowInCellEditing);
	}

	SSERROR SetAllowMoveRange(BOOL bAllowMoveRange)
	{
		return SSSetAllowMoveRange(m_hSS, bAllowMoveRange);
	}

	SSERROR SetAllowObjSelections(BOOL bAllowObjSelections)
	{
		return SSSetAllowObjSelections(m_hSS, bAllowObjSelections);
	}

	SSERROR SetAllowResize(BOOL bAllowResize)
	{
		return SSSetAllowResize(m_hSS, bAllowResize);
	}

	SSERROR SetAllowSelections(BOOL bAllowSelections)
	{
		return SSSetAllowSelections(m_hSS, bAllowSelections);
	}

	SSERROR SetAllowTabs(BOOL bAllowTabs)
	{
		return SSSetAllowTabs(m_hSS, bAllowTabs);
	}

	SSERROR SetAllowFormulas(BOOL bAllowFormulas)
	{
		return SSSetAllowFormulas(m_hSS, bAllowFormulas);
	}

	SSERROR SetAppName(LPCSTR pAppName)
	{
		return SSSetAppName(m_hSS, pAppName);
	}

	SSERROR SetAutoRecalc(BOOL bAutoRecalc)
	{
		return SSSetAutoRecalc(m_hSS, bAutoRecalc);
	}

	SSERROR SetBackColor(COLORREF crBackColor)
	{
		return SSSetBackColor(m_hSS, crBackColor);
	}

	SSERROR SetBorder(int nOutline, int nLeft, int nRight,
			int nTop, int nBottom, int nShade, 
			COLORREF crOutline, COLORREF crLeft, COLORREF crRight,
			COLORREF crTop, COLORREF crBottom)
	{
		return SSSetBorder(m_hSS, nOutline, nLeft, nRight, nTop, nBottom, nShade,
				crOutline, crLeft, crRight, crTop, crBottom);
	}

	SSERROR SetColText(RC nCol, LPCSTR pColText)
	{
		return SSSetColText(m_hSS, nCol, pColText);
	}

	SSERROR SetColWidth(RC nC1, RC nC2, int nWidth, BOOL bDefColWidth)
	{
		return SSSetColWidth(m_hSS, nC1, nC2, nWidth, bDefColWidth);
	}

	SSERROR SetColWidthAuto(RC nR1, RC nC1, RC nR2, RC nC2, BOOL bSetDefaults)
	{
		return SSSetColWidthAuto(m_hSS, nR1, nC1, nR2, nC2, bSetDefaults);
	}

	SSERROR SetDefinedName(LPCSTR pName, LPCSTR pFormula)
	{
		return SSSetDefinedName(m_hSS, pName, pFormula);
	}

	SSERROR SetDefWindowProc(WNDPROC pWindowProc)
	{
		return SSSetDefWindowProc(m_hSS, pWindowProc);
	}

	SSERROR SetDoSetCursor(BOOL bDoSetCursor)
	{
		return SSSetDoSetCursor(m_hSS, bDoSetCursor);
	}

	SSERROR SetEnableProtection(BOOL bEnableProtection)
	{
		return SSSetEnableProtection(m_hSS, bEnableProtection);
	}

	SSERROR SetEnterMovesDown(BOOL bEnterMovesDown)
	{
		return SSSetEnterMovesDown(m_hSS, bEnterMovesDown);
	}

	SSERROR SetEntry(LPCSTR pEntry)
	{
		return SSSetEntry(m_hSS, pEntry);
	}

	SSERROR SetEntryRC(RC nRow, RC nCol, LPCSTR pEntry)
	{
		return SSSetEntryRC(m_hSS, nRow, nCol, pEntry);
	}

	SSERROR SetExtraColor(COLORREF crExtraColor)
	{
		return SSSetExtraColor(m_hSS, crExtraColor);
	}

	SSERROR SetFireEvent(UINT nEvent, BOOL bFireIt)
	{
		return SSSetFireEvent(m_hSS, nEvent, bFireIt);
	}

	SSERROR SetFixedCols(RC nC1, RC nCols)
	{
		return SSSetFixedCols(m_hSS, nC1, nCols);
	}

	SSERROR SetFixedRows(RC nR1, RC nRows)
	{
		return SSSetFixedRows(m_hSS, nR1, nRows);
	}

	SSERROR SetFont(LPCSTR pName, int nSize, BOOL bBold, BOOL bItalic, BOOL bUnderline, BOOL bStrikeout, COLORREF crColor, BOOL bOutline, BOOL bShadow)
	{
		return SSSetFont(m_hSS, pName, nSize, bBold, bItalic, bUnderline, bStrikeout, crColor, bOutline, bShadow);
	}

	SSERROR SetFormula(LPCSTR pFormula)
	{
		return SSSetFormula(m_hSS, pFormula);
	}

	SSERROR SetFormulaRC(RC nRow, RC nCol, LPCSTR pFormula)
	{
		return SSSetFormulaRC(m_hSS, nRow, nCol, pFormula);
	}

	SSERROR SetHdrHeight(int nHeight)
	{
		return SSSetHdrHeight(m_hSS, nHeight);
	}

	SSERROR SetHdrSelection(BOOL bTopLeftHdr, BOOL bRowHdr, BOOL bColHdr)
	{
		return SSSetHdrSelection(m_hSS, bTopLeftHdr, bRowHdr, bColHdr);
	}

	SSERROR SetHdrWidth(int nWidth)
	{
		return SSSetHdrWidth(m_hSS, nWidth);
	}

	SSERROR SetIteration(BOOL bIteration, int nMaxIterations, double nMaxChange)
	{
		return SSSetIteration(m_hSS, bIteration, nMaxIterations, nMaxChange);
	}

	SSERROR SetLeftCol(RC nLeftCol)
	{
		return SSSetLeftCol(m_hSS, nLeftCol);
	}

	SSERROR SetLineStyle(int nStyle, COLORREF crColor, int nWeight)
	{
		return SSSetLineStyle(m_hSS, nStyle, crColor, nWeight);
	}

	SSERROR SetLogicalRC(RC nRow, RC nCol, BOOL bIsTrue)
	{
		return SSSetLogicalRC(m_hSS, nRow, nCol, bIsTrue);
	}

	SSERROR SetMinCol(RC nMinCol)
	{
		return SSSetMinCol(m_hSS, nMinCol);
	}

	SSERROR SetMinRow(RC nMinRow)
	{
		return SSSetMinRow(m_hSS, nMinRow);
	}

	SSERROR SetMaxCol(RC nMaxCol)
	{
		return SSSetMaxCol(m_hSS, nMaxCol);
	}

	SSERROR SetMaxRow(RC nMaxRow)
	{
		return SSSetMaxRow(m_hSS, nMaxRow);
	}

	SSERROR SetMode(int nMode)
	{
		return SSSetMode(m_hSS, nMode);
	}

	SSERROR SetNumber(double nNumber)
	{
		return SSSetNumber(m_hSS, nNumber);
	}

	SSERROR SetNumberFormat(LPCSTR pNumberFormat)
	{
		return SSSetNumberFormat(m_hSS, pNumberFormat);
	}

	SSERROR SetNumberRC(RC nRow, RC nCol, double nNumber)
	{
		return SSSetNumberRC(m_hSS, nRow, nCol, nNumber);
	}

	SSERROR SetPattern(int nPattern, COLORREF crFG, COLORREF crBG)
	{
		return SSSetPattern(m_hSS, nPattern, crFG, crBG);
	}

	SSERROR SetPolyEditMode(int nPolyEditMode)
	{
		return SSSetPolyEditMode(m_hSS, nPolyEditMode);
	}

	SSERROR SetPrintArea(LPCSTR pFormula)
	{
		return SSSetPrintArea(m_hSS, pFormula);
	}

	SSERROR SetPrintAreaFromSelection()
	{
		return SSSetPrintAreaFromSelection(m_hSS);
	}

	SSERROR SetPrintBottomMargin(double nMargin)
	{
		return SSSetPrintBottomMargin(m_hSS, nMargin);
	}

	SSERROR SetPrintColHeading(BOOL bColHeading)
	{
		return SSSetPrintColHeading(m_hSS, bColHeading);
	}

	SSERROR SetPrintFooter(LPCSTR pPrintFooter)
	{
		return SSSetPrintFooter(m_hSS, pPrintFooter);
	}

	SSERROR SetPrintGridLines(BOOL bGridLines)
	{
		return SSSetPrintGridLines(m_hSS, bGridLines);
	}

	SSERROR SetPrintHCenter(BOOL bHCenter)
	{
		return SSSetPrintHCenter(m_hSS, bHCenter);
	}

	SSERROR SetPrintHeader(LPCSTR pPrintHeader)
	{
		return SSSetPrintHeader(m_hSS, pPrintHeader);
	}

	SSERROR SetPrintLeftMargin(double nMargin)
	{
		return SSSetPrintLeftMargin(m_hSS, nMargin);
	}

	SSERROR SetPrintLeftToRight(BOOL bLeftToRight)
	{
		return SSSetPrintLeftToRight(m_hSS, bLeftToRight);
	}

	SSERROR SetPrintNoColor(BOOL bNoColor)
	{
		return SSSetPrintNoColor(m_hSS, bNoColor);
	}

	SSERROR SetPrintRightMargin(double nMargin)
	{
		return SSSetPrintRightMargin(m_hSS, nMargin);
	}

	SSERROR SetPrintRowHeading(BOOL bRowHeading)
	{
		return SSSetPrintRowHeading(m_hSS, bRowHeading);
	}

	SSERROR SetPrintScale(int nScale, BOOL bFitToPage, int nVPages, int nHPages)
	{
		return SSSetPrintScale(m_hSS, nScale, bFitToPage, nVPages, nHPages);
	}

	SSERROR SetPrintTitles(LPCSTR pFormula)
	{
		return SSSetPrintTitles(m_hSS, pFormula);
	}

	SSERROR SetPrintTitlesFromSelection()
	{
		return SSSetPrintTitlesFromSelection(m_hSS);
	}

	SSERROR SetPrintTopMargin(double nMargin)
	{
		return SSSetPrintTopMargin(m_hSS, nMargin);
	}

	SSERROR SetPrintVCenter(BOOL bVCenter)
	{
		return SSSetPrintVCenter(m_hSS, bVCenter);
	}

	SSERROR SetProtection(BOOL bLocked, BOOL bHidden)
	{
		return SSSetProtection(m_hSS, bLocked, bHidden);
	}

	SSERROR SetRepaint(BOOL bRepaint)
	{
		return SSSetRepaint(m_hSS, bRepaint);
	}

	SSERROR SetRowHeight(RC nR1, RC nR2, int nHeight, BOOL bDefRowHeight)
	{
		return SSSetRowHeight(m_hSS, nR1, nR2, nHeight, bDefRowHeight);
	}

	SSERROR SetRowHeightAuto(RC nR1, RC nC1, RC nR2, RC nC2, BOOL bSetDefaults)
	{
		return SSSetRowHeightAuto(m_hSS, nR1, nC1, nR2, nC2, bSetDefaults);
	}

	SSERROR SetRowMode(BOOL bRowMode)
	{
		return SSSetRowMode(m_hSS, bRowMode);
	}

	SSERROR SetRowText(RC nRow, LPCSTR pRowText)
	{
		return SSSetRowText(m_hSS, nRow, pRowText);
	}

	SSERROR SetScale(int nScale)
	{
		return SSSetScale(m_hSS, nScale);
	}

	SSERROR SetSelection(RC nR1, RC nC1, RC nR2, RC nC2)
	{
		return SSSetSelection(m_hSS, nR1, nC1, nR2, nC2);
	}

	SSERROR SetSelectionRef(LPCSTR pFormula)
	{
		return SSSetSelectionRef(m_hSS, pFormula);
	}

	SSERROR SetShowColHeading(BOOL bColHeading)
	{
		return SSSetShowColHeading(m_hSS, bColHeading);
	}

	SSERROR SetShowFormulas(BOOL bFormulas)
	{
		return SSSetShowFormulas(m_hSS, bFormulas);
	}

	SSERROR SetShowGridLines(BOOL bGridLines)
	{
		return SSSetShowGridLines(m_hSS, bGridLines);
	}

	SSERROR SetShowHScrollBar(int nShowHScrollBar)
	{
		return SSSetShowHScrollBar(m_hSS, nShowHScrollBar);
	}

	SSERROR SetShowRowHeading(BOOL bRowHeading)
	{
		return SSSetShowRowHeading(m_hSS, bRowHeading);
	}

	SSERROR SetShowSelections(int nSelections)
	{
		return SSSetShowSelections(m_hSS, nSelections);
	}

	SSERROR SetShowVScrollBar(int nShowVScrollBar)
	{
		return SSSetShowVScrollBar(m_hSS, nShowVScrollBar);
	}

	SSERROR SetShowZeroValues(BOOL bZeroValues)
	{
		return SSSetShowZeroValues(m_hSS, bZeroValues);
	}

	SSERROR SetSSEdit(HSSEDIT hSSEdit)
	{
		return SSSetSSEdit(m_hSS, hSSEdit);
	}

	SSERROR SetSSEdit(CSSEdit *pSSEdit)
	{
		return SSSetSSEdit(m_hSS, *pSSEdit);
	}

	SSERROR SetText(LPCSTR pText)
	{
		return SSSetText(m_hSS, pText);
	}

	SSERROR SetTextRC(RC nRow, RC nCol, LPCSTR pText)
	{
		return SSSetTextRC(m_hSS, nRow, nCol, pText);
	}

	SSERROR SetTitle(LPCSTR pTitle)
	{
		return SSSetTitle(m_hSS, pTitle);
	}

	SSERROR SetTopLeftText(LPCSTR pTopLeftText)
	{
		return SSSetTopLeftText(m_hSS, pTopLeftText);
	}

	SSERROR SetTopRow(RC nTopRow)
	{
		return SSSetTopRow(m_hSS, nTopRow);
	}

	SSERROR ShowActiveCell()
	{
		return SSShowActiveCell(m_hSS);
	}

	SSERROR Sort(RC nR1, RC nC1, RC nR2, RC nC2, BOOL bSortByRows, LPINT pKeys, int nKeys)
	{
		return SSSort(m_hSS, nR1, nC1, nR2, nC2, bSortByRows, pKeys, nKeys);
	}

	SSERROR Sort3(RC nR1, RC nC1, RC nR2, RC nC2, BOOL bSortByRows, int nKey1, int nKey2, int nKey3)
	{
		return SSSort3(m_hSS, nR1, nC1, nR2, nC2, bSortByRows, nKey1, nKey2, nKey3);
	}

	SSERROR SortDlg()
	{
		return SSSortDlg(m_hSS);
	}

	SSERROR StartEdit(BOOL bClear, BOOL bInCellEditFocus, BOOL bArrowsExitEditMode)
	{
		return SSStartEdit(m_hSS, bClear, bInCellEditFocus, bArrowsExitEditMode);
	}

	SSERROR SwapTables(HSS hSS2)
	{
		return SSSwapTables(m_hSS, hSS2);
	}

	SSERROR TransactCommit()
	{
		return SSTransactCommit(m_hSS);
	}

	SSERROR TransactRollback()
	{
		return SSTransactRollback(m_hSS);
	}

	SSERROR TransactStart()
	{
		return SSTransactStart(m_hSS);
	}

	SSERROR TwipsToRC(LONG x, LONG y, LPRC pRow, LPRC pCol)
	{
		return SSTwipsToRC(m_hSS, x, y, pRow, pCol);
	}

	static void SSUpdate()
	{
		SSUpdate();
	}

	static WORD SSVersion()
	{
		return SSVersion();
	}

	SSERROR Write(LPCSTR pPathName, int nFileType)
	{
		return SSWrite(m_hSS, pPathName, nFileType);
	}

	SSERROR WriteIO(DWORD dwUserData, IOFUNC ioFunc, DWORD FAR *pUserRet)
	{
		return SSWriteIO(m_hSS, dwUserData, ioFunc, pUserRet);
	}
	
private:
	BOOL m_bDeleteInDestructor;
	HSS m_hSS;
};

typedef CSS FAR *LPSS;

#endif	// __cplusplus

#endif // __VTSS_H_
