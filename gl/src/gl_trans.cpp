/*
   General Ledger  - (C) Copyright 1993 by Tegara Corporation
   Delete general ledger accounts.
*/

#include <tbowl.h>
#include "d4all.hpp"
#include "glclass.h"
#include "gl.h"
#include "glb.h"

#include "db_accts.h"
#include "db_dept.h"
#include "db_profi.h"
#include "db_btchd.h"
#include "db_btch.h"
#include "db_dist.h"
#include "db_srce.h"
#include "math.h"
#include "gl_main.h"
#include "gl_acctl.h"
#include "gl_dlg.h"

extern Code4 cb;

_CLASSDEF(TGLTransInq)
class TGLTransInq : public TMainDialog
{
 public:
 PTglaccts glaccts;
 PTglbtchd glbtchd;
 TAccountList *GLACCOUNT;
 TGLTransInq(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TMainDialog(AParent,AName){};
  TGLTransInq(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName){};
 virtual void SetupWindow();
 virtual void Search         (RTMessage Msg) = [ID_FIRST  + ID_SEARCH];
 virtual void Print          (RTMessage Msg) = [ID_FIRST  + ID_PRINT]{};
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST  + IDCANCEL];
 virtual void SetData        (RTMessage Msg) = [CM_FIRST  + ID_ACCTLIST];
 virtual void Update         (RTMessage Msg) = [ID_FIRST  + ID_SETDATA];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST  + IDOK];
 void SetBatchList(char *szAccount);
};


void TGlWindow::GLTransInquiry (RTMessage)
 {
  GetApplication()->ExecDialog(new TGLTransInq(this,1029));
 }

void TGLTransInq::Update (RTMessage)
{
 SendMessage(HWindow,WM_COMMAND,ID_ACCTLIST,NULL);
}

void TGLTransInq::Ok             (RTMessage ){}

void TGLTransInq::Cancel (RTMessage)
{
 delete glaccts;
 ShutDownWindow();
}

void TGLTransInq::SetupWindow()
{
 HourGlassOn();
 TDialog::SetupWindow();
 glbtchd = new Tglbtchd(&cb);
 glaccts = new Tglaccts(&cb);
 glaccts->SetIndex("account");

 glbtchd->SetIndex("acctdept");

 GLACCOUNT = new TAccountList(this,"",glaccts);
 GLACCOUNT->EnableAutoCreate();
 GetApplication()->MakeWindow(GLACCOUNT);


 HWND SHWindow;
 RECT rc;
 SS_CELLTYPE 	CellType;
 LOGFONT 	lfFont;
 HFONT   	hFont;


 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 
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
 GetClientRect(SHWindow,&rc);
 short d1Width,d2Width,d3Width,d5Width,d6Width;
 SSSetMaxCols(SHWindow,5);
 SSSetColWidth(SHWindow,1,4);
 SSSetColWidth(SHWindow,2,7);
 SSSetColWidth(SHWindow,4,10);
 SSSetColWidth(SHWindow,5,10);

 SSGetColWidthInPixels(SHWindow,1,(LPSHORT)&d1Width);
 SSGetColWidthInPixels(SHWindow,2,(LPSHORT)&d2Width);
 SSGetColWidthInPixels(SHWindow,4,(LPSHORT)&d3Width);
 SSGetColWidthInPixels(SHWindow,5,(LPSHORT)&d5Width);

 short ColWidth = (rc.right - rc.left)- d1Width
						- d2Width
						- d3Width  // The fourth column is adjusted.
						- d5Width
						- GetSystemMetrics(SM_CXHSCROLL)+1;
 SSSetColWidthInPixels(SHWindow,3,ColWidth);

 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);
 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,TRUE);
 SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);

 SSSetData(SHWindow,1,SS_HEADER, "Pd");
 SSSetData(SHWindow,2,SS_HEADER, "Btch No.");
 SSSetData(SHWindow,3,SS_HEADER, "Description");
 SSSetData(SHWindow,4,SS_HEADER, "Debit");
 SSSetData(SHWindow,5,SS_HEADER, "Credit");

 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);

 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,999999999999.99);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,5, SS_ALLROWS, &CellType);
 glaccts->top();
 glbtchd->top();
 SendDlgItemMsg(ID_ACCOUNTS,WM_SETTEXT,glaccts->accounts->len(),(LONG)(LPSTR)glaccts->accounts->str());
 SendDlgItemMsg(ID_DESCRIPT,WM_SETTEXT,glaccts->descript->len(),(LONG)(LPSTR)glaccts->descript->str());
 SendDlgItemMsg(ID_DEPART,WM_SETTEXT,glaccts->dept->len(),(LONG)(LPSTR)glaccts->dept->str());
 HourGlassOff();
}


void TGLTransInq::Search(RTMessage)
{
 if(!IsWindowVisible(GLACCOUNT->HWindow))
   GLACCOUNT->Show(SW_SHOW);
 else
   GLACCOUNT->Show(SW_HIDE);
}

void TGLTransInq::SetData(RTMessage)
{
 char *szAccount = new char[glaccts->accounts->len() + glaccts->dept->len() + 1];
 char *szDept     = new char[glaccts->dept->len() + 1];
 SendDlgItemMsg(ID_ACCOUNTS,WM_GETTEXT,sizeof(szAccount),(LONG)(LPSTR)szAccount);
 SendDlgItemMsg(ID_DEPART,WM_GETTEXT,sizeof(szDept),(LONG)(LPSTR)szDept);
 strcat(szAccount,szDept);
 SetBatchList(szAccount);
 delete [] szAccount;
 delete [] szDept;
}


void TGLTransInq::SetBatchList(char *szAccount)
{
 glaccts->seek(szAccount);
 SendDlgItemMsg(ID_JDESCRIPT,WM_SETTEXT,NULL,(LPARAM)(LPSTR)glaccts->descript->str());
 HourGlassOn();
 double dDebit,dCredit,dDifference;
 dDebit = dCredit = dDifference = 0;
 char szBuffer[15];
 long i=4;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSClearData(SHWindow,SS_ALLCOLS,SS_ALLROWS);
 SSSetBool(SHWindow,SSB_REDRAW,FALSE);
 SSSetData(SHWindow,4,1,"Opening Account Balance");
 double dGLBalance = glaccts->Open_bal();
 glbtchd->SetIndex("account");
 char szQuery[200];
 char szDept[5];
 strcpy(szDept,GetField(ID_DEPART));
 wsprintf(szQuery,"ACCOUNT = '%s' .AND. DEPT = '%s' .AND. MODE = '0'", GetField(ID_ACCOUNTS),szDept);
 Relate4set *rQuery = new Relate4set(glbtchd);
 rQuery->querySet(szQuery);
 for(int qc = rQuery->top();qc!=r4eof;qc=rQuery->skip(1L) )
   {
	SSSetData(SHWindow,1,i,glbtchd->period->str());
	SSSetData(SHWindow,2,i,glbtchd->batchno->str());
	SSSetData(SHWindow,3,i,glbtchd->descript->str());
	SSSetData(SHWindow,4,i,glbtchd->debit->str());
	SSSetData(SHWindow,5,i,glbtchd->credit->str());
	i++;
	dDebit  += glbtchd->Debit();
	dCredit += glbtchd->Credit();
  }

 rQuery->unlock();
 rQuery->free();


double dBalance = dGLBalance;
dGLBalance = fabs(dBalance);
gcvt(dGLBalance,14,szBuffer);
if(dBalance > 0)
	 {
		SSSetData(SHWindow,4,1,szBuffer);
	 }
    else
    {
     SSSetData(SHWindow,5,1,szBuffer);
    }

 i++;
 SSSetData(SHWindow,3,i,"Account Balance");
 dDifference = dBalance + dDebit - dCredit;
 gcvt(dDifference,14,szBuffer);
 if(dDifference > 0)
  SSSetData(SHWindow,4,i,szBuffer);
 else
 {
  dDifference = fabs(dDifference);
  gcvt(dDifference,14,szBuffer);
  SSSetData(SHWindow,5,i,szBuffer);
 }
 if(i>17)
  SSSetMaxRows(SHWindow,i);
 else
  SSSetMaxRows(SHWindow,17);
 SSSetBool(SHWindow,SSB_REDRAW,TRUE);
 HourGlassOff();
}


