#include "tbowl.h"
#include "d4all.hpp"

#include "ar.h"
#include "db_dept.h"
#include "db_arbtc.h"
#include "db_itemm.h"
#include "db_medrp.h"

#include "db_cash.h"
#include "ar_class.h"
#include "ar_custl.h"
#include "ar_btchl.h"
#include "ar_btch.h"
#include "math.h"

#define ID_INVOICE 1030

#define ID_VIN         1004
#define ID_UNITS       1005
#define ID_ORDERDATE   1006
#define ID_DOCTORS     6042


void TAPInvoiceEntry::RecoverPostedInvoice()
{
 long dBatchNo;
 IntGetValue(GetDlgItem(HWindow,ID_BATCHNO),(LPLONG)&dBatchNo);
 if(apbtch->seek(dBatchNo) == 0)
 {
     apbtch->mode->assign("1");
     apbtch->flush();
     apbtch->unlock();
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
 if(apdist->seek(dBatchNo) == 0 )
  {
   while (double (Str4ptr(apdist->batchno->str())) == dBatchNo)
    {
     apdist->deleteRec();
     apdist->skip();
    }
	apdist->pack();
  }

 }
}




void TAPInvoiceEntry::CreateAPBatchEntry(int iBatchno)
{
	apbtch->SetIndex("batchno");
	apbtchd->SetIndex("batchno");

	if(apbtch->seek(iBatchno)== 0 && !apbtch->Open()){
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
	strcpy(szMedrep,GetField(1002));
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

void TAPInvoiceEntry::LoadDoctors()
{
 SendDlgItemMsg(ID_DRCODES,CB_RESETCONTENT,NULL,NULL);
 drcodes->SetIndex("drcode");
  for(drcodes->top();!drcodes->eof();drcodes->skip())
	 SendDlgItemMsg(ID_DRCODES,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)drcodes->rpname->str());
}


#pragma argsused
void TAPInvoiceEntry::UpdateBatchHeader(int iBatchno, double AP, double VAT, double REBATE)
{
  Date4 Today;
  Today.today();
  BOOL bAppend = TRUE;  

  if(apbtch->seek(iBatchno) == 0)
    bAppend = FALSE;

  if(bAppend)
    apbtch->appendStart(0);

         apbtch->batchno->assignLong(iBatchno);
	 apbtch->mode->assign("1");
	 apbtch->period->assignDouble(GetCurrentPeriodn());
	 apbtch->orderno->assign(GetField(ID_ORDERNO));
	 apbtch->vendorno->assign(GetField(ID_VENDORNO));
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
	 apbtch->payable->assignDouble(AP);
	 apbtch->vatamt->assignDouble(VAT);
	 apbtch->vin->assign(GetField(ID_VIN));
	 apbtch->orderdate->assign(GetDate(ID_ORDERDATE));
	 apbtch->units->assign(GetField(ID_UNITS));

  if(bAppend)
    apbtch->append();
  else
    apbtch->flush();
    apbtch->unlock();
}



void TAPInvoiceEntry::UpdateBatchDetail(int iBatchno, char *szDescript, char *szAccount,char *szDepart,char *szProduct,
	char *szMedrep,char *szDoctor,double dEntry)
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

 if(apbtchd->seek(iBatchno) == 0)
  {
	while (iBatchno == apbtchd->lBatchno())
	 {
	  if(strcmp(StrTrim(szAccount),StrTrim(apbtchd->account->str())) == 0 && strcmp(StrTrim(szDepart),StrTrim(apbtchd->dept->str())) == 0)
		{
		 AddLine(iEntryno,iBatchno,szDescript,szAccount,szMedrep,szProduct,szDoctor,szDepart,dGetField(1037),dGetField(1038),dEntry+apbtchd->dDebit()-apbtchd->dCredit());
		 apbtchd->flush();
		 return;
		}
		apbtchd->skip();
	 }
  }

 // if not found create a new line entry.
 apbtchd->appendStart();
 AddLine(iEntryno,iBatchno,szDescript,szAccount,szMedrep,szProduct,szDoctor,szDepart,dGetField(1037),dGetField(1038),dEntry+apbtchd->dDebit()-apbtchd->dCredit());
 apbtchd->append();
 apbtchd->unlock();
 return;
}


void TAPInvoiceEntry::AddLine(long iEntryno,
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
 apbtchd->entryno->assignLong(iEntryno);
 apbtchd->batchno->assignLong(iBatchno);
 if(strcmp(StrTrim(szDescript),"") != 0)
 apbtchd->descript->assign(szDescript);
 apbtchd->account->assign(szAccount);
 apbtchd->dept->assign(szDepart);
 apbtchd->medreps->assign(szMedrep);
 apbtchd->product->assign(szProduct);
 apbtchd->drcode->assign(szDoctor);
 apbtchd->qty->assignDouble(dQty);
 apbtchd->price->assignDouble(dEntry);
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
}

void TAPInvoiceEntry::AddBatchDetail(int iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry)
{
 // If the entry for that Account and Department code has already been entered just update it.
  int iEntryno = 1;
  apbtchd->appendStart();
  apbtchd->entryno->assignLong((int)dGetField(3033));
  apbtchd->batchno->assignLong(iBatchno);
  apbtchd->descript->assign(szDescript);
  apbtchd->medreps->assign(GetField(1002));
  apbtchd->drcode->assign(GetField(1008));
  apbtchd->account->assign(szAccount);
  apbtchd->dept->assign(szDepart);

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

  apbtchd->SetIndex("batchno");
 return;
}

void TAPInvoiceEntry::UpdateAR(int iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry)
{
 if(apbtchd->seek(iBatchno) == 0)
  {
   while (iBatchno == apbtchd->lBatchno())
    {
     if(strcmp(StrTrim(szAccount),StrTrim(apbtchd->account->str())) == 0 && strcmp(StrTrim(szDepart),StrTrim(apbtchd->dept->str())) == 0)
      {
       AddLine(apbtchd->iJournal(),iBatchno,"",szAccount,GetField(1002),GetField(1003),GetField(1008),szDepart,dGetField(1037),dGetField(1038),dEntry);
       apbtchd->flush();
       return;
      }      
      apbtchd->skip();
    }
  }

 return;
}


void TAPInvoiceEntry::UpdateVAT(int iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry)
{

 if(apbtchd->seek(iBatchno) == 0)
  {
   while (iBatchno == apbtchd->lBatchno())
    {
     if(strcmp(StrTrim(szAccount),StrTrim(apbtchd->account->str())) == 0 && strcmp(StrTrim(szDepart),StrTrim(apbtchd->dept->str())) == 0)
      {
       AddLine(apbtchd->iJournal(),iBatchno,"",szAccount,GetField(1002),GetField(1003),GetField(1008),szDepart,dGetField(1037),dGetField(1038),dEntry);
       apbtchd->flush();
       return;
      }      
      apbtchd->skip();
    }
  }

 return;
}

