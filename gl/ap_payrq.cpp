#include "ap.h"
#include "gl_dlg.h"
#include "db_apbtc.h"
#include "db_vend.h"
#include "db_jsum.h"
#include "math.h"
#include "db_reqst.h"
#include "db_apdst.h"
#include "db_dist.h"
#include "gl_setat.hpp"
#include "utildlg.h"

extern Code4 cb;
extern char szApp[20];

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
  PTapbtch	     apbtch;
  PTapreqst     apreqst;
  PTapjsum      apjsum;
  PTSpread      InvoiceList;
  TAPCreatePayment(PTWindowsObject AParent, int ATitle):TMainDialog(AParent,ATitle)
  {
  InvoiceList = new TSpread(this,ID_DISTLIST);
  bHasInvoices = FALSE;
  };
  virtual void SetupWindow();
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  virtual void SetDate        (RTMessage Msg) = [ID_FIRST + ID_SETDATE];
  virtual void Ok	            (RTMessage Msg) = [ID_FIRST + IDOK]; // Create the request.
  virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual void ViewAll        (RTMessage Msg) = [ID_FIRST + ID_VIEW];
  virtual void GetBankAccount (RTMessage Msg) = [ID_FIRST + ID_DESCRIPT];
  virtual BOOL DistList(char *szQuery);
  virtual void SetCashAccounts();
  void PrintPaymentList(void);
  void SetChequeNo();
  void CreateGLEntry(void);
};

class TAPHoldForce: public TMainDialog
{
 public: 
 PTapdist apdist;
 BOOL Hold;
 BOOL Force;
 TAPHoldForce(PTWindowsObject AParent, int AName,PTapdist a): TMainDialog(AParent,AName)
 {
  apdist = a;
 };
 virtual void SetupWindow();
 virtual void Ok            (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel        (RTMessage Msg) = [ID_FIRST + IDCANCEL] {ShutDownWindow();};
};


void TAPCreatePayment::GetBankAccount(RTMessage Msg)
{
 GL_SETAT gl_setat;
 gl_setat.open(cb);
 gl_setat.SetIndex("name");
 DWORD Indx;
 char *szBankAccount = new char[gl_setat.name->len()+1];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  Indx = SendDlgItemMsg(ID_DESCRIPT,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
   {
    SendDlgItemMsg(ID_DESCRIPT,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM) ((LPSTR) szBankAccount));

    if(gl_setat.seek(szBankAccount) == 0)
     {
		SetField(ID_ACCOUNTS,gl_setat.account->str());
		SetField(ID_DEPART,gl_setat.dept->str());
	  }
     SetFocus(GetDlgItem(HWindow,2017));
   }
 }
 delete [] szBankAccount;
 gl_setat.close();
}

void TAPCreatePayment::SetCashAccounts()
{
 GL_SETAT gl_setat;
 gl_setat.open(cb);
 gl_setat.SetIndex("cash");
 for(gl_setat.top();!gl_setat.eof();gl_setat.skip())
  {    
	 SendDlgItemMsg(ID_DESCRIPT,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)gl_setat.name->str());
  }
 gl_setat.close();
}





void TAPCreatePayment::Ok (RTMessage)
{
 HourGlassOn();
 double iStartingReq = 0;
 FloatGetDlgItemValue(HWindow,ID_APCONTROL,(LPDOUBLE)&iStartingReq);

 if(iStartingReq  == 0)
 {
	MessageBox(HWindow,GetString(ID_REQUESTNO),szApp,MB_OK |MB_ICONINFORMATION);   // If no request number return.
  return;
 }
  Data4 glaccts(&cb,"glaccts");
  glaccts.SetIndex("accounts");
  char *szSeek = new char [12];
  strcpy(szSeek,GetField(ID_ACCOUNTS));
  strcat(szSeek,GetField(ID_DEPART));

  if(glaccts.seek(szSeek) != 0)
  {
	MessageBox(HWindow,GetString(4007),szApp,MB_ICONINFORMATION);
	glaccts.close();
	return;
  }
  else
  glaccts.close();

 apdist->SetIndex("vendorno");
 Date4 dPayment;
 Date4 dDatepaid;

 dPayment.assign(GetField(ID_DUEDATE),GetString(4024));
 dDatepaid.assign(GetField(ID_DATE),GetString(4024));
 dPayment.format(GetString(4023));
 dDatepaid.format(GetString(4023));
 if(bHasInvoices)
 {
  if(MessageBox(HWindow,GetString(4009),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
   //loop to add amount paid and to register requested flag flag.
    apdist->top();  
	 char szVendorNo[7];
    strcpy(szVendorNo,apdist->vendorno->str());
     while(!apdist->eof())
	  {
		Date4 dDueDate(apdist->duedate->str());
		if(apdist->dAmount() > 0)
		 {
		  if(dDueDate <= dPayment && (strcmp(apdist->paymentctl->str(),"O") == 0
										  || strcmp(apdist->paymentctl->str(),"F") == 0 ) )
		  {
		  apdist->paymentctl->assign("N");
		  if(strcmp(StrTrim(szVendorNo),StrTrim(apdist->vendorno->str())) != 0)
			iStartingReq++;
		  apdist->chequeno->assignDouble(iStartingReq);
		  apdist->account->assign(GetField(ID_ACCOUNTS));
		  apdist->dept->assign(GetField(ID_DEPART));
		  apdist->amountpaid->assignDouble(apdist->dAmount());
		  apdist->datepaid->assign(dDatepaid.str());
		  strcpy(szVendorNo,apdist->vendorno->str());
		  apdist->UpdateUserTime(szUser);
		  apdist->flush();
		  wsprintf(szMsg,GetString(4008),apdist->chequeno->str());
		  SetField(IDD_STATUSBOX,szMsg);
		 }
      }
		apdist->skip();
	 }
  }
 CreateGLEntry();
 //"PAYMENTCTL = 'H' .OR. PAYMENTCTL = 'F' .OR. PAYMENTCTL = 'O' .OR. PAYMENTCTL = 'P'"
 string szQuery(GetString(4006));
 bHasInvoices = DistList((char *) szQuery.c_str());
 }
 else
 MessageBox(HWindow,GetString(4074),szApp,MB_OK);
 HourGlassOff();
 return;
}



void TAPHoldForce::SetupWindow()
{
 TDialog::SetupWindow();
 if(strcmp(apdist->paymentctl->str(),"R") == 0 || strcmp(apdist->paymentctl->str(),"P") == 0)
  {
	MessageBox(HWindow,GetString(4010),szApp,MB_ICONINFORMATION);
	ShutDownWindow();
  }
 SetField(ID_VENDORNO,apdist->vendorno);
 SetField(ID_INVOICENO,apdist->invoiceno);
 SetBitmap(IDCANCEL);
 SetBitmap(IDOK);
}

void TAPHoldForce::Ok(RTMessage)
  {
   if(SendDlgItemMsg(ID_FORCE,BM_GETSTATE,NULL,NULL))
    {
	  apdist->paymentctl->assign("F");
	  apdist->UpdateUserTime(szUser);
	  apdist->flush();
    }
	else if (SendDlgItemMsg(ID_HOLD,BM_GETSTATE,NULL,NULL))
	 {
	  apdist->UpdateUserTime(szUser);
	  apdist->paymentctl->assign("H");
	  apdist->flush();
	 }
   else
	 {
	  apdist->UpdateUserTime(szUser);
	  apdist->paymentctl->assign("O");
	  apdist->flush();
	 }
	ShutDownWindow();
  };


void TAPCreatePayment::SelectCell(RTMessage Msg)
{
 char szVendorno[21];
 char szInvoiceno[11];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,3,Cell->Row,szVendorno);
 SSGetData(SHWindow,4,Cell->Row,szInvoiceno);
 strcat(szVendorno,szInvoiceno);
 apdist->SetIndex("invoiceno");
 if(apdist->seek(szVendorno)==0)
 {
  GetApplication()->ExecDialog(new TAPHoldForce(this,1066,apdist));
 }
 SSSetData(SHWindow,2,Cell->Row,apdist->paymentctl->str());
}

void TAPCreatePayment::CreateGLEntry()
{
   
   
	apdist->SetIndex("paymentreq");
   Relate4set *rQuery = new Relate4set(apdist);
   rQuery->querySet("PAYMENTCTL = 'N'");
	for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
    {
	 apdist->paymentctl->assign("P");
	 apdist->UpdateUserTime(szUser);
	 apdist->flush();
	 apjsum->appendStart();
	 apjsum->account->assign      (apdist->apaccount->str());
	 apjsum->dept->assign         (apdist->apdepart->str());
	 apjsum->period->assign       (apdist->period->str());
	 apjsum->mode->assign         ("1");
	 apjsum->date->assign         (apdist->date->str());
	 apjsum->type->assign         ("AP");
	 apjsum->batchno->assignDouble(double(Str4ptr(apdist->batchno->str())));
	 apjsum->reference->assign    (apdist->reference->str());
	 apjsum->descript->assign     (apdist->descript->str());
	 apjsum->debit->assignDouble  (apdist->dAmountpaid());
	 apjsum->credit->assignDouble (0.0);
	 apjsum->append();

	 apjsum->appendStart();
	 apjsum->account->assign      (apdist->account->str());
	 apjsum->dept->assign         (apdist->dept->str());
	 apjsum->period->assign       (apdist->period->str());
	 apjsum->date->assign         (apdist->date->str());
	 apjsum->mode->assign         ("1");
         apjsum->type->assign         ("AP");
	 apjsum->batchno->assignDouble(double(Str4ptr(apdist->batchno->str())));
	 apjsum->reference->assign    (apdist->reference->str());
	 apjsum->descript->assign     (apdist->descript->str());
	 apjsum->debit->assignDouble  (0);
	 apjsum->credit->assignDouble (apdist->dAmountpaid());
	 apjsum->append();
	 }
   rQuery->unlock();
   rQuery->free();	          
}



void TAPCreatePayment::Print(RTMessage)
{
  if(MessageBox(HWindow,"Do you want to print a list of current cheques?",szApp,MB_YESNO|MB_ICONQUESTION) == IDYES)
  PrintPaymentList();
}

void TManilaGL::APCreateGLSlip(RTMessage)
{
 GetApplication()->ExecDialog(new TAPCreatePayment(this,1045));
}


void TAPCreatePayment::SetDate (RTMessage)
{
 //"PAYMENTCTL = 'H' .OR. PAYMENTCTL = 'F' .OR. PAYMENTCTL = 'O'"
 string szQuery(GetString(4011));
 bHasInvoices = DistList((char *)szQuery.c_str());
 if(!bHasInvoices)
  MessageBox(HWindow,GetString(4028),szApp,MB_OK);
}

void TAPCreatePayment::ViewAll(RTMessage)
{
 DistList(".NOT. DELETED()");
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
 InvoiceList->SetColWidth(4,9);
 InvoiceList->SetColWidth(5,9);
 InvoiceList->SetColWidth(6,9);
 InvoiceList->SetColWidth(7,9);
 InvoiceList->SetColWidth(8,9);
 InvoiceList->SetColWidth(9,9);


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

 //lfFont.lfHeight          = -13;
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
 InvoiceList->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 InvoiceList->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
 InvoiceList->SetBool(SSB_PROTECT,TRUE);
 InvoiceList->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 InvoiceList->SetBool(SSB_NOBEEP,TRUE);
 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 InvoiceList->SetBool(SSB_VERTSCROLLBAR,TRUE);

 InvoiceList->SetMaxRows(1000);

 InvoiceList->SetData(1,SS_HEADER, GetString(ID_VENDORNAME));
 InvoiceList->SetData(2,SS_HEADER, GetString(ID_CONTROL));
 InvoiceList->SetData(3,SS_HEADER, GetString(ID_VENDORCODE));
 InvoiceList->SetData(4,SS_HEADER, GetString(ID_INVOICENO));
 InvoiceList->SetData(5,SS_HEADER, GetString(ID_DUEDATE));
 InvoiceList->SetData(6,SS_HEADER, GetString(ID_AMOUNT));
 InvoiceList->SetData(7,SS_HEADER, GetString(ID_VAT));
 InvoiceList->SetData(8,SS_HEADER, GetString(ID_REBATE));
 InvoiceList->SetData(9,SS_HEADER, GetString(ID_AMOUNTDUE));

 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 InvoiceList->SetTypeEdit(&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 InvoiceList->SetCellType(1, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeEdit(&CellType,ES_LEFT,10,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 InvoiceList->SetCellType(2, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeEdit(&CellType,ES_RIGHT,10,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 InvoiceList->SetCellType(3, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(4, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(5, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeFloat(&CellType,FS_SEPARATOR,9,2,0,(999999999999.99));
 InvoiceList->SetCellType(6, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(7, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(8, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(9, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(10, SS_ALLROWS, &CellType);


 // Open databases...
 if(!CheckFile("AP_BTCH",Parent->HWindow) ||
	 !CheckFile("AP_BTCHD",Parent->HWindow) ||
	 !CheckFile("AP_VEND",Parent->HWindow) ||
	 !CheckFile("GLACCTS",Parent->HWindow) ||
	 !CheckFile("GLDEPT",Parent->HWindow) ||
	 !CheckFile("AP_JSUM",Parent->HWindow) ||
	 !CheckFile("AP_DIST",Parent->HWindow) ||
	 !CheckFile("AP_REQST",Parent->HWindow))
	{
	 cb.init();
	 ShutDownWindow();
	 return;
	}

 apdist   =  new Tapdist(&cb);
 apvendor =  new Tapvendor(&cb);
 apreqst  =  new Tapreqst(&cb);
 apbtch   =  new Tapbtch(&cb);
 apreqst->SetIndex("paymentreq");
 apjsum   =  new Tapjsum(&cb);

 //bHasInvoices = DistList();
 bHasInvoices = FALSE;

 SetCashAccounts();
 SetChequeNo();

 SetBitmap(IDCANCEL);
 SetBitmap(IDOK);
 SetBitmap(ID_VIEW);
 SetBitmap(ID_PRINT);
 SetBitmap(ID_SETDATE);
 HourGlassOff();
}



void TAPCreatePayment::Cancel  (RTMessage)
{
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}
BOOL TAPCreatePayment::DistList(char *szQuery)
{
 HourGlassOn();
 int l = 1;
 char *szControl = new char [2];
 // "Checking for invoices that are due."
 SetField(IDD_STATUSBOX,GetString(4026));
 Date4 dPayment;
 dPayment.assign(GetField(ID_DUEDATE),GetString(4024));
 dPayment.format(GetString(4023));
 InvoiceList->SetBool(SSB_REDRAW,FALSE);
 InvoiceList->ClearData(SS_ALLCOLS,SS_ALLROWS);
 apdist->SetIndex("invoiceno");
 char szDate[11];
 Date4 dInvoiceDueDate;
 Relate4set *rQuery = new Relate4set(apdist);
 rQuery->querySet(szQuery);
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip())
  {
	  Date4 dDueDate(apdist->duedate->str());
	  if( dDueDate <= dPayment )
		 {
		 apvendor->seek(apdist->vendorno->str());
		 InvoiceList->SetData(1,l,apvendor->vdshort->str());  // Vendor Name
		 InvoiceList->SetData(2,l,apdist->paymentctl->str()); // Payment Control
		 InvoiceList->SetData(3,l,apdist->vendorno->str());  // Vendor Code
		 InvoiceList->SetData(4,l,apdist->invoiceno->str()); // Invoice Number
		 Date4 dInvoiceDueDate(apdist->duedate->str());
		 dInvoiceDueDate.format(szDate,GetString(4024));
		 InvoiceList->SetData(5,l,szDate);   // Due Date
		 InvoiceList->SetData(6,l,apdist->amount->str());   // Invoice Amount
		 InvoiceList->SetData(7,l,apdist->vatamt->str()); // Adjustment
		 InvoiceList->SetData(8,l,apdist->rebateamt->str()); // Adjustment
		 double dAmountDue = apdist->dAmount() - apdist->dAmountpaid();
		 Str4large szAmount;
		 szAmount.assignDouble(dAmountDue,16,2);
		 InvoiceList->SetData(9,l,szAmount.str()); // Adjustment
		 l++;
		 }
  }
 rQuery->unlock();
 rQuery->free();

 if(l>15)
 InvoiceList->SetMaxRows(l);
 else
 InvoiceList->SetMaxRows(16);
 InvoiceList->SetBool(SSB_REDRAW,TRUE);
 //SetChequeNo();
 SetField(IDD_STATUSBOX,"Completed search...");
 delete szControl;
 HourGlassOff();
 if(l>1)
 return TRUE;
 else
 return FALSE;
}


void TAPCreatePayment::SetChequeNo()
 {
  apdist->SetIndex("chequeno");
  apdist->bottom();
  double iNewRequestNumber =1;
  if(double (Str4ptr(apdist->chequeno->str())) > 0)
    {
    iNewRequestNumber = double (Str4ptr(apdist->chequeno->str())) + 1;
    }
  IntSetDlgItemValue(HWindow,ID_APCONTROL,iNewRequestNumber);
  IntSetDlgItemRange(HWindow,ID_APCONTROL,iNewRequestNumber,100000);
 }


void TAPCreatePayment::PrintPaymentList(void)
{
  PrintReport("ap_reqst.rpt",GetApplication()->MainWindow->HWindow);
  return;
}
