#include "tbowl.h"
#include "d4all.hpp"

#include "ar.h"
#include "gl_dlg.h"
#include "db_apbtc.h"
#include "db_vend.h"
#include "db_dist.h"
#include "math.h"
#include "db_reqst.h"
#include "db_ardst.h"
#include "ar_distl.h"
#include "assert.h"
#include "utildlg.h"
#include "gl_setat.hpp"
//#include "db_accts.h"

#define ID_BANK 2014
#define ID_DISCOUNT 2023 
#define ID_PARTIAL  2025

extern char szApp[20];
extern Code4 cb;

Str4ten szSales,szSalesDept,szVAT,szVATDept,szAR,szARDept;


_CLASSDEF (TARCreatePayment)
class TARCreatePayment : public TMainDialog
{
 public:
  PTARDistList D;
  BOOL          bHasInvoices;
  SS_CELLTYPE 	CellType;
  LOGFONT 	lfFont;
  HFONT   	hFont;
  HWND 		SHWindow;
  PTgldist      gldist;
  PTapdist      ardist;
  PTapvendor    apvendor;
  PTapbtch	apbtch; 
  PTapreqst     apreqst;
  PTSpread      InvoiceList;
  TARCreatePayment(PTWindowsObject AParent, LPSTR ATitle):TMainDialog(AParent,ATitle)
  {
  InvoiceList = new TSpread(this,ID_DISTLIST);
  bHasInvoices = FALSE;
  };
  virtual void SetupWindow();
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  virtual void SetDate        (RTMessage Msg) = [ID_FIRST + ID_DUEDATE];
  virtual void Ok	            (RTMessage Msg) = [ID_FIRST + IDOK]; // Create Journal entry.
  virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual void ViewAll        (RTMessage Msg) = [ID_FIRST + ID_VIEWALL];
 
  virtual BOOL DistList(char *szQuery);
  void 	 SetRequestNumber(char *Vendorno,int iStartingReq);
  void    PrintPaymentList(void);
  void    SetDistPaid();
  void    SetDistDiscount();
  char   *GetDescription(char *szAccount,char *szDept);
};

class TARClear: public TMainDialog
{
 public: 
 PTapdist ardist;
 BOOL Hold;
 BOOL Force;
 BOOL Partial;
 BOOL Discount;
 TARClear(PTWindowsObject AParent, LPSTR AName,PTapdist a): TMainDialog(AParent,AName)
 {
  ardist = a;
 };
 virtual void SetupWindow();
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL] {ShutDownWindow();};
 virtual void GetBankAccount (RTMessage Msg) = [ID_FIRST + ID_DESCRIPT];
 virtual void SetPartial     (RTMessage Msg) = [ID_FIRST + ID_PARTIAL];
 virtual void SetDiscount    (RTMessage Msg) = [ID_FIRST + ID_DISCOUNT];
 void    SetCashAccounts();
 BOOL CheckAccount(char *szAccount,char *szDepart);
 virtual void SetData(Data4 *dbf,int ID,char *szField);
};

void TARClear::SetPartial  (RTMessage)
{
  if(SendDlgItemMsg(ID_PARTIAL,BM_GETCHECK,NULL,NULL))
    SendDlgItemMsg(ID_PARTIAL,BM_SETCHECK,FALSE,NULL);
  else
    SendDlgItemMsg(ID_PARTIAL,BM_SETCHECK,TRUE,NULL);
    SendDlgItemMsg(ID_DISCOUNT,BM_SETCHECK,FALSE,NULL);
}


void TARClear::SetDiscount (RTMessage)
{
  if(SendDlgItemMsg(ID_DISCOUNT,BM_GETCHECK,NULL,NULL))
    SendDlgItemMsg(ID_DISCOUNT,BM_SETCHECK,FALSE,NULL);
  else
    SendDlgItemMsg(ID_DISCOUNT,BM_SETCHECK,TRUE,NULL);
    SendDlgItemMsg(ID_PARTIAL,BM_SETCHECK,FALSE,NULL);
}

void TARClear::SetData(Data4 *dbf,int ID,char *szField)
{
  Field4 *field = new Field4(*(Data4 *)dbf,szField);
  SetField(ID,field);
} 

void TARClear::GetBankAccount(RTMessage Msg)
{
 GL_SETAT gl_setat;
 gl_setat.open(cb);
 gl_setat.SetIndex("name");
 DWORD Indx;
 char *szBankAccount = new char[gl_setat.name.len()+1];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  Indx = (DWORD) SendDlgItemMsg(ID_DESCRIPT,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
	{
    SendDlgItemMsg(ID_DESCRIPT,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM) ((LPSTR) szBankAccount));
    if(gl_setat.seek(szBankAccount) == 0)
     {
      SetField(ID_ACCOUNTS,gl_setat.account.str());
      SetField(ID_DEPART,gl_setat.dept.str());
     }
     SetFocus(GetDlgItem(HWindow,2017));
   }
 }
 delete [] szBankAccount;
 gl_setat.close();
}

void TARClear::SetCashAccounts()
{
 GL_SETAT gl_setat;
 gl_setat.open(cb);
 gl_setat.SetIndex("cash");
 for(gl_setat.top();!gl_setat.eof();gl_setat.skip())
  {    
    SendDlgItemMsg(ID_DESCRIPT,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)gl_setat.name.str());
  }
 gl_setat.close();
}



BOOL TARClear::CheckAccount(char *szAccount, char *szDepart)
{
 BOOL bFound = FALSE;
 Data4 *glaccts = new Data4(&cb,"glaccts");
 glaccts->SetIndex("account");
 Field4 accounts(*(Data4 *)glaccts,"account");
 Field4 dept(*(Data4 *)glaccts,"dept");
 glaccts->SetIndex("accounts");
 char *szSeek = new char [accounts.len() + dept.len() + 1];
 wsprintf(szSeek,"%s%s",szAccount,szDepart);
 if(glaccts->seek(szSeek) == 0)
   bFound = TRUE;
 else
	MessageBox(HWindow,GetString(1089),szApp,MB_ICONEXCLAMATION);
 glaccts->close();
 delete [] szSeek;
 return bFound;
}


void TARClear::SetupWindow()
{
 TMainDialog::SetupWindow();
 SetCashAccounts();

 Data4 *ar_cust = new Data4(&cb,"ar_cust");
 ar_cust->SetIndex("vendorno");

 SetField(ID_VENDORNO,ardist->vendorno);
 SetField(ID_INVOICENO,ardist->invoiceno);
 Str4large szDue;
 szDue.assignDouble(ardist->dAmount()-ardist->dAmountpaid(),16,2);
 SetField(ID_DEBIT,szDue.str());
 SetField(ID_ACCOUNTS,ardist->account);
 SetField(ID_DEPART,ardist->dept);
 SetField(ID_DATE,ardist->datepaid);
 if(ar_cust->seek(ardist->vendorno->str()) == 0)
  {
   SetData(ar_cust,2016,"ar_acct");
   SetData(ar_cust,2017,"ar_dept");
   SetData(ar_cust,2018,"vat_acct");
   SetData(ar_cust,2019,"vat_dept");
   SetData(ar_cust,2020,"account");
   SetData(ar_cust,2021,"depart");
  }
}

void TARClear::Ok(RTMessage)
  {
	 // || strcmp(ardist->paymentctl->str(),"P") == 0

	 BOOL bClear = TRUE;
	 BOOL bDiscount = FALSE;
	 BOOL bPartial  = FALSE;

	 if(SendDlgItemMsg(ID_DISCOUNT,BM_GETCHECK,NULL,NULL))
		bDiscount = TRUE;
	 else
	 if(SendDlgItemMsg(ID_PARTIAL,BM_GETCHECK,NULL,NULL))
		bPartial = TRUE;
	 else
		bClear = TRUE;

	 if(strcmp(ardist->paymentctl->str(),"C") == 0 || strcmp(ardist->paymentctl->str(),"D") == 0 )
	 {
	  MessageBox(HWindow,GetString(8127),szApp,MB_ICONINFORMATION);
	  return;
	 }

	 Date4 dPayment;
	 dPayment.assign(GetField(ID_DATE),"MM/DD/YY");
	 dPayment.format("CCCCMMDD");

	 double dAmountPaid = dGetField(ID_DEBIT);
	 double dAmount     = ardist->dAmount();

	 if(!CheckAccount(GetField(ID_ACCOUNTS),""))
	   return;

	 if(strlen(StrTrim(GetField(ID_DATE))) == 0 )
	   return;

	 if(dAmountPaid > dAmount)
	 {
	    MessageBox(HWindow,GetString(1091),szApp,MB_ICONINFORMATION);
	    return;
	  }

         char szPaymentctl[2];


	 if(bClear)
	  strcpy(szPaymentctl,"P");

	 if(bPartial)
	  strcpy(szPaymentctl,"R");

	 if(bPartial && dAmountPaid == dAmount)
          {
	   MessageBox(HWindow,GetString(1098),szApp,MB_ICONINFORMATION);
	   return;
          }

	  if(bDiscount && dAmountPaid == dAmount)
          {
	   MessageBox(HWindow,GetString(1097),szApp,MB_ICONINFORMATION);
	   return;
          }



	 if(bDiscount)
	  strcpy(szPaymentctl,"D");

	 if(dAmountPaid < dAmount && !bDiscount && !bPartial)
         {
	   switch(MessageBox(HWindow,GetString(1090),szApp,MB_YESNOCANCEL | MB_ICONQUESTION))
	    {
			case IDYES :strcpy(szPaymentctl,"D"); break;
			case IDNO  :strcpy(szPaymentctl,"R"); break;
			case IDCANCEL :return;
	    }
	  }

	  ardist->paymentctl->assign(szPaymentctl);
	  ardist->account->assign(StrTrim(GetField(ID_ACCOUNTS)));
	  ardist->dept->assign(StrTrim(GetField(ID_DEPART)));
	  ardist->datepaid->assign(dPayment.str());
          ardist->mode->assign("0");	  
	  ardist->amountpaid->assignDouble(dAmountPaid);
	  double dRemainder = ardist->dRemainder()-dAmountPaid;
	  ardist->remainder->assignDouble(dRemainder);
	  szSales.assign(GetField(2016));
	  szSalesDept.assign(GetField(2017));
	  szVAT.assign(GetField(2018));
	  szVATDept.assign(GetField(2019));
	  szAR.assign(GetField(2020));
	  szARDept.assign(GetField(2021));
	  ardist->flush();
	  ardist->unlock();
	  ShutDownWindow();

};


void TARCreatePayment::SelectCell(RTMessage Msg)
{
 char szVendorno[21];
 char szInvoiceno[11];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,3,Cell->Row,szVendorno);
 SSGetData(SHWindow,4,Cell->Row,szInvoiceno);
 strcat(szVendorno,szInvoiceno);
 ardist->SetIndex("invoiceno");
 if(ardist->seek(szVendorno)==0)
 {
  GetApplication()->ExecDialog(new TARClear(this,"AR_CLEAR",ardist));
 }
 SSSetData(SHWindow,2,Cell->Row,ardist->paymentctl->str());
 Str4large szAmountPaid;
 szAmountPaid.assignDouble(ardist->dAmountpaid(),16,2);
 SSSetData(SHWindow,9,Cell->Row,szAmountPaid.ptr());
 if(ardist->Paid() || ardist->Remainder())
   SetDistPaid();
 else
 if(ardist->Discount())
  SetDistDiscount();
}



void TARCreatePayment::Print(RTMessage)
{
  if(DlgMsg(4076,ID_CHEQUE,MB_YESNO|MB_ICONQUESTION) == IDYES)
  PrintPaymentList();
}

void TGlWindow::APCreateGLSlip(RTMessage)
{
 GetApplication()->ExecDialog(new TARCreatePayment(this,"AR_CREATESLIP"));
}

void TARCreatePayment::SetDistPaid()
{

  HWND hWnd = GetDlgItem(D->HWindow,ID_DISTLIST);
  SSSetBool(hWnd,SSB_REDRAW,FALSE);
  SSClearData(hWnd,SS_ALLCOLS,SS_ALLROWS);
  char szDescript[51];  
  strcpy(szDescript,GetDescription(ardist->account->str(),""));
  SSSetData(hWnd,1,1,"1");
  SSSetData(hWnd,2,1,szDescript);
  SSSetData(hWnd,3,1,ardist->account->str());
  SSSetData(hWnd,4,1,ardist->dept->str());
  SSSetData(hWnd,5,1,ardist->amountpaid->str());

  strcpy(szDescript,GetDescription(ardist->apaccount->str(),""));
  SSSetData(hWnd,1,2,"2");
  SSSetData(hWnd,2,2,szDescript);
  SSSetData(hWnd,3,2,ardist->apaccount->str());
  SSSetData(hWnd,4,2,ardist->apdepart->str());
  SSSetData(hWnd,6,2,ardist->amountpaid->str());
  SSSetBool(hWnd,SSB_REDRAW,TRUE);
}

char *TARCreatePayment::GetDescription(char *szAccount, char *szDept)
{
 Data4 GLACCTS(cb,"glaccts");
 char szDescript[41];
 GLACCTS.SetIndex("Accounts");
 Field4 descript(GLACCTS,"descript");
 Field4 account(GLACCTS,1);
 Field4 depart(GLACCTS,2);
 Str4large szSeek;
 szSeek.setLen (account.len()+ depart.len());
 szSeek.set(' ');
 szSeek.replace(Str4ptr(szAccount));
 szSeek.replace(Str4ptr(""),account.len());
 if(GLACCTS.seek(szSeek.str()) == 0)
  {
	strcpy(szDescript,descript.str());
  }
 GLACCTS.close();
 return (LPSTR) szDescript;
}

void TARCreatePayment::SetDistDiscount()
{
 
 double dOriginalAR    = ardist->dAmount();
 double dVatPercent    = ardist->dVatamt()/(dOriginalAR-ardist->dVatamt());
 double dOriginalSales = dOriginalAR/(1+dVatPercent);
 double dOriginalVat   = dOriginalAR - dOriginalSales;
 double dAmountPaid    = ardist->dAmountpaid();             //CASH
 double dNewSales      = dAmountPaid/(1+dVatPercent); 
 double dNewVat        = dAmountPaid - dNewSales;
 double dSalesDiscount = dOriginalSales - dNewSales;
 double dVatDiscount   = dOriginalVat - dNewVat;
 

 HWND hWnd = GetDlgItem(D->HWindow,ID_DISTLIST);
 SSSetBool(hWnd,SSB_REDRAW,FALSE);
 SSClearData(hWnd,SS_ALLCOLS,SS_ALLROWS);



 // Cash Account
 Str4large szCash;
 szCash.assignDouble(dAmountPaid,16,2);
 char szDescript[51];
 strcpy(szDescript,GetDescription(ardist->account->str(),ardist->dept->str()));
 SSSetData(hWnd,1,1,"1");
 SSSetData(hWnd,2,1,szDescript);
 SSSetData(hWnd,3,1,ardist->account->str());
 SSSetData(hWnd,4,1,ardist->dept->str());
 SSSetData(hWnd,5,1,szCash.str());

 
// Sales Account
 Str4large szSalesDiscount;
 szSalesDiscount.assignDouble(dSalesDiscount,16,2);
 strcpy(szDescript,GetDescription(szSales.str(),szSalesDept.str()));
 SSSetData(hWnd,1,2,"1");
 SSSetData(hWnd,2,2,szDescript);
 SSSetData(hWnd,3,2,szSales.str());
 SSSetData(hWnd,4,2,szSalesDept.str());
 SSSetData(hWnd,5,2,szSalesDiscount.str());

 //VAT Account
 
 Str4large szVatDiscount;
 strcpy(szDescript,GetDescription(szVAT.str(),szVATDept.str()));
 szVatDiscount.assignDouble(dVatDiscount,16,2);
 SSSetData(hWnd,1,3,"3");
 SSSetData(hWnd,2,3,szDescript);
 SSSetData(hWnd,3,3,szVAT.str());
 SSSetData(hWnd,4,3,szVATDept.str());
 SSSetData(hWnd,5,3,szVatDiscount.str());
 
 // AR Account
 Str4large szOriginalAR;
 szOriginalAR.assignDouble(dOriginalAR,16,2);
 strcpy(szDescript,GetDescription(szAR.str(),szARDept.str()));
 SSSetData(hWnd,1,4,"4");
 SSSetData(hWnd,2,4,szDescript);
 SSSetData(hWnd,3,4,ardist->apaccount->str());
 SSSetData(hWnd,4,4,ardist->apdepart->str());
 SSSetData(hWnd,6,4,szOriginalAR.ptr());
 SSSetBool(hWnd,SSB_REDRAW,TRUE);
}


void TARCreatePayment::Ok (RTMessage)
{

   ardist->SetIndex("batchno");
   Relate4set *rQuery = new Relate4set(ardist);
   rQuery->querySet("PAYMENTCTL = 'P'");
    for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
     {
	 // If the total amount of the invoice has been paid
	 // make the invoice a cleared invoice.

	 ardist->paymentctl->assign("C");
	 ardist->flush();

	 gldist->appendStart();
	 gldist->account->assign      (ardist->account->str());
	 gldist->dept->assign         (ardist->dept->str());
	 gldist->period->assign       (ardist->period->str());
	 gldist->mode->assign         ("1");
	 gldist->date->assign         (ardist->date->str());
         gldist->type->assign         ("AR");
	 gldist->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 gldist->reference->assign    (ardist->reference->str());
	 gldist->descript->assign     (StrTrim(ardist->descript->str()));
	 gldist->debit->assignDouble  (ardist->dAmountpaid());
	 gldist->credit->assignDouble (0.0);
	 gldist->append();

	 gldist->appendStart();
	 gldist->account->assign      (ardist->apaccount->str());
	 gldist->dept->assign         (ardist->apdepart->str());
	 gldist->period->assign       (ardist->period->str());
	 gldist->date->assign         (ardist->date->str());
	 gldist->mode->assign         ("1");
         gldist->type->assign         ("AR");
	 gldist->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 gldist->reference->assign    (ardist->reference->str());
	 gldist->descript->assign     (StrTrim(ardist->descript->str()));
	 gldist->debit->assignDouble  (0);
	 gldist->credit->assignDouble (ardist->dAmountpaid());
	 gldist->append();
     }
   rQuery->unlock();
   rQuery->free();

   Relate4set *pQuery = new Relate4set(ardist);
   pQuery->querySet("PAYMENTCTL = 'R' .AND. MODE = '0'");
    for(qc=pQuery->top();qc!=r4eof;qc=pQuery->skip(1L))
     {
	 // If the total amount of the invoice has been paid
	 // make the invoice a cleared invoice.
	 
         ardist->mode->assign("1");
	 if(ardist->dRemainder() == 0)
	   ardist->paymentctl->assign("C");
	 ardist->flush();


	 gldist->appendStart();
	 gldist->account->assign      (ardist->account->str());
	 gldist->dept->assign         (ardist->dept->str());
	 gldist->period->assign       (ardist->period->str());
	 gldist->mode->assign         ("1");
	 gldist->date->assign         (ardist->date->str());
         gldist->type->assign         ("AR");
	 gldist->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 gldist->reference->assign    (ardist->reference->str());
	 gldist->descript->assign     (ardist->descript->str());
	 gldist->debit->assignDouble  (ardist->dAmountpaid());
	 gldist->credit->assignDouble (0.0);
	 gldist->append();

	 gldist->appendStart();
	 gldist->account->assign      (ardist->apaccount->str());
	 gldist->dept->assign         (ardist->apdepart->str());
	 gldist->period->assign       (ardist->period->str());
	 gldist->date->assign         (ardist->date->str());
	 gldist->mode->assign         ("1");
         gldist->type->assign         ("AR");
	 gldist->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 gldist->reference->assign    (ardist->reference->str());
	 gldist->descript->assign     (ardist->descript->str());
	 gldist->debit->assignDouble  (0);
	 gldist->credit->assignDouble (ardist->dAmountpaid());
	 gldist->append();
     }
   pQuery->unlock();
   pQuery->free();



   Relate4set *rDiscount= new Relate4set(ardist);
   rDiscount->querySet("PAYMENTCTL = 'D'");
   for(qc=rDiscount->top();qc!=r4eof;qc=rDiscount->skip(1L))
     {
	 Data4 ar_cust(cb,"ar_cust");
	 ar_cust.SetIndex("vendorno");

       if(ar_cust.seek(ardist->vendorno->str()) == 0)
        {
	 Field4 Sales(ar_cust,17);
	 Field4 SalesD(ar_cust,16);
	 Field4 VAT1(ar_cust,20);
	 Field4 VAT1Dept(ar_cust,21);
	 ardist->remainder->assignDouble(0);
	 ardist->paymentctl->assign("C");
	 ardist->flush();

	 double dOriginalAR    = ardist->dAmount();  //AR
	 double dVatPercent    = ardist->dVatamt()/(ardist->dAmount()-ardist->dVatamt());
	 double dAmountPaid    = ardist->dAmountpaid();             //CASH
	 double dSalesDiscount = (dOriginalAR - dAmountPaid)/(1+dVatPercent); // SALES DISCOUNT
	 double dVatDiscount   = dSalesDiscount * dVatPercent; 	// VAT ADJUSTMENT



         // CASH
	 gldist->appendStart();
	 gldist->account->assign      (ardist->account->str());
	 gldist->dept->assign         (ardist->dept->str());
	 gldist->period->assign       (ardist->period->str());
	 gldist->mode->assign         ("1");
	 gldist->date->assign         (ardist->date->str());
         gldist->type->assign         ("AR");
	 gldist->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 gldist->reference->assign    (ardist->reference->str());
	 gldist->descript->assign     (ardist->descript->str());
	 gldist->debit->assignDouble  (dAmountPaid);
	 gldist->credit->assignDouble (0.0);
	 gldist->append();


	 // SALES DISCOUNT
	 char szAccount[8];
	// strcpy(szAccount,GetAccount(SALESACCT));



	 gldist->appendStart();
	 gldist->account->assign      (szSales.str());
	 gldist->dept->assign         (szSalesDept.str());
	 gldist->period->assign       (ardist->period->str());
	 gldist->date->assign         (ardist->date->str());
	 gldist->mode->assign         ("1");
         gldist->type->assign         ("AR");
	 gldist->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 gldist->reference->assign    (ardist->reference->str());
	 gldist->descript->assign     (GetDescription(Sales.str(),SalesD.str()));
	 gldist->debit->assignDouble  (dSalesDiscount);
	 gldist->credit->assignDouble (0);
	 gldist->append();

	 // VAT Change
	// strcpy(szAccount,GetAccount(VATACCT));
	 gldist->appendStart();
	 gldist->account->assign      (VAT1.str());
	 gldist->dept->assign         (VAT1Dept.str());
	 gldist->period->assign       (ardist->period->str());
	 gldist->date->assign         (ardist->date->str());
	 gldist->mode->assign         ("1");
         gldist->type->assign         ("AR");
	 gldist->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 gldist->reference->assign    (ardist->reference->str());
	 gldist->descript->assign     (GetDescription(VAT1.str(),VAT1Dept.str()));
	 gldist->debit->assignDouble  (dVatDiscount);
	 gldist->credit->assignDouble (0);
	 gldist->append();

	 // AR Amount
	 gldist->appendStart();
	 gldist->account->assign      (ardist->apaccount->str());
	 gldist->dept->assign         (ardist->apdepart->str());
	 gldist->period->assign       (ardist->period->str());
	 gldist->date->assign         (ardist->date->str());
	 gldist->mode->assign         ("1");
         gldist->type->assign         ("AR");
	 gldist->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 gldist->reference->assign    (ardist->reference->str());
	 gldist->descript->assign     (ardist->descript->str());
	 gldist->debit->assignDouble  (0);
	 gldist->credit->assignDouble (dOriginalAR);
	 gldist->append();

		}
     }
   rDiscount->unlock();
   rDiscount->free();


 DistList("PAYMENTCTL <> ' '");
 return;
}

void TARCreatePayment::SetRequestNumber(char *szVendorno,int iRequestNumber)
 {

 }

void TARCreatePayment::SetDate (RTMessage)
{
 bHasInvoices = DistList("PAYMENTCTL = 'H' .OR. PAYMENTCTL = 'F' .OR. PAYMENTCTL = 'O'");
 if(!bHasInvoices)
  DlgMsg(ID_NOINVOICES,ID_SETDATE,MB_OK);
}

void TARCreatePayment::ViewAll(RTMessage)
{
 DistList("PAYMENTCTL <> ' '");
}

void TARCreatePayment::SetupWindow()
{
 TDialog::SetupWindow();
 D = new TARDistList(this,"AR_DISTLIST");
 D->EnableAutoCreate();
 GetApplication()->MakeWindow(D);


 HourGlassOn();
 RECT rc;
 InvoiceList->GetClientRect(&rc);
 short d0Width,d2Width,d3Width,d4Width,d5Width,d6Width,d7Width,d8Width,d9Width,d10Width;
 InvoiceList->SetMaxCols(10);
 InvoiceList->SetColWidth(2,5);
 InvoiceList->SetColWidth(3,6);
 InvoiceList->SetColWidth(4,9);
 InvoiceList->SetColWidth(5,9);
 InvoiceList->SetColWidth(6,9);
 InvoiceList->SetColWidth(7,9);
 InvoiceList->SetColWidth(8,9);
 InvoiceList->SetColWidth(9,9);
 InvoiceList->SetColWidth(10,9);
 
 InvoiceList->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 InvoiceList->GetColWidthInPixels(3,(LPSHORT)&d3Width);
 InvoiceList->GetColWidthInPixels(4,(LPSHORT)&d4Width);
 InvoiceList->GetColWidthInPixels(5,(LPSHORT)&d5Width);
 InvoiceList->GetColWidthInPixels(6,(LPSHORT)&d6Width);
 InvoiceList->GetColWidthInPixels(7,(LPSHORT)&d7Width);
 InvoiceList->GetColWidthInPixels(8,(LPSHORT)&d8Width);
 InvoiceList->GetColWidthInPixels(9,(LPSHORT)&d9Width);
 InvoiceList->GetColWidthInPixels(10,(LPSHORT)&d10Width);


 short ColWidth = (rc.right - rc.left)
		  - d2Width
		  - d3Width
		  - d4Width
		  - d5Width
		  - d6Width
		  - d7Width
		  - d8Width
		  - d9Width
		  - d10Width;

 InvoiceList->SetColWidthInPixels(1,ColWidth);

 lfFont.lfHeight          = -13;
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
 InvoiceList->SetData(1,SS_HEADER, GetString(ID_VENDORNAME));
 InvoiceList->SetData(2,SS_HEADER, GetString(ID_CONTROL));
 InvoiceList->SetData(3,SS_HEADER, GetString(ID_VENDORCODE));
 InvoiceList->SetData(4,SS_HEADER, GetString(ID_INVOICENO));
 InvoiceList->SetData(5,SS_HEADER, GetString(ID_DUEDATE));
 InvoiceList->SetData(6,SS_HEADER, GetString(ID_AMOUNT));
 InvoiceList->SetData(7,SS_HEADER, GetString(ID_VAT));
 InvoiceList->SetData(8,SS_HEADER, "Inv. Total");
 InvoiceList->SetData(9,SS_HEADER, "Amt Paid");
 InvoiceList->SetData(10,SS_HEADER, "Remainder");


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
 InvoiceList->SetCellType(10, SS_ALLROWS, &CellType);


 // Open databases...
 ardist   =  new Tapdist(&cb);
 apvendor =  new Tapvendor(&cb); 
 apbtch   =  new Tapbtch(&cb);
 gldist   =  new Tgldist(&cb);

 bHasInvoices = FALSE;
 
 HourGlassOff();
 DistList("PAYMENTCTL <> ' '");
 D->Show(SW_SHOW);
 SetBitmap(ID_VIEWALL,"LOOK");
 SetBitmap(IDCANCEL,"CANCEL");
 SetBitmap(IDOK,"ADD");
}

void TARCreatePayment::Cancel  (RTMessage)
{
 cb.closeAll();
 ShutDownWindow();
}
BOOL TARCreatePayment::DistList(char *szQuery)
{
 HourGlassOn();
 int l = 1;
 char *szControl = new char [2];
 SetField(IDD_STATUSBOX,"Checking for invoices that are due.");
 Date4 dPayment;
 dPayment.assign(GetField(ID_DATE),"MM/DD/YY");
 dPayment.format("CCCCMMDD");
 InvoiceList->SetBool(SSB_REDRAW,FALSE);
 InvoiceList->ClearData(SS_ALLCOLS,SS_ALLROWS);
 ardist->SetIndex("invoiceno");
 Relate4set *rQuery = new Relate4set(ardist);
 rQuery->querySet(szQuery);
    for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
    {
       Date4 dDueDate(ardist->duedate->str());
	// if( dDueDate <= dPayment )
	// {
	    apvendor->seek(ardist->vendorno->str());
	    InvoiceList->SetData(1,l,apvendor->vdshort->str());  // Vendor Name
	    InvoiceList->SetData(2,l,ardist->paymentctl->str()); // Payment Control
	    InvoiceList->SetData(3,l,ardist->vendorno->str());  // Vendor Code
	    InvoiceList->SetData(4,l,ardist->invoiceno->str()); // Invoice Number
	    InvoiceList->SetData(5,l,ardist->duedate->str());   // Due Date
	    Str4large szSales;
	    double dSalesAmount = ardist->dAmount()-ardist->dVatamt();
            szSales.assignDouble(dSalesAmount,16,2);
	    InvoiceList->SetData(6,l,szSales.str());   // Invoice Amount
	    InvoiceList->SetData(7,l,ardist->vatamt->str()); // VAT
	    Str4large szRemainder;
	    szRemainder.assignDouble(ardist->dRemainder(),16,2);

	    Str4large szAmount;
	    szAmount.assignDouble(ardist->dAmount(),16,2);
	        
	    InvoiceList->SetData(8,l,szAmount.str());
	    Str4large szAmountPaid;
	    szAmountPaid.assignDouble(ardist->dAmountpaid(),16,2);
	    InvoiceList->SetData(9,l,szAmountPaid.ptr());
	    InvoiceList->SetData(10,l,szRemainder.ptr());
	    l++;
       //  }
    } 
    rQuery->unlock();
    rQuery->free();

 if(l>15)
 InvoiceList->SetMaxRows(l--);
 else
 InvoiceList->SetMaxRows(16);
 InvoiceList->SetBool(SSB_REDRAW,TRUE);

 SetField(IDD_STATUSBOX,"Completed search...");
 delete szControl;
 HourGlassOff();
 if(l>1)
 return TRUE;
 else
 return FALSE;
}


void TARCreatePayment::PrintPaymentList(void)
{
  //PrintCR(GetApplication()->MainWindow->HWindow,"ap_reqst.rpt"))
  return;
}


void TARDistList::SetupWindow()
{
 TDialog::SetupWindow();
 RECT rc;
 HourGlassOn();
 SetListSize(8);

 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);


 SSSetMaxCols(SHWindow, 6);
 SSGetClientRect(SHWindow,&rc);
 short d0Width,d2Width,d3Width,d4Width,d5Width;

 SSSetColWidth(SHWindow,1,3);
 SSSetColWidth(SHWindow,3,8);
 SSSetColWidth(SHWindow,4,5);
 SSSetColWidth(SHWindow,5,10);
 SSSetColWidth(SHWindow,6,10);

 SSGetColWidthInPixels(SHWindow,1,(LPSHORT)&d0Width);
 SSGetColWidthInPixels(SHWindow,3,(LPSHORT)&d2Width);
 SSGetColWidthInPixels(SHWindow,4,(LPSHORT)&d3Width);
 SSGetColWidthInPixels(SHWindow,5,(LPSHORT)&d4Width);
 SSGetColWidthInPixels(SHWindow,6,(LPSHORT)&d5Width);

 short ColWidth = (rc.right - rc.left)
 		  - d0Width
		  - d2Width
		  - d3Width
		  - d4Width
		  - d5Width;


 SSSetColWidthInPixels(SHWindow,2,ColWidth);
 SSSetData(SHWindow,1,SS_HEADER, "Ln");
 SSSetData(SHWindow,2,SS_HEADER, "Description");
 SSSetData(SHWindow,3,SS_HEADER, "Acct");
 SSSetData(SHWindow,4,SS_HEADER, "Dept");
 SSSetData(SHWindow,5,SS_HEADER, "Debit");
 SSSetData(SHWindow,6,SS_HEADER, "Credit");

 SSSetTypeEdit(SHWindow,&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 SSSetTypeEdit(SHWindow,&CellType,ES_RIGHT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);

 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,9,2,0,999999999.99);
 SSSetCellType(SHWindow,5, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,6, SS_ALLROWS, &CellType);

 HourGlassOff();
}

void TARDistList::SetListSize (int Rows)
{
 RECT rc,wd;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 short iHeight;
 LOGFONT lfFont;
 HFONT   hFont;
 lfFont.lfHeight          = -13;
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
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE); 

 SSSetBool(SHWindow,SSB_AUTOSIZE,FALSE);
 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,TRUE);
 SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetBool(SHWindow,SSB_ALLOWUSERRESIZE,FALSE); 
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetAutoSizeVisible(SHWindow,6,Rows);

 SSGetRowHeightInPixels(SHWindow,1,&iHeight);
 int Height =  GetSystemMetrics(SM_CYCAPTION)+ 2*GetSystemMetrics(SM_CXDLGFRAME) + (Rows*iHeight) + 1;
 GetWindowRect(HWindow,&wd);
 MoveWindow(HWindow,wd.left,wd.top,wd.right-wd.left,Height,TRUE);
 GetClientRect(HWindow,&rc); 
 GetWindowRect(GetDlgItem(HWindow,IDD_STATUSBOX),&wd);
 MoveWindow(SHWindow,rc.left+1, rc.top, rc.right - (wd.right-wd.left) , rc.bottom - rc.top, TRUE);
 GetWindowRect(SHWindow,&rc);
 MoveWindow(GetDlgItem(HWindow,IDD_STATUSBOX),rc.right,rc.top,rc.right-rc.left -(rc.right - (wd.right-wd.left)),rc.bottom-rc.top,TRUE);
 SSSetMaxRows(SHWindow,Rows+1);
}

