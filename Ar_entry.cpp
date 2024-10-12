#include "std_inc.h"

#include "tbowl.h"
#include "d4all.hpp"
#include "glclass.h"
#include "ar.h"

#include "db_arbtc.h"
#include "db_itemm.h"
#include "db_medrp.h"

#include "db_cash.h"

#include "ar_custl.h"
#include "ar_btchl.h"
#include "ar_btch.h"
#include "math.h"


#define ID_INVOICE 1030
#define ID_PROD        1003
#define ID_VIN         1004
#define ID_UNITS       1005
#define ID_ORDERDATE   1006
#define ID_VATACCT     1034
#define ID_VATDEPT     1035
#define ID_PERIOD_USED 1073



void TARInvoiceEntry::RecoverPostedInvoice()
{
 long dBatchNo;
 IntGetValue(GetDlgItem(HWindow,ID_BATCHNO),(LPLONG)&dBatchNo);
 if(arbtch.seek(dBatchNo) == 0)
 {
	  arbtch.mode.assign("1");
	  arbtch.UpdateUserTime(szUser);
	  arbtch.flush();

 }

 gldist.SetIndex("apdist");
 if(bDeleted)
 {
  if(gldist.seek(dBatchNo) == 0 )
  {
	while (double (Str4ptr(gldist.batchno.str())) == dBatchNo)
	 {
		 gldist.deleteRec();
		 gldist.skip();
	 }
	gldist.pack();
  }
 if(ardist.seek(dBatchNo) == 0 )
  {
	while (double (Str4ptr(ardist.batchno.str())) == dBatchNo)
	 {
	  ardist.deleteRec();
	  ardist.skip();
	 }
	ardist.pack();
  }
 }
}




void TARInvoiceEntry::CreateAPBatchEntry(long iBatchno)
{
	arbtch.SetIndex(GetString(8140));
	arbtchd.SetIndex(GetString(8140));

	if(arbtch.seek(iBatchno)== 0 && !arbtch.Open()){
	 RecoverPostedInvoice();
	 return;
	 }

	double dVatPercentage = 1;

	if(dGetField(ID_VATPERCENTAGE) > 0)
		  //dVatPercentage = (100+dGetField(ID_VATPERCENTAGE))/100;
		  // If there is no VAT with the price remove the  -1 .
		  //dVatPercentage = (100+dGetField(ID_VATPERCENTAGE))/100;
		  dVatPercentage = (100+dGetField(ID_VATPERCENTAGE))/100;
	else
		dVatPercentage = 1;

	//double AR = 0;
	double SALES = 0;
	double VAT = 0;

	if(dGetField(1038) == 0 && dGetField(1030) != 0)
	 SALES = dGetField(1030);
	else
	 SALES     = dGetField(1037)*dGetField(1038);  //AR Total
	if(SALES == 0)
	 return;

	/*
	  Make a check in the GL.INI file to set whether the VAT is included or excluded from the
	  selling price. Depending upon the setup calculate the VAT amount from the sales total.
	*/

	//double SALES  = round(AR/dVatPercentage,2);
	//double VAT    = AR - SALES;

	double AR1  = round(SALES * dVatPercentage,2);
	VAT = (AR1 - SALES);

	//Header

	UpdateBatchHeader(iBatchno,AR1,VAT,VAT);
	// UpdateBatchHeader(iBatchno,AR,VAT,VAT);
	// Use the above line for prices including VAT.
	//Sales Amounts

	char szDescript[255];
	if(strcmp(StrTrim(GetField(ID_PRODUCTS)),"") == 0)
	  strcpy(szDescript,"");
	else
	  strcpy(szDescript,GetField(ID_PRODUCTS));

	LPLINEITEM* Item = new LPLINEITEM;
	Item->iBatchno   = iBatchno;
	Item->iEntryno   = (int)dGetField(3033);
	Item->szAccount  = GetField(ID_AR_ACCT);
	Item->szDept     = GetField(ID_AR_DEPT);
	Item->szDescript = GetSalesDescription();
	Item->szMedrep   = GetDept();
	Item->szProduct  = GetField(ID_PROD);
	Item->szDoctor   = GetField(1008);
	Item->dEntry     = -1*SALES;
	Item->dQty       = dGetField(1037);
	Item->dPrice     = dGetField(1038);

	/* Sales Account Line */
	UpdateBatchDetail(Item);

	//VAT if any.

	if(dVatPercentage != 1)
	 {
	  Item->szDept    = GetField(ID_VATDEPT);
	  Item->szAccount = GetField(ID_VATACCT);
	  Item->iBatchno   = iBatchno;
	  Item->iEntryno   = 1;
	  Item->szDescript = "";
	  Item->szMedrep   = GetDept();
	  Item->szProduct  = "";
	  Item->szDoctor   = "";
	  Item->dEntry     = -1*VAT;
	  Item->dQty       = 0;
	  Item->dPrice     = 0;

	  UpdateBatchDetail(Item);
	 }
	//AR Total
	  Item->szDept    = GetField(ID_APDEPART);
	  Item->szAccount = GetField(ID_APCONTROL);
	  Item->iBatchno   = iBatchno;
	  Item->iEntryno   = 1;
	  Item->szDescript = "";
	  Item->szMedrep   = GetDept();
	  Item->szProduct  = GetField(ID_PROD);
	  Item->szDoctor   = GetField(1008);
	  Item->dEntry     = AR1;
	  Item->dQty       = 0;
	  Item->dPrice     = 0;
	  UpdateBatchDetail(Item);

	  string szMedRep(GetField(ID_REPS));
	  string szProduct(GetField(ID_PROD));
	  string szDoctor(GetField(1008));
	  //UpdateBatchSupp(iBatchno,&szMedRep,&szProduct,&szDoctor);
	  delete Item;
	  return;
}

void TARInvoiceEntry::LoadDoctors()
{
 SendDlgItemMsg(ID_DRCODES,CB_RESETCONTENT,NULL,NULL);
 drcodes.SetIndex("drcode");
  for(drcodes.top();!drcodes.eof();drcodes.skip())
	 SendDlgItemMsg(ID_DRCODES,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)drcodes.rpname.str());
}

void TARInvoiceEntry::UpdateBatchSupp(long iBatchno,string* szMedRep,string* szProduct,string* szDoctor)
{
  Tar_suppl ar_suppl(cb);
  if(ar_suppl.seek(iBatchno) != 0)
  {
	ar_suppl.appendStart();
	ar_suppl.batchno.assignLong(iBatchno);
	ar_suppl.medreps = szMedRep;
	ar_suppl.product = szProduct;
	ar_suppl.drcodes = szDoctor;
	ar_suppl.append();
  }
  else
  {
	ar_suppl.batchno.assignLong(iBatchno);
	ar_suppl.medreps = szMedRep;
	ar_suppl.product = szProduct;
	ar_suppl.drcodes  = szDoctor;
  }
}



#pragma argsused
void TARInvoiceEntry::UpdateBatchHeader(long iBatchno, double AMOUNT, double VAT, double REBATE)
{
  Date4 Today;
  Today.today();
  BOOL bAppend = TRUE;

  if(arbtch.seek(iBatchno) == 0)
	 bAppend = FALSE;

  if(bAppend)
	 arbtch.appendStart(0);
	 arbtch.batchno.assignLong(iBatchno);
	 arbtch.mode.assign((LPSTR)mode.c_str());
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
		  arbtch.period.assignLong(dGetField(ID_PERIOD_USED));
		 else
		  arbtch.period.assignLong(iCurrentPd);
		}
	 else
	  arbtch.period.assignDouble(GetPeriod());


	 arbtch.orderno.assign(GetField(ID_ORDERNO));
	 arbtch.vendorno.assign(GetField(ID_VENDORNO));
	 arbtch.invoiceno.assign(GetField(ID_INVOICENO));

	 arbtch.date.assign(Today.str());
	 arbtch.duedate.assign(DueDate.str());
	 arbtch.invdate.assign(InvoiceDate.str());
	 arbtch.reference.assign(GetField(ID_REFERENCE));
	 arbtch.descript.assign(GetField(ID_JDESCRIPT));
	 arbtch.account.assign(GetField(ID_APCONTROL));
	 arbtch.depart.assign(GetField(ID_APDEPART));
	 arbtch.sales_acct.assign(GetField(ID_AR_ACCT));
	 arbtch.sales_dept.assign(GetField(ID_AR_DEPT));
	 arbtch.vatpercent.assignDouble(dGetField(ID_VATPERCENTAGE));
	 arbtch.terms.assignLong(lGetField(ID_TERMS));
	 arbtch.payable.assignDouble(AMOUNT);
	 arbtch.vatamt.assignDouble(VAT);
	 arbtch.vin.assign(GetField(ID_VIN));
	 arbtch.orderdate.assign(GetDate(ID_ORDERDATE));
	 arbtch.units.assign(GetField(ID_UNITS));
	 arbtch.UpdateUserTime(szUser);

  if(bAppend)
	 arbtch.append();
  else
	 arbtch.flush();

}



void TARInvoiceEntry::UpdateBatchDetail(LPLINEITEM* lineItem)
{
 string szSeek(lineItem->szAccount);
 szSeek += lineItem->szDept;
 glacct.SetIndex("accounts");
 glacct.seek((char *)szSeek.c_str());

 if(strcmp(StrTrim((char *)lineItem->szDescript.c_str()),"") == 0){
	 lineItem->szDescript = glacct.descript.str();
 }

 if(arbtchd.seek(lineItem->iBatchno) == 0)
  {
	while (lineItem->iBatchno == arbtchd.lBatchno())
	 {
	  if(strcmp(StrTrim((char *)lineItem->szAccount.c_str()),StrTrim(arbtchd.account.str())) == 0
		  && strcmp(StrTrim((char *)lineItem->szDept.c_str()),StrTrim(arbtchd.dept.str())) == 0)
		{
		 if(lineItem->dEntry < 0)
			{
			 arbtchd.debit.assignDouble(0.00);
			 arbtchd.credit.assignDouble(fabs(lineItem->dEntry));
			}
		 else if (lineItem->dEntry > 0)
		  {
			arbtchd.debit.assignDouble(fabs(lineItem->dEntry));
			arbtchd.credit.assignDouble(0.00);
		  }
		 // The line has been updated so return.
		 return;
		}
		arbtchd.skip();
	 }
  }

 // if not found create a new line entry.
 arbtchd.appendStart();
 AddLine(lineItem);
 arbtchd.append();

 return;
}


void TARInvoiceEntry::AddLine(LPLINEITEM *lineItem)
{
 if(lineItem->iEntryno != 0)
 arbtchd.entryno.assignLong(lineItem->iEntryno);
 arbtchd.batchno.assignLong(lineItem->iBatchno);
 if(strcmp(StrTrim((char *)lineItem->szDescript.c_str()),"") != 0)
 arbtchd.descript.assign((char *)lineItem->szDescript.c_str());
 arbtchd.account.assign((char *)lineItem->szAccount.c_str());
 arbtchd.dept.assign((char *)lineItem->szDept.c_str());
 arbtchd.depart.assign((char *)lineItem->szMedrep.c_str());
 arbtchd.product.assign((char *)lineItem->szProduct.c_str());
 arbtchd.drcode.assign((char *)lineItem->szDoctor.c_str());
 arbtchd.qty.assignDouble(lineItem->dQty);
 arbtchd.price.assignDouble(lineItem->dPrice);
 if(lineItem->dEntry < 0)
	{
	 arbtchd.debit.assignDouble(0.00);
	 arbtchd.credit.assignDouble(fabs(lineItem->dEntry));
	}
 else if (lineItem->dEntry > 0)
	{
	arbtchd.debit.assignDouble(fabs(lineItem->dEntry));
	arbtchd.credit.assignDouble(0.00);
	}
}

void TARInvoiceEntry::AddBatchDetail(LPLINEITEM* lineItem)
{
 // If the entry for that Account and Department code has already been entered just update it.
  arbtchd.appendStart();
  // Line Item (int)dGetField(3033)
  AddLine(lineItem);
  arbtchd.append();


 arbtchd.SetIndex(GetString(8140));
 return;
}
#pragma argsused
void TARInvoiceEntry::UpdateAR(long iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry)
{
 return;
}


#pragma argsused
void TARInvoiceEntry::UpdateVAT(long iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry)
{
 return;
}

void TARInvoiceEntry::ChangeDept(RTMessage Msg)
{
  char szDept[11];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  int Indx = (int)SendDlgItemMsg(ID_DEPTCODES,CB_GETCURSEL,0,0);

   //
	if(Indx != CB_ERR){
	  SendDlgItemMsg(ID_DEPTCODES,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szDept);

			if(gldept.seek(szDept) == 0){
			SetField(3038,gldept.descript.str());
			}
	}

 }

}

void TARInvoiceEntry::ModifyCurrentSlip(long iBatchno)
{
	int iLine;
	IntGetDlgItemValue(HWindow,3033,(LPLONG)&iLine);

	FloatGetDlgItemValue(D->HWindow,ID_DEBITTOTAL,&DebitTotal);
	FloatGetDlgItemValue(D->HWindow,ID_CREDITTOTAL,&CreditTotal);
	//arbtch.batchno.assignLong(iBatchno);
	arbtch.mode.assign((LPSTR)mode.c_str());
	arbtch.date.assign(InvoiceDate.str());
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
		  arbtch.period.assignLong(dGetField(ID_PERIOD_USED));
		 else
		  arbtch.period.assignLong(iCurrentPd);
		}
	 else
	  arbtch.period.assignDouble(GetPeriod());

	arbtch.orderno.assign(GetField(ID_ORDERNO));
	//arbtch.vendorno.assign(GetField(ID_VENDORNO));
	arbtch.invoiceno.assign(GetField(ID_INVOICENO));
	arbtch.duedate.assign(DueDate.str());
	arbtch.invdate.assign(InvoiceDate.str());
	arbtch.reference.assign(GetField(ID_REFERENCE));
	arbtch.descript.assign(GetSalesDescription());
	arbtch.account.assign(GetField(ID_APCONTROL));
	arbtch.depart.assign(GetField(ID_APDEPART));
	arbtch.sales_acct.assign(GetField(ID_AR_ACCT));
	arbtch.sales_dept.assign(GetField(ID_AR_DEPT));
	arbtch.entryno.assignLong(lGetField(ID_LINENO));
	arbtch.terms.assignLong(lGetField(ID_TERMS));
	arbtch.UpdateUserTime(szUser);
	arbtch.flush();


	double dVatPercentage = 1;

	if(!bModify)   // Then append a new line.
	 {
		//Add a new line item.

		double dCurrentPayable = arbtch.dPayable();
		double dCurrentVat     = arbtch.dVatamt();
		//double dCurrentSales   = dCurrentPayable - dCurrentVat;

		double dVatPercent = dGetField(ID_VATPERCENTAGE);
		if((int)dVatPercent > (int)0)
			  dVatPercentage = (100+dVatPercent)/100;
		else
		  dVatPercentage = 1;

		/*
		double AR_AMT = 0;
		if(dGetField(1038) == 0 && dGetField(1030) != 0)
		 AR_AMT = dGetField(1030);
		else
		 AR_AMT  = dGetField(1037)*dGetField(1038);  //AR Total
		*/

		double SALES = 0;
		double AR1    = 0;
		double VAT   = 0;

		if(dGetField(1038) == 0 && dGetField(1030) != 0)
			 SALES = dGetField(1030);
		else
			 SALES     = dGetField(1037)*dGetField(1038);  //AR Total
		if(SALES == 0)
			 return;

		AR1  = round(SALES * dVatPercentage,2);
		VAT = (AR1 - SALES);


	  double new_VAT = dCurrentVat + VAT;
	  double new_AR  = dCurrentPayable + AR1;
	  //double new_SALES = dCurrentSales + SALES;

	  arbtch.payable.assignDouble(new_AR);
	  arbtch.vatamt.assignDouble(new_VAT);
	  arbtch.UpdateUserTime(szUser);
	  arbtch.flush();


	  LPLINEITEM* Item = new LPLINEITEM;
	  Item->iBatchno   = iBatchno;
	  Item->iEntryno   = (int)dGetField(3033);
	  Item->szAccount  = GetField(ID_AR_ACCT);
	  Item->szDept     = GetField(ID_AR_DEPT);
	  Item->szDescript = GetSalesDescription();
	  //Item->szMedrep   = GetField(ID_REPS);
	  Item->szMedrep   = GetDept();
	  Item->szProduct  = GetField(ID_PROD);
	  Item->szDoctor   = GetField(1008);
	  Item->dEntry     = -1*SALES;
	  Item->dQty       = dGetField(1037);
	  Item->dPrice     = dGetField(1038);

	  AddBatchDetail(Item);

	  if(dVatPercentage != 1)
		{
		 Item->szDept    = GetField(ID_VATDEPT);
		 Item->szAccount = GetField(ID_VATACCT);
		 Item->iBatchno   = iBatchno;
		 Item->iEntryno   = 1;
		 Item->szDescript = "";
		 Item->szMedrep   = GetDept();
		 Item->szProduct  = "";
		 Item->szDoctor   = "";
		 Item->dEntry     = -1*new_VAT;
		 Item->dQty       = dGetField(1037);
		 Item->dPrice     = dGetField(1038);
		 UpdateBatchDetail(Item);
		}

	  Item->szDept    = GetField(ID_APDEPART);
	  Item->szAccount = GetField(ID_APCONTROL);
	  Item->iBatchno   = iBatchno;
	  Item->iEntryno   = 1;
	  Item->szDescript = "";
	  Item->szMedrep   = GetDept();
	  Item->szProduct  = "";
	  Item->szDoctor   = "";
	  Item->dEntry     = new_AR;
	  Item->dQty       = 0;
	  Item->dPrice     = 0;

	  UpdateBatchDetail(Item);
	  bTransaction = TRUE;
	 delete Item;
	 return;
	 }
	 else
	 {
		lCredit = dGetField(ID_CREDIT);
		lDebit  = dGetField(ID_DEBIT);

		if((lCredit == 0 && lDebit == 0) || (lCredit > 0 && lDebit > 0))
		 {
		 bModify = FALSE;
		 MessageBox(HWindow,GetString(4113),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
		 return;
		 }
	  // Modify an existing record.
	  if(arbtchd.iCurrentRecord != arbtchd.recNo())
	  {
		bModify = FALSE;
		return;
	  }

	 string VatAcct(GetField(ID_VATACCT));
	 string Account(GetField(ID_ACCOUNTS));

	 if(VatAcct == Account)
		{
		 if(lDebit > 0)
		  arbtch.vatamt.assignDouble(-lDebit);
		 else
		 if(lCredit > 0)
		  arbtch.vatamt.assignDouble(lCredit);
		}

	  string szARAccount(GetField(ID_APCONTROL));
	  if(Account == szARAccount)
		{
		  if(lDebit > 0)
		  arbtch.payable.assignDouble(dGetField(1999));
		  else
		  if(lCredit > 0)
  		  arbtch.payable.assignDouble(dGetField(1999));			
		}

	  arbtch.descript.assign(GetField(ID_JDESCRIPT));
	  arbtch.UpdateUserTime(szUser);
	  arbtch.flush();

	  arbtchd.batchno.assignLong(iBatchno);
	  arbtchd.entryno.assignLong(iLine);
	  arbtchd.account.assign(GetField(ID_ACCOUNTS));
	  arbtchd.dept.assign(GetField(ID_DEPART));
	  string SaleAcct(GetField(ID_AR_ACCT));
	  if(Account == SaleAcct)
		arbtchd.descript.assign(GetSalesDescription());
	  else
		arbtchd.descript.assign(GetDesc(ID_ACCOUNTS));
	  arbtchd.drcode.assign(GetField(1008));
	  arbtchd.product.assign(GetField(ID_PROD));
	  arbtchd.depart.assign(GetDept());
	  arbtchd.debit.assignDouble(dGetField(ID_DEBIT));
	  arbtchd.credit.assignDouble(dGetField(ID_CREDIT));
	  arbtchd.flush();

	 }
	bModify = FALSE;
	return;
}


