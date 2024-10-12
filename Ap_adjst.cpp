#include "std_inc.h"
#define ID_PERIOD_USED 1037


//#include "db_dept.h"
//#include "db_apbtc.h"
//#include "db_cash.h"
//#include "db_itemm.h"
//#include "db_medrp.h"
//#include "db_drcod.h"
//#include "ap_class.h"
//#include "ap_setup.h"
//#include "ap_btch.h"
//#include "ap_btchl.h"
//#include "utildlg.h"
//#include "gl_pds.hpp"
#include "ap_vendo.h"
//#include "utildlg.h"
//#include "gl_setat.hpp"

//#include "math.h"



#define ID_INVOICE 1030
#define ID_VATACCT 1034
#define ID_VATDEPT 1035
#define ID_COMMIT 1056



extern BOOL bNEW;

void TAPInvoiceEntry::RecoverPostedInvoice()
{
 long dBatchNo;

 IntGetValue(GetDlgItem(HWindow,ID_BATCHNO),(LPLONG)&dBatchNo);

 if(apbtch.seek(dBatchNo) == 0)
 {
	  apbtch.mode.assign((LPSTR)mode.c_str());
	  apbtch.UpdateUserTime(szUser);
	  apbtch.flush();
 }

  gldist.SetIndex("apdist");
  if(gldist.seek(dBatchNo) == 0 )
  {
	while (double (Str4ptr(gldist.batchno.str())) == dBatchNo)
	 {
		 gldist.deleteRec();
		 gldist.skip();
	 }
  }
 if(apdist.seek(dBatchNo) == 0 )
  {
	while (double (Str4ptr(apdist.batchno.str())) == dBatchNo)
    {
	  apdist.deleteRec();
     apdist.skip();
	 }
  }
  gldist.pack();


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



void TAPInvoiceEntry::CreateAPBatchEntry(long iBatchno)
{

	string szSeekAccount;
	string szRebateAcct;
	string szRebateDept;

	//apbtch.SetIndex(GetString(8140));
	//apbtchd.SetIndex(GetString(8140));


	if(dGetField(ID_REBATE) > 0)
	{
	 if(SendDlgItemMsg(ID_INDIVIDUAL,BM_GETSTATE,NULL,NULL))
	  {
		szSeekAccount =  strIndTaxAccount;
		szSeekAccount += strIndTaxDept;
		szRebateAcct  =  strIndTaxAccount;
		szRebateDept  =  strIndTaxDept;
	  }
	 else if(SendDlgItemMsg(ID_COMPANY,BM_GETSTATE,NULL,NULL))
	 {
		szSeekAccount =  strCompanyTaxAccount;
		szSeekAccount += strCompanyTaxDept;
		szRebateAcct  =  strCompanyTaxAccount;
		szRebateDept  =  strCompanyTaxDept;
	 }
	 else if(!SendDlgItemMsg(ID_COMPANY,BM_GETSTATE,NULL,NULL) && !SendDlgItemMsg(ID_INDIVIDUAL,BM_GETSTATE,NULL,NULL))
	 {
	  MessageBox(HWindow,GetString(4101),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	  return;
	 }
	}

	FloatGetDlgItemValue(HWindow,ID_AMOUNT,(LPDOUBLE) &lInvoiceAmount);

	if(dGetField(ID_REBATE) > 0)
		 dRebatePercentage = (dGetField(ID_REBATE))/100;
	else
		dRebatePercentage = 1;

	if(dGetField(ID_VATPERCENTAGE) > 0)
		dVatPercentage = (dGetField(ID_VATPERCENTAGE))/100;
	else
		dVatPercentage = 1;

	AP_EXPENSE* expense = new AP_EXPENSE;

	double dOldAP = 0;
	if(apbtch.seek(iBatchno) == 0)
		dOldAP = apbtch.dPayable();

	expense->REBATE_RATE    = dRebatePercentage;
	expense->VAT_RATE 		= dVatPercentage;
	expense->INVOICE_AMT 	= dGetField(ID_AMOUNT);
	expense->REBATE  			= 0.0;
	expense->VAT     	  		= 0.0;
	expense->NEWAP  	 	  	= dOldAP + dGetField(ID_AMOUNT) - apbtch.dRebateamt();
	expense->OLDAP   	  		= dOldAP;
	expense->APAMT   	  		= 0;
	expense->NEWEXPENSE 		= 0;


	GetAPExpense(expense);

	UpdateBatchHeader((long)iBatchno,expense->NEWAP,expense->VAT,expense->REBATE);

	string szExpenseDescript(GetField(ID_JDESCRIPT));
	string szAccount(GetField(ID_ACCOUNTS));
	string szDept(GetField(ID_DEPT));
	string szMedRep(GetDept());
	string szProduct(GetProduct());
	string szDoctor(GetDoctor());
	//UpdateBatchSupp((long)iBatchno,&szMedRep,&szProduct,&szDoctor);
	//1. Expenses
	UpdateBatchExpense((long)iBatchno,szExpenseDescript,
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
		UpdateBatchDetail((long)iBatchno, "",(LPSTR)strVATAccount.c_str(),(LPSTR)strVATDept.c_str(), expense->VAT,"","","");
	  else
		{
		 string szVATAccount(GetField(ID_VATACCT));
		 string szVATDept(GetField(ID_VATDEPT));
		 UpdateBatchDetail((long)iBatchno, "",(char *)szVATAccount.c_str(),(char *)szVATDept.c_str(), expense->VAT,"","","");
		}
	 }

  // 3. WithHolding Tax
  if(expense->REBATE != 0)
	 UpdateBatchDetail((long)iBatchno, "",(char *)szRebateAcct.c_str(),(char *)szRebateDept.c_str(),expense->REBATE,"","","");


  // 4. Accounts Payable
  szAccount.assign(GetField(ID_APCONTROL));
  szDept.assign(GetField(ID_APDEPART));
  UpdateBatchDetail((long)iBatchno,"",(char *)szAccount.c_str(),(char *)szDept.c_str(),-1*(expense->APAMT),"","","");
  delete expense;
  return;
}

#pragma argsused
void TAPInvoiceEntry::GetAPExpense(AP_EXPENSE* ex)
 {

  // Withholding tax should be deducted from the AP Amount.
  if(!GetWithHolding())
  {
	 // If a previous entry was made accumulate the amounts.

	 ex->NEWEXPENSE = dGetField(ID_AMOUNT);

	 if(ex->OLDAP >  0)
	  ex->OLDEXPENSE = ex->OLDAP - apbtch.dVatamt() - apbtch.dRebateamt();
	 else
	  ex->OLDEXPENSE = 0;

	 // If we have vat calculate new vat amount and expense minus vat.
	 if(ex->VAT_RATE != 1)
	  {
		ex->NEWEXPENSE  = round (ex->INVOICE_AMT/(1+ex->VAT_RATE),2);
		ex->VAT = ex->INVOICE_AMT - ex->NEWEXPENSE;
		if(apbtch.dVatamt() > 0)
		 ex->VAT = ex->VAT + apbtch.dVatamt();
	  }
	 // If we have withholding tax calculate w/h tax
	 if(ex->REBATE_RATE != 1)
	  {
		 ex->REBATE  = round(-1*((ex->NEWAP)*ex->REBATE_RATE),2);
		 if(apbtch.dRebateamt() > 0)
		 ex->REBATE = ex->REBATE + apbtch.dRebateamt();
	  }

	 ex->APAMT = (ex->OLDEXPENSE + ex->NEWEXPENSE + ex->VAT) + round(ex->REBATE,2);

  }
  else
  {

	 ex->NEWAP = apbtch.dPayable() + ex->INVOICE_AMT;

	 if (ex->OLDAP == 0)
	  ex->OLDEXPENSE = 0;
	 else
	  ex->OLDEXPENSE = ex->OLDAP - apbtch.dVatamt() - apbtch.dRebateamt();

	 double dVatRebateTotal = 1;
	 if(dRebatePercentage != 1)
		  dVatRebateTotal += dRebatePercentage;
	 if(dVatPercentage != 1)
		  dVatRebateTotal += dVatPercentage;


	 ex->NEWEXPENSE = round(dGetField(ID_AMOUNT)/dVatRebateTotal,2);

	 if(dVatPercentage != 1)
	  ex->VAT        = round(ex->NEWEXPENSE * dVatPercentage,2) + apbtch.dVatamt();
	 else
	  ex->VAT        = 0;

	 if(dRebatePercentage != 1)
	  ex->REBATE     = round(ex->NEWEXPENSE * dRebatePercentage,2)+ apbtch.dRebateamt();
	 else
	  ex->REBATE     = 0;

	  ex->NEWEXPENSE = ex->NEWAP - (ex->VAT + ex->REBATE + ex->OLDEXPENSE);
	  ex->APAMT      = ex->NEWAP;
	}
}


void TAPInvoiceEntry::Cancel (RTMessage )
{
 if(bTransaction)
 {
  switch(MessageBox(HWindow,GetString(ID_COMMIT),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNOCANCEL |MB_ICONQUESTION))
	{
	 case IDCANCEL: return;
	 case IDNO    : SendMessage(HWindow,WM_COMMAND,CM_ROLLBACK,NULL); break;
	 case IDYES   : break;
	}
 }
 if(VendorList != NULL)
  VendorList->List->SavePosition();
 if(AcctList != NULL)
  AcctList->List->SavePosition();
 if(SlipList != NULL)
  SlipList->List->SavePosition();
 D->SavePosition();
 
 glacct.close();
 gldist.close();
 apdist.close();
 apsetup.close();
 apbtch.close();
 apbtchd.close();
 apvendor.close();
 prods.close();
 gldept.close();
 drcodes.close();
 ShutDownWindow();
 return;
}


void TAPInvoiceEntry::UpdateBatchSupp(long iBatchno,string* szMedRep,string* szProduct,string* szDoctor)
{
  Tap_suppl ap_suppl(cb);
  if(ap_suppl.seek(iBatchno) != 0)
  {
	ap_suppl.appendStart();
	ap_suppl.batchno.assignLong(iBatchno);
	ap_suppl.medreps = szMedRep;
	ap_suppl.product = szProduct;
	ap_suppl.drcodes = szDoctor;
	ap_suppl.append();
  }
  else
  {
	ap_suppl.batchno.assignLong(iBatchno);
	ap_suppl.medreps = szMedRep;
	ap_suppl.product = szProduct;
	ap_suppl.drcodes  = szDoctor;
  }
}

void TAPInvoiceEntry::UpdateBatchHeader(long iBatchno, double APAMT, double VAT, double REBATE)
{
  Date4 Today;
  Today.today();
  if(apbtch.seek(iBatchno) == 0)
	{
	 apbtch.mode.assign((LPSTR)mode.c_str());
	 apbtch.adj.assign("N");
	 /****************************************************************************
	 If the current open periods does not equal the period which the invoice was
	 created in ask the user whether they would like to modify the current
	 period to match the open period or keep the origal period.
	 ****************************************************************************/

	  if(GetPeriod() != (long) dGetField(ID_PERIOD_USED))
		{
		 char szPeriod[10];
		 long iPeriod = (long) dGetField(ID_PERIOD_USED);
		 long iCurrentPd = GetPeriod();

		 string strMsg(GetString(113));
		 ltoa(iPeriod,szPeriod,10);
		 strMsg += szPeriod;
		 strMsg += " to ";
		 ltoa(iCurrentPd,szPeriod,10);
		 strMsg += szPeriod;
		 strMsg += "?";

		 if(MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO) == IDNO)
		  apbtch.period.assignLong(dGetField(ID_PERIOD_USED));
		 else
		  apbtch.period.assignLong(iCurrentPd);
		}
	 else
	  apbtch.period.assignLong(GetPeriod());


	 apbtch.orderno.assign(GetField(ID_CURRBAL));
	 apbtch.vendorno.assign(GetField(ID_VENDORNO));
	 apbtch.vdname.assign(GetField(ID_VENDORNAME));
	 apbtch.invoiceno.assign(StrTrim(GetField(ID_INVOICENO)));
	 apbtch.date.assign(Today.str());
	 apbtch.duedate.assign(DueDate.str());
	 apbtch.invdate.assign(InvoiceDate.str());
	 apbtch.reference.assign(GetField(ID_REFERENCE));
	 apbtch.descript.assign(GetField(ID_JDESCRIPT));
	 apbtch.account.assign(GetField(ID_APCONTROL));
	 apbtch.depart.assign(GetField(ID_APDEPART));
	 apbtch.vatpercent.assignDouble(lGetField(ID_VATPERCENTAGE));
	 apbtch.terms.assignLong(lGetField(ID_TERMS));
	 apbtch.payable.assignDouble(APAMT);
	 apbtch.vatamt.assignDouble(VAT);
	 apbtch.rebateamt.assignDouble(REBATE);
	 apbtch.rebate.assignDouble(dGetField(ID_REBATE));
	 apbtch.UpdateUserTime(szUser);
	 apbtch.flush();
  }
  else
  {
	 apbtch.appendStart();
	 apbtch.batchno.assignLong(iBatchno);
	 apbtch.mode.assign((LPSTR)mode.c_str());
	 apbtch.adj.assign("N");
	 apbtch.period.assignDouble(GetPeriod());
	 apbtch.orderno.assign(GetField(ID_CURRBAL));
	 apbtch.vendorno.assign(GetField(ID_VENDORNO));
	 apbtch.vdname.assign(GetField(ID_VENDORNAME));
	 apbtch.invoiceno.assign(StrTrim(GetField(ID_INVOICENO)));
	 apbtch.duedate.assign(DueDate.str());
	 apbtch.invdate.assign(InvoiceDate.str());
	 apbtch.reference.assign(GetField(ID_REFERENCE));
	 apbtch.descript.assign(GetField(ID_JDESCRIPT));
	 apbtch.account.assign(GetField(ID_APCONTROL));
	 apbtch.depart.assign(GetField(ID_APDEPART));
	 apbtch.vatpercent.assignDouble(dGetField(ID_VATPERCENTAGE));
	 apbtch.terms.assignLong(lGetField(ID_TERMS));
	 apbtch.payable.assignDouble(APAMT);
	 apbtch.vatamt.assignDouble(VAT);
	 apbtch.rebateamt.assignDouble(REBATE);
	 apbtch.rebate.assignDouble(dGetField(ID_REBATE));
	 apbtch.UpdateUserTime(szUser);
	 apbtch.append();

  }

}


void TAPInvoiceEntry::UpdateBatchDetail(long iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry,char *szMedRep,char *szProduct,char *szDoctor)
{


 string szReference;
 if(strcmp(szDescript,"") == 0)
	{
	 char *szSeek = new char[12];
	 strcpy(szSeek,szAccount);
	 glacct.SetIndex("accounts");
	 glacct.seek(strcat(szSeek,szDepart));
	 szReference = glacct.descript.str();
	 delete szSeek;
	 }
  else
  szReference = szDescript;


 if(apbtchd.seek(iBatchno) == 0)
 {
		// If the entry for that Account and Department code has already been entered just update it.
	int iEntryno = 1;
	while(iBatchno == apbtchd.lBatchno())
	{
	 if(strcmp(StrTrim(szAccount),StrTrim(apbtchd.account.str())) == 0
		  && strcmp(StrTrim(szDepart),StrTrim(apbtchd.dept.str())) == 0 )
	  {

		  apbtchd.descript.assign((char *)szReference.c_str());
				 if(dEntry > 0)
				  {
					apbtchd.debit.assignDouble(fabs(dEntry));
					apbtchd.credit.assignDouble(0.00);
				  }
				 else if (dEntry < 0)
				  {
					apbtchd.debit.assignDouble(0.00);
					apbtchd.credit.assignDouble(fabs(dEntry));
				  }

		  apbtchd.flush();

				 return;
	  }
	 apbtchd.skip();
	 iEntryno++;
		  }
	  // if not found create a new line entry.
	  apbtchd.appendStart();
	  apbtchd.batchno.assignLong(iBatchno);
	  apbtchd.descript.assign((char *)szReference.c_str());
	  apbtchd.account.assign(szAccount);
	  apbtchd.dept.assign(szDepart);
	  apbtchd.product.assign(szProduct);
	  apbtchd.depart.assign(szMedRep);
	  apbtchd.drcodes.assign(szDoctor);

	  if(dEntry < 0)
		{
		 apbtchd.debit.assignDouble(0.00);
		 apbtchd.credit.assignDouble(fabs(dEntry));
		}
	  else if (dEntry > 0)
		{
		 apbtchd.debit.assignDouble(fabs(dEntry));
		 apbtchd.credit.assignDouble(0.00);
		}
		apbtchd.append();

  }
 else
 {

	  apbtchd.appendStart();
	  apbtchd.batchno.assignLong(iBatchno);
	  apbtchd.descript.assign((char *)szReference.c_str());
	  apbtchd.account.assign(szAccount);
	  apbtchd.dept.assign(szDepart);
	  apbtchd.product.assign(szProduct);
	  apbtchd.depart.assign(szMedRep);
	  apbtchd.drcodes.assign(szDoctor);

	  if(dEntry < 0)
		{
		 apbtchd.debit.assignDouble(0.00);
		 apbtchd.credit.assignDouble(fabs(dEntry));
		}
	  else if (dEntry > 0)
		{
		 apbtchd.debit.assignDouble(fabs(dEntry));
		 apbtchd.credit.assignDouble(0.00);
		}
	  apbtchd.append();


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
	 glacct.SetIndex("accounts");
	 glacct.seek(strcat(szSeek,(char *)szDepart.c_str()));
	 szReference.assign(glacct.descript.str());
	 delete szSeek;
	 }
 else
	szReference = szDescript;

	apbtchd.appendStart();
	apbtchd.batchno.assignLong(iBatchno);
	apbtchd.descript.assign((char *)szReference.c_str());
	apbtchd.account.assign((char *)szAccount.c_str());
	apbtchd.dept.assign((char *)szDepart.c_str());
	apbtchd.product.assign((char *)szProduct.c_str());
	apbtchd.depart.assign((char *)szMedRep.c_str());
	apbtchd.drcodes.assign((char *)szDoctor.c_str());

	  if(dEntry < 0)
		{
		 apbtchd.debit.assignDouble(0.00);
		 apbtchd.credit.assignDouble(fabs(dEntry));
		}
	  else if (dEntry > 0)
		{
		 apbtchd.debit.assignDouble(fabs(dEntry));
		 apbtchd.credit.assignDouble(0.00);
		}
	  apbtchd.append();
}

/*
char  *TAPInvoiceEntry::GetMedRep()
{
	string szString;
	Str4ten szCode;
	szCode.set(' ');
	szCode.setLen(medreps.repcode.len());
	int Indx = (int)SendDlgItemMsg(ID_MEDREPS,CB_GETCURSEL,NULL,NULL);
	if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(ID_MEDREPS,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szString.c_str());
	 if(medreps.seek(szString))
		 strcpy(szCode.ptr(),medreps.repcode.str());
	}
  return szCode.str();
}
*/

char  *TAPInvoiceEntry::GetProduct()
{
	string szString;
	Str4ten szCode;
	szCode.set(' ');
	szCode.setLen(prods.fPcode.len());
	int Indx = (int)SendDlgItemMsg(ID_PRODUCTS,CB_GETCURSEL,NULL,NULL);
	 if(Indx != CB_ERR)
	 {
	  SendDlgItemMsg(ID_PRODUCTS,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szString.c_str());
	  if(prods.seek(szString))
		 strcpy(szCode.ptr(),prods.fPcode.str());
	}
  return szCode.str();
}

char  *TAPInvoiceEntry::GetDoctor()
{
	 string szString;
	 Str4ten szCode;// = new Str4ten;
	 szCode.set(' ');
	 szCode.setLen(drcodes.drcode.len());
	 int Indx = (int)SendDlgItemMsg(ID_DRCODES,CB_GETCURSEL,NULL,NULL);
	 if(Indx != CB_ERR)
	  {
		SendDlgItemMsg(ID_DRCODES,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szString.c_str());
		drcodes.SetIndex("rpname");
		if(drcodes.seek(szString))
		  strcpy(szCode.ptr(),drcodes.drcode.str());
	  }
	 return szCode.str();
}





void TAPInvoiceEntry::Ok (RTMessage Msg )
{
 apbtch.SetIndexBatchno();
 string szVendorNo = GetField(ID_VENDORNO);
 if(strlen(StrTrim((LPSTR)szVendorNo.c_str())) != 0)
 if(apvendor.seek((LPSTR)szVendorNo.c_str()) !=0)
  {
	MessageBeep(MB_ICONEXCLAMATION);
	/*
	if(MessageBox(HWindow,GetString(6029),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
	 D->Show(SW_HIDE);
	 AddVendor(Msg);
	 VendorList->List->Redraw();
	 D->Show(SW_SHOW);
	 VList(Msg);
	}
	*/
	return;
  }

 if(strlen(StrTrim((LPSTR)szVendorNo.c_str())) == 0)
	{
	 if(VendorList == NULL)
	 {
	  VendorList = new TCustomerList(this,"");
	  VendorList->EnableAutoCreate();
	  GetApplication()->MakeWindow(VendorList);
	 }
	 if(!IsWindowVisible(VendorList->HWindow))
	  VList(Msg);
	 else
	  SetFocus(VendorList->HWindow);
	 return;
	}

 if(apbtch.New(lGetField(ID_BATCHNO)))
 {
  //string invoiceno(GetApplication()->hInstance,8181);
  apbtch.SetIndex("INVOICENO");
  string szInvoiceno(GetField(ID_VENDORNO));
  szInvoiceno += GetField(ID_INVOICENO);
  if(apbtch.seek((LPSTR)szInvoiceno.c_str()) == 0)
  {
	  MessageBox(HWindow,GetString(1039),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	  apbtch.SetIndexBatchno();
	  return;
  }
 }
 apbtch.SetIndexBatchno();



 //szAdj = strcpy(new char[apbtch.adj.len() + 1],"N");
 SendDlgItemMsg(ID_DISTLIST, WM_SETREDRAW, FALSE, 0L);
 iPeriodNo = (int)lGetField(ID_PERIOD);

 if(strlen(StrTrim((LPSTR)szVendorNo.c_str())) == 0)
	{
	 //CheckError(ID_VENDORNO);
	 SetFocus(GetDlgItem(HWindow,ID_VENDORNO));
	 return;
	}

 if(apvendor.seek((LPSTR)szVendorNo.c_str())!=0)
  {
	MessageBeep(MB_ICONEXCLAMATION);
	if(IsWindowVisible(VendorList->HWindow))
	 VendorList->Show(SW_HIDE);
	else
	 VendorList->Show(SW_SHOW);
	SetFocus(VendorList->HWindow);
	return;
  }


 if(strlen(StrTrim(GetField(ID_INVOICENO)))==0)
  {
	//CheckError(ID_INVOICENO);
	SetFocus(GetDlgItem(HWindow,ID_INVOICENO));
	return;
  }

 lInvoiceAmount = dGetField(ID_AMOUNT);
 if(lInvoiceAmount == 0 )
	 {
	  //CheckError(ID_INVOICE);
	  SetFocus(GetDlgItem(HWindow,ID_INVOICE));
	  return;
	 }


 char *szInvoiceDate = strcpy(new char[apbtch.date.len() + 1],GetField(ID_INVDATE));
 if(*szInvoiceDate==0)
  {
	//CheckError(ID_INVDATE);
	SetFocus(GetDlgItem(HWindow,ID_INVDATE));
	return;
  }
 InvoiceDate.assign(szInvoiceDate,GetString(4024));
 delete szInvoiceDate;

 char *szDueDate = strcpy(new char[apbtch.date.len() + 1],GetField(ID_DUEDATE));
 if(*szDueDate==0)
	{
	 //CheckError(ID_DUEDATE);
	 SetFocus(GetDlgItem(HWindow,ID_DUEDATE));
	 return;
	}
 DueDate.assign(szDueDate,GetString(4024));
 delete szDueDate;

 dTerms = dGetField(ID_TERMS);
 long lDueDate = long (InvoiceDate) + dTerms;
 DueDate.assign(lDueDate);

 //szOrderNo = strcpy(new char[apbtch.orderno.len() + 1],GetField(ID_CURRBAL));
 //szDepart = strcpy(new char[apbtch.depart.len() + 1],GetField(ID_DEPART));
 //szAccounts = strcpy(new char[apbtch.account.len() + 1],GetField(ID_ACCOUNTS));
 //LPSTR szSeek = new char[apbtch.account.len() + apbtch.depart.len() + 1];
 //strcpy(szSeek,szAccounts);
 //strcat(szSeek,szDepart);
 string Acct(GetField(ID_ACCOUNTS));
 string Dept(GetField(ID_DEPART));
 if(!ValidateGL((LPSTR)Acct.c_str(),(LPSTR)Dept.c_str()))
  {
	Accountlist();
	MessageBeep(MB_ICONEXCLAMATION);
	return;
  }


 if(strlen(StrTrim(GetField(ID_JDESCRIPT))) == 0)
 {
  //CheckError(ID_JDESCRIPT);
  SetFocus(GetDlgItem(HWindow,ID_JDESCRIPT));
  return;
 }

 if(strlen(StrTrim(GetField(ID_REFERENCE)))==0) {
	//CheckError(ID_REFERENCE);
	SetFocus(GetDlgItem(HWindow,ID_REFERENCE));
	return;
 }

 iLineno = lGetField(ID_LINENO);

 dVatPercentage = dGetField(ID_VATPERCENTAGE);

 if(dVatPercentage != 0 && strlen(StrTrim(GetField(ID_VATACCT))) == 0)
  {
	 MessageBox(HWindow,GetString(8157),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	 SetFocus(GetDlgItem(HWindow,ID_VATACCT));
	 return;
  }

 string VatAccount(GetField(ID_VATACCT));
 string VatDept(GetField(ID_VATDEPT));
 if(!ValidateGL ( (LPSTR)VatAccount.c_str() , (LPSTR)VatDept.c_str() ))
  {
	MessageBox(HWindow,GetString(5124),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	SetFocus(GetDlgItem(HWindow,ID_VATACCT));
	return;
  }

 string APAcct(GetField(ID_APCONTROL));
 string APDept(GetField(ID_APDEPART));
 if(!ValidateGL((LPSTR)APAcct.c_str(),(LPSTR)APDept.c_str()))
  {
	MessageBox(HWindow,GetString(5124),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	SetFocus(GetDlgItem(HWindow,ID_APCONTROL));
	return;
  }

  dRebatePercentage = dGetField(ID_REBATE);

  // Must do a check here for the last batch number.
  // Lock the database and get the last batch no.
  // When the function returns unlock the database.

  iBatchno = (long)lGetField(ID_BATCHNO);

  //apbtch.SetIndex(GetString(8140));
  apbtch.SetIndexBatchno();



  SetFocus(HWindow);
  if(bModify)
	{
	 ModifyCurrentSlip();
	}
  else
	{
	  bSlipList = TRUE;
	  //apbtch->bottom();
	  //iBatchno = (int)apbtch->lBatchno();
	  //iBatchno++;
	 //if(SlipList == NULL)
	 //{
	 //	CHourGlass wait;
	 //	SlipList  = new TBatchList(this,"");
	 //	SlipList->EnableAutoCreate();
	 //	GetApplication()->MakeWindow(SlipList);
	 // }

	  if(bNEW)
	  {
		apbtch.bottom();
		iBatchno = (long)apbtch.lBatchno();
		iBatchno++;
	  }
	 CreateAPBatchEntry(iBatchno);
	 bTransaction = TRUE;
	 bModify = FALSE;
	}
  //SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
  CurrentBatch(Msg);
  return;
}




