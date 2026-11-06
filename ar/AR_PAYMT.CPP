
#include "ar.h"
#include "gl_dlg.h"
#include "db_dist.h"
#include "db_vend.h"
#include "db_reqst.h"
#include "db_apdst.h"
#include "ar_ctrl.h"
#include "math.h"

extern Code4 cb;
#define ID_INVOICE 1030


_CLASSDEF (TAPPayment)
class TAPPayment : public TMainDialog
{
 public:
  SS_CELLTYPE 	CellType;
  LOGFONT 	lfFont;
  HFONT   	hFont;
  HWND 		SHWindow;
  BOOL          bHasInvoices;
  PTglaccts     glaccts;
  PTapctrl      apctrl;
  PTapvendor    apvendor;
  PTapreqst     apreqst;
  PTapdist      apdist;
  PTgldist      gldist;
  PTListArray   ListArray;
  PTSpread      InvoiceList;
  double dAPControl;
  TAPPayment(PTWindowsObject AParent, LPSTR ATitle):TMainDialog(AParent,ATitle)
  {
  ListArray   = new TListArray(1000, 0, 50);
  InvoiceList = new TSpread(this,ID_DISTLIST);
  };
  virtual void SetupWindow();
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  virtual void SetDate        (RTMessage Msg) = [ID_FIRST + ID_DUEDATE];
  virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual BOOL LoadDistributionList();
  void         PrintPaymentList(void);
  void         CreateGLEntry();
  void         APAccountUpdate();
  ~TAPPayment(){delete ListArray;};
};

_CLASSDEF (TAPSetChequeNumber)
class TAPSetChequeNumber: public TMainDialog
{
 public:
 PTapreqst     apreqst;
 PTSpread      InvoiceList;
 char          szInvoiceno[11];
 TAPSetChequeNumber(PTWindowsObject AParent,LPSTR ATitle,PTapreqst reqst,PTSpread List)
 :TMainDialog(AParent,ATitle)
  {   
   apreqst = reqst;
   InvoiceList = List;
  }
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL]{ShutDownWindow();};
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
};


void TAPSetChequeNumber::SetupWindow()
{
 TDialog::SetupWindow();
 SetField(ID_INVOICE,apreqst->paymentreq); 
 SetField(ID_CHEQUE,apreqst->cheque);
 SetField(ID_DEBIT,apreqst->amountpaid);
 SetField(ID_ACCOUNTS,apreqst->bankacct);
 SetField(ID_DEPART,apreqst->bankdept);
}

void TAPSetChequeNumber::Ok (RTMessage)
{
 Date4 DatePaid;
 DatePaid.assign(GetField(ID_DATE),"MM/DD/YY");
 DatePaid.format("CCCCMMDD");
 SS_COORD Row,Col;
 
 InvoiceList->GetActiveCell(&Col,&Row); 
 InvoiceList->SetData(3,Row,GetField(ID_ACCOUNTS));
 InvoiceList->SetData(4,Row,GetField(ID_DEPART));
 InvoiceList->SetData(5,Row,GetField(ID_CHEQUE));
 apreqst->cheque->assign(GetField(ID_CHEQUE));
 apreqst->bankacct->assign(GetField(ID_ACCOUNTS));
 apreqst->bankdept->assign(GetField(ID_DEPART));
 apreqst->datepaid->assign(DatePaid.str());
 apreqst->cleared->assign("0");
 apreqst->flush();
 ShutDownWindow();
}


void TGlWindow::APPayment(RTMessage)
{
 GetApplication()->ExecDialog(new TAPPayment(this,"AR_PAYMENT"));
}

void TAPPayment::SelectCell (RTMessage Msg)
{
 char szRequest[11];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD) Msg.LParam;
 InvoiceList->GetData(2,Cell->Row,szRequest);
 StrTrim(szRequest);
 if(apreqst->seek(szRequest) == 0)
  {  
    GetApplication()->ExecDialog(new TAPSetChequeNumber(this,"AR_SETCHEQUE",apreqst,InvoiceList));
  }
 else
 MessageBox(HWindow,"Invoice Not Found!","GetString(ID_WARNING)",MB_ICONEXCLAMATION);
}

void TAPPayment::Print (RTMessage)
{
 if(MessageBox(HWindow,GetString(8004),"W A R N I N G",MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
  {
   PrintPaymentList();
  }
 return;
}



void TAPPayment::SetDate (RTMessage)
{
 bHasInvoices = LoadDistributionList();
}


void TAPPayment::SetupWindow()
{
 TMainDialog::SetupWindow();
 HourGlassOn();
 RECT rc;
 InvoiceList->GetClientRect(&rc);
 short d1Width,d0Width,d2Width,d3Width,d4Width,d5Width;
 apreqst  =  new Tapreqst(&cb);
 apvendor =  new Tapvendor(&cb);
 apdist   =  new Tapdist(&cb);
 apctrl   =  new Tapctrl(&cb);
 glaccts  =  new Tglaccts(&cb);
 gldist   =  new Tgldist(&cb);

 InvoiceList->SetMaxCols(6);
 InvoiceList->SetColWidth(1,apreqst->vendorno->len());
 InvoiceList->SetColWidth(2,apreqst->paymentreq->len());
 InvoiceList->SetColWidth(3,apreqst->bankacct->len());
 InvoiceList->SetColWidth(4,apreqst->bankdept->len());
 InvoiceList->SetColWidth(5,apreqst->cheque->len());

 InvoiceList->GetColWidthInPixels(0,(LPSHORT)&d0Width);
 InvoiceList->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 InvoiceList->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 InvoiceList->GetColWidthInPixels(3,(LPSHORT)&d3Width);
 InvoiceList->GetColWidthInPixels(4,(LPSHORT)&d4Width);
 InvoiceList->GetColWidthInPixels(5,(LPSHORT)&d5Width);

 
 short ColWidth = (rc.right - rc.left)
		  - d0Width
		  - d1Width
		  - d2Width
		  - d3Width
		  - d4Width
		  - d5Width;

 InvoiceList->SetColWidthInPixels(6,ColWidth);

 lfFont.lfHeight          = -15;
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


 InvoiceList->SetMaxRows(1000);
 InvoiceList->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);

 
 InvoiceList->SetData(1,SS_HEADER, "Vendor Code");
 InvoiceList->SetData(2,SS_HEADER, "Request");
 InvoiceList->SetData(3,SS_HEADER, "Bank Account");
 InvoiceList->SetData(4,SS_HEADER, "Dept");
 InvoiceList->SetData(5,SS_HEADER, "Invoice No.");
 InvoiceList->SetData(6,SS_HEADER, "Amount Due") ;

 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 InvoiceList->SetTypeEdit(&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 InvoiceList->SetCellType(1, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(2, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(3, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeFloat(&CellType,FS_SEPARATOR,9,2,0,999999999.99);
 InvoiceList->SetCellType(4, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(5, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(6, SS_ALLROWS, &CellType);

 // Open databases...
 cb.safety = 0;

 apdist->SetIndex("invoiceno");

 bHasInvoices  = LoadDistributionList();
 HourGlassOff();
}

void TAPPayment::Cancel  (RTMessage)
{
 cb.closeAll();
 ShutDownWindow();
}

BOOL TAPPayment::LoadDistributionList()
{

 int l = 1;
 char szPaymentDate[9];

 GetDlgItemText(HWindow,ID_DATE,szPaymentDate,sizeof(szPaymentDate));
 Date4 dPayment(szPaymentDate);
 apreqst->SetIndex("paymentreq");
 InvoiceList->SetBool(SSB_REDRAW,FALSE);
 InvoiceList->ClearData(SS_ALLCOLS,SS_ALLROWS);
 Relate4set *rQuery;
 rQuery = new Relate4set(apreqst);
 rQuery->querySet("vendorno <> '  '");

  for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
	   InvoiceList->SetData(1,l,apreqst->vendorno->str());  // Vendor Code
	   InvoiceList->SetData(2,l,apreqst->paymentreq->str());  // Payment Request
	   InvoiceList->SetData(3,l,apreqst->bankacct->str());
	   InvoiceList->SetData(4,l,apreqst->bankdept->str());	  
	   InvoiceList->SetData(5,l,apreqst->cheque->str());  // Cheque Number
	   double dAmountDue = apreqst->dAmountpaid();
	   char szBuf[16];
	   gcvt(dAmountDue,16,szBuf);
	   InvoiceList->SetData(6,l,szBuf); // Adjustment
	   l++;
  }

 InvoiceList->SetBool(SSB_REDRAW,TRUE);
 rQuery->unlock();
 rQuery->free();

 if(l>9)
  InvoiceList->SetMaxRows(l);
 else
  InvoiceList->SetMaxRows(9);

 if(l>1)
 return TRUE;
 else
 return FALSE;

}


void TAPPayment::PrintPaymentList(void)
{  

  return;
}

void TAPPayment::Ok (RTMessage)
{

 if(MessageBox(HWindow,"Close the current open AP entries.","W A R N I N G",MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
   HourGlassOn();
   CreateGLEntry();
   HourGlassOff();
  }
 bHasInvoices = LoadDistributionList();
}


void TAPPayment::CreateGLEntry()
{
   if(*GetField(ID_DESCRIPT) == 0)
   {
    MessageBox(HWindow,"Enter a AP Posting description.","Post Cash",MB_ICONEXCLAMATION | MB_OK);
    SetFocus(GetDlgItem(HWindow,ID_DESCRIPT));
    return;
   }
   apdist->SetIndex("paymentreq");
   apreqst->SetIndex("paymentreq");
   Relate4set *rQuery = new Relate4set(apreqst);   
   rQuery->querySet("cleared = '0'");
    for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
     {
      if(apdist->seek(apreqst->paymentreq->str()) == 0)
       {
	while(int (Str4ptr(apdist->paymentreq->str())) == int (Str4ptr(apreqst->paymentreq->str())))
	{ 
	 apdist->amountpaid->assignDouble(apdist->dAmount()+apdist->dAdjustment()-fabs(apdist->dRebateamt()));
	 apdist->account->assign(apreqst->bankacct->str());
	 apdist->chequeno->assign(apreqst->cheque->str());
	 apdist->dept->assign(apreqst->bankdept->str());
	 apdist->datepaid->assign(apreqst->datepaid->str());
	 apdist->paymentctl->assign("P");
	 apdist->flush();

	 gldist->appendStart();
	 gldist->account->assign      (apdist->apaccount->str());
	 gldist->dept->assign         (apdist->apdepart->str());
	 gldist->period->assign       (apdist->period->str());
	 gldist->mode->assign         ("1");
	 gldist->date->assign         (apdist->date->str());
         gldist->type->assign         ("AP");
	 gldist->batchno->assignDouble(double(Str4ptr(apdist->batchno->str())));
	 gldist->reference->assign    (apdist->reference->str());
	 gldist->descript->assign     (apdist->descript->str());
	 gldist->debit->assignDouble  (apdist->dAmountpaid());
	 gldist->credit->assignDouble (0.0);
	 gldist->append();

	 gldist->appendStart();
	 gldist->account->assign      (apdist->account->str());
	 gldist->dept->assign         (apdist->dept->str());
	 gldist->period->assign       (apdist->period->str());
	 gldist->date->assign         (apdist->date->str());
	 gldist->mode->assign         ("1");
         gldist->type->assign         ("AP");
	 gldist->batchno->assignDouble(double(Str4ptr(apdist->batchno->str())));
	 gldist->reference->assign    (apdist->reference->str());
	 gldist->descript->assign     (apdist->descript->str());
	 gldist->debit->assignDouble  (0);
	 gldist->credit->assignDouble (apdist->dAmountpaid());
	 gldist->append();
	 apdist->skip();         
	 apreqst->cleared->assign("1");
       	}
       }
     }
   rQuery->unlock();
   rQuery->free();


}

