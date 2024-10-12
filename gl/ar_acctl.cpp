
/*
	Accounts Receivable - (C) Copyright 1994 by Tegara Corporation
	Modify, add and delete accounts functions.
*/
#include "tbowl.h"
#include "d4all.hpp"

#include "ar.h"
#include "gl_dlg.h"
#include "gl_acct.h"
#include "gl_acctl.h"
#include "ar_custl.h"
#include "ar_btchl.h"
#define PROFILE "GL.INI"
extern HINSTANCE hInst;


/*
VOID     FAR cdecl Debug             (LPSTR,...);
void DebugW(LPSTR,...);
void DebugW(LPSTR,...)
{
 #ifdef DEBUG
  Debug(
}
*/

extern Code4 cb;

void TARCustomerList::SetupWindow()
{ 
 TWindow::SetupWindow();
 SetWndDetail();
}

void TARCustomerList::SetWndDetail()
{
 RECT rc;
 short d1Width,d2Width = 0;
 SetCaption("Accounts Receivable Customers List");
 
 List->SetFields("ar_cust","vendorno",3);
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, "Cust No.");
 List->SetData(2,SS_HEADER, "Description");
 List->SetData(3,SS_HEADER, "Name");
 List->Redraw();
}

void TARCustomerList::WMSize (RTMessage Msg)
{
  // NEW
  TWindow::WMSize(Msg);
  int iRows = arcust->recCount();

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
void TARCustomerList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1;
 List->GetColWidth(1,&iColWidth1); 
 char *szVendorCode = new char [iColWidth1+1];
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,szVendorCode);
 arcust->SetIndex("vendorno");
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(arcust->seek(szVendorCode) == 0)
  {
   SendMessage(Parent->HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
   List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(bHide)
   Show(SW_HIDE);
 delete [] szVendorCode;
 return;
}


void TARBatchList::SetupWindow()
{ 
 TWindow::SetupWindow();
 SetWndDetail();
}

void TARBatchList::SetWndDetail()
{
 RECT rc;
 short d1Width,d2Width = 0;
 //"Accounts Receivable Batch List"
 SetCaption(GetString(hInst,8147));

 List->SetFields("ar_btch",GetString(hInst,8140));
 List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 List->GetClientRect(&rc);
 short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
 List->SetColWidthInPixels(3,ColWidth);
 List->SetData(1,SS_HEADER, GetString(hInst,8141));
 List->SetData(2,SS_HEADER, "  ");
 List->SetData(3,SS_HEADER, GetString(hInst,8142));
 List->Redraw();
}

void TARBatchList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1;
 List->GetColWidth(1,&iColWidth1);
 char *szBatchno = new char [iColWidth1+1];
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,szBatchno);
 ar_btch->SetIndex(GetString(hInst,8140));
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 if(ar_btch->seek(szBatchno) == 0)
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

