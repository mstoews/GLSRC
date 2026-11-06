/*
	General Ledger  - (C) Copyright 1992 by Tegara Corporation
	Modify Batch Entry
*/
#include <tbowl.h>
#include "d4all.hpp"
#include "gl.h"
#include "gl_manil.h"
#include "glclass.h"
#include "gl_main.h"
#include "gl_dlg.h"
//#include "gl_pick.h"
#include "crpe.h"
#include "db_acct.h"
#include "db_dept.h"
#include "db_dist.h"
#include "db_itemm.h"
#include "db_btchd.h"
#include "db_btch.h"
#include "db_medrp.h"
#include "tList_w.h"
#include "gl_medrl.h"
#include "gl_medrp.h"
#include "gl_btche.h"
#include "gl_distl.h"
#include "gl_pds.hpp"
#include "utildlg.h"



#define  ID_DESC  2999
#define ID_DRCODES 1098
#define ID_COMMIT     1056

void  SetPeriods(HWND hWnd);

BOOL Data4::seek(string s)
{
  if(d4seek( data, (char *)s.c_str()) == 0)
	return TRUE;
  else
	return FALSE;
}

BOOL Data4::seek(string* s)
{
  if(d4seek( data, (char *)s->c_str()) == 0)
	return TRUE;
  else
	return FALSE;
}



static int iCurrentSlipTotal;
static COLORREF lpBack, lpFore;
extern BOOL crpeOpen;
extern char szApp[20];
extern HINSTANCE hInst;

int GetCurrentPeriodn()
{
 GL_PDS gl_pds;
 gl_pds.open(cb);
 gl_pds.SetIndex("PERIOD");
 gl_pds.top();
 int rc = 0;
 while(!gl_pds.eof())
  {
	if(strcmp(gl_pds.status.str(),"0") == 0)
	 rc = int(Str4ptr(gl_pds.period.str()));
	gl_pds.skip();
  }
 return rc;
}


void TGLBatchEntry::SetPeriods(RTMessage Msg)
{
 char szPeriod[3];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  GL_PDS gl_pds;
  gl_pds.open(cb);
  gl_pds.SetIndex("PERIOD");
  int Indx = (int)SendDlgItemMsg(ID_PERIOD,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(ID_PERIOD,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szPeriod);
	 if(gl_pds.seek(szPeriod) == 0){
		SetField(1084,gl_pds.pd_desc.str());
	  }
	}
  gl_pds.close();
 }

}

void TGLBatchEntry::RollBack(RTMessage)
{
 HourGlassOn();
 int iBatchno = (int)lGetField(ID_BATCHNO);
 glbtch->SetIndex(GetString(8140));
 if(glbtch->seek(iBatchno) == 0)
	glbtch->deleteRec();
 glbtchd->SetIndex(GetString(8140));
 if(glbtchd->seek(iBatchno) == 0)
  while((int)glbtchd->Batchno() == iBatchno)
		{
	 glbtchd->deleteRec();
	 glbtchd->skip();
		}
 glbtchd->pack();
 glbtch->pack();
 HourGlassOff();
 Clear();
 return;
}


/*
  Initial the databases for the window.
  Check to make sure that the indexes and databases are okay.
  If locked or an index is missing return to the main window and
  reset the database accounts.
*/
BOOL TGLBatchEntry::TestDatabase()
 {
  return TRUE;
 }


void TGLBatchEntry::SetupWindow()
{
 HourGlassOn();
 bTransaction = FALSE;
 TMainDialog::SetupWindow();
 if(
	 !CheckFile(GetString(2046),Parent->HWindow) || // GLACCTS
	 !CheckFile(GetString(2047),Parent->HWindow) || // GLDEPT
	 !CheckFile(GetString(2050),Parent->HWindow) || // GL_ITEMM
	 !CheckFile(GetString(2051),Parent->HWindow) || // GL_MEDRP
	 !CheckFile(GetString(2052),Parent->HWindow) || // GL_DRCOD
	 !CheckFile(GetString(2053),Parent->HWindow) || // GL_BTCH
	 !CheckFile(GetString(2054),Parent->HWindow))   // GL_BTCHD
	{
	 ShutDownWindow();
	 return;
	}


 SetMode("1");
 DebitTotal=0;
 CreditTotal=0;
 ::SetPeriods(HWindow);

 glbtch   = new Tglbtch   (&cb);
 glbtchd  = new Tglbtchd  (&cb);
 glaccts  = new Tglaccts  (&cb);
 gldept   = new Tgldept   (&cb);
 glprod   = new Tprods    (&cb);
 glmedrep = new Tglmedreps (&cb);
 gldrcodes = new Tgldrcodes(&cb);


 LoadDoctors();
 LoadMedReps();
 LoadProducts();

 bAcctList = TRUE;
 bNewSlipList = TRUE;
 bMedReps = FALSE;
 bProducts = FALSE;

 AcctList = new TAccountList(this,"",glaccts);
 AcctList->EnableAutoCreate();
 AcctList->Attr.Style |= WS_THICKFRAME;
 GetApplication()->MakeWindow(AcctList);


 B = new TGLViewBatchList(this,"",glbtch);
 B->Attr.Style |= WS_THICKFRAME;
 B->EnableAutoCreate();
 GetApplication()->MakeWindow(B);


 D = new TGLDistList(this,GetString(8156),glbtchd);
 D->EnableAutoCreate();
 GetApplication()->MakeWindow(D);


 SSGetColor(GetDlgItem(D->HWindow,ID_DISTLIST),SS_ALLCOLS,SS_ALLROWS,&lpBack,&lpFore);
 double db;
 glbtch->bottom();
 if(glbtch->eof())
 {
  char szString[10];
  GetPrivateProfileString("PERIOD","BATCHNO","",szString,sizeof(szString),PROFILE);
  db = atof(szString);
 }
 else
 db = glbtch->Batchno();
 db++;
 IntSetDlgItemValue(HWindow,ID_BATCHNO,db);
 glaccts->SetIndex("accounts");
 glaccts->top();

 IntSetDlgItemValue(HWindow,ID_LINENO,1);

 ToolBox = new TGLToolBox(this,"ToolBox");
 ToolBox->Attr.Style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT | WS_POPUPWINDOW;
 GetApplication()->MakeWindow(ToolBox);

 RECT rc;
 GetWindowRect(HWindow,&rc);
 top 		= rc.top;
 bottom  = rc.bottom;
 right   = rc.right;
 left    = rc.left;


 SetBitmap(ID_TOP);
 SetBitmap(ID_BOTTOM);
 SetBitmap(ID_NEXT);
 SetBitmap(ID_PREVIOUS);
 SetBitmap(ID_NEW);
 SetBitmap(ID_SLIPLIST);
 SetBitmap(ID_ADD);
 SetBitmap(ID_DELETE);
 SetBitmap(ID_VIEW);
 SetBitmap(IDCANCEL);
 SetBitmap(ID_PRINT);

 D->Show(SW_SHOW);
 SetFocus(GetDlgItem(HWindow,ID_PERIOD));
 HourGlassOff();
}

void TGLBatchEntry::Print(RTMessage)
{
 if(glbtch->seek(dGetField(ID_BATCHNO)) == 0)
	 PrintSlip(D->GetReportName(),(int)dGetField(ID_BATCHNO));
}

void TGLBatchEntry::LoadDoctors(void)
 {
  SendDlgItemMsg(ID_DRCODES,CB_RESETCONTENT,NULL,NULL);
  gldrcodes->SetIndex("drcode");
  for(gldrcodes->top();!gldrcodes->eof();gldrcodes->skip())
	 SendDlgItemMsg(ID_DRCODES,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)gldrcodes->rpname->str());
  SendDlgItemMsg(ID_DRCODES,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)" ");

 }

void TGLBatchEntry::LoadMedReps(void)
{
  SendDlgItemMsg(ID_MEDREPS,CB_RESETCONTENT,NULL,NULL);
  glmedrep->SetIndex("rpname");

  for(glmedrep->top();!glmedrep->eof();glmedrep->skip())
	 SendDlgItemMsg(ID_MEDREPS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)glmedrep->rpname->str());
  SendDlgItemMsg(ID_MEDREPS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)" ");
}

void TGLBatchEntry::LoadProducts(void)
{
  SendDlgItemMsg(ID_PRODUCTS,CB_RESETCONTENT,NULL,NULL);
  glprod->SetIndex("ffulln");

  for(glprod->top();!glprod->eof();glprod->skip())
	SendDlgItemMsg(ID_PRODUCTS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)glprod->fFulln->str());
  SendDlgItemMsg(ID_PRODUCTS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)" ");
}


BOOL Tglbtch::Open()
{
 if(int(Str4ptr(mode->str())) == 1)
  return TRUE;
 else
  return FALSE;
}

BOOL Tglbtch::Closed()
{
 if(int(Str4ptr(mode->str())) == 0)
 return TRUE;
 else
 return FALSE;

}

BOOL Tglbtch::Adjustment()
{
 if(int(Str4ptr(mode->str())) == 3)
 return TRUE;
 else
 return FALSE;

}


void TGlWindow::GLBatchEnt(RTMessage)
 {
  GetApplication()->ExecDialog(new TGLBatchEntry(this,1013));
 }



void TGLDistList::SetupWindow()
{
 TWindow::SetupWindow();
 RECT rc;
 HourGlassOn();
 LOGFONT lfFont;
 HFONT   hFont;

 lfFont.lfHeight          = -(GetFontSize());
 lfFont.lfWidth           = 0;
 lfFont.lfEscapement      = 0;
 lfFont.lfOrientation     = 0;
 lfFont.lfWeight          = FW_NORMAL;
 lfFont.lfItalic          = 0;
 lfFont.lfUnderline       = 0;
 lfFont.lfStrikeOut       = 0;
 lfFont.lfCharSet         = 1; //default character set.
 lfFont.lfOutPrecision    = 0;
 lfFont.lfClipPrecision   = 0;
 lfFont.lfQuality         = 0;
 lfFont.lfPitchAndFamily  = FF_SWISS;
 StrCpy (lfFont.lfFaceName, "HELV");
 hFont = CreateFontIndirect((LPLOGFONT) &lfFont);
 List->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 List->SetBool(SSB_AUTOSIZE,FALSE);
 List->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
 List->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 List->SetBool(SSB_NOBEEP,TRUE);
 List->SetBool(SSB_ALLOWUSERRESIZE,TRUE);
 List->SetBool(SSB_PROTECT,TRUE);
 List->SetBool(SSB_SHOWROWHEADERS,FALSE);
 List->SetBool(SSB_NOBEEP,TRUE);



 List->GetColor(SS_ALLCOLS,SS_ALLROWS,&lpBack,&lpFore);

 List->SetMaxCols(6);
 List->GetClientRect(&rc);
 iLeft = iTop = iRight = iBottom = 0;
 string szClassName(GetClassName());
 iLeft    = GetPrivateProfileInt(szClassName.c_str(),"LEFT",0,PROFILE);
 iTop     = GetPrivateProfileInt(szClassName.c_str(),"TOP",0,PROFILE);
 iRight   = GetPrivateProfileInt(szClassName.c_str(),"RIGHT",0,PROFILE);
 iBottom  = GetPrivateProfileInt(szClassName.c_str(),"BOTTOM",0,PROFILE);
 if(iTop == 0 || iBottom == 0 || iLeft == 0 || iRight == 0)
 {
	iBottom = 100;
 }
 SetPosition();

 List->SetColWidth(1,3);
 List->SetColWidth(3,6);
 List->SetColWidth(4,4);
 List->SetColWidth(5,13);
 List->SetColWidth(6,13);

 List->GetColWidthInPixels(1,(LPSHORT)&d0Width);
 List->GetColWidthInPixels(3,(LPSHORT)&d2Width);
 List->GetColWidthInPixels(4,(LPSHORT)&d3Width);
 List->GetColWidthInPixels(5,(LPSHORT)&d4Width);
 List->GetColWidthInPixels(6,(LPSHORT)&d5Width);

 short ColWidth = (rc.right - rc.left)
		  - d0Width
		  - d2Width
		  - d3Width
		  - d4Width
		  - d5Width;


 List->SetColWidthInPixels(2,ColWidth);
 List->SetData(1,SS_HEADER, GetString(GetApplication()->hInstance,6067));
 List->SetData(2,SS_HEADER, GetString(GetApplication()->hInstance,6044));
 List->SetData(3,SS_HEADER, GetString(GetApplication()->hInstance,6068));
 List->SetData(4,SS_HEADER, GetString(GetApplication()->hInstance,6054));
 List->SetData(5,SS_HEADER, GetString(GetApplication()->hInstance,6069));
 List->SetData(6,SS_HEADER, GetString(GetApplication()->hInstance,6070));

 List->SetTypeEdit(&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 List->SetCellType(1, SS_ALLROWS, &CellType);
 List->SetCellType(2, SS_ALLROWS, &CellType);
 List->SetTypeEdit(&CellType,ES_RIGHT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 List->SetCellType(3, SS_ALLROWS, &CellType);
 List->SetCellType(4, SS_ALLROWS, &CellType);
 List->SetTypeFloat(&CellType,FS_SEPARATOR,15,2,0,9999999999999.99);
 List->SetCellType(5, SS_ALLROWS, &CellType);
 List->SetCellType(6, SS_ALLROWS, &CellType);

 HourGlassOff();
}



int TGLBatchEntry::CheckGLAccount()
{
 char *szAccount = new char[(int)12];
 strcpy(szAccount,GetField(ID_ACCOUNTS));
 strcat(szAccount,GetField(ID_DEPART));
 if(glaccts->seek(szAccount)==0)
 return TRUE;
 else
 return FALSE;
}

int TGLBatchEntry::CheckGLDept()
{
 char *szAccount = new char[(int)5];
 strcpy(szAccount,GetField(ID_DEPART));
 if(glaccts->seek(szAccount)==0)
 return TRUE;
 else
 return FALSE;
}


void TGlWindow::GLModifyBatch(RTMessage)
 {
  GetApplication()->ExecDialog(new TGLBatchEntry(this,1013));
 }


void TGLViewBatchList::SetupWindow()
{
 TWindow::SetupWindow();
 SetWndDetail();
}

void TGLViewBatchList::SetWndDetail()
{
 RECT rc;
 short d1Width,d2Width = 0;
 SetCaption(GetString(GetApplication()->hInstance,6071));
 List->SetFields("gl_btch",GetString(hInst,8140));
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, GetString(GetApplication()->hInstance,6077));
 List->SetData(2,SS_HEADER, " ");
 List->SetData(3,SS_HEADER, GetString(GetApplication()->hInstance,6079));
}

void TGLViewBatchList::WMSize(RTMessage Msg)
{
  TWindow::WMSize(Msg);
  int iRows = glbtch->recCount();

  SetWindowPos(List->HWindow, 0, -1, -1, LOWORD(Msg.LParam)+2,
  HIWORD(Msg.LParam)+2, SWP_NOZORDER);
  List->SetBool(SSB_REDRAW,FALSE);
  RECT rc;
  short d1Width,d2Width,iHeight = 0;
  List->GetClientRect(&rc);
  List->GetRowHeightInPixels(1,&iHeight);
  int cRows = ((rc.bottom - rc.top) / iHeight) + 1;

  if(iRows < (cRows))
	List->SetMaxRows(cRows);
  else
	List->SetMaxRows(iRows);

  List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
  List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
  short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
  List->SetColWidthInPixels(3,ColWidth);
  List->SetBool(SSB_REDRAW,TRUE);
}


void TAccountList::WMSize(RTMessage Msg)
{

	 // NEW
  TWindow::WMSize(Msg);
  int iRows = glaccts->recCount();

  SetWindowPos(List->HWindow, 0, -1, -1, LOWORD(Msg.LParam)+2,
  HIWORD(Msg.LParam)+2, SWP_NOZORDER);
  List->SetBool(SSB_REDRAW,FALSE);
  RECT rc;
  short d1Width,d2Width,iHeight = 0;
  List->GetClientRect(&rc);
  List->GetRowHeightInPixels(1,&iHeight);
  int cRows = ((rc.bottom - rc.top) / iHeight) + 1;

  if(iRows < (cRows))
	List->SetMaxRows(cRows);
  else
	List->SetMaxRows(iRows);

  List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
  List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
  short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
  List->SetColWidthInPixels(3,ColWidth);
  List->SetBool(SSB_REDRAW,TRUE);

}


void TAccountList::SetupWindow()
{
 TWindow::SetupWindow();
 SetWndDetail();
}

void TAccountList::SetWndDetail()
{
 RECT rc;
 short d1Width,d2Width = 0;
 SetCaption(GetString(GetApplication()->hInstance,6084));
 List->SetFields("glaccts","accounts",3);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, GetString(GetApplication()->hInstance,6068));
 List->SetData(2,SS_HEADER, GetString(GetApplication()->hInstance,6054));
 List->SetData(3,SS_HEADER, GetString(GetApplication()->hInstance,6044));
}

void TGLDistList::SelectCell (RTMessage Msg)
{
  long lBatchno;
  IntGetValue(GetDlgItem(Parent->HWindow,ID_BATCHNO),&lBatchno);
  int nRecord = 0;
  LPSS_CELLCOORD Cell = (LPSS_CELLCOORD) Msg.LParam;
  glbtchd->SetIndex(GetString(hInst,8140));

  if(glbtchd->seek(lBatchno) == 0){
	  while((long) glbtchd->Batchno() == lBatchno){
		 if( (int)glbtchd->iJournal() == (int)Cell->Row){
			  List->SetBool(SSB_REDRAW,FALSE);
			  List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
			  List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
			  List->SetBool(SSB_REDRAW,TRUE);
			  nRecord = (int)glbtchd->recNo();
		 }
		 if(nRecord > 0)
		  break;
		 glbtchd->skip();
	  }
  }

  if(nRecord == 0){
	SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	return;
  }
  glbtchd->go(nRecord);
  SetField(3035,glbtchd->date);
  SetField(3036,glbtchd->reference);
  SetField(3037,glbtchd->descript);
  SetField(6070,glbtchd->cheque);
  SetField(ID_ACCOUNTS,glbtchd->account);
  SetField(ID_DEPART,glbtchd->dept);
  SetField(ID_DEBIT,glbtchd->Debit());
  SetField(ID_CREDIT,glbtchd->Credit());
  SetField(ID_LINENO,(long)glbtchd->iJournal());
  SetFocus(GetDlgItem(Parent->HWindow,ID_PERIOD));
}



void TAccountList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1,iColWidth2;
 List->GetColWidth(1,&iColWidth1);
 List->GetColWidth(2,&iColWidth2);
 char *szBuffer1 = new char [iColWidth1+1];
 char *szBuffer2 = new char [iColWidth2+1];
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,szBuffer1);
 List->GetData(2,Cell->Row,szBuffer2);
 char *szSeek = new char [iColWidth1+iColWidth2+1];
 glaccts->SetIndex("accounts");
 wsprintf(szSeek,"%s%s",szBuffer1,szBuffer2);
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 if(glaccts->seek(szSeek) == 0)
  {
	SendMessage(GetDlgItem(Parent->HWindow,ID_JDESCRIPT),WM_SETTEXT,NULL,(LONG)(LPSTR)glaccts->descript->str());
	SendMessage(GetDlgItem(Parent->HWindow,ID_ACCOUNTS),WM_SETTEXT,NULL,(LONG)(LPSTR)glaccts->accounts->str());
	SendMessage(GetDlgItem(Parent->HWindow,ID_DEPART),WM_SETTEXT,NULL,(LONG)(LPSTR)glaccts->dept->str());
	SendMessage(Parent->HWindow,WM_COMMAND,ID_ACCTLIST,NULL);
        SetFocus(GetDlgItem(Parent->HWindow,ID_CHEQUE));
	List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else

 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 if(bHide)
	Show(SW_HIDE);
 List->iSeekLength = 0;	
 delete [] szBuffer1;
 delete [] szBuffer2;
 delete [] szSeek;
 return;
}

void TGLViewBatchList::HandleListBox( RTMessage Msg)
{
 char szBuffer[10];
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,szBuffer);
 glbtch->SetIndex(GetString(hInst,8140));

 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 if(glbtch->seek(szBuffer) == 0)
  {
	SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else

 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 Show(SW_HIDE);
 return;
}

void TGLBatchEntry::Next (RTMessage Msg)
{
 int iBatchno = 0;
 IntGetDlgItemValue(HWindow,ID_BATCHNO,(LPLONG)&iBatchno);
 if(glbtch->seek(iBatchno) == 0)
 {
   if(!glbtch->eof())
	{
	 glbtch->skip();
    if(glbtch->eof())
	 glbtch->bottom();
    CurrentBatch(Msg);
   }
 }
 else
 return;
}
void TGLBatchEntry::Previous (RTMessage Msg)
{
 int iBatchno = 0;
 IntGetDlgItemValue(HWindow,ID_BATCHNO,(LPLONG)&iBatchno);
 if(glbtch->seek(iBatchno) == 0)
 {
	if(!glbtch->bof())
	{
	 glbtch->skip(-1);
	 CurrentBatch(Msg);
	}
 }
 else
 return;

}
void TGLBatchEntry::Top (RTMessage Msg)
{
 glbtch->top();
 if(glbtch->recCount() == 0)
   return;

 while(glbtch->Closed() && !glbtch->eof())
  glbtch->skip();
 CurrentBatch(Msg);
}

void TGLBatchEntry::Bottom (RTMessage Msg)
{
 if(glbtch->recCount() == 0)
  return;
 glbtch->bottom();
 while(glbtch->Closed() && !glbtch->bof())
  glbtch->skip(-1);
 CurrentBatch(Msg);
}


void TGLBatchEntry::CurrentBatch(RTMessage)
{


  D->List->SetBool(SSB_REDRAW,FALSE);
  D->List->SetMaxRows(glbtchd->recCount());
  double lDebitTotal,lCreditTotal;
  lDebitTotal=lCreditTotal=0;

  if(glbtch->recCount() == 0)
	return;
  if(strcmp(glbtch->mode->str(),"0")==0)
	return;


  D->List->ClearData(SS_ALLCOLS,SS_ALLROWS);

  glbtchd->SetIndex(GetString(8140));
  if(glbtchd->seek(glbtch->Batchno()) != 0)
	{
	 MessageBox(HWindow,GetString(6087),szApp,MB_ICONINFORMATION);
	 return;
	}
  IntSetDlgItemValue(HWindow,ID_LINENO,glbtch->Entryno()+1);
  IntSetDlgItemValue(HWindow,ID_BATCHNO,(int)glbtch->Batchno());
  SetField(ID_DESC,glbtch->descript);
  Date4 Date;
  char szDate[11];
  Date.assign(glbtch->date->str(),"CCYYMMDD");
  Date.format(szDate,GetString(4024));
  SetField(ID_DATE,szDate);
  SetField(ID_ACCOUNTS,"");
  SetField(ID_JDESCRIPT,"");
  SetField(ID_DEPART,"");
  SetField(ID_CHEQUE,glbtch->cheque);
  SetField(ID_DEBIT,"");
  SetField(ID_CREDIT,"");
  Str4ten iJournal;
  Str4large dDebit,dCredit;

  long i=1;
  while ((int)glbtch->Batchno() == (int)glbtchd->Batchno())
	{
	 if(glbtchd->Debit() > 0 && glbtchd->Credit() ==  0)
	  {
		iJournal.assignLong(i,3);
		D->List->SetData(1,i,iJournal.str());
		D->List->SetData(2,i,glbtchd->descript->str());
		D->List->SetData(3,i,glbtchd->account->str());
		D->List->SetData(4,i,glbtchd->dept->str());
		D->List->SetData(5,i,glbtchd->debit->str());
		D->List->SetData(6,i,glbtchd->credit->str());
		glbtchd->journal->assignLong(i);
		i++;
	  }
	  lCreditTotal += glbtchd->Credit();
	  lDebitTotal += glbtchd->Debit();
	  glbtchd->skip(1);
	}
	dDebit.assignDouble(lDebitTotal,16,2);
	dCredit.assignDouble(lCreditTotal,16,2);

	glbtchd->top();
	if(glbtchd->seek(glbtch->Batchno()) == 0)
	{
	 while ((int)glbtch->Batchno() == (int)glbtchd->Batchno())
	 {
	 if(glbtchd->Credit() > 0 && glbtchd->Debit() == 0)
	  {
		iJournal.assignLong(i,3);
		D->List->SetData(1,i,iJournal.str());
		D->List->SetData(2,i,glbtchd->descript->str());
		D->List->SetData(3,i,glbtchd->account->str());
		D->List->SetData(4,i,glbtchd->dept->str());
		D->List->SetData(5,i,glbtchd->debit->str());
		D->List->SetData(6,i,glbtchd->credit->str());
		glbtchd->journal->assignLong(i);
		i++;
	  }
		glbtchd->skip(1);
	 }
	}
	glbtchd->flush();

	glbtchd->top();
	if(glbtchd->seek(glbtch->Batchno()) == 0)
	{
	 while ((int)glbtch->Batchno() == (int)glbtchd->Batchno())
	 {
	 if(glbtchd->Credit() > 0 && glbtchd->Debit() > 0)

	  {
		iJournal.assignLong(i,3);
		D->List->SetData(1,i,iJournal.str());
		D->List->SetData(2,i,glbtchd->descript->str());
		D->List->SetData(3,i,glbtchd->account->str());
		D->List->SetData(4,i,glbtchd->dept->str());
		D->List->SetData(5,i,glbtchd->debit->str());
		D->List->SetData(6,i,glbtchd->credit->str());
		glbtchd->journal->assignLong(i);
		i++;
	  }
		glbtchd->skip(1);
	 }
	}
	glbtchd->flush();

	Str4large szDebitTotal;
	Str4large szCreditTotal;
	szDebitTotal.assignDouble(lDebitTotal,16,2);
	szCreditTotal.assignDouble(lCreditTotal,16,2);
	IntSetDlgItemValue(HWindow,ID_LINENO,i);
	i++;
	
	short iHeight = 0;
	RECT Drc;
	GetClientRect(D->HWindow,&Drc);
	D->List->GetRowHeightInPixels(1,&iHeight);
	int cRows = (Drc.bottom - Drc.top) / iHeight + 1;
	if(i < cRows)
	 D->iRows = cRows;
	else
	 D->iRows = i;

	D->List->SetMaxRows(D->iRows);
	D->List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
	if(lDebitTotal != lCreditTotal)
	{
	 D->List->SetData(2,i,"Voucher Out Of Balance");
	 D->List->SetColor(2,i,RGBCOLOR_WHITE,RGBCOLOR_RED);
	}
	else
	{
	 D->List->SetData(2,i,"Total");
	}
	D->List->SetData(5,i,szDebitTotal.str());
	D->List->SetData(6,i,szCreditTotal.str());

	D->List->SetColor(5,i,RGB(192,220,192), RGBCOLOR_BLUE);
	D->List->SetColor(6,i,RGB(192,220,192), RGBCOLOR_BLUE);
	D->List->SetBool(SSB_REDRAW,TRUE);

	glbtch->debit->assignDouble(lDebitTotal);
	glbtch->credit->assignDouble(lCreditTotal);
	glbtch->UpdateUserTime(szUser);
	glbtch->flush();
	glbtch->unlock();


}


void TGLBatchEntry::ShowToolBox(RTMessage)
{

 if(!SendDlgItemMsg(IDLISTBOX,BM_GETCHECK,NULL,NULL))
 {
  ToolBox->Show(SW_SHOW);
  SendDlgItemMsg(IDLISTBOX,BM_SETCHECK,1,0L);
  RECT rc,button;
  GetWindowRect(HWindow,&rc);
  GetWindowRect(GetDlgItem(HWindow,ID_NEW),&button);
  int width = rc.right - (rc.left + (button.right - button.left + 10));
  MoveWindow(HWindow,rc.left,rc.top,width,rc.bottom-rc.top,TRUE);
  SetWindowText(GetDlgItem(HWindow,IDLISTBOX),"&Hide");
 }
 else
 {
  ToolBox->Show(SW_SHOW);
  SendDlgItemMsg(IDLISTBOX,BM_SETCHECK,0,0L);
  ToolBox->Show(SW_HIDE);
  MoveWindow(HWindow,left,top,right-left,bottom-top,TRUE);
  SetWindowText(GetDlgItem(HWindow,IDLISTBOX),GetString(6066));
 }
}


  /*
  char szMsg[85];
  wsprintf(szMsg,"Left %i Right %i Top %i Bottom %i",rc.left,rc.right,rc.top,rc.bottom);
  MessageBox(HWindow,szMsg,"Coord",MB_OK);
  */


void TGLBatchEntry::ViewAccts  (RTMessage)
 {
	AcctList->List->Seek(ID_ACCOUNTS);
	AcctList->Show(SW_SHOW);
 }

void TGLBatchEntry::ViewBatch (RTMessage)
 {
  if(bNewSlipList)
  {
	B->List->Redraw();
	bNewSlipList=FALSE;
  }

  if(IsWindowVisible(B->HWindow))
   B->Show(SW_HIDE);
  else
	B->Show(SW_SHOW);
 }

void TGLBatchEntry::Delete (RTMessage Msg)
{
  if(dGetField(ID_DEBIT) > 0 || dGetField(ID_CREDIT) > 0){
		int iEntryno;
		IntGetValue(GetDlgItem(HWindow,ID_LINENO),(LPLONG)&iEntryno);
		glbtchd->SetIndex(GetString(8140));
			if(glbtchd->seek(glbtch->Batchno()) == 0){
					if(MessageBox(HWindow,GetString(6098),szApp,MB_YESNO | MB_ICONQUESTION) == IDYES){
						while(glbtchd->Batchno() == glbtch->Batchno()){
							if(glbtchd->iJournal() == iEntryno)
							glbtchd->deleteRec();
						glbtchd->skip();
					  }
					  glbtchd->pack();
					  CurrentBatch(Msg);
					}
			}
  }
}



void TGLBatchEntry::Cancel (RTMessage)
 {

  if(bTransaction)
	{
	 switch(MessageBox(HWindow,GetString(ID_COMMIT),szApp,MB_YESNOCANCEL |MB_ICONQUESTION))
	  {
		case IDYES    : Clear(); break;
		case IDNO     : SendMessage(HWindow,WM_COMMAND,CM_ROLLBACK,NULL); break;
		default       : return;
	  }
	}

  AcctList->List->SavePosition();
  B->List->SavePosition();
  D->SavePosition();
  cb.closeAll();
  cb.init();
  ShutDownWindow();
}

void TGLBatchEntry::Clear()
{
 bTransaction = FALSE;
 glbtch->bottom();
 int db = 1;
 if(!glbtch->eof())
  db = (int) glbtch->Batchno();
 else
	db = GetPrivateProfileInt("PERIOD","BATCHNO",1,PROFILE);
 db++;


 IntSetDlgItemValue(HWindow,ID_BATCHNO,db);
 IntSetDlgItemValue(HWindow,ID_LINENO,1);

 SendDlgItemMsg(ID_REFERENCE,WM_SETTEXT,NULL,(LONG)(LPSTR)"");
 SendDlgItemMsg(ID_JDESCRIPT,WM_SETTEXT,NULL,(LONG)(LPSTR)"");
 SendDlgItemMsg(ID_DESC,WM_SETTEXT,NULL,(LONG)(LPSTR)"");
 SendDlgItemMsg(6070,WM_SETTEXT,NULL,(LONG)(LPSTR)"");
 SendDlgItemMsg(ID_ACCOUNTS,WM_SETTEXT,NULL,(LONG)(LPSTR)"");

 FloatSetDlgItemValue(HWindow,ID_CREDIT,0.00);
 FloatSetDlgItemValue(HWindow,ID_DEBIT,0.00);

 D->List->SetColor(SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
 D->List->ClearData(SS_ALLCOLS,SS_ALLROWS);

 long Indx = SendDlgItemMsg(ID_PRODUCTS,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_PRODUCTS,CB_SETCURSEL,(DWORD)Indx,NULL);

 Indx = SendDlgItemMsg(ID_MEDREPS,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_MEDREPS,CB_SETCURSEL,Indx,NULL);

 Indx = SendDlgItemMsg(ID_DRCODES,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_DRCODES,CB_SETCURSEL,Indx,NULL);

 SetFocus(GetDlgItem(HWindow,ID_DESC));
}

void TGLBatchEntry::New (RTMessage )
{
 if(bTransaction)
 {
  switch(MessageBox(HWindow,GetString(ID_COMMIT),szApp,MB_YESNOCANCEL |MB_ICONQUESTION))
	{
	 case IDYES    : Clear(); break;
	 case IDNO     : SendMessage(HWindow,WM_COMMAND,CM_ROLLBACK,NULL); break;
	 default       : return;
	}
 }
 else
  Clear();
 return;
}


void TGLBatchEntry::Add (RTMessage Msg)
{

 if(szEnterError(ID_DESC))
   return;

 if(szEnterError(ID_REFERENCE))
   return;


 if(!CheckGLAccount())
 {
  MessageBeep(MB_ICONEXCLAMATION);
  ViewAccts(Msg);
  return;
 }

 // Batch Detail
 if(*GetField(ID_DATE) == 0)
 {
  MessageBox(HWindow,GetString(8090),szApp,MB_OK |MB_ICONEXCLAMATION );
  return;
 }

 double lCredit = dGetField(ID_CREDIT);
 double lDebit = dGetField(ID_DEBIT);

 if(lDebit == 0 && lCredit ==0 )
   {
	MessageBox(HWindow,GetString(8092),szApp,MB_OK |MB_ICONEXCLAMATION );
	return;
	}

 if(lDebit > 0 && lCredit > 0 )
  {
	MessageBox(HWindow,GetString(8093),szApp,MB_OK);
	return;
  }

 long lineno;
 IntGetDlgItemValue(HWindow,ID_LINENO,(LPLONG)&lineno);

 glbtch->SetIndex("BATCHNO");

 if(glbtch->seek(lGetField(ID_BATCHNO))!=0)        // Brand new entry no summary record in gl_btch.
  AppendNewEntry();
 else if ( (int) lGetField(ID_LINENO) == glbtchd->iJournal() ) //Summary record exists and modifying current journal number.
 {
  DebitTotal = double(Str4ptr(glbtch->debit->str())) + lDebit - double(Str4ptr(glbtchd->debit->str()));
  CreditTotal = double(Str4ptr(glbtch->credit->str())) + lCredit - double(Str4ptr(glbtchd->credit->str()));
  FloatSetDlgItemValue(D->HWindow,ID_CREDITTOTAL,CreditTotal);
  FloatSetDlgItemValue(D->HWindow,ID_DEBITTOTAL,DebitTotal);
  lDifference = CreditTotal - DebitTotal;
  FloatSetDlgItemValue(D->HWindow,ID_DIFFERENCE,lDifference);
  ReplaceDetail();
  ReplaceHeader();
 }
 else if (  (int) lGetField(ID_LINENO) > glbtchd->iJournal() ) //Summary record exists and entry number is greater than current journal number.
 { 
  DebitTotal = double(Str4ptr(glbtch->debit->str())) + lDebit ;
  CreditTotal = double(Str4ptr(glbtch->credit->str())) + lCredit;
  FloatSetDlgItemValue(D->HWindow,ID_CREDITTOTAL,CreditTotal);
  FloatSetDlgItemValue(D->HWindow,ID_DEBITTOTAL,DebitTotal);
  lDifference = CreditTotal - DebitTotal;
  FloatSetDlgItemValue(D->HWindow,ID_DIFFERENCE,lDifference);
  AppendDetail(glbtch->Batchno());
  ReplaceHeader();
 }

 IntSetDlgItemValue(HWindow,ID_LINENO,(int(Str4ptr(glbtch->entryno->str()))+1));

 FloatSetDlgItemValue(HWindow,ID_DEBIT,0.00);
 FloatSetDlgItemValue(HWindow,ID_CREDIT,0.00);

 CurrentBatch(Msg);
 B->List->Redraw();
}

void TGLBatchEntry::AppendNewEntry(void)
{
  bTransaction = TRUE;
  DebitTotal = double(Str4ptr(glbtch->debit->str())) + dGetField(ID_DEBIT) - double(Str4ptr(glbtchd->debit->str()));
  CreditTotal = double(Str4ptr(glbtch->credit->str())) + dGetField(ID_CREDIT) - double(Str4ptr(glbtchd->credit->str()));
  FloatSetDlgItemValue(D->HWindow,ID_CREDITTOTAL,CreditTotal);
  FloatSetDlgItemValue(D->HWindow,ID_DEBITTOTAL,DebitTotal);
  lDifference = CreditTotal - DebitTotal;
  FloatSetDlgItemValue(D->HWindow,ID_DIFFERENCE,lDifference);

  glbtch->bottom();
  long iBatchno = glbtch->Batchno();
  ++iBatchno;

  AppendHeader(iBatchno);
  AppendDetail(iBatchno);

  if(bNewSlipList == FALSE)
  {
	iCurrentSlipTotal++;
	if(iCurrentSlipTotal > 8)
	 B->List->SetMaxRows(iCurrentSlipTotal);
	else
	 B->List->SetMaxRows(8);
	B->List->InsRow(iCurrentSlipTotal);
	B->List->SetData(1,iCurrentSlipTotal,glbtch->batchno->str());
	B->List->SetData(2,iCurrentSlipTotal,glbtch->descript->str());
  }

}

void TGLBatchEntry::ReplaceHeader(void)
{
  glbtch->mode->assign(szMode);
  glbtch->date->assign(GetDate(ID_DATE));
  glbtch->period->assignLong(GetPeriod());
  glbtch->descript->assign(GetField(ID_DESC));
  glbtch->debit->assignDouble(DebitTotal);
  glbtch->credit->assignDouble(CreditTotal);
  glbtch->entryno->assignLong(lGetField(ID_ENTRYNO));
  glbtch->cheque->assign(GetField(ID_CHEQUE));
  glbtch->UpdateUserTime(szUser);
  glbtch->flush();
}

void TGLBatchEntry::ReplaceDetail(void)
{
  glbtchd->journal->assignLong(lGetField(ID_LINENO));
  glbtchd->date->assign(GetDate(ID_DATE));
  glbtchd->period->assignLong(GetPeriod());
  glbtchd->reference->assign(GetField(ID_REFERENCE));
  glbtchd->descript->assign(GetField(ID_JDESCRIPT));
  glbtchd->account->assign(GetField(ID_ACCOUNTS));
  glbtchd->dept->assign(GetField(ID_DEPART));
  glbtchd->cheque->assign(GetField(ID_CHEQUE));
  glbtchd->medrep->assign(GetMedRep());
  glbtchd->product->assign(GetProduct());
  glbtchd->drcode->assign(GetDoctor());
  glbtchd->debit->assignDouble(dGetField(ID_DEBIT));
  glbtchd->credit->assignDouble(dGetField(ID_CREDIT));
  glbtchd->flush();

}

char  *TGLBatchEntry::GetMedRep()
{

	char szString[41];
	Str4ten *szCode = new Str4ten;
	szCode->set(' ');
	szCode->setLen(glmedrep->repcode->len());
	int Indx = (int)SendDlgItemMsg(ID_MEDREPS,CB_GETCURSEL,NULL,NULL);
	if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(ID_MEDREPS,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szString);
	 if(glmedrep->seek(szString)== 0)
		 strcpy(szCode->ptr(),glmedrep->repcode->str());

	}
  return szCode->str();
}

char  *TGLBatchEntry::GetProduct()
{
	char szString[41];
	Str4ten *szCode = new Str4ten;
	szCode->set(' ');
	szCode->setLen(glprod->fPcode->len());
	int Indx = (int)SendDlgItemMsg(ID_PRODUCTS,CB_GETCURSEL,NULL,NULL);
	 if(Indx != CB_ERR)
	 {
	  SendDlgItemMsg(ID_PRODUCTS,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szString);
	  if(glprod->seek(szString)== 0)
		 strcpy(szCode->ptr(),glprod->fPcode->str());
	}
  return szCode->str();
}

char  *TGLBatchEntry::GetDoctor()
{
	 char szString[41];
	 Str4ten *szCode = new Str4ten;
	 szCode->set(' ');
	 szCode->setLen(gldrcodes->drcode->len());
	 int Indx = (int)SendDlgItemMsg(ID_DRCODES,CB_GETCURSEL,NULL,NULL);
	 if(Indx != CB_ERR)
	  {
		SendDlgItemMsg(ID_DRCODES,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szString);
		gldrcodes->SetIndex("rpname");
		if(gldrcodes->seek(szString) == 0)
		  strcpy(szCode->ptr(),gldrcodes->drcode->str());
	  }
	 return szCode->str();
}




long TGLBatchEntry::GetPeriod()
{
 char szPeriod[3];
 DWORD Indx = SendDlgItemMsg(ID_PERIOD,CB_GETCURSEL,0,0);
 SendDlgItemMsg(ID_PERIOD,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szPeriod);
 return atol(szPeriod);
}



void TGLBatchEntry::AppendHeader(long iBatchno)
{
  glbtch->appendStart();
  glbtch->batchno->assignLong(iBatchno);
  glbtch->mode->assign(szMode);
  glbtch->date->assign(GetDate(ID_DATE));
  glbtch->period->assignLong(GetPeriod());
  glbtch->descript->assign(GetField(ID_DESC));
  glbtch->debit->assignDouble(DebitTotal);
  glbtch->credit->assignDouble(CreditTotal);
  glbtch->entryno->assignLong(lGetField(ID_ENTRYNO));
  glbtch->cheque->assign(GetField(ID_CHEQUE));
  glbtch->UpdateUserTime(szUser);
  glbtch->append();
}


void TGLBatchEntry::AppendDetail(long iBatchno)
{
  glbtchd->appendStart();
  glbtchd->batchno->assignLong(iBatchno);
  glbtchd->journal->assignLong(lGetField(ID_LINENO));
  glbtchd->date->assign(GetDate(ID_DATE));
  glbtchd->period->assignLong(GetPeriod());
  glbtchd->reference->assign(GetField(ID_REFERENCE));
  glbtchd->descript->assign(GetField(ID_JDESCRIPT));
  glbtchd->account->assign(GetField(ID_ACCOUNTS));
  glbtchd->dept->assign(GetField(ID_DEPART));
  glbtchd->medrep->assign(GetMedRep());
  glbtchd->product->assign(GetProduct());
  glbtchd->drcode->assign(GetDoctor());
  glbtchd->cheque->assign(GetField(ID_CHEQUE));
  glbtchd->debit->assignDouble(dGetField(ID_DEBIT));
  glbtchd->credit->assignDouble(dGetField(ID_CREDIT));
  glbtchd->append();
}



void TGLToolBox::WMActivate      (RTMessage )
{
 GetClientRect(HWindow,&rc);
 rc.bottom = rc.top + GetSystemMetrics(SM_CYCAPTION) /2;
 InvalidateRect(HWindow,&rc,FALSE);
}

void TGLToolBox::WMPaint         (RTMessage )
{
 hCaptionFont = CreateFont(-1*(GetSystemMetrics(SM_CYCAPTION)/2),
				0,0,0, 400, FALSE, FALSE, FALSE,
			   ANSI_CHARSET,OUT_CHARACTER_PRECIS,
			   CLIP_DEFAULT_PRECIS,PROOF_QUALITY,
                           VARIABLE_PITCH | FF_SWISS, (LPSTR) "Small Font");

 WORD wCaptionHeight = GetSystemMetrics(SM_CYCAPTION) /2;
 hDC = BeginPaint(HWindow,&ps);
 hOldFont =(HFONT) SelectObject(hDC, hCaptionFont);
 GetClientRect(HWindow,&rc);
 rc.bottom = wCaptionHeight;
 rc.left   = wCaptionHeight;
 GetWindowText(HWindow,szCaption,sizeof(szCaption));
 WORD wWidth = LOWORD (GetTextExtent(hDC,szCaption,strlen(szCaption)));
 SetBkMode(hDC,TRANSPARENT);
 if (GetFocus() == HWindow)
  {
   hCaptionBrush = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION));
   SetTextColor(hDC,GetSysColor(COLOR_CAPTIONTEXT));
  }
 else
  {
	hCaptionBrush = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION));
   SetTextColor(hDC,GetSysColor(COLOR_INACTIVECAPTIONTEXT));
  }
 FillRect(hDC,&rc,hCaptionBrush);
 DeleteObject(hCaptionBrush);
 ExtTextOut(hDC,rc.left + (rc.right-rc.left-wWidth)/2, -1, ETO_CLIPPED, &rc, szCaption,strlen(szCaption),NULL);
 SelectObject(hDC,hOldFont);
 rc.right = rc.left;
 rc.left = 0;
 SetBkColor (hDC,RGB(192,192,192));

 ExtTextOut(hDC,0,0,ETO_OPAQUE | ETO_CLIPPED, &rc, NULL, 0 ,NULL);
 GetClientRect(HWindow,&rc);
 hOldPen = (HPEN)SelectObject(hDC, GetStockObject(BLACK_PEN));
 MoveTo(hDC, ((wCaptionHeight+2)/4)-1, (wCaptionHeight/2)-1);
 LineTo(hDC, (3*((wCaptionHeight+2)/4))-1, (wCaptionHeight/2)-1);
 MoveTo(hDC, wCaptionHeight, 0);
 LineTo(hDC, wCaptionHeight,wCaptionHeight);
 MoveTo(hDC, 0, wCaptionHeight-1);
 LineTo(hDC, rc.right+1, wCaptionHeight-1);
 SelectObject(hDC, hOldPen);
 DeleteObject(hOldPen);
 EndPaint(HWindow,&ps);
 
}

void TGLToolBox::WMNchittest     (RTMessage Msg)
{
 
      GetClientRect (HWindow, &rc );

      // Figure the height of the fake caption
		rc.bottom = rc.top + GetSystemMetrics ( SM_CYCAPTION )/2;

      // remove out the area where the fake system menu is
		rc.left   = GetSystemMetrics ( SM_CYCAPTION )/2;

      pt.x = (int) LOWORD (Msg.LParam);
      pt.y = (int) HIWORD (Msg.LParam);
      ScreenToClient (HWindow, &pt );
		if (PtInRect (&rc, pt))
	Msg.Result  = HTCAPTION;
		else
	DefWndProc (Msg);

}


void TGLToolBox::SetupWindow()
{
 RECT rect;
 GetWindowRect(Parent->HWindow,&rect);

 int iWidth = 2*30;
 int iHeight = 5*25+GetSystemMetrics(4)/2;

 MoveWindow(HWindow,rect.right-110,rect.top+20,iWidth,iHeight,0);

 bSearchAccounts = new TSuperButton(this,ID_VIEW,"",10,10,10,10);
 bBottom         = new TSuperButton(this,ID_BOTTOM,"",10,10,10,10);
 bTop            = new TSuperButton(this,ID_TOP,"",10,10,10,10);
 bNext           = new TSuperButton(this,ID_NEXT,"",10,10,10,10);
 bPrevious       = new TSuperButton(this,ID_PREVIOUS,"",10,10,10,10);
 bNew            = new TSuperButton(this,ID_NEW,"",10,10,10,10);
 bSlips          = new TSuperButton(this,ID_SLIPLIST,"",10,10,10,10);
 bOk             = new TSuperButton(this,IDOK,"",10,10,10,10);
 bDelete         = new TSuperButton(this,ID_DELETE,"",10,10,10,10);
 bCancel         = new TSuperButton(this,IDCANCEL,"",10,10,10,10);

 GetApplication()->MakeWindow(bSearchAccounts);
 GetApplication()->MakeWindow(bBottom);
 GetApplication()->MakeWindow(bTop);
 GetApplication()->MakeWindow(bNext);
 GetApplication()->MakeWindow(bPrevious);
 GetApplication()->MakeWindow(bNew);
 GetApplication()->MakeWindow(bSlips);
 GetApplication()->MakeWindow(bOk);
 GetApplication()->MakeWindow(bDelete);
 GetApplication()->MakeWindow(bCancel);

 SetBitmap(ID_TOP			,1);
 SetBitmap(ID_BOTTOM		,2);
 SetBitmap(ID_NEXT		,3);
 SetBitmap(ID_PREVIOUS  ,4);
 SetBitmap(ID_NEW			,5);
 SetBitmap(ID_SLIPLIST  ,6);
 SetBitmap(IDOK		   ,7);
 SetBitmap(ID_DELETE	   ,8);
 SetBitmap(ID_VIEW	   ,9);
 SetBitmap(IDCANCEL	   ,10);
 SendMessage(GetDlgItem(HWindow,IDCANCEL),SBM_SETPICT_BTNDOWN,SUPERBTN_PICT_BITMAP,(long)(LPSTR)"QUIT_1");
 SendMessage(GetDlgItem(HWindow,IDOK),SBM_SETPICT_BTNDOWN,SUPERBTN_PICT_BITMAP,(long)IDOK);
}

void TGLToolBox::SetBitmap(int ID,int iPosition)
{
 RECT rect;
 GetClientRect(HWindow,&rect);
 int iWidth = 30;
 int iHeight = 25;
 int left = rect.left;
 int top  = rect.top+10;
 switch (iPosition)
	{
	 case 1: break;
	 case 2: left = left + iWidth;break;
	 case 3: top = top + iHeight;break;
	 case 4: left = left + iWidth;top = top + iHeight;break;
	 case 5: top = top + 2*iHeight;break;
	 case 6: left = left + iWidth;top = top + 2*iHeight;break;
	 case 7: top = top + 3*iHeight;break;
	 case 8: left = left + iWidth;top = top + 3*iHeight;break;
	 case 9: top = top + 4*iHeight;break;
	 case 10: left = left + iWidth;top = top + 4*iHeight;break;
	 case 11: top = top + 5*iHeight;break;
	 case 12: left = left + iWidth;top = top + 5*iHeight;break;
	}
 MoveWindow(GetDlgItem(HWindow,ID),left,top,iWidth,iHeight,0);
 SendMessage(GetDlgItem(HWindow,ID),SBM_SETSHADOWSIZE,1,NULL);
 SendMessage(GetDlgItem(HWindow,ID),SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)ID);
}
