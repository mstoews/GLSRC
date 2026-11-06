/*
   General Ledger  - (C) Copyright 1992 by Tegara Corporation
   Income Statement - Report
*/
#include <bwcc.h>
#include <tbowl.h>

#include <commdlg.h>
#include "d4all.h"
#include "d4data.hpp"
#ifndef  __GLCLASS_H
#include "glclass.h"
#endif

#ifndef  __GL_H
#include "gl.h"
#endif

#include "gl_stat.h"
#include "glrepts.h"
#include <math.h>
#include <dir.h>

#include "db_accts.h"
#include "db_dept.h"
#include "db_profi.h"
#include "db_btchd.h"
#include "db_btch.h"
#include "db_dist.h"
#include "db_alloc.h"
#include "db_srce.h"

#include "gl_main.h"
#include "gl_dlg.h"
#include "gl_wks.h"




extern Code4 cb;
extern HMENU HGLMenu,HGLExMenu, HARMenu, HAPMenu;
//BOOL FAR PASCAL GetAccountValue(LPSS_VALUE Result, LPSS_VALUE Values, short ValueCnt);

void TSetBorder::WMInitDialog (RTMessage)
{
 SendDlgItemMsg(4003,BM_SETCHECK,1,0L);
 SendDlgItemMsg(4007,BM_SETCHECK,1,0L);
 BORDERTYPE = SS_BORDERTYPE_BOTTOM;
 BORDERSTYLE = SS_BORDERSTYLE_SOLID;
}

void TSetBorder::Ok  (RTMessage){
 if(ecol == 0)
 {
  Spread->SetBorder(col,row,BORDERTYPE,BORDERSTYLE,NULL);
 }
 else
 Spread->SetBorderRange(col,row,ecol,erow,BORDERTYPE,BORDERSTYLE,NULL);
 InvalidateRect(Spread->HWindow, NULL, TRUE );
 Spread->SetBool(SSB_CHANGEMADE,TRUE);
 ShutDownWindow();
}

void TSetBorder::NoBorderBtn      (RTMessage){
 SendDlgItemMsg(4001,BM_SETCHECK,1,0L);
 SendDlgItemMsg(4002,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4003,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4004,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4005,BM_SETCHECK,0,0L);
 BORDERSTYLE = SS_BORDERTYPE_NONE;
}
void TSetBorder::TopBtn           (RTMessage) {
 SendDlgItemMsg(4001,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4002,BM_SETCHECK,1,0L);
 SendDlgItemMsg(4003,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4004,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4005,BM_SETCHECK,0,0L);
 BORDERSTYLE = SS_BORDERTYPE_TOP;
}
void TSetBorder::BottomBtn        (RTMessage) {
 SendDlgItemMsg(4001,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4002,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4003,BM_SETCHECK,1,0L);
 SendDlgItemMsg(4004,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4005,BM_SETCHECK,0,0L);
 BORDERSTYLE = SS_BORDERTYPE_BOTTOM;
}
void TSetBorder::RightBtn         (RTMessage) {
 SendDlgItemMsg(4001,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4002,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4003,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4004,BM_SETCHECK,1,0L);
 SendDlgItemMsg(4005,BM_SETCHECK,0,0L);
 BORDERSTYLE = SS_BORDERTYPE_RIGHT;
}
void TSetBorder::LeftBtn          (RTMessage) {
 SendDlgItemMsg(4001,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4002,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4003,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4004,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4005,BM_SETCHECK,1,0L);
 BORDERSTYLE = SS_BORDERTYPE_LEFT;
}
void TSetBorder::NoneBtn          (RTMessage) {
 SendDlgItemMsg(4006,BM_SETCHECK,1,0L);
 SendDlgItemMsg(4007,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4008,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4009,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4010,BM_SETCHECK,0,0L);
 BORDERSTYLE = SS_BORDERSTYLE_NONE;
}
void TSetBorder::SolidBtn         (RTMessage) {
 SendDlgItemMsg(4006,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4007,BM_SETCHECK,1,0L);
 SendDlgItemMsg(4008,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4009,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4010,BM_SETCHECK,0,0L);
 BORDERSTYLE = SS_BORDERSTYLE_SOLID;
}
void TSetBorder::DashBtn          (RTMessage) {
 SendDlgItemMsg(4006,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4007,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4008,BM_SETCHECK,1,0L);
 SendDlgItemMsg(4009,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4010,BM_SETCHECK,0,0L);
 BORDERSTYLE = SS_BORDERSTYLE_DASHDOT;
}
void TSetBorder::DotBtn           (RTMessage) {
 SendDlgItemMsg(4006,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4007,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4008,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4009,BM_SETCHECK,1,0L);
 SendDlgItemMsg(4010,BM_SETCHECK,0,0L);
 BORDERSTYLE = SS_BORDERSTYLE_DOT;
}
void TSetBorder::DashDotBtn       (RTMessage) {
 SendDlgItemMsg(4006,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4007,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4008,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4009,BM_SETCHECK,0,0L);
 SendDlgItemMsg(4010,BM_SETCHECK,1,0L);
 BORDERSTYLE = SS_BORDERSTYLE_DASHDOT;
}


void TSetValue::SetupWindow()
{
 TDialog::SetupWindow();
 HCURSOR  hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT)) ;
 ShowCursor (TRUE);
 IntSetValue(GetDlgItem(HWindow,ID_ROW),0);
 IntSetValue(GetDlgItem(HWindow,ID_COL),0);
 cb.optimize = -1;
 glaccts = (Tglaccts *) new Tglaccts(&cb,"glaccts");
 RECT rc;
 SS_CELLTYPE 	CellType;
 LOGFONT 	lfFont;
 HFONT   	hFont;

 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 GetClientRect(SHWindow,&rc);  
 short d1Width,d2Width,d4Width;
 SSSetColWidth(SHWindow,1,7);
 SSSetColWidth(SHWindow,2,5);
 SSSetColWidth(SHWindow,4,10);
 SSGetColWidthInPixels(SHWindow,1,(LPSHORT)&d1Width);
 SSGetColWidthInPixels(SHWindow,2,(LPSHORT)&d2Width);
 SSGetColWidthInPixels(SHWindow,4,(LPSHORT)&d4Width);
 short ColWidth = (rc.right - rc.left) - d1Width - d2Width - d4Width- GetSystemMetrics(SM_CXVSCROLL);
 SSSetColWidthInPixels(SHWindow,3,ColWidth); 
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);
 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
 SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetBool(SHWindow,SSB_ALLOWUSERRESIZE,FALSE);
 //SSSetBool(SHWindow,SSB_AUTOSIZE,TRUE);
 SSSetBool(SHWindow,SSB_HORZSCROLLBAR,FALSE);


 //lfFont.lfHeight          = -13;
 lfFont.lfHeight          = -(GetFontSize());
 lfFont.lfWidth           = 0;
 lfFont.lfEscapement      = 0;
 lfFont.lfOrientation     = 0;
 lfFont.lfWeight          = FW_NORMAL;
 lfFont.lfItalic          = 0;
 lfFont.lfUnderline       = 0;
 lfFont.lfStrikeOut       = 0;
 lfFont.lfCharSet         = USER_CHAR_SET; //default character set.
  lfFont.lfOutPrecision    = 0;
 lfFont.lfClipPrecision   = 0;
 lfFont.lfQuality         = 0;
 lfFont.lfPitchAndFamily  = FF_SWISS;
 StrCpy (lfFont.lfFaceName, "HELV");
 hFont = CreateFontIndirect((LPLOGFONT) &lfFont);

 SSSetMaxCols(SHWindow, 4);
 SSSetMaxRows(SHWindow,glaccts->recCount());

 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 SSSetData(SHWindow,1,SS_HEADER, "Account");
 SSSetData(SHWindow,2,SS_HEADER, "Dept");
 SSSetData(SHWindow,3,SS_HEADER, "Description");
 SSSetData(SHWindow,4,SS_HEADER, "Period Balance");
 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,SS_ALLCOLS, SS_ALLROWS, &CellType);


 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,13,2,0,999999999999.99);
 SSSetCellType(SHWindow,4,SS_ALLROWS,&CellType);
 glaccts->SetIndex("account");
 glaccts->top();
 long i = 1;
 while (!glaccts->eof())
  {
   SSSetData(SHWindow,1,i,glaccts->accounts->str());
   SSSetData(SHWindow,2,i,glaccts->dept->str());
   SSSetData(SHWindow,3,i,glaccts->descript->str());
   SSSetData(SHWindow,4,i,glaccts->curr_bal->str());
   i++;
   glaccts->skip(1);
  }
 glaccts->close();
 SS_COORD Col,Row;
 SSGetActiveCell(Spread->HWindow,&Col,&Row);

 ShowCursor (FALSE);
 SetCursor (hCursor) ;
}

void TSetValue::Ok (RTMessage)
{
  char szSAccount[8];
  char szSDept[5]; 
  SS_COORD Col,Row;
  SSGetActiveCell(Spread->HWindow,&Col,&Row);
  if(Col < 1 || Row < 1) return; 
  SendDlgItemMsg(ID_ACCOUNTS,WM_GETTEXT,sizeof(szSAccount),(LONG)(LPSTR)szSAccount);
  SendDlgItemMsg(ID_DEPART,WM_GETTEXT,sizeof(szSDept),(LONG)(LPSTR)szSDept);
  SSSetTypeFloat(GetDlgItem(Parent->HWindow,ID_SPREAD),&CellType,FS_SEPARATOR,15,2,0,9999999999999.99);

  SSSetCellType(GetDlgItem(Parent->HWindow,ID_SPREAD),Col,Row, &CellType);
}

void TSetValue::Budget (RTMessage)
{
  char szSAccount[8];
  char szSDept[5];
  char szPeriod[2];
  SS_COORD Col,Row;
  SSGetActiveCell(Spread->HWindow,&Col,&Row);
  if(Col < 1 || Row < 1) return;

  SendDlgItemMsg(ID_ACCOUNTS,WM_GETTEXT,sizeof(szSAccount),(LONG)(LPSTR)szSAccount);
  SendDlgItemMsg(ID_DEPART,WM_GETTEXT,sizeof(szSDept),(LONG)(LPSTR)szSDept);
  SSSetTypeFloat(GetDlgItem(Parent->HWindow,ID_SPREAD),&CellType,FS_SEPARATOR,15,2,0,9999999999999.99);
  SSSetCellType(GetDlgItem(Parent->HWindow,ID_SPREAD),Col,Row, &CellType);
  char szFormula[85];
  StrCpy(szFormula,"PD(");
  StrCat(szFormula,szSAccount);
  StrCat(szFormula,",");
  if(*szSDept != 0)
  StrCat(szFormula,szSDept);
  else
  StrCat(szFormula,"0");
  StrCat(szFormula,",");
  itoa((int)GetCurrentPeriodn(),szPeriod,10);
  StrCat(szFormula,szPeriod);
  StrCat(szFormula,")");
  Spread->SetFormula(Col,Row,szFormula,TRUE);
}
void TSetValue::CurrentBal (RTMessage)
{
  char szSAccount[8];
  char szSDept[5];
  char szPeriod[2];
  SS_COORD Col,Row;
  SSGetActiveCell(Spread->HWindow,&Col,&Row);
  if(Col < 1 || Row < 1) return;
 
  SendDlgItemMsg(ID_ACCOUNTS,WM_GETTEXT,sizeof(szSAccount),(LONG)(LPSTR)szSAccount);
  SendDlgItemMsg(ID_DEPART,WM_GETTEXT,sizeof(szSDept),(LONG)(LPSTR)szSDept);
  SSSetTypeFloat(GetDlgItem(Parent->HWindow,ID_SPREAD),&CellType,FS_SEPARATOR,15,2,0,9999999999999.99);
  SSSetCellType(GetDlgItem(Parent->HWindow,ID_SPREAD),Col,Row, &CellType);
  char szFormula[85];
  StrCpy(szFormula,"PD(");
  StrCat(szFormula,szSAccount);
  StrCat(szFormula,",");
  if(*szSDept != 0)
  StrCat(szFormula,szSDept);
  else
  StrCat(szFormula,"0");
  StrCat(szFormula,",");
  itoa((int)GetCurrentPeriodn(),szPeriod,10);
  StrCat(szFormula,szPeriod);
  StrCat(szFormula,")");
  Spread->SetFormula(Col,Row,szFormula,TRUE);

}

void TSetValue::Set(RTMessage)
{
  SS_COORD Col,Row;
  SSGetActiveCell(Spread->HWindow,&Col,&Row);
  LPSTR lpFormula;
  GLOBALHANDLE hFormula;
  hFormula = GlobalAlloc(GHND,(DWORD)lstrlen(GetField(ID_FORMULA)));
  lpFormula = GlobalLock(hFormula);
  lstrcpy(lpFormula,GetField(ID_FORMULA));
  Spread->SetFormula(Col,Row,lpFormula,TRUE);
  GlobalUnlock(hFormula);  
  GlobalFree(hFormula);
}
void TSetValue::Get(RTMessage)
{
  SS_COORD Col,Row;
  SSGetActiveCell(Spread->HWindow,&Col,&Row);
  LPSTR lpFormula;
  GLOBALHANDLE hFormula;
  hFormula = GlobalAlloc(GHND,(DWORD)Spread->GetFormulaLen(Col,Row));
  lpFormula = GlobalLock(hFormula);
  Spread->GetFormula(Col,Row,lpFormula);
  GlobalUnlock(hFormula);
  SetDlgItemText(HWindow,ID_FORMULA,lpFormula);
  GlobalFree(hFormula);
}


void TSetValue::HandleSearchBtn (RTMessage Msg)
{ 
  HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
  LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
  char szAccount[8],szDept[5];
  SS_COORD Col,Row;
  SSGetData(SHWindow,1,Cell->Row,szAccount); //Account Number            
  SendDlgItemMsg(ID_ACCOUNTS,WM_SETTEXT,NULL,(LONG)(LPSTR)szAccount);
  SSGetData(SHWindow,2,Cell->Row,szDept); //Department
  SendDlgItemMsg(ID_DEPART,WM_SETTEXT,NULL,(LONG)(LPSTR)szDept);
  SSGetActiveCell(Spread->HWindow,&Col,&Row);
  IntSetValue(GetDlgItem(HWindow,ID_ROW),Row);
  IntSetValue(GetDlgItem(HWindow,ID_COL),Col);
}


/*

_CLASSDEF(TGLAccountInq)
class TGLAccountInq: public TWorksheet
{
 public:
 TGLAccountInq(PTWindowsObject AParent, LPSTR ATitle):TWorksheet(AParent,ATitle)
 {
 Spread = new TSpread(this,ID_SPREAD,"",0,0,0,0);
 strcpy(szTitle,ATitle);
 };
 virtual void SetupWindow();
 virtual BOOL CanClose (){return TRUE;};
 virtual void WMRButtonDown  (RTMessage Msg) = [WM_FIRST + SSM_RBUTTON]{};
};

void TGLAccountInq::SetupWindow()
{ 
 TWindow::SetupWindow();
 HCURSOR  hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT)) ;
 ShowCursor (TRUE);

 PTglaccts glaccts = new Tglaccts(&cb);
 PTgldept  gldept  = new Tgldept (&cb);
 glaccts->SetIndex("accounts");

 Spread->SetBool(SSB_ALLOWUSERSELBLOCK,TRUE);
 Spread->SetBool(SSB_ALLOWUSERFORMULAS,TRUE);
 Spread->SetBool(SSB_ALLOWDRAGDROP,TRUE);
 Spread->SetBool(SSB_CHANGEMADE,FALSE);
 Spread->SetMaxCols(20);
 Spread->SetTypeEdit (&CellType,ES_LEFT,50,SS_CHRSET_CHR,SS_CASE_NOCASE);
 Spread->SetCellTypeRange(1,1,30,3,&CellType);
 Spread->SetCellTypeRange(1,1,4,200,&CellType);
 Spread->SetTypeFloat(&CellType,FS_SEPARATOR,9,2,0,9999999999999.99);
 Spread->SetCellTypeRange(5,4,30,200,&CellType);
 Spread->SetColWidth(1,9);
 Spread->SetColWidth(2,6);
 Spread->SetColWidth(3,20);
 Spread->SetColWidth(4,2);
 lfFont.lfHeight          = USER_CHAR_SIZE;
 lfFont.lfWidth           = 0;
 lfFont.lfEscapement      = 0;
 lfFont.lfOrientation     = 0;
 lfFont.lfWeight          = FW_NORMAL;
 lfFont.lfItalic          = 0;
 lfFont.lfUnderline       = 0;
 lfFont.lfStrikeOut       = 0;
 lfFont.lfCharSet         = USER_CHAR_SET; //default character set.
  lfFont.lfOutPrecision    = 0;
 lfFont.lfClipPrecision   = 0;
 lfFont.lfQuality         = 0;
 lfFont.lfPitchAndFamily  = FF_SWISS;
 StrCpy (lfFont.lfFaceName, "HELV");
 hFont = CreateFontIndirect((LPLOGFONT) &lfFont);
 
 Spread->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 Spread->SetColHeaderDisplay(SS_HEADERDISPLAY_BLANK);
 Spread->SetData(1,1,"General Ledger Account Inquiry");
 {
 for(int i=1;i<20;i++)
 {
  Spread->SetData(i,2, "Acct");i++;
  Spread->SetData(i,2, "Dept");            i++;
  Spread->SetData(i,2, "Description");     i++;
  Spread->SetData(i,2, "Tp");              i++;
  Spread->SetData(i,2, "Current Balance"); i++;
  Spread->SetData(i,2, "April");           i++;
  Spread->SetData(i,2, "May");             i++;
  Spread->SetData(i,2, "June");            i++;
  Spread->SetData(i,2, "July");            i++;
  Spread->SetData(i,2, "August");          i++;
  Spread->SetData(i,2, "September");       i++;
  Spread->SetData(i,2, "October");         i++;
  Spread->SetData(i,2, "November");        i++;
  Spread->SetData(i,2, "December");        i++;
  Spread->SetData(i,2, "January");         i++;
  Spread->SetData(i,2, "February");        i++;
  Spread->SetData(i,2, "March");           i++;
  Spread->SetData(i,2, "Open Bal");        i++;
  Spread->SetData(i,2, "Total");           i++;
 }
 }

 long i=5;
 glaccts->top();
 Spread->SetBool(SSB_REDRAW,FALSE);
 while(!glaccts->eof())
 {
     Spread->SetColWidth(i,12);
     Spread->SetData(1,i,glaccts->accounts->str());
     Spread->SetData(2,i,glaccts->dept->str());
     Spread->SetData(3,i,glaccts->descript->str());
     Spread->SetData(4,i,glaccts->type->str());
     Spread->SetData(5,i,glaccts->curr_bal->str());
     Spread->SetData(6,i,glaccts->period1->str());
     Spread->SetData(7,i,glaccts->period2->str());
     Spread->SetData(8,i,glaccts->period3->str());
     Spread->SetData(9,i,glaccts->period4->str());
     Spread->SetData(10,i,glaccts->period5->str());
     Spread->SetData(11,i,glaccts->period6->str());
     Spread->SetData(12,i,glaccts->period7->str());
     Spread->SetData(13,i,glaccts->period8->str());
     Spread->SetData(14,i,glaccts->period9->str());
     Spread->SetData(15,i,glaccts->period10->str());
     Spread->SetData(16,i,glaccts->period11->str());
     Spread->SetData(17,i,glaccts->period12->str());
     Spread->SetData(18,i,glaccts->previous12->str());
     Spread->SetFormula(19,i,"F#:R#",FALSE);
     i++;
     glaccts->skip(1);
 }
 Spread->SetMaxRows(i);
 Spread->SetBool(SSB_REDRAW,TRUE);
 Spread->ReCalc();
 SetCaption("General Ledger Inquiry");
 delete glaccts;
 delete gldept;
 cb.closeAll();
 ShowCursor (FALSE);
 SetCursor (hCursor) ;

}
*/
