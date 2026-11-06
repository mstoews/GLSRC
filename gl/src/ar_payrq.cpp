#include "tbowl.h"
#include "d4all.hpp"
#include "glclass.h"
#include "ar.h"
#include "gl_dlg.h"
#include "db_arbtc.h"
#include "db_cust.h"
#include "db_arjsm.h"
#include "math.h"
#include "db_reqst.h"
#include "db_ardst.h"
#include "ar_distl.h"
#include "utildlg.h"
#include "gl_setat.hpp"
#include "ar_clear.h"
//#include "db_accts.h"


extern char szApp[20];
extern Code4 cb;
BOOL bReset;

Str4ten szSales,szSalesDept,szVAT,szVATDept,szAR,szARDept;

_CLASSDEF (TARCreatePayment)
class TARCreatePayment : public TMainDialog
{
 public:
  TARDistribution *D;
  BOOL          bHasInvoices;
  SS_CELLTYPE 	CellType;
  LOGFONT 	lfFont;
  HFONT   	hFont;
  HWND 		SHWindow;
  PTarjsum      arjsum;
  PTardist      ardist;
  PTarcust      arcust;
  PTarbtch	    arbtch;
  PTSpread      InvoiceList;
  TARCreatePayment(PTWindowsObject AParent, int ATitle):TMainDialog(AParent,ATitle)
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
  virtual void ViewAll        (RTMessage Msg) = [ID_FIRST + ID_VIEW];
  virtual void SetData        (RTMessage Msg) = [ID_FIRST + ID_SETDATA];

  virtual BOOL DistList(char *szQuery);
  void 	 SetRequestNumber(char *Vendorno,int iStartingReq);
  void    PrintPaymentList(void);
  void    SetDistPaid();
  void    SetDistDiscount();
  char   *GetDescription(char *szAccount,char *szDept);
};

// Creates a dialog box to modify the query for the
// viewing of the AR accounts.

class TSetData: public TMainDialog
{
 public:
 PTardist ardist;
 Str4large* szSortKey;
 TSetData(PTWindowsObject AParent, int AName,PTardist a,Str4large* szSort): TMainDialog(AParent,AName)
 {
  ardist = a;
  szSortKey = szSort;
 };
 virtual void Ok     (RTMessage Msg) = [ID_FIRST + IDOK]
 {
  char szKey[10];
  //"PAYMENTCTL <> 'C'"
  szSortKey->assign(GetString(4021));
  int Indx = (int)SendDlgItemMsg(ID_COMPANY,CB_GETCURSEL,NULL,NULL);
  if(Indx != CB_ERR)
  {
	SendDlgItemMsg(ID_COMPANY,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szKey);
	// "PAYMENTCTL <> 'C'"
	// ".AND. VENDORNO = '"
	szSortKey->assign(GetString(4021));
	szSortKey->add(GetString(4022));
	szSortKey->add(szKey);
	szSortKey->add("'");
  }
  ShutDownWindow();
 };
 virtual void Cancel (RTMessage Msg) = [ID_FIRST + IDCANCEL]
 {
  szSortKey->assign("NO");
  ShutDownWindow();
 };
 virtual void SetupWindow();
};


void TSetData::SetupWindow()
{
  Str4large szVendorCode,szCompareCode;
  //ardist->top();
  //szVendorCode

  for(ardist->top();!ardist->eof();ardist->skip())
	{
	  szVendorCode.assign(*ardist->vendorno);
	  if(szVendorCode > szCompareCode)
		{
		 SendDlgItemMsg(ID_COMPANY,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)ardist->vendorno->str());
		 szCompareCode.assign(*ardist->vendorno);
		}
	}
}

 void TARCreatePayment::SetData(RTMessage)
 {
	Str4large* szSortKey;
	szSortKey = new Str4large;
	GetApplication()->ExecDialog(new TSetData(this,ID_SETDATA,ardist,szSortKey));
	if(strcmp(szSortKey->str(),"NO") != 0)
	  DistList(szSortKey->ptr());
	return;
 }


class TARClear: public TMainDialog
{
 public:
 PTardist ardist;
 BOOL Hold;
 BOOL Force;
 BOOL Partial;
 BOOL Discount;
 TARClear(PTWindowsObject AParent, int AName,PTardist a): TMainDialog(AParent,AName)
 {
  ardist = a;
 };
 virtual void SetupWindow();
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL] {bReset = TRUE; ShutDownWindow();};
 virtual void GetBankAccount (RTMessage Msg) = [ID_FIRST + ID_DESCRIPT];
 virtual void SetPartial     (RTMessage Msg) = [ID_FIRST + ID_PARTIAL];
 virtual void SetDiscount    (RTMessage Msg) = [ID_FIRST + ID_DISCOUNT];
 virtual void SetWithholding (RTMessage Msg) = [ID_FIRST + ID_WITHHOLDING];
 virtual void UpdateAmount   (RTMessage Msg) = [ID_FIRST + ID_UPDATE];
 virtual void Reset          (RTMessage Msg) = [ID_FIRST + ID_RESET];
 void    SetCashAccounts();
 BOOL CheckAccount(char *szAccount,char *szDepart);
 virtual void SetData(Data4 *dbf,int ID,char *szField);
};

void TARClear::Reset(RTMessage)
{
  ardist->amountpaid->assignDouble(0);
  bReset = TRUE;
  ShutDownWindow();
  return;
}

void TARClear::UpdateAmount (RTMessage Msg)
{

}

void TARClear::SetPartial  (RTMessage)
{
  if(SendDlgItemMsg(ID_PARTIAL,BM_GETCHECK,NULL,NULL))
	 SendDlgItemMsg(ID_PARTIAL,BM_SETCHECK,FALSE,NULL);
  else
	 SendDlgItemMsg(ID_PARTIAL,BM_SETCHECK,TRUE,NULL);
	 SendDlgItemMsg(ID_DISCOUNT,BM_SETCHECK,FALSE,NULL);
	 SendDlgItemMsg(ID_WITHHOLDING,BM_SETCHECK,FALSE,NULL);
}


void TARClear::SetDiscount (RTMessage)
{
  if(SendDlgItemMsg(ID_DISCOUNT,BM_GETCHECK,NULL,NULL))
	 SendDlgItemMsg(ID_DISCOUNT,BM_SETCHECK,FALSE,NULL);
  else
	 SendDlgItemMsg(ID_DISCOUNT,BM_SETCHECK,TRUE,NULL);
	 SendDlgItemMsg(ID_PARTIAL,BM_SETCHECK,FALSE,NULL);
	 SendDlgItemMsg(ID_WITHHOLDING,BM_SETCHECK,FALSE,NULL);

}

void TARClear::SetWithholding (RTMessage)
{
  if(SendDlgItemMsg(ID_WITHHOLDING,BM_GETCHECK,NULL,NULL))
	 SendDlgItemMsg(ID_WITHHOLDING,BM_SETCHECK,FALSE,NULL);
  else
	 SendDlgItemMsg(ID_WITHHOLDING,BM_SETCHECK,TRUE,NULL);
	 SendDlgItemMsg(ID_PARTIAL,BM_SETCHECK,FALSE,NULL);
	 SendDlgItemMsg(ID_DISCOUNT,BM_SETCHECK,FALSE,NULL);
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

 char *szBankAccount = new char[gl_setat.name->len()+1];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
	long Indx =  SendDlgItemMsg(ID_DESCRIPT,CB_GETCURSEL,0,0);
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

void TARClear::SetCashAccounts()
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



BOOL TARClear::CheckAccount(char *szAccount, char *szDepart)
{
 BOOL bFound = FALSE;
 Data4 *glaccts = new Data4(&cb,"glaccts");
 glaccts->SetIndex("account");
 Field4 accounts(*(Data4 *)glaccts,1);
 Field4 dept(*(Data4 *)glaccts,2);
 glaccts->SetIndex("accounts");
 char *szSeek = new char [accounts.len() + dept.len() + 1];
 wsprintf(szSeek,"%s%s",szAccount,szDepart);
 if(glaccts->seek(szSeek) == 0)
   bFound = TRUE;
 else
	MessageBox(HWindow,GetString(8131),szApp,MB_ICONEXCLAMATION);
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
 Str4large szDue,szPaid;
 szDue.assignDouble(ardist->dRemainder(),16,2);
 szPaid.assignDouble(ardist->dAmount()-ardist->dRemainder(),16,2);
 SetField(ID_DEBIT,ardist->amount->str());
 SetField(ID_PAIDTODATE,szPaid.ptr());
 SetField(ID_DUEAMOUNT,szDue.str());
 SetField(ID_AMOUNT,szDue.str());
 SetField(ID_ACCOUNTS,ardist->account);
 SetField(ID_DEPART,ardist->dept);
 char szDate[11];
 Date4 dDate(ardist->datepaid->str());
 dDate.format(szDate,GetString(4024));
 SetField(ID_DATE,szDate);
 if(strlen(StrTrim(ardist->datepaid->str())) == 0)
 {
  Date4 Today;
  Today.today();
  SetField(ID_DATE,Today.str());
 }

 if(ar_cust->seek(ardist->vendorno->str()) == 0)
  {
	SetData(ar_cust,2016,"ar_acct");
	SetData(ar_cust,2017,"ar_dept");
	SetData(ar_cust,2018,"vat_acct");
	SetData(ar_cust,2019,"vat_dept");
	SetData(ar_cust,2020,"account");
	SetData(ar_cust,2021,"depart");
	SetData(ar_cust,2122,"withtax");
	SetData(ar_cust,2123,"withdept");
  }
  SetBitmap(ID_RESET);
}

void TARClear::Ok(RTMessage)
  {
	 // || strcmp(ardist->paymentctl->str(),"P") == 0

	 BOOL bClear = TRUE;
	 BOOL bDiscount = FALSE;
	 BOOL bPartial  = FALSE;
	 bReset = FALSE;
	 if(SendDlgItemMsg(ID_DISCOUNT,BM_GETCHECK,NULL,NULL))
		bDiscount = TRUE;
	 else
	 if(SendDlgItemMsg(ID_PARTIAL,BM_GETCHECK,NULL,NULL))
		bPartial = TRUE;
	 else
		bClear = TRUE;

	 if(strcmp(ardist->paymentctl->str(),"C") == 0 )
	 {
	  MessageBox(HWindow,GetString(8127),szApp,MB_ICONINFORMATION);
	  return;
	 }

	 Date4 dPayment;
	 // GetString(4024)
	 dPayment.assign(GetField(ID_DATE),GetString(4024));
	 // "CCCCMMDD"
	 dPayment.format(GetString(4023));

	 double dAmountPaid = dGetField(ID_AMOUNT);
	 double dAmount     = ardist->dAmount();

	 if(!CheckAccount(GetField(ID_ACCOUNTS),""))
		return;

	 if(strlen(StrTrim(GetField(ID_DATE))) == 0 )
		return;

	 if(dAmountPaid == 0)
	  {
		 MessageBox(HWindow,GetString(1037),szApp,MB_ICONINFORMATION);
		 return;
	  }

	 if(dAmountPaid > dAmount)
	 {
		 MessageBox(HWindow,GetString(8133),szApp,MB_ICONINFORMATION);
		 return;
	  }

			char szPaymentctl[2];


	 if(bClear)
	  strcpy(szPaymentctl,"P");

	 if(bPartial)
	  strcpy(szPaymentctl,"R");

	 if(bPartial && dAmountPaid == dAmount)
			 {
		MessageBox(HWindow,GetString(8134),szApp,MB_ICONINFORMATION);
		return;
			 }

	  if(bDiscount && dAmountPaid == dAmount)
			 {
		MessageBox(HWindow,GetString(8134),szApp,MB_ICONINFORMATION);
		return;
			 }



	 if(bDiscount)
	  strcpy(szPaymentctl,"D");

	 if(dAmountPaid < dAmount && !bDiscount && !bPartial)
			{
		switch(MessageBox(HWindow,GetString(8132),szApp,MB_YESNOCANCEL | MB_ICONQUESTION))
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
	  double dRemainder = ardist->dAmount()-ardist->dAmountpaid();
	  //ardist->remainder->assignDouble(dRemainder);
	  szSales.assign(GetField(2016));
	  szSalesDept.assign(GetField(2017));
	  szVAT.assign(GetField(2018));
	  szVATDept.assign(GetField(2019));
	  szAR.assign(GetField(2020));
	  szARDept.assign(GetField(2021));
	  ardist->UpdateUserTime(szUser);
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
  if(strcmp(ardist->paymentctl->str(),"C")== 0)
  return;
  SSSetColor(SHWindow,SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
  GetApplication()->ExecDialog(new TARClear(this,1155,ardist));
  SSSetColor(SHWindow,SS_ALLCOLS,Cell->Row,RGB(250,250,250),RGB(0,0,0));

  SSSetData(SHWindow,2,Cell->Row,ardist->paymentctl->str());
  Str4large szAmountPaid,szRemainder;
  szAmountPaid.assignDouble(ardist->dAmountpaid(),16,2);
  SSSetData(SHWindow,9,Cell->Row,szAmountPaid.ptr());
  //SSSetData(SHWindow,10,Cell->Row,ardist->remainder->str());
  szRemainder.assignDouble(ardist->dRemainder()-ardist->dAmountpaid(),16,2);
  SSSetData(SHWindow,10,Cell->Row,szRemainder.ptr());
  SSClearData(GetDlgItem(D->HWindow,ID_DISTLIST),SS_ALLCOLS,SS_ALLROWS);
  if(ardist->Paid() || ardist->Remainder() && !bReset)
	SetDistPaid();
  else
  if(ardist->Discount())
	SetDistDiscount();
 }
}



void TARCreatePayment::Print(RTMessage)
{
  if(MessageBox(HWindow,GetString(4076),szApp,MB_YESNO|MB_ICONQUESTION) == IDYES)
  PrintPaymentList();
}

void TManilaGL::ARCreateSlip(RTMessage)
{
 GetApplication()->ExecDialog(new TARCreatePayment(this,1156));
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
 Field4 descript(GLACCTS,3);
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
 return szDescript;
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
	 ardist->remainder->assignDouble(0);
	 ardist->UpdateUserTime(szUser);
	 ardist->flush();

	 arjsum->appendStart();
	 arjsum->account->assign      (ardist->account->str());
	 arjsum->dept->assign         (ardist->dept->str());
	 arjsum->period->assign       (ardist->period->str());
	 arjsum->mode->assign         ("1");
	 arjsum->date->assign         (ardist->date->str());
	 arjsum->type->assign         ("AR");
	 arjsum->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 arjsum->reference->assign    (ardist->reference->str());
	 arjsum->descript->assign     (StrTrim(ardist->descript->str()));
	 arjsum->debit->assignDouble  (ardist->dAmountpaid());
	 arjsum->credit->assignDouble (0.0);
	 arjsum->append();

	 arjsum->appendStart();
	 arjsum->account->assign      (ardist->apaccount->str());
	 arjsum->dept->assign         (ardist->apdepart->str());
	 arjsum->period->assign       (ardist->period->str());
	 arjsum->date->assign         (ardist->date->str());
	 arjsum->mode->assign         ("1");
	 arjsum->type->assign         ("AR");
	 arjsum->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 arjsum->reference->assign    (ardist->reference->str());
	 arjsum->descript->assign     (StrTrim(ardist->descript->str()));
	 arjsum->debit->assignDouble  (0);
	 arjsum->credit->assignDouble (ardist->dAmountpaid());
	 arjsum->append();
	  }
	rQuery->unlock();
	rQuery->free();

	Relate4set *pQuery = new Relate4set(ardist);
	pQuery->querySet("PAYMENTCTL = 'R' .AND. AMOUNTPAID > 0");
	 for(qc=pQuery->top();qc!=r4eof;qc=pQuery->skip(1L))
	  {
	 // If the total amount of the invoice has been paid
	 // make the invoice a cleared invoice.

	 ardist->mode->assign("1");
	 double dCurrentRemainder = ardist->dRemainder() - ardist->dAmountpaid();
	 ardist->remainder->assignDouble(dCurrentRemainder);
	 ardist->amountpaid->assignDouble(0);
	 if(ardist->dRemainder() == 0)
	  ardist->paymentctl->assign("C");
	 ardist->UpdateUserTime(szUser);
	 ardist->flush();

	 arjsum->appendStart();
	 arjsum->account->assign      (ardist->account->str());
	 arjsum->dept->assign         (ardist->dept->str());
	 arjsum->period->assign       (ardist->period->str());
	 arjsum->mode->assign         ("1");
	 arjsum->date->assign         (ardist->date->str());
	 arjsum->type->assign         ("AR");
	 arjsum->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 arjsum->reference->assign    (ardist->reference->str());
	 arjsum->descript->assign     (ardist->descript->str());
	 arjsum->debit->assignDouble  (ardist->dAmountpaid());
	 arjsum->credit->assignDouble (0.0);
	 arjsum->append();

	 arjsum->appendStart();
	 arjsum->account->assign      (ardist->apaccount->str());
	 arjsum->dept->assign         (ardist->apdepart->str());
	 arjsum->period->assign       (ardist->period->str());
	 arjsum->date->assign         (ardist->date->str());
	 arjsum->mode->assign         ("1");
	 arjsum->type->assign         ("AR");
	 arjsum->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 arjsum->reference->assign    (ardist->reference->str());
	 arjsum->descript->assign     (ardist->descript->str());
	 arjsum->debit->assignDouble  (0);
	 arjsum->credit->assignDouble (ardist->dAmountpaid());
	 arjsum->append();
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
	 ardist->UpdateUserTime(szUser);
	 ardist->flush();

	 double dOriginalAR    = ardist->dAmount();  //AR
	 double dVatPercent    = ardist->dVatamt()/(ardist->dAmount()-ardist->dVatamt());
	 double dAmountPaid    = ardist->dAmountpaid();             //CASH
	 double dSalesDiscount = (dOriginalAR - dAmountPaid)/(1+dVatPercent); // SALES DISCOUNT
	 double dVatDiscount   = dSalesDiscount * dVatPercent; 	// VAT ADJUSTMENT



         // CASH
	 arjsum->appendStart();
	 arjsum->account->assign      (ardist->account->str());
	 arjsum->dept->assign         (ardist->dept->str());
	 arjsum->period->assign       (ardist->period->str());
	 arjsum->mode->assign         ("1");
	 arjsum->date->assign         (ardist->date->str());
	 arjsum->type->assign         ("AR");
	 arjsum->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 arjsum->reference->assign    (ardist->reference->str());
	 arjsum->descript->assign     (ardist->descript->str());
	 arjsum->debit->assignDouble  (dAmountPaid);
	 arjsum->credit->assignDouble (0.0);
	 arjsum->append();


	 // SALES DISCOUNT
	 char szAccount[8];
	// strcpy(szAccount,GetAccount(SALESACCT));



	 arjsum->appendStart();
	 arjsum->account->assign      (szSales.str());
	 arjsum->dept->assign         (szSalesDept.str());
	 arjsum->period->assign       (ardist->period->str());
	 arjsum->date->assign         (ardist->date->str());
	 arjsum->mode->assign         ("1");
         arjsum->type->assign         ("AR");
	 arjsum->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 arjsum->reference->assign    (ardist->reference->str());
	 arjsum->descript->assign     (GetDescription(Sales.str(),SalesD.str()));
	 arjsum->debit->assignDouble  (dSalesDiscount);
	 arjsum->credit->assignDouble (0);
	 arjsum->append();

	 // VAT Change
	// strcpy(szAccount,GetAccount(VATACCT));
	 arjsum->appendStart();
	 arjsum->account->assign      (VAT1.str());
	 arjsum->dept->assign         (VAT1Dept.str());
	 arjsum->period->assign       (ardist->period->str());
	 arjsum->date->assign         (ardist->date->str());
	 arjsum->mode->assign         ("1");
         arjsum->type->assign         ("AR");
	 arjsum->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 arjsum->reference->assign    (ardist->reference->str());
	 arjsum->descript->assign     (GetDescription(VAT1.str(),VAT1Dept.str()));
	 arjsum->debit->assignDouble  (dVatDiscount);
	 arjsum->credit->assignDouble (0);
	 arjsum->append();

	 // AR Amount
	 arjsum->appendStart();
	 arjsum->account->assign      (ardist->apaccount->str());
	 arjsum->dept->assign         (ardist->apdepart->str());
	 arjsum->period->assign       (ardist->period->str());
	 arjsum->date->assign         (ardist->date->str());
	 arjsum->mode->assign         ("1");
			arjsum->type->assign         ("AR");
	 arjsum->batchno->assignDouble(double(Str4ptr(ardist->batchno->str())));
	 arjsum->reference->assign    (ardist->reference->str());
	 arjsum->descript->assign     (ardist->descript->str());
	 arjsum->debit->assignDouble  (0);
	 arjsum->credit->assignDouble (dOriginalAR);
	 arjsum->append();

		}
     }
	rDiscount->unlock();
   rDiscount->free();


 DistList("PAYMENTCTL <> 'C'");
 return;
}
#pragma argsused
void TARCreatePayment::SetRequestNumber(char *szVendorno,int iRequestNumber){}

void TARCreatePayment::SetDate (RTMessage)
{
 // "PAYMENTCTL = 'H' .OR. PAYMENTCTL = 'F' .OR. PAYMENTCTL = 'O'"
 bHasInvoices = DistList(GetString(6021));
 if(!bHasInvoices)
  MessageBox(HWindow,GetString(4074),szApp,MB_OK);
}

void TARCreatePayment::ViewAll(RTMessage)
{
 DistList(".NOT. DELETED()");
}

void TARCreatePayment::SetupWindow()
{
 TDialog::SetupWindow();
 D = new TARDistribution(this,6067);
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

 InvoiceList->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
 InvoiceList->SetBool(SSB_PROTECT,TRUE);
 InvoiceList->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 InvoiceList->SetBool(SSB_NOBEEP,TRUE);
 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 InvoiceList->SetBool(SSB_VERTSCROLLBAR,TRUE);

 InvoiceList->SetMaxRows(1000);
 InvoiceList->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);
 InvoiceList->SetData(1,SS_HEADER, "Customer");
 InvoiceList->SetData(2,SS_HEADER, GetString(ID_CONTROL));
 InvoiceList->SetData(3,SS_HEADER, GetString(ID_VENDORCODE));
 InvoiceList->SetData(4,SS_HEADER, GetString(ID_INVOICENO));
 InvoiceList->SetData(5,SS_HEADER, GetString(ID_DUEDATE));
 InvoiceList->SetData(6,SS_HEADER, GetString(ID_AMOUNT));
 InvoiceList->SetData(7,SS_HEADER, "VAT");
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
 InvoiceList->SetTypeFloat(&CellType,FS_SEPARATOR,9,2,0,999999999999.99);
 InvoiceList->SetCellType(6, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(7, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(8, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(9, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(10, SS_ALLROWS, &CellType);


 // Open databases...
 ardist   =  new Tardist(&cb);
 arcust   =  new Tarcust(&cb);
 arbtch   =  new Tarbtch(&cb);
 arjsum   =  new Tarjsum(&cb);

 bHasInvoices = FALSE;

 HourGlassOff();
 DistList("PAYMENTCTL <> 'C'");
 D->Show(SW_SHOW);
 SetBitmap(ID_VIEW);
 SetBitmap(IDCANCEL);
 SetBitmap(IDOK);
 SetBitmap(ID_SETDATA);
}

void TARCreatePayment::Cancel  (RTMessage)
{
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}
BOOL TARCreatePayment::DistList(char *szQuery)
{
 HourGlassOn();
 int l = 1;
 char *szControl = new char [2];
  // "Checking for invoices that are due."
 SetField(IDD_STATUSBOX,GetString(6019));
 Date4 dPayment;
 dPayment.assign(GetField(ID_DATE),GetString(4024));
 dPayment.format(GetString(4023));
 InvoiceList->SetBool(SSB_REDRAW,FALSE);
 InvoiceList->ClearData(SS_ALLCOLS,SS_ALLROWS);
 ardist->SetIndex("invoiceno");
 Relate4set *rQuery = new Relate4set(ardist);
 rQuery->querySet(szQuery);
 InvoiceList->SetColor(SS_ALLCOLS,SS_ALLROWS,RGB(250,250,250),RGB(0,0,0));
 char szDate[11];
	 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
	 {
		 Date4 dDueDate(ardist->duedate->str());
	// if( dDueDate <= dPayment )
	// {
		 arcust->seek(ardist->vendorno->str());
		 InvoiceList->SetData(1,l,arcust->vdshort->str());  // Vendor Name
		 InvoiceList->SetData(2,l,ardist->paymentctl->str()); // Payment Control
		 InvoiceList->SetData(3,l,ardist->vendorno->str());  // Vendor Code
		 InvoiceList->SetData(4,l,ardist->invoiceno->str()); // Invoice Number
		 Date4 dDate(ardist->duedate->str());
		 dDate.format(szDate,GetString(4024));
	    InvoiceList->SetData(5,l,szDate);   // Due Date
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
		 if(strcmp(ardist->paymentctl->str(),"C") == 0)
		 InvoiceList->SetColor(SS_ALLCOLS,l,RGB(255,255,201),RGBCOLOR_BLUE);
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

 SetField(IDD_STATUSBOX,GetString(6020));
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


void TARDistribution::SetupWindow()
{
 TDialog::SetupWindow();
 HourGlassOn();
 int Rows = 8;
 RECT rc,wd;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 short iHeight;
 LOGFONT lfFont;
 HFONT   hFont;
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
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);
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


 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,999999999999.99);
 SSSetCellType(SHWindow,5, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,6, SS_ALLROWS, &CellType);

 HourGlassOff();
}
/*
void TARDistribution::SetListSize (int Rows)
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
*/
