
/*
	General Ledger  - (C) Copyright 1992 by Tegara Corporation
	Modify, add and delete accounts functions.
*/
#include "std_inc.h"

#include "ap_vendr.h"
#include "gl_acctl.h"
#include "ap_btchl.h"
#include "ap_cashl.h"


#define PROFILE "GL.INI"
#define IDS_VENDORLIST 6151

#pragma argused
double round(double fAmount,double iDec = 2)
{
  double base = pow(10,iDec);
  double divide = .5/base;
  return floor((fAmount+divide)*base)/base;
}

void TCustomerList::SetupWindow()
{
 if(apvendor.open(*cb) < 0)
  {
	cb->closeAll();
	ShutDownWindow();
	return;
  }
 TWindow::SetupWindow();
 List->SetMaxRows(apvendor.recCount());
 List->SetFields();
}

void TCustomerList::WMSize(RTMessage Msg)
{
  TWindow::WMSize(Msg);
  iRows = apvendor.recCount();
  SetWindowPos(List->HWindow, 0, -1, -1, LOWORD(Msg.LParam)+2,
  HIWORD(Msg.LParam)+2, SWP_NOZORDER);
  List->SetBool(SSB_REDRAW,FALSE);
  RECT rc;
  short d1Width,d2Width,iHeight = 0;
  List->GetClientRect(&rc);
  List->GetRowHeightInPixels(1,&iHeight);
  long cRows = ((rc.bottom - rc.top) / iHeight) + 1;

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
}


void TCustomerList::HandleListBox( RTMessage Msg)
{
 double iColWidth1;
 List->GetColWidth(1,&iColWidth1);
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,(LPSTR)strKey.c_str());
 apvendor.SetIndexVendor();
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 if(apvendor.seek((LPSTR)strKey.c_str()) == 0)
  {
	SendMessage(Parent->HWindow,WM_COMMAND,CM_SETVENDOR,NULL);
	List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);

 Show(SW_HIDE);
 return;
}




void TBatchList::SetupWindow()
{
 TWindow::SetupWindow();
 //SetWndDetail();
 List->SetFields();
}

void TBatchList::SetQuery()
{
}

void TBatchList::SetWndDetail()
{
 /*
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
 */
}
void TBatchList::HandleListBox( RTMessage Msg)
{
 BOOL bHide = TRUE;
 double iColWidth1;
 string strBatchno;
 Tapbtch apbtch(cb);
 apbtch.SetIndexBatchno();
 List->GetColWidth(1,&iColWidth1);
 LPSS_CELLCOORD Cell =(LPSS_CELLCOORD) Msg.LParam;
 List->GetData(1,Cell->Row,(LPSTR)strBatchno.c_str());
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 if(apbtch.seek((LPSTR)strBatchno.c_str()) == 0)
  {
   SendMessage(GetDlgItem(Parent->HWindow,ID_BATCHNO),WM_SETTEXT,NULL,(LPARAM)(LPSTR)strBatchno.c_str());
	SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 if(bHide)
	Show(SW_HIDE);
 apbtch.close();
 return;
}

void TBatchList::WMSize(RTMessage Msg)
{
  // NEW
  TWindow::WMSize(Msg);
  Tapbtch apbtch(cb);
  long iRows = apbtch.recCount();
  apbtch.close();

  SetWindowPos(List->HWindow, 0, -1, -1, LOWORD(Msg.LParam)+2,
  HIWORD(Msg.LParam)+2, SWP_NOZORDER);
  List->SetBool(SSB_REDRAW,FALSE);
  RECT rc;
  short d1Width,d2Width,iHeight = 0;
  List->GetClientRect(&rc);
  List->GetRowHeightInPixels(1,&iHeight);
  long cRows = ((rc.bottom - rc.top) / iHeight) + 1;

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
 List->SetFields();
 //SetWndDetail();
}

void TCashList::SetWndDetail()
{
 /*
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
 */
}

void TCashList::WMSize(RTMessage Msg)
{

	 // NEW
  TWindow::WMSize(Msg);
  long iRows = apcash->recCount();

  SetWindowPos(List->HWindow, 0, -1, -1, LOWORD(Msg.LParam)+2,
  HIWORD(Msg.LParam)+2, SWP_NOZORDER);
  List->SetBool(SSB_REDRAW,FALSE);
  RECT rc;
  short d1Width,d2Width,iHeight = 0;
  List->GetClientRect(&rc);
  List->GetRowHeightInPixels(1,&iHeight);
  long cRows = ((rc.bottom - rc.top) / iHeight) + 1;

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
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 if(apcash->seek(szCashno) == 0)
  {
   SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	List->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  }
 else
 List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 if(bHide)
   Show(SW_HIDE);
 delete [] szCashno;
 return;
}


