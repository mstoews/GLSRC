#include <tbowl.h>
#include "d4all.hpp"
#include "gl.h"
#include "gl_manil.h"
#include "glclass.h"
#include "gl_main.h"
#include "gl_man.h"
#include "gl_dlg.h"
#include "gl_pick.h"
#include "crpe.h"

#include "db_dept.h"
#include "db_apbtc.h"

#include "db_cash.h"
#include "db_itemm.h"
#include "db_medrp.h"
#include "db_drcod.h"
#include "ap_class.h"
#include "ap_setup.h"
#include "ap_btch.h"
#include "ap_btchl.h"
#include "utildlg.h"
#include "gl_pds.hpp"
#include "ap_vendo.h"
#include "utildlg.h"
#include "gl_setat.hpp"

#include "math.h"



#define ID_INVOICE 1030
#define ID_VATACCT 1034
#define ID_VATDEPT 1035

extern char szApp[20];

void TAPInvoiceEntry::RecoverPostedInvoice()
{
 long dBatchNo;

 IntGetValue(GetDlgItem(HWindow,ID_BATCHNO),(LPLONG)&dBatchNo);

 if(apbtch->seek(dBatchNo) == 0)
 {
	  apbtch->mode->assign("1");
	  apbtch->UpdateUserTime(szUser);
     apbtch->flush();
     apbtch->unlock();
 }

  gldist->SetIndex("apdist");
  if(gldist->seek(dBatchNo) == 0 )
  {
	while (double (Str4ptr(gldist->batchno->str())) == dBatchNo)
    {
		 gldist->deleteRec();
		 gldist->skip();
    }
  }
 if(apdist->seek(dBatchNo) == 0 )
  {
   while (double (Str4ptr(apdist->batchno->str())) == dBatchNo)
    {
     apdist->deleteRec();
     apdist->skip();
    }
  }
  gldist->pack();
  gldist->unlock();
  apdist->unlock();
}

	/*
	double AMOUNT  = dGetField(ID_INVOICE);
	double EXPENSE = AMOUNT/dVatPercentage;
	double VAT     = AMOUNT-EXPENSE;

	double REBATE  = 0.0;
	if(dRebatePercentage != 1)
		REBATE  = -1*(EXPENSE*dRebatePercentage);

	double APAMT = -1*(AMOUNT + REBATE);
	*/



void TAPInvoiceEntry::CreateAPBatchEntry(int iBatchno)
{
	string szSeekAccount;
	string szRebateAcct;
	string szRebateDept;

	apbtch->SetIndex(GetString(8140));
	apbtchd->SetIndex(GetString(8140));

	if(apbtch->seek(iBatchno)== 0 && !apbtch->Open()){
	 RecoverPostedInvoice();
	 return;
	 }


	if(dGetField(ID_REBATE) > 0)
	{
	 if(SendDlgItemMsg(ID_INDIVIDUAL,BM_GETSTATE,NULL,NULL))
	  {
		szSeekAccount =  szIndTaxAccount;
		szSeekAccount += szIndTaxDept;
		szRebateAcct  =  szIndTaxAccount;
		szRebateDept  = szIndTaxDept;
	  }
	 else if(SendDlgItemMsg(ID_COMPANY,BM_GETSTATE,NULL,NULL))
	 {
		szSeekAccount = szCompanyTaxAccount;
		szSeekAccount += szCompanyTaxDept;
		szRebateAcct  =  szCompanyTaxAccount;
		szRebateDept  =  szCompanyTaxDept;
	 }
	 else if(!SendDlgItemMsg(ID_COMPANY,BM_GETSTATE,NULL,NULL) && !SendDlgItemMsg(ID_INDIVIDUAL,BM_GETSTATE,NULL,NULL))
	 {
	  MessageBox(HWindow,GetString(4101),szApp,MB_ICONEXCLAMATION);
	  return;
	 }
	}

	FloatGetDlgItemValue(HWindow,ID_INVOICE,(LPDOUBLE) &lInvoiceAmount);

	if(dGetField(ID_REBATE) > 0)
		 dRebatePercentage = (dGetField(ID_REBATE))/100;
	else
		dRebatePercentage = 0;

	if(dGetField(ID_VATPERCENTAGE) > 0)
		dVatPercentage = (dGetField(ID_VATPERCENTAGE))/100;
	else
		dVatPercentage = 1;

	AP_EXPENSE* expense = new AP_EXPENSE;
	GetAPExpense(expense,lInvoiceAmount,dVatPercentage,dRebatePercentage);

	UpdateBatchHeader(iBatchno,expense->NEWAP,expense->VAT,expense->REBATE);

	string szExpenseDescript(GetField(ID_JDESCRIPT));
	string szAccount(GetField(ID_ACCOUNTS));
	string szDept(GetField(ID_DEPT));
	string szMedRep(GetMedRep());
	string szProduct(GetProduct());
	string szDoctor(GetDoctor());


	//1. Expenses
	UpdateBatchExpense(iBatchno,szExpenseDescript,
										 szAccount,
										 szDept,
										 expense->NEWEXPENSE,
										 szMedRep,
										 szProduct,
										 szDoctor);

	//2. VAT
	if(expense->VAT != 0)
	 {
	  if(strlen(StrTrim(GetField(ID_VATACCT))) == 0)
		UpdateBatchDetail(iBatchno, "",szVATAccount,szVATDept, expense->VAT,"","","");
	  else
		{
		 string szVATAccount(GetField(ID_VATACCT));
		 string szVATDept(GetField(ID_VATDEPT));
		 UpdateBatchDetail(iBatchno, "",(char *)szVATAccount.c_str(),(char *)szVATDept.c_str(), expense->VAT,"","","");
		}
	 }

  // 3. WithHolding Tax
  if(expense->REBATE != 0)
	 UpdateBatchDetail(iBatchno, "",(char *)szRebateAcct.c_str(),(char *)szRebateDept.c_str(),expense->REBATE,"","","");


  // 4. Accounts Payable
  szAccount.assign(GetField(ID_APCONTROL));
  szDept.assign(GetField(ID_APDEPART));
  UpdateBatchDetail(iBatchno,"",(char *)szAccount.c_str(),(char *)szDept.c_str(),-1*(expense->APAMT),"","","");
  return;
}

void TAPInvoiceEntry::UpdateBatchHeader(long iBatchno, double APAMT, double VAT, double REBATE)
{
  Date4 Today;
  Today.today();
  if(apbtch->seek(iBatchno) == 0)
	{
	 apbtch->mode->assign("1");
	 apbtch->adj->assign("N");
	 apbtch->period->assignDouble(GetPeriod(HWindow));
	 apbtch->orderno->assign(GetField(ID_CURRBAL));
	 apbtch->vendorno->assign(GetField(ID_VENDORNO));
	 apbtch->vdname->assign(GetField(ID_VENDORNAME));
	 apbtch->invoiceno->assign(GetField(ID_INVOICENO));
	 apbtch->date->assign(Today.str());
	 apbtch->duedate->assign(DueDate.str());
	 apbtch->invdate->assign(InvoiceDate.str());
	 apbtch->reference->assign(GetField(ID_REFERENCE));
	 apbtch->descript->assign(GetField(ID_JDESCRIPT));
	 apbtch->account->assign(GetField(ID_APCONTROL));
	 apbtch->depart->assign(GetField(ID_APDEPART));
	 apbtch->vatpercent->assignDouble(dGetField(ID_VATPERCENTAGE));
	 apbtch->terms->assignLong(lGetField(ID_TERMS));
	 apbtch->payable->assignDouble(APAMT);
	 apbtch->vatamt->assignDouble(VAT);
	 apbtch->rebateamt->assignDouble(REBATE);
	 apbtch->rebate->assignDouble(dGetField(ID_REBATE));
	 apbtch->UpdateUserTime(szUser);
	 apbtch->flush();
	 apbtch->unlock();
  }
  else
  {
	 apbtch->appendStart();
	 apbtch->batchno->assignLong(iBatchno);
	 apbtch->mode->assign("1");
    apbtch->adj->assign("N");
    apbtch->period->assignDouble(GetPeriod(HWindow));
	 apbtch->orderno->assign(GetField(ID_CURRBAL));
	 apbtch->vendorno->assign(GetField(ID_VENDORNO));
    apbtch->vdname->assign(GetField(ID_VENDORNAME));
	 apbtch->invoiceno->assign(GetField(ID_INVOICENO));
	 apbtch->duedate->assign(DueDate.str());
	 apbtch->invdate->assign(InvoiceDate.str());
	 apbtch->reference->assign(GetField(ID_REFERENCE));
	 apbtch->descript->assign(GetField(ID_JDESCRIPT));
	 apbtch->account->assign(GetField(ID_APCONTROL));
	 apbtch->depart->assign(GetField(ID_APDEPART));
	 apbtch->vatpercent->assignDouble(dGetField(ID_VATPERCENTAGE));
	 apbtch->terms->assignLong(lGetField(ID_TERMS));
	 apbtch->payable->assignDouble(APAMT);
	 apbtch->vatamt->assignDouble(VAT);
	 apbtch->rebateamt->assignDouble(REBATE);
	 apbtch->rebate->assignDouble(dGetField(ID_REBATE));
	 apbtch->UpdateUserTime(szUser);
	 apbtch->append();
	 apbtch->unlock();
  }

}


void TAPInvoiceEntry::UpdateBatchDetail(long iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry,char *szMedRep,char *szProduct,char *szDoctor)
{
 char *szReference = new char[50+1];
 if(strcmp(szDescript,"") == 0)
	{
	 char *szSeek = new char[12];
	 strcpy(szSeek,szAccount);
	 glacct->SetIndex("accounts");
	 glacct->seek(strcat(szSeek,szDepart));
	 strcpy(szReference,glacct->descript->str());
	 delete szSeek;
	 }
  else
  strcpy(szReference,szDescript);

 apbtchd->SetIndex(GetString(8140));
 if(apbtchd->seek(iBatchno) == 0)
 {
		// If the entry for that Account and Department code has already been entered just update it.
	int iEntryno = 1;
	while(iBatchno == apbtchd->lBatchno())
	{
	 if(strcmp(StrTrim(szAccount),StrTrim(apbtchd->account->str())) == 0
		  && strcmp(StrTrim(szDepart),StrTrim(apbtchd->dept->str())) == 0 )
	  {

		  apbtchd->descript->assign(szReference);
				 if(dEntry > 0)
				  {
					apbtchd->debit->assignDouble(fabs(dEntry));
					apbtchd->credit->assignDouble(0.00);
				  }
				 else if (dEntry < 0)
				  {
					apbtchd->debit->assignDouble(0.00);
					apbtchd->credit->assignDouble(fabs(dEntry));
				  }
		  
		  apbtchd->flush();
		  apbtchd->unlock();
				 return;
	  }
	 apbtchd->skip();
	 iEntryno++;
		  }
	  // if not found create a new line entry.
	  apbtchd->appendStart();
	  apbtchd->batchno->assignLong(iBatchno);
	  apbtchd->descript->assign(szReference);
	  apbtchd->account->assign(szAccount);
	  apbtchd->dept->assign(szDepart);
	  apbtchd->product->assign(szProduct);
	  apbtchd->medreps->assign(szMedRep);
	  apbtchd->drcodes->assign(szDoctor);

	  if(dEntry < 0)
		{
		 apbtchd->debit->assignDouble(0.00);
		 apbtchd->credit->assignDouble(fabs(dEntry));
		}
	  else if (dEntry > 0)
		{
		 apbtchd->debit->assignDouble(fabs(dEntry));
		 apbtchd->credit->assignDouble(0.00);
		}
		apbtchd->append();
		apbtchd->unlock();
  }
 else
 {

	  apbtchd->appendStart();
	  apbtchd->batchno->assignLong(iBatchno);
	  apbtchd->descript->assign(szReference);
	  apbtchd->account->assign(szAccount);
	  apbtchd->dept->assign(szDepart);
	  apbtchd->product->assign(szProduct);
	  apbtchd->medreps->assign(szMedRep);
	  apbtchd->drcodes->assign(szDoctor);

	  if(dEntry < 0)
		{
		 apbtchd->debit->assignDouble(0.00);
		 apbtchd->credit->assignDouble(fabs(dEntry));
		}
	  else if (dEntry > 0)
		{
		 apbtchd->debit->assignDouble(fabs(dEntry));
		 apbtchd->credit->assignDouble(0.00);
		}
	  apbtchd->append();
	  apbtchd->unlock();

 }
 return;
}

void TAPInvoiceEntry::UpdateBatchExpense(long iBatchno, string szDescript, string szAccount,string szDepart,double dEntry,string szMedRep,string szProduct,string szDoctor)
{
 string szReference;
 if(szDescript == "")
	{
	 char *szSeek = new char[12];
	 strcpy(szSeek,(char *)szAccount.c_str());
	 glacct->SetIndex("accounts");
	 glacct->seek(strcat(szSeek,(char *)szDepart.c_str()));
	 szReference.assign(glacct->descript->str());
	 delete szSeek;
	 }
 else
	szReference = szDescript;

	apbtchd->appendStart();
	apbtchd->batchno->assignLong(iBatchno);
	apbtchd->descript->assign((char *)szReference.c_str());
	apbtchd->account->assign((char *)szAccount.c_str());
	apbtchd->dept->assign((char *)szDepart.c_str());
	apbtchd->product->assign((char *)szProduct.c_str());
	apbtchd->medreps->assign((char *)szMedRep.c_str());
	apbtchd->drcodes->assign((char *)szDoctor.c_str());

	  if(dEntry < 0)
		{
		 apbtchd->debit->assignDouble(0.00);
		 apbtchd->credit->assignDouble(fabs(dEntry));
		}
	  else if (dEntry > 0)
		{
		 apbtchd->debit->assignDouble(fabs(dEntry));
		 apbtchd->credit->assignDouble(0.00);
		}
	  apbtchd->append();
	  apbtchd->unlock();

}


char  *TAPInvoiceEntry::GetMedRep()
{
	char szString[41];
	Str4ten *szCode = new Str4ten;
	szCode->set(' ');
	szCode->setLen(medreps->repcode->len());
	int Indx = (int)SendDlgItemMsg(ID_MEDREPS,CB_GETCURSEL,NULL,NULL);
	if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(ID_MEDREPS,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szString);
	 if(medreps->seek(szString)== 0)
		 strcpy(szCode->ptr(),medreps->repcode->str());
	}
  return szCode->str();
}

char  *TAPInvoiceEntry::GetProduct()
{
	char szString[41];
	Str4ten *szCode = new Str4ten;
	szCode->set(' ');
	szCode->setLen(prods->fPcode->len());
	int Indx = (int)SendDlgItemMsg(ID_PRODUCTS,CB_GETCURSEL,NULL,NULL);
	 if(Indx != CB_ERR)
	 {
	  SendDlgItemMsg(ID_PRODUCTS,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szString);
	  if(prods->seek(szString)== 0)
		 strcpy(szCode->ptr(),prods->fPcode->str());
	}
  return szCode->str();
}

char  *TAPInvoiceEntry::GetDoctor()
{
	 char szString[41];
	 Str4ten *szCode = new Str4ten;
	 szCode->set(' ');
	 szCode->setLen(drcodes->drcode->len());
	 int Indx = (int)SendDlgItemMsg(ID_DRCODES,CB_GETCURSEL,NULL,NULL);
	 if(Indx != CB_ERR)
	  {
		SendDlgItemMsg(ID_DRCODES,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szString);
		drcodes->SetIndex("rpname");
		if(drcodes->seek(szString) == 0)
		  strcpy(szCode->ptr(),drcodes->drcode->str());
	  }
	 return szCode->str();
}






