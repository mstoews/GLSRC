#include "ap.h"
#include "gl_dlg.h"
#include "db_arbtc.h"
#include "db_vend.h"
#include "db_dist.h"

#include "db_reqst.h"
#include "db_ardst.h"

extern Code4 cb;

_CLASSDEF (TAPCreatePayment)
class TAPCreatePayment : public TMainDialog
{
 public:
  BOOL          bHasInvoices;
  SS_CELLTYPE 	CellType;
  LOGFONT 	lfFont;
  HFONT   	hFont;
  HWND 		SHWindow;
  PTapdist      apdist;
  PTapvendor    apvendor;
  PTapbtch	apbtch;
 
  PTapreqst     apreqst;
  PTListArray   ListArray;
  PTSpread      InvoiceList;
  TAPCreatePayment(PTWindowsObject AParent, LPSTR ATitle):TMainDialog(AParent,ATitle)
  {
  ListArray   = new TListArray(1000, 0, 50);
  InvoiceList = new TSpread(this,ID_DISTLIST);
  bHasInvoices = FALSE;
  };
  virtual void SetupWindow();
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK]{};
  virtual void SetDate        (RTMessage Msg) = [ID_FIRST + ID_DUEDATE];
  virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual void ViewAll        (RTMessage Msg) = [ID_FIRST + ID_VIEWALL];
  virtual BOOL LoadDistributionList();
  void         PrintPaymentList(void);
  ~TAPCreatePayment(){delete ListArray;};
};

void TAPCreatePayment::Print(RTMessage)
{
 if(BWCCMessageBox(HWindow,"Print a summary of invoice cheque requests.","Cheque Requests",MB_YESNO | MB_ICONQUESTION) == IDYES)
  PrintPaymentList();
}

void TGlWindow::APCreateGLSlip(RTMessage)
{
 GetApplication()->ExecDialog(new TAPCreatePayment(this,"AR_CREATESLIP"));
}


void TAPCreatePayment::Ok (RTMessage)
{ 
 char szPaymentDate[9];
 GetDlgItemText(HWindow,ID_DATE,szPaymentDate,sizeof(szPaymentDate));
 unsigned int iStartingReq = 0;
 IntGetDlgItemValue(HWindow,ID_APCONTROL,(LPLONG)&iStartingReq);
 if(iStartingReq  == 0)
 {
  BWCCMessageBox(HWindow,"Enter a starting request number.","Create Request",MB_OK);
  return;
 }
 apdist->SetIndex("vendorno");

 Date4 dPayment;
 dPayment.assign(GetField(ID_DATE),"MM/DD/YY");
 dPayment.format("CCCCMMDD");

 if(bHasInvoices)
 {
  if(BWCCMessageBox(HWindow,GetString(8003),"W A R N I N G",MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
   apdist->top();
  
   char szVendorNo[7];
   while (!apdist->eof())
   {
   if(strcmp(apdist->paymentctl->str(),"O") == 0 )
     {
	Date4 dDueDate(apdist->duedate->str());
        strcpy(szVendorNo,apdist->vendorno->str());
	if(dDueDate <= dPayment)
	 {
	  apdist->paymentctl->assign("R");
	  apdist->paymentreq->assignLong((long)iStartingReq);
	  apdist->amountpaid->assignDouble(apdist->dAmount()+apdist->dAdjustment()-apdist->dRebateamt());
	  apdist->flush();
	 }

     }
     apdist->skip();
     if(strcmp(szVendorNo,apdist->vendorno->str()) != 0)
     iStartingReq++;
   }
  }
 }
 else
 BWCCMessageBox(HWindow,"There are no invoices listed.\n\nUse Set Date to view due invoices.","Create Request",MB_OK);
 bHasInvoices = LoadDistributionList();
 return;
}


void TAPCreatePayment::SetDate (RTMessage)
{
 bHasInvoices = LoadDistributionList();
 if(!bHasInvoices)
 BWCCMessageBox(HWindow,"No invoices are due with this period","Set Date",MB_OK);
}

void TAPCreatePayment::ViewAll(RTMessage)
{
 int l = 1;

 Date4 dPayment;
 dPayment.assign(GetField(ID_DATE),"MM/DD/YY");
 dPayment.format("CCCCMMDD");
 InvoiceList->SetBool(SSB_REDRAW,FALSE);
 InvoiceList->ClearData(SS_ALLCOLS,SS_ALLROWS);
 apdist->SetIndex("invoiceno");
 apdist->top();
 while (!apdist->eof())
  {
	 Date4 dDueDate(apdist->duedate->str());
	    apvendor->seek(apdist->vendorno->str());
	    InvoiceList->SetData(1,l,apvendor->vdname->str());  // Vendor Name
            InvoiceList->SetData(2,l,apdist->paymentctl->str());
	    InvoiceList->SetData(3,l,apdist->vendorno->str());  // Vendor Code
	    InvoiceList->SetData(4,l,apdist->invoiceno->str()); // Invoice Number
	    InvoiceList->SetData(5,l,apdist->duedate->str());   // Due Date
	    InvoiceList->SetData(6,l,apdist->amount->str());   // Invoice Amount
	    InvoiceList->SetData(7,l,apdist->adjustment->str()); // Adjustment
	    InvoiceList->SetData(8,l,apdist->rebateamt->str()); // Adjustment
	    double dAmountDue = apdist->dAmount()+apdist->dAdjustment()-apdist->dRebateamt();
	    char szBuf[16];
	    gcvt(dAmountDue,3,szBuf);
	    InvoiceList->SetData(9,l,szBuf); // Adjustment
	    l++;
   apdist->skip();
  }
 if(l>6)
 InvoiceList->SetMaxRows(l);
 else
 InvoiceList->SetMaxRows(7);
 InvoiceList->SetBool(SSB_REDRAW,TRUE);

}

void TAPCreatePayment::SetupWindow()
{
 TDialog::SetupWindow();
 HourGlassOn();
 RECT rc;
 InvoiceList->GetClientRect(&rc);
 short d0Width,d2Width,d3Width,d4Width,d5Width,d6Width,d7Width,d8Width,d9Width;
 InvoiceList->SetMaxCols(9);
 InvoiceList->SetColWidth(2,3);
 InvoiceList->SetColWidth(3,6);
 InvoiceList->SetColWidth(4,10);
 InvoiceList->SetColWidth(5,10);
 InvoiceList->SetColWidth(6,10);
 InvoiceList->SetColWidth(7,10);
 InvoiceList->SetColWidth(8,10);
 InvoiceList->SetColWidth(9,10);


 InvoiceList->GetColWidthInPixels(0,(LPSHORT)&d0Width);
 InvoiceList->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 InvoiceList->GetColWidthInPixels(3,(LPSHORT)&d3Width);
 InvoiceList->GetColWidthInPixels(4,(LPSHORT)&d4Width);
 InvoiceList->GetColWidthInPixels(5,(LPSHORT)&d5Width);
 InvoiceList->GetColWidthInPixels(6,(LPSHORT)&d6Width);
 InvoiceList->GetColWidthInPixels(7,(LPSHORT)&d7Width);
 InvoiceList->GetColWidthInPixels(8,(LPSHORT)&d8Width);
 InvoiceList->GetColWidthInPixels(9,(LPSHORT)&d9Width);
 
 short ColWidth = (rc.right - rc.left)
		  - d0Width
		  - d2Width
		  - d3Width
		  - d4Width
		  - d5Width
		  - d6Width
		  - d7Width
		  - d8Width
		  - d9Width;

 InvoiceList->SetColWidthInPixels(1,ColWidth);

 lfFont.lfHeight          = 8;
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

 InvoiceList->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
 InvoiceList->SetBool(SSB_PROTECT,TRUE);
 InvoiceList->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 InvoiceList->SetBool(SSB_NOBEEP,TRUE);
 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 InvoiceList->SetBool(SSB_VERTSCROLLBAR,TRUE);

 InvoiceList->SetMaxRows(1000);
 InvoiceList->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);

 InvoiceList->SetData(1,SS_HEADER, "Vendor Name");
 InvoiceList->SetData(2,SS_HEADER, "Ctl");
 InvoiceList->SetData(3,SS_HEADER, "Vendor Code");
 InvoiceList->SetData(4,SS_HEADER, "Invoice Number");
 InvoiceList->SetData(5,SS_HEADER, "Due Date");
 InvoiceList->SetData(6,SS_HEADER, "Invoice Amount");
 InvoiceList->SetData(7,SS_HEADER, "Adjustment");
 InvoiceList->SetData(8,SS_HEADER, "Rebate");
 InvoiceList->SetData(9,SS_HEADER, "Amount Due");
 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 InvoiceList->SetTypeEdit(&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 InvoiceList->SetCellType(1, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeEdit(&CellType,ES_LEFT,10,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 InvoiceList->SetCellType(2, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeEdit(&CellType,ES_RIGHT,10,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 InvoiceList->SetCellType(3, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(4, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(5, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeFloat(&CellType,FS_SEPARATOR,9,2,0,999999999.99);
 InvoiceList->SetCellType(6, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(7, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(8, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(9, SS_ALLROWS, &CellType);


 // Open databases...
 apdist   =  new Tapdist(&cb);
 apvendor =  new Tapvendor(&cb);
 apreqst  =  new Tapreqst(&cb);
 apbtch   =  new Tapbtch(&cb);


 bHasInvoices = LoadDistributionList();

 HourGlassOff();

}

void TAPCreatePayment::Cancel  (RTMessage)
{
 cb.closeAll();
 ShutDownWindow();
}

BOOL TAPCreatePayment::LoadDistributionList()
{
 int l = 1;

 Date4 dPayment;
 dPayment.assign(GetField(ID_DATE),"MM/DD/YY");
 dPayment.format("CCCCMMDD");
 InvoiceList->SetBool(SSB_REDRAW,FALSE);
 InvoiceList->ClearData(SS_ALLCOLS,SS_ALLROWS);
 apdist->SetIndex("invoiceno");
 apdist->top();
 while (!apdist->eof())
  {
  //if(strcmp(apbtch->mode->str(),"0") == 0)
  //
  if(strcmp(apdist->paymentctl->str(),"O") == 0) // Account is still open.
    {
	 Date4 dDueDate(apdist->duedate->str());
	 if( dDueDate <= dPayment )
	 {
	    apvendor->seek(apdist->vendorno->str());
	    InvoiceList->SetData(1,l,apvendor->vdname->str());  // Vendor Name
            InvoiceList->SetData(2,l,apdist->paymentctl->str()); // Payment Control
	    InvoiceList->SetData(3,l,apdist->vendorno->str());  // Vendor Code
	    InvoiceList->SetData(4,l,apdist->invoiceno->str()); // Invoice Number
	    InvoiceList->SetData(5,l,apdist->duedate->str());   // Due Date
	    InvoiceList->SetData(6,l,apdist->amount->str());   // Invoice Amount
	    InvoiceList->SetData(7,l,apdist->adjustment->str()); // Adjustment
	    InvoiceList->SetData(8,l,apdist->rebateamt->str()); // Adjustment
	    double dAmountDue = apdist->dAmount()+apdist->dAdjustment()-apdist->dRebateamt();
	    char szBuf[16];
	    gcvt(dAmountDue,3,szBuf);
	    InvoiceList->SetData(9,l,szBuf); // Adjustment
	    l++;
         }
    }
   apdist->skip();
  }
 if(l>6)
 InvoiceList->SetMaxRows(l);
 else
 InvoiceList->SetMaxRows(7);
 InvoiceList->SetBool(SSB_REDRAW,TRUE);

 if(l>1)
 return TRUE;
 else
 return FALSE;
}


void TAPCreatePayment::PrintPaymentList(void)
{
  char szRepName[85];
  char szFileName[11];
  strcpy(szRepName,"Accounts Payable Payment Requests");
  strcpy(szFileName,"ap_reqst.rpt");
  if(!PrintCR(GetApplication()->MainWindow->HWindow,szRepName,szFileName))
  BWCCMessageBox(HWindow,GetString(ID_PRINTERROR),WARNING,MB_ICONEXCLAMATION);
  return;
}