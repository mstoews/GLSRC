#include "tbowl.h"
#include "d4all.hpp"
#include "glclass.h"
#include "ar.h"
#include "db_dept.h"
#include "db_arbtc.h"
#include "db_itemm.h"
#include "db_medrp.h"
									  
#include "db_cash.h"
//#include "ar_class.h"
#include "ar_custl.h"
#include "ar_btchl.h"
#include "ar_btch.h"
#include "math.h"

#define ID_INVOICE 1030

#define ID_VIN         1004
#define ID_UNITS       1005
#define ID_ORDERDATE   1006



void TARInvoiceEntry::RecoverPostedInvoice()
{
 long dBatchNo;
 IntGetValue(GetDlgItem(HWindow,ID_BATCHNO),(LPLONG)&dBatchNo);
 if(arbtch->seek(dBatchNo) == 0)
 {
	  arbtch->mode->assign("1");
	  arbtch->UpdateUserTime(szUser);
     arbtch->flush();
     arbtch->unlock();
 }

 gldist->SetIndex("apdist");
 if(bDeleted)
 {
  if(gldist->seek(dBatchNo) == 0 )
  {
	while (double (Str4ptr(gldist->batchno->str())) == dBatchNo)
	 {
       gldist->deleteRec();
       gldist->skip();
	 }
   gldist->pack();
  }
 if(ardist->seek(dBatchNo) == 0 )
  {
   while (double (Str4ptr(ardist->batchno->str())) == dBatchNo)
    {
     ardist->deleteRec();
     ardist->skip();
    }
	ardist->pack();
  }

 }
}




void TARInvoiceEntry::CreateAPBatchEntry(int iBatchno)
{
	arbtch->SetIndex(GetString(8140));
	arbtchd->SetIndex(GetString(8140));

	if(arbtch->seek(iBatchno)== 0 && !arbtch->Open()){
	 RecoverPostedInvoice();
	 return;
	 }

	double dVatPercentage = 1;

	if(dGetField(ID_VATPERCENTAGE) > 0)
		  dVatPercentage = (100+dGetField(ID_VATPERCENTAGE))/100;
	else
		dVatPercentage = 1;

	double AR;
	if(dGetField(1038) == 0 && dGetField(1030) != 0)
	 AR = dGetField(1030);
	else
	 AR     = dGetField(1037)*dGetField(1038);  //AR Total
	if(AR == 0)
	 return;

	double SALES  = floor(AR/dVatPercentage*1000)/1000;
	double VAT    = AR - SALES;

	//Header

	UpdateBatchHeader(iBatchno,AR,VAT,VAT);
	//Sales Amounts

	char szDescript[255];
	if(strcmp(StrTrim(GetField(ID_PRODUCTS)),"") == 0)
	  strcpy(szDescript,"");
	else
	  strcpy(szDescript,GetField(ID_PRODUCTS));

	char szDept[5];
	char szMedrep[6];
	char szProduct[11];
	strcpy(szDept,GetField(ID_AR_DEPT));
	strcpy(szMedrep,GetField(ID_REPS));
	strcpy(szProduct,GetField(1003));

	UpdateBatchDetail(iBatchno,GetSalesDescription(),GetField(ID_AR_ACCT),szDept,szProduct,szMedrep,GetField(1008),(-1*SALES));
	//VAT if any.
	strcpy(szDescript,"");
	if(dVatPercentage != 1)
	 {
	  strcpy(szDept,GetField(ID_VATDEPT));
	  UpdateBatchDetail(iBatchno,szDescript,GetField(ID_VATACCT),szDept,"","","",(-1*VAT)); // creates the VAT line
	 }
	//AR Total
	strcpy(szDescript,"");
	strcpy(szDept,GetField(ID_APDEPART));
	UpdateBatchDetail(iBatchno,szDescript,GetField(ID_APCONTROL),szDept,"","","",AR);
	return;
}

void TARInvoiceEntry::LoadDoctors()
{
 SendDlgItemMsg(ID_DRCODES,CB_RESETCONTENT,NULL,NULL);
 drcodes->SetIndex("drcode");
  for(drcodes->top();!drcodes->eof();drcodes->skip())
	 SendDlgItemMsg(ID_DRCODES,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)drcodes->rpname->str());
}


#pragma argsused
void TARInvoiceEntry::UpdateBatchHeader(int iBatchno, double AP, double VAT, double REBATE)
{
  Date4 Today;
  Today.today();
  BOOL bAppend = TRUE;

  if(arbtch->seek(iBatchno) == 0)
	 bAppend = FALSE;

  if(bAppend)
	 arbtch->appendStart(0);

			arbtch->batchno->assignLong(iBatchno);
	 arbtch->mode->assign("1");
	 arbtch->period->assignDouble(GetPeriod());
	 arbtch->orderno->assign(GetField(ID_ORDERNO));
	 arbtch->vendorno->assign(GetField(ID_VENDORNO));
	 arbtch->invoiceno->assign(GetField(ID_INVOICENO));

	 arbtch->date->assign(Today.str());
	 arbtch->duedate->assign(DueDate.str());
	 arbtch->invdate->assign(InvoiceDate.str());
	 arbtch->reference->assign(GetField(ID_REFERENCE));
	 arbtch->descript->assign(GetField(ID_JDESCRIPT));
	 arbtch->account->assign(GetField(ID_APCONTROL));
	 arbtch->depart->assign(GetField(ID_APDEPART));
	 arbtch->vatpercent->assignDouble(dGetField(ID_VATPERCENTAGE));
	 arbtch->terms->assignLong(lGetField(ID_TERMS));
	 arbtch->payable->assignDouble(AP);
	 arbtch->vatamt->assignDouble(VAT);
	 arbtch->vin->assign(GetField(ID_VIN));
	 arbtch->orderdate->assign(GetDate(ID_ORDERDATE));
	 arbtch->units->assign(GetField(ID_UNITS));
	 arbtch->UpdateUserTime(szUser);

  if(bAppend)
	 arbtch->append();
  else
	 arbtch->flush();
	 arbtch->unlock();
}



void TARInvoiceEntry::UpdateBatchDetail(int iBatchno,
													 char *szDescript,
													 char *szAccount,
													 char *szDepart,
													 char *szProduct,
													 char *szMedrep,
													 char *szDoctor,
													 double dEntry)
{


 char *szSeek = new char[12];
 strcpy(szSeek,szAccount);
 strcat(szSeek,szDepart);
 glacct->SetIndex("accounts");
 glacct->seek(szSeek);
 delete szSeek;

 if(strcmp(StrTrim(szDescript),"") == 0){
	strcpy(szDescript,glacct->descript->str());
  }


 if(arbtchd->seek(iBatchno) == 0)
  {
	while (iBatchno == arbtchd->lBatchno())
	 {
	  if(strcmp(StrTrim(szAccount),StrTrim(arbtchd->account->str())) == 0 && strcmp(StrTrim(szDepart),StrTrim(arbtchd->dept->str())) == 0)
		{
		 //AddLine(iEntryno,iBatchno,szDescript,szAccount,szMedrep,szProduct,szDoctor,szDepart,dGetField(1037),dGetField(1038),dEntry+arbtchd->dDebit()-arbtchd->dCredit());
		 //AddLine(iEntryno,iBatchno,szDescript,szAccount,szMedrep,szProduct,szDoctor,szDepart,dGetField(1037),dGetField(1038),dEntry+arbtchd->dDebit()-arbtchd->dCredit());
		 if(dEntry < 0)
			{
			 arbtchd->debit->assignDouble(0.00);
			 arbtchd->credit->assignDouble(fabs(dEntry));
			}
		 else if (dEntry > 0)
		  {
			 arbtchd->debit->assignDouble(fabs(dEntry));
			 arbtchd->credit->assignDouble(0.00);
		  }
		  
		 arbtchd->flush();
		 return;
		}
		arbtchd->skip();
	 }
  }

 // if not found create a new line entry.
 arbtchd->appendStart();
 AddLine(iEntryno,iBatchno,szDescript,szAccount,szMedrep,szProduct,szDoctor,szDepart,dGetField(1037),dGetField(1038),dEntry+arbtchd->dDebit()-arbtchd->dCredit());
 arbtchd->append();
 arbtchd->unlock();
 return;
}


void TARInvoiceEntry::AddLine(long iEntryno,
			long iBatchno,
			char *szDescript,
				char *szAccount,
			char *szMedrep,
			char *szProduct,
			char *szDoctor,
			char *szDepart,
			double dQty,
			double dPrice,
			double dEntry)
{

 if(iEntryno != 0)
 arbtchd->entryno->assignLong(iEntryno);
 arbtchd->batchno->assignLong(iBatchno);
 if(strcmp(StrTrim(szDescript),"") != 0)
 arbtchd->descript->assign(szDescript);
 arbtchd->account->assign(szAccount);
 arbtchd->dept->assign(szDepart);
 arbtchd->medreps->assign(szMedrep);
 arbtchd->product->assign(szProduct);
 arbtchd->drcode->assign(szDoctor);
 arbtchd->qty->assignDouble(dQty);
 arbtchd->price->assignDouble(dPrice);
 if(dEntry < 0)
	{
	 arbtchd->debit->assignDouble(0.00);
	 arbtchd->credit->assignDouble(fabs(dEntry));
	}
 else if (dEntry > 0)
	{
	arbtchd->debit->assignDouble(fabs(dEntry));
	arbtchd->credit->assignDouble(0.00);
	}
}

void TARInvoiceEntry::AddBatchDetail(int iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry)
{
 // If the entry for that Account and Department code has already been entered just update it.

  arbtchd->appendStart();
  arbtchd->entryno->assignLong((int)dGetField(3033));
  arbtchd->batchno->assignLong(iBatchno);
  arbtchd->descript->assign(szDescript);
  arbtchd->product->assign(GetField(1003));
  arbtchd->medreps->assign(GetField(5015));
  arbtchd->drcode->assign(GetField(1008));
  arbtchd->account->assign(szAccount);
  arbtchd->dept->assign(szDepart);
  arbtchd->qty->assignDouble(dGetField(1037));
  arbtchd->price->assignDouble(dGetField(1038));
	  if(dEntry < 0)
		{
		 arbtchd->debit->assignDouble(0.00);
		 arbtchd->credit->assignDouble(fabs(dEntry));
		}
	  else if (dEntry > 0)
		{
		 arbtchd->debit->assignDouble(fabs(dEntry));
		 arbtchd->credit->assignDouble(0.00);
		}
		arbtchd->append();
		arbtchd->unlock();

  arbtchd->SetIndex(GetString(8140));
 return;
}
#pragma argsused
void TARInvoiceEntry::UpdateAR(int iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry)
{
 wsprintf(szMsg,"Desc = %s, Account = %s, Dept =%s",szDescript,szAccount,szDepart);
 MessageBox(HWindow,szMsg,"UpdateAR",MB_OK);
 if(arbtchd->seek(iBatchno) == 0)
  {
	while (iBatchno == arbtchd->lBatchno())
	 {
	  if(strcmp(StrTrim(szAccount),StrTrim(arbtchd->account->str())) == 0 && strcmp(StrTrim(szDepart),StrTrim(arbtchd->dept->str())) == 0)
		{
		 //char *szMedrep  = strcpy(new char [arbtchd->medreps->len() + 1],GetString(1002));
		 //char *szProduct = strcpy(new char [arbtchd->product->len() + 1],GetString(1003));
		 //char *szDoctor  = strcpy(new char [arbtchd->drcode->len() + 1],GetString(1008));
		 AddLine(arbtchd->iJournal(),iBatchno,"",szAccount,"","","",szDepart,dGetField(1037),dGetField(1038),dEntry);
		 //delete szMedrep;
		 //delete szProduct;
		 //delete szDoctor;
		 arbtchd->flush();
		 return;
		}
		arbtchd->skip();
	 }
  }
 MessageBox(HWindow,szMsg,"UpdateAR",MB_OK);
 return;
}


#pragma argsused
void TARInvoiceEntry::UpdateVAT(int iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry)
{

 if(arbtchd->seek(iBatchno) == 0)
  {
	while (iBatchno == arbtchd->lBatchno())
	 {
	  if(strcmp(StrTrim(szAccount),StrTrim(arbtchd->account->str())) == 0 && strcmp(StrTrim(szDepart),StrTrim(arbtchd->dept->str())) == 0)
		{
		 char *szMedrep  = strcpy(new char [arbtchd->medreps->len() + 1],GetString(ID_REPS));
		 char *szProduct = strcpy(new char [arbtchd->product->len() + 1],GetString(1003));
		 char *szDoctor  = strcpy(new char [arbtchd->drcode->len() + 1],GetString(1008));
		 AddLine(arbtchd->iJournal(),iBatchno,"",szAccount,szMedrep,szProduct,szDoctor,szDepart,dGetField(1037),dGetField(1038),dEntry);
		 delete szMedrep;
		 delete szProduct;
		 delete szDoctor;
		 arbtchd->flush();
		 return;
		}
		arbtchd->skip();
	 }
  }

 return;
}

