#include "ap.h"
//#include "db_dept.h"
#include "math.h"
#include "db_apbtc.h"
#include "db_cash.h"
#include "db_itemm.h"
#include "db_medrp.h"
#include "ap_class.h"
#include "ap_btch.h"
#include "ap_cashb.h"
#include "utildlg.h"

#define ID_INVOICE 1030
#define  ID_VATACCT 1034
#define  ID_VATDEPT 1035


extern Date4 InvoiceDate;
extern Date4 DueDate;

#pragma argsused
void TAPCashEntry::CreateNewApSlip(PTapcash apcash,PTapcashd apcashd,PTAPCashDist D)
{   


   int iLineno = 1;
   // Calculate the VAT amount
   double EXPENSE = fabs(lInvoiceAmount);

	// Add line for expense amount
	apcashd->appendStart();
	apcashd->batchno->assignLong(iBatchno);
	apcashd->entryno->assignLong(iLineno);
	apcashd->descript->assign(GetField(ID_JDESCRIPT));
	apcashd->account->assign(szAccounts);  // AP account is always set to the default.s
	apcashd->product->assign(GetProduct());
	apcashd->medreps->assign(GetMedRep());
	apcashd->drcodes->assign(GetDoctor());

	apcashd->dept->assign(szDepart);        // AP department code is set to the default dept code.
	if(lInvoiceAmount > 0)
	{
	 apcashd->debit->assignDouble(EXPENSE);
	 apcashd->credit->assignDouble(0.00);
	}
	else if (lInvoiceAmount < 0)
	{
	 apcashd->debit->assignDouble(0.00);
	 apcashd->credit->assignDouble(EXPENSE);
	}
	apcashd->append();


	iLineno++;

	apcashd->appendStart();
	apcashd->batchno->assignLong(iBatchno);
	apcashd->entryno->assignLong(iLineno);
	char szDept[5];
	strcpy(szDept,GetField(ID_APDEPART));
	apcashd->descript->assign(GetControlAcct(GetField(ID_APCONTROL),szDept));
	apcashd->account->assign(GetField(ID_APCONTROL));
	apcashd->dept->assign(GetField(ID_APDEPART));
	apcashd->product->assign(GetProduct());
	apcashd->medreps->assign(GetMedRep());
	apcashd->drcodes->assign(GetDoctor());

	if(lInvoiceAmount > 0)
   {
   apcashd->debit->assignDouble(0.00);
   apcashd->credit->assignDouble(EXPENSE);
   }
   else if (lInvoiceAmount < 0)
   {
   apcashd->debit->assignDouble(EXPENSE);
   apcashd->credit->assignDouble(0.00);
   }
   apcashd->append();


   apcash->appendStart();
   apcash->batchno->assignLong(iBatchno);
   apcash->mode->assign("1");
   apcash->adj->assign("N");
   apcash->date->assign(InvoiceDate.str());
   apcash->period->assignDouble(GetPeriod(HWindow));
   apcash->chequeno->assign(GetField(ID_CHEQUE));
   apcash->orderno->assign(GetField(ID_CURRBAL));
	apcash->vendorno->assign(GetField(ID_VENDORNO));
   apcash->vdname->assign(GetField(ID_VENDORNAME));
	apcash->invoiceno->assign(GetField(ID_INVOICENO));
   apcash->duedate->assign(DueDate.str());
   apcash->invdate->assign(InvoiceDate.str());
   apcash->reference->assign(GetField(ID_REFERENCE));
	apcash->descript->assign(GetField(ID_JDESCRIPT));
   apcash->account->assign(GetField(ID_APCONTROL));
   apcash->depart->assign(GetField(ID_APDEPART));
   apcash->vatacct->assign(GetField(ID_VATACCT));
   apcash->vatdept->assign(GetField(ID_VATDEPT));
   apcash->payable->assignDouble(dGetField(ID_INVOICE));
	apcash->entryno->assignLong(lGetField(ID_LINENO));
	apcash->UpdateUserTime(szUser);
   apcash->append();



	bSlipList = TRUE;
   SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
   return;
}



#pragma argsused
void TAPCashEntry::CreateVatSlip(PTapcash apcash,PTapcashd apcashd,PTAPCashDist D)
{   


   double dPercent = dVatPercentage;    
    dVatPercentage = (dVatPercentage + 100)/100;
   // Calculate the VAT amount
   double APAMT   = fabs(lInvoiceAmount);
   double EXPENSE = APAMT/dVatPercentage;
   double VAT     = APAMT-EXPENSE;
   


   int iLineno = 1;


   
	// Add line for expense amount
	apcashd->appendStart();
	apcashd->batchno->assignLong(iBatchno);
	apcashd->entryno->assignLong(iLineno);
	apcashd->descript->assign(GetField(ID_JDESCRIPT));
	apcashd->account->assign(GetField(ID_ACCOUNTS));  // APAMT account is always set to the default.s
	apcashd->dept->assign(GetField(ID_DEPART));        // APAMT department code is set to the default dept code.
	apcashd->product->assign(GetProduct());
	apcashd->medreps->assign(GetMedRep());
	apcashd->drcodes->assign(GetDoctor());
	if(lInvoiceAmount > 0)
	{
	 apcashd->debit->assignDouble(EXPENSE);
	 apcashd->credit->assignDouble(0.00);
	}
	else if (lInvoiceAmount < 0)
	{
	 apcashd->debit->assignDouble(0.00);
	 apcashd->credit->assignDouble(EXPENSE);
	}

	apcashd->append();




	// Add line for VAT amount.

	apcashd->appendStart();
	apcashd->batchno->assignLong(iBatchno);
	apcashd->entryno->assignLong(iLineno);
	char szDept[5];
	strcpy(szDept,GetField(ID_VATDEPT));
	apcashd->descript->assign(GetControlAcct(GetField(ID_VATACCT),szDept));
	apcashd->account->assign(GetField(ID_VATACCT));  // APAMT account is always set to the default.s
	apcashd->dept->assign(GetField(ID_VATDEPT));        // APAMT department code is set to the default dept code.
	apcashd->product->assign(GetProduct());
	apcashd->medreps->assign(GetMedRep());
	apcashd->drcodes->assign(GetDoctor());
	if(lInvoiceAmount > 0)
	{
	apcashd->debit->assignDouble(VAT);
	apcashd->credit->assignDouble(0.00);    //VAT credit is set to zero.
	}
	else if(lInvoiceAmount < 0)
	{
	apcashd->debit->assignDouble(0.00);
	apcashd->credit->assignDouble(VAT);    //VAT credit is set to zero.
	}
	apcashd->append();


	apcashd->appendStart();
	apcashd->batchno->assignLong(iBatchno);
	apcashd->entryno->assignLong(iLineno);
	strcpy(szDept,GetField(ID_APDEPART));
	apcashd->descript->assign(GetControlAcct(GetField(ID_APCONTROL),szDept));
	apcashd->account->assign(GetField(ID_APCONTROL));  // APAMT account is always set to the default.
	apcashd->dept->assign(GetField(ID_APDEPART));           // APAMT department code is set to the default dept code.
	apcashd->product->assign(GetProduct());
	apcashd->medreps->assign(GetMedRep());
	apcashd->drcodes->assign(GetDoctor());

	if(lInvoiceAmount > 0)
	{
	 apcashd->debit->assignDouble(0.00);
	 apcashd->credit->assignDouble(APAMT);
   }
   else if (lInvoiceAmount < 0)
   {
    apcashd->debit->assignDouble(APAMT);
    apcashd->credit->assignDouble(0.00);
   }
	apcashd->append();




	apcash->appendStart();                                   //1 118
   apcash->batchno->assignLong(iBatchno);      
   apcash->mode->assign("1");
   apcash->adj->assign("N");
   apcash->date->assign(InvoiceDate.str());
   apcash->period->assignDouble(GetPeriod(HWindow));
   apcash->chequeno->assign(GetField(ID_CHEQUE));
   apcash->orderno->assign(GetField(ID_CURRBAL));
   apcash->vendorno->assign(GetField(ID_VENDORNO));
   apcash->vdname->assign(GetField(ID_VENDORNAME));
   apcash->invoiceno->assign(GetField(ID_INVOICENO));
   apcash->duedate->assign(DueDate.str());
   apcash->invdate->assign(InvoiceDate.str());
   apcash->reference->assign(GetField(ID_REFERENCE));
   apcash->descript->assign(GetField(ID_JDESCRIPT));
   apcash->vatpercent->assignDouble(dPercent);
   apcash->rebate->assignDouble(dRebatePercentage);
   apcash->account->assign(GetField(ID_APCONTROL));
   apcash->depart->assign(GetField(ID_APDEPART));
   apcash->vatacct->assign(GetField(ID_VATACCT));
   apcash->vatdept->assign(GetField(ID_VATDEPT));

   apcash->payable->assignDouble(dGetField(ID_INVOICE));
	apcash->entryno->assignLong(lGetField(ID_LINENO));
	apcash->UpdateUserTime(szUser);
	apcash->append();


   SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
   bSlipList = TRUE;
   return;
}


#pragma argsused
void TAPCashEntry::CreateRebateSlip(PTapcash apcash,PTapcashd apcashd,PTAPCashDist D)
{  

   char szSeekAccount[12];
   char szAccount[8];
   char szDepart[5];

   // Check to see if the vendor is an individual or a company.
   if(SendDlgItemMsg(ID_INDIVIDUAL,BM_GETSTATE,NULL,NULL))
   {
    strcpy(szAccount,szIndTaxAccount);
    strcpy(szDepart,szIndTaxDept);
   }
   else
   if(SendDlgItemMsg(ID_COMPANY,BM_GETSTATE,NULL,NULL))
   {
    strcpy(szAccount,szCompanyTaxAccount);
    strcpy(szDepart,szCompanyTaxDept);
   } 



   double dPercent = 0;
   double EXPENSE  = fabs(lInvoiceAmount);
   double REBATE   = EXPENSE*(dRebatePercentage/100);
   double APAMT       = EXPENSE - REBATE;
   

   int iEntryNo  = 1;
   // Add line for expense amount
	apcashd->appendStart();
   apcashd->batchno->assignLong(iBatchno);
   apcashd->entryno->assignLong(iLineno);
   apcashd->descript->assign(GetField(ID_JDESCRIPT));
   apcashd->account->assign(GetField(ID_ACCOUNTS));  // APAMT account is always set to the default.s
	apcashd->dept->assign(GetField(ID_DEPART));        // APAMT department code is set to the default dept code.
	apcashd->product->assign(GetProduct());
	apcashd->medreps->assign(GetMedRep());
	apcashd->drcodes->assign(GetDoctor());

	if(lInvoiceAmount > 0)
	{
    apcashd->debit->assignDouble(EXPENSE);
    apcashd->credit->assignDouble(0.00);
   }
   else if (lInvoiceAmount < 0)
   {
    apcashd->debit->assignDouble(0.00);
    apcashd->credit->assignDouble(EXPENSE);
   }

	apcashd->append();



   // Add Cash amount.
   iEntryNo++;   
	apcashd->appendStart();
   apcashd->batchno->assignLong(iBatchno);
   apcashd->entryno->assignLong(iEntryNo);
   strcpy(szSeekAccount,GetField(ID_APCONTROL));
   strcat(szSeekAccount,GetField(ID_APDEPART));
   glacct->seek(szSeekAccount);
   apcashd->descript->assign(glacct->descript->str());
   apcashd->account->assign(GetField(ID_APCONTROL));  // APAMT account is always set to the default.
	apcashd->dept->assign(GetField(ID_APDEPART));           // APAMT department code is set to the default dept code.
	apcashd->product->assign(GetProduct());
	apcashd->medreps->assign(GetMedRep());
	apcashd->drcodes->assign(GetDoctor());

	if(lInvoiceAmount > 0)
	{
   apcashd->debit->assignDouble(0.00);
   apcashd->credit->assignDouble(APAMT);
   }
   else if(lInvoiceAmount > 0)
   {
   apcashd->debit->assignDouble(APAMT);
   apcashd->credit->assignDouble(0.00);
   }

	apcashd->append();



   // Add taxes payable line.
   iEntryNo++;
	apcashd->appendStart();
   apcashd->batchno->assignLong(iBatchno);
   apcashd->entryno->assignLong(iEntryNo);
   strcpy(szSeekAccount,szAccount);
   strcat(szSeekAccount,szAccount);
   glacct->seek(szSeekAccount);
   apcashd->descript->assign(glacct->descript->str());
   apcashd->account->assign(szAccount);  // APAMT account is always set to the default.s
	apcashd->dept->assign(szDepart);        // APAMT department code is set to the default dept code.
	apcashd->product->assign(GetProduct());
	apcashd->medreps->assign(GetMedRep());
	apcashd->drcodes->assign(GetDoctor());


	if(lInvoiceAmount > 0)
	{
	apcashd->debit->assignDouble(0.00);
	apcashd->credit->assignDouble(REBATE);    //VAT credit is set to zero.
	}
	else if (lInvoiceAmount < 0)
	{
	apcashd->debit->assignDouble(REBATE);
	apcashd->credit->assignDouble(0.00);    //VAT credit is set to zero.
	}

	apcashd->append();


	apcash->appendStart();                                   //1 118
	apcash->batchno->assignLong(iBatchno);
	apcash->mode->assign("1");
	apcash->adj->assign("N");
	apcash->date->assign(InvoiceDate.str());
	apcash->period->assignDouble(GetPeriod(HWindow));
	apcash->chequeno->assign(GetField(ID_CHEQUE));
	apcash->orderno->assign(GetField(ID_CURRBAL));
	apcash->vendorno->assign(GetField(ID_VENDORNO));
	apcash->vdname->assign(GetField(ID_VENDORNAME));
	apcash->invoiceno->assign(GetField(ID_INVOICENO));
	apcash->duedate->assign(DueDate.str());
	apcash->invdate->assign(InvoiceDate.str());
	apcash->reference->assign(GetField(ID_REFERENCE));
	apcash->descript->assign(GetField(ID_JDESCRIPT));
	apcash->vatpercent->assignDouble(dPercent);
	apcash->rebate->assignDouble(dRebatePercentage);
	apcash->account->assign(GetField(ID_APCONTROL));
	apcash->depart->assign(GetField(ID_APDEPART));
	apcash->vatacct->assign(GetField(ID_VATACCT));
	apcash->vatdept->assign(GetField(ID_VATDEPT));

	apcash->payable->assignDouble(dGetField(ID_INVOICE));
	apcash->entryno->assignLong(lGetField(ID_LINENO));
	apcash->UpdateUserTime(szUser);
	apcash->append();



	SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	bSlipList = TRUE;
	return;
}

#pragma argsused
void TAPCashEntry::CreateVat_RebateSlip(PTapcash apcash,PTapcashd apcashd,PTAPCashDist D)
{


	char szSeekAccount[12];
	char szAccount[8];
	char szDepart[5];
	// Check to see if the vendor is an individual or a company.
	if(SendDlgItemMsg(ID_INDIVIDUAL,BM_GETSTATE,NULL,NULL))
	{
	 strcpy(szSeekAccount,szIndTaxAccount);
	 strcat(szSeekAccount,szIndTaxDept);
	 strcpy(szAccount,szIndTaxAccount);
	 strcpy(szDepart,szIndTaxDept);
	}
	else
	if(SendDlgItemMsg(ID_COMPANY,BM_GETSTATE,NULL,NULL))
	{
	 strcpy(szSeekAccount,szCompanyTaxAccount);
	 strcat(szSeekAccount,szCompanyTaxDept);
	 strcpy(szAccount,szCompanyTaxAccount);
	 strcpy(szDepart,szCompanyTaxDept);
	}


	// Calculate the Invoice Amounts.
	FloatGetDlgItemValue(HWindow,ID_VATPERCENTAGE, (LPDOUBLE) &dVatPercentage);
	double dPercent       = dVatPercentage;
	dVatPercentage = (dVatPercentage + 100)/100;
	double AMOUNT  = fabs(lInvoiceAmount);
	double EXPENSE = AMOUNT/dVatPercentage;
	double VAT     = AMOUNT-EXPENSE;
	double REBATE  = EXPENSE*(dRebatePercentage/100);
	double APAMT      = AMOUNT - REBATE;

	// Add Expense amount to detail database.

	int iEntryNo = 1;

	apcashd->appendStart();
	apcashd->batchno->assignLong(iBatchno);
	apcashd->entryno->assignLong(iEntryNo);
	apcashd->descript->assign(GetField(ID_JDESCRIPT));
	apcashd->account->assign(GetField(ID_ACCOUNTS));
	apcashd->dept->assign(GetField(ID_DEPART));
	apcashd->product->assign(GetProduct());
	apcashd->medreps->assign(GetMedRep());
	apcashd->drcodes->assign(GetDoctor());


	if(lInvoiceAmount > 0)
   {        
   apcashd->debit->assignDouble(EXPENSE);
   apcashd->credit->assignDouble(0.00);
   }
   else if (lInvoiceAmount < 0)
   {
   apcashd->debit->assignDouble(0.00);
   apcashd->credit->assignDouble(EXPENSE);
   }
	apcashd->append();


   // Add VAT amount to detail database.
   iEntryNo++;
	apcashd->appendStart();
   apcashd->batchno->assignLong(iBatchno);
	apcashd->entryno->assignLong(iEntryNo);
	char szDept[5];
	strcpy(szDept,GetField(ID_VATDEPT));
	apcashd->descript->assign(GetControlAcct(GetField(ID_VATACCT),szDept));
	apcashd->account->assign(GetField(ID_VATACCT));  // APAMT account is always set to the default.s
	apcashd->dept->assign(GetField(ID_VATDEPT));        // APAMT department code is set to the default dept code.
	apcashd->product->assign(GetProduct());
	apcashd->medreps->assign(GetMedRep());
	apcashd->drcodes->assign(GetDoctor());

	if(lInvoiceAmount > 0)
	{
	apcashd->debit->assignDouble(VAT);      //VAT credit is set to zero.
	apcashd->credit->assignDouble(0.00);
	}
	else if (lInvoiceAmount < 0)
	{
	apcashd->debit->assignDouble(0.00);    //VAT credit is set to zero.
	apcashd->credit->assignDouble(VAT);
	}
	apcashd->append();


	// Add APAMT amount.

	iEntryNo++;
	apcashd->appendStart();
	apcashd->batchno->assignLong(iBatchno);
	apcashd->entryno->assignLong(iEntryNo);

	strcpy(szDept,GetField(ID_APDEPART));
	apcashd->descript->assign(GetControlAcct(GetField(ID_APCONTROL),szDept));
	apcashd->account->assign(GetField(ID_APCONTROL));
	apcashd->dept->assign(GetField(ID_APDEPART));
	apcashd->product->assign(GetProduct());
	apcashd->medreps->assign(GetMedRep());
	apcashd->drcodes->assign(GetDoctor());

	//apcashd->dept->assign(GetField(ID_APDEPART));
	if(lInvoiceAmount > 0)
	{
	apcashd->debit->assignDouble(0.00);
	apcashd->credit->assignDouble(APAMT);
	}
	else if (lInvoiceAmount < 0)
	{
	apcashd->debit->assignDouble(APAMT);
	apcashd->credit->assignDouble(0.00);
	}
	apcashd->append();


	// Add REBATE amount.

	iEntryNo++;
	apcashd->appendStart();
	apcashd->batchno->assignLong(iBatchno);
	apcashd->entryno->assignLong(iEntryNo);
	glacct->seek(szSeekAccount);
	apcashd->descript->assign(glacct->descript->str());
	apcashd->account->assign(szAccount);
	apcashd->dept->assign(szDepart);
	apcashd->product->assign(GetProduct());
	apcashd->medreps->assign(GetMedRep());
	apcashd->drcodes->assign(GetDoctor());

	if(lInvoiceAmount > 0)
	{
	apcashd->debit->assignDouble(0.00);
	apcashd->credit->assignDouble(REBATE);
	}
	else if (lInvoiceAmount < 0)
	{
	apcashd->debit->assignDouble(REBATE);
	apcashd->credit->assignDouble(0.00);
   }


	apcashd->append();


    // Add the entry to the batch master.
	apcash->appendStart();                                   //1 118
   apcash->batchno->assignLong(iBatchno);
   apcash->mode->assign("1");
   apcash->adj->assign("N");
   apcash->date->assign(InvoiceDate.str());
   apcash->period->assignDouble(GetPeriod(HWindow));
   apcash->chequeno->assign(GetField(ID_CHEQUE));
	apcash->orderno->assign(GetField(ID_CURRBAL));
   apcash->vendorno->assign(GetField(ID_VENDORNO));
   apcash->vdname->assign(GetField(ID_VENDORNAME));
	apcash->invoiceno->assign(GetField(ID_INVOICENO));
   apcash->duedate->assign(DueDate.str());
	apcash->invdate->assign(InvoiceDate.str());
   apcash->reference->assign(GetField(ID_REFERENCE));
   apcash->descript->assign(GetField(ID_JDESCRIPT));
   apcash->vatpercent->assignDouble(dPercent);
   apcash->rebate->assignDouble(dRebatePercentage);
   apcash->account->assign(GetField(ID_APCONTROL));
	apcash->depart->assign(GetField(ID_APDEPART));
   apcash->vatacct->assign(GetField(ID_VATACCT));
   apcash->vatdept->assign(GetField(ID_VATDEPT));

   apcash->payable->assignDouble(dGetField(ID_INVOICE));
	apcash->entryno->assignLong(lGetField(ID_LINENO));
	apcash->UpdateUserTime(szUser);
	apcash->append();



   SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
   bSlipList = TRUE;
   return;
}

#pragma argsused
void TAPCashEntry::CreateLineItem(PTapcash apcash,PTapcashd apcashd)
{
   int iLine;
   IntGetDlgItemValue(HWindow,3033,(LPLONG)&iLine);

   FloatGetDlgItemValue(D->HWindow,ID_DEBITTOTAL,&DebitTotal);
   FloatGetDlgItemValue(D->HWindow,ID_CREDITTOTAL,&CreditTotal);

	apcash->appendStart();
   apcash->batchno->assignLong(iBatchno);
   apcash->mode->assign("1");
   apcash->adj->assign("N");
   apcash->date->assign(InvoiceDate.str());
	apcash->period->assignDouble(GetPeriod(HWindow));
   apcash->orderno->assign(GetField(ID_CURRBAL));
   apcash->vendorno->assign(GetField(ID_VENDORNO));
	apcash->vdname->assign(GetField(ID_VENDORNAME));
   apcash->invoiceno->assign(GetField(ID_INVOICENO));
	apcash->duedate->assign(DueDate.str());
	apcash->invdate->assign(InvoiceDate.str());
	apcash->reference->assign(GetField(ID_REFERENCE));
	apcash->descript->assign(GetField(ID_JDESCRIPT));
	apcash->account->assign(GetField(1031));
	apcash->depart->assign("");
	apcash->payable->assignDouble(dGetField(ID_INVOICE));
	apcash->entryno->assignLong(lGetField(ID_LINENO));
	apcash->UpdateUserTime(szUser);
	apcash->append();





	apcashd->appendStart();
	apcashd->batchno->assignLong(iBatchno);
	apcashd->entryno->assignLong(1);
	apcashd->descript->assign(GetField(ID_JDESCRIPT));
	apcashd->account->assign(GetField(ID_ACCOUNTS));
	apcashd->dept->assign(GetField(ID_DEPART));
	apcashd->product->assign(GetProduct());
	apcashd->medreps->assign(GetMedRep());
	apcashd->drcodes->assign(GetDoctor());

   if(dGetField(ID_DEBIT) > 0)
   {        
    apcashd->debit->assignDouble(dGetField(ID_DEBIT));
    apcashd->credit->assignDouble(0.00);
   }
   else if (dGetField(ID_CREDIT) > 0)
   {
    apcashd->debit->assignDouble(0.00);
    apcashd->credit->assignDouble(dGetField(ID_CREDIT));
   }
	apcashd->append();


   SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);   
}