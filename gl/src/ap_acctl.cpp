
/*
	General Ledger  - (C) Copyright 1992 by Tegara Corporation
	Modify, add and delete accounts functions.
*/

#include <tbowl.h>
#include "d4all.hpp"
#include "gl.h"
#include "gl_manil.h"
#include "glclass.h"
#include "gl_main.h"
#include "gl_man.h"
#include "crpe.h"

#include "db_accts.h"
#include "gl_dlg.h"
#include "gl_acct.h"
#include "ap_vendr.h"
#include "gl_acctl.h"
#include "ap_btchl.h"
#include "ap_cashl.h"

extern Code4 cb;
#define PROFILE "GL.INI"

#pragma argused

void TCustomerList::SetupWindow()
{
 TWindow::SetupWindow();
 SetWndDetail();
}

void TCustomerList::WMSize(RTMessage Msg)
{
  TWindow::WMSize(Msg);
  int iRows = apvendor->recCount();

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

void TCustomerList::SetWndDetail()
{
 RECT rc;
 short d1Width,d2Width = 0;
 SetCaption("Accounts Receivable Customers List");
 
 List->SetFields("ap_vend","vendorno",3);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, "Vendor No.");
 List->SetData(2,SS_HEADER, "Name");
 List->SetData(3,SS_HEADER, "Description");
}


void TCustomerList::HandleListBox( RTMessage Msg)
{

 double iColWidth1;
 List->GetColWidth(1,&iColWidth1);
 string *szVendorCode = new string;
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,(char *)szVendorCode->c_str());
 apvendor->SetIndex("vendorno");
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));

 if(apvendor->seek(szVendorCode))
  {
	SendMessage(Parent->HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
	List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));

 Show(SW_HIDE);
 delete szVendorCode;
 return;
}




void TBatchList::SetupWindow()
{ 
 TWindow::SetupWindow();
 SetWndDetail();
}

void TBatchList::SetWndDetail()
{
 RECT rc;
 short d1Width,d2Width = 0;
 SetCaption("Accounts Payable Batch List");
 
 List->SetFields("ap_btch","batchno",3);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, "Batch No.");
 List->SetData(2,SS_HEADER, " ");
 List->SetData(3,SS_HEADER, "Description");
}
void TBatchList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1;
 List->GetColWidth(1,&iColWidth1); 
 char *szBatchno = new char [iColWidth1+1];
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,szBatchno);
 apbtch->SetIndex(GetString(GetApplication()->hInstance,8140));
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(apbtch->seek(szBatchno) == 0)
  {
   SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
   List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(bHide)
	Show(SW_HIDE);
 delete [] szBatchno;
 return;
}

void TBatchList::WMSize(RTMessage Msg)
{
  // NEW
  TWindow::WMSize(Msg);
  int iRows = apbtch->recCount();

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


void TCashList::SetupWindow()
{
 TWindow::SetupWindow();
 SetWndDetail();
}

void TCashList::SetWndDetail()
{
 RECT rc;
 short d1Width,d2Width = 0;
 SetCaption("Accounts Receivable Cash List");
 List->SetFields("ap_cash","batchno",3);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, "Cash No.");
 List->SetData(2,SS_HEADER, " ");
 List->SetData(3,SS_HEADER, "Description");
}

void TCashList::WMSize(RTMessage Msg)
{

	 // NEW
  TWindow::WMSize(Msg);
  int iRows = apcash->recCount();

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


void TCashList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1;
 List->GetColWidth(1,&iColWidth1); 
 char *szCashno = new char [iColWidth1+1];
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,szCashno);
 apcash->SetIndex(GetString(GetApplication()->hInstance,8140));
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(apcash->seek(szCashno) == 0)
  {
   SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
   List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(bHide)
   Show(SW_HIDE);
 delete [] szCashno;
 return;
}


