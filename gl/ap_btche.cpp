//#include "ap.h"
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

//#include "db_cash.h"
#include "db_itemm.h"
#include "db_medrp.h"
#include "db_drcod.h"
#include "ap_class.h"
#include "ap_setup.h"
#include "ap_btch.h"
#include "utildlg.h"
#include "ap_vendo.h"
#include "gl_setat.hpp"
#include "gl_pds.hpp"
#include "math.h"
#define ID_INVOICE 1030
#define ID_COMMIT 1056
#define  ID_VATACCT 1034
#define  ID_VATDEPT 1035
#define  ID_VIN_NUM 1004
#define PROFILE "GL.INI"
#define  NEWBATCH  10
#define  OLDBATCH  11
#define  MODBATCH  12

extern HINSTANCE hInst;


void TAPInvoiceEntry::LoadDoctors(void)
 {
  SendDlgItemMsg(ID_DRCODES,CB_RESETCONTENT,NULL,NULL);
  drcodes->SetIndex("drcode");
  for(drcodes->top();!drcodes->eof();drcodes->skip())
	 SendDlgItemMsg(ID_DRCODES,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)drcodes->rpname->str());
	 SendDlgItemMsg(ID_DRCODES,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)" ");
 }

void TAPInvoiceEntry::Company(RTMessage)
{
 if(SendDlgItemMsg(ID_COMPANY,BM_GETCHECK,NULL,NULL))
  SendDlgItemMsg(ID_COMPANY,BM_SETCHECK,FALSE,NULL);
 else
  {
	SendDlgItemMsg(ID_COMPANY,BM_SETCHECK,TRUE,NULL);
	SendDlgItemMsg(ID_INDIVIDUAL,BM_SETCHECK,FALSE,NULL);
  }

}

void TAPInvoiceEntry::Individual(RTMessage)
{
 if(SendDlgItemMsg(ID_INDIVIDUAL,BM_GETCHECK,NULL,NULL))
  SendDlgItemMsg(ID_INDIVIDUAL,BM_SETCHECK,FALSE,NULL);
 else
  {
	SendDlgItemMsg(ID_INDIVIDUAL,BM_SETCHECK,TRUE,NULL);
	SendDlgItemMsg(ID_COMPANY,BM_SETCHECK,FALSE,NULL);
  }

}


void TAPInvoiceEntry::LoadMedReps(void)
{
  SendDlgItemMsg(ID_MEDREPS,CB_RESETCONTENT,NULL,NULL);
  medreps->SetIndex("rpname");
  for(medreps->top();!medreps->eof();medreps->skip())
	 SendDlgItemMsg(ID_MEDREPS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)medreps->rpname->str());
	 SendDlgItemMsg(ID_MEDREPS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)" ");
}

void TAPInvoiceEntry::LoadProducts(void)
{
  SendDlgItemMsg(ID_PRODUCTS,CB_RESETCONTENT,NULL,NULL);
  prods->SetIndex("ffulln");
  for(prods->top();!prods->eof();prods->skip())
	SendDlgItemMsg(ID_PRODUCTS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)prods->fFulln->str());
	SendDlgItemMsg(ID_PRODUCTS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)" ");
}



static COLORREF lpBack, lpFore;
extern char szApp[20];

BOOL Tapbtch::Posted()
{
 if(strcmp(mode->str(),"0") == 0)
  return TRUE;
 else
  return FALSE;
}

BOOL Tapbtch::Open()
{
 if(strcmp(mode->str(),"1") == 0)
  return TRUE;
 else
  return FALSE;
}

BOOL Tapbtch::Closed()
{
 if(strcmp(mode->str(),"0") == 0)
  return TRUE;
 else
  return FALSE;
}

BOOL Tapbtch::New(long iBatchno)
{
 SetIndex(GetString(hInst,8140));
 if(seek(iBatchno) != 0)
  return TRUE;
 else
  return FALSE;
}

void TAPInvoiceEntry::AddVendor(RTMessage)
{
 GetApplication()->ExecDialog(new TAPVendorBatchAdd(this,1041,apvendor,glacct));
 return;
}


void TAPInvoiceEntry::Top(RTMessage Msg)
{
 apbtch->SetIndex(GetString(8140));
 apbtch->top();
 while(apbtch->Closed() && !apbtch->eof())
  apbtch->skip();
 if(!apbtch->eof()) 
 CurrentBatch(Msg);
};

void TAPInvoiceEntry::Bottom(RTMessage Msg)
{
 apbtch->SetIndex(GetString(8140));
 apbtch->bottom();
 BOOL bList = FALSE;
 while(apbtch->Closed() && !apbtch->bof())
	apbtch->skip(-1);
 if(!apbtch->bof())
  CurrentBatch(Msg);
};

void TAPInvoiceEntry::Next(RTMessage Msg)
{
 apbtch->SetIndex(GetString(8140));
 int iBatchno = 0;
 IntGetDlgItemValue(HWindow,ID_BATCHNO,(LPLONG)&iBatchno);
 if(apbtch->seek(iBatchno) == 0)
 { 
	if(!apbtch->eof())
	{
	 apbtch->skip();
	 while(apbtch->Closed() && !apbtch->eof())
		apbtch->skip();
	 if(apbtch->eof())
	 apbtch->bottom();
	 CurrentBatch(Msg);
	}
 }
 else
 return;

};
void TAPInvoiceEntry::Previous(RTMessage Msg)
{
 apbtch->SetIndex(GetString(8140));
 int iBatchno = 0;
 IntGetDlgItemValue(HWindow,ID_BATCHNO,(LPLONG)&iBatchno);
 if(apbtch->seek(iBatchno) == 0)
 {
	if(!apbtch->bof())
	{
	 apbtch->skip(-1);
	 if(apbtch->Closed())
     apbtch->skip();
	 CurrentBatch(Msg);
   }
 }
 else if (apbtch->eof())
 {
    apbtch->bottom();
    CurrentBatch(Msg);
 }
 else
 return;
};



void TAPInvoiceEntry::Delete(RTMessage Msg)
{
  int iEntryno;
  IntGetValue(GetDlgItem(HWindow,ID_LINENO),(LPLONG)&iEntryno);
  apbtchd->SetIndex(GetString(8140));
  if(apbtchd->seek(apbtch->lBatchno()) == 0)
  {
	if(MessageBox(HWindow,GetString(6098),szApp,MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
     HourGlassOn();
	  while(apbtchd->lBatchno() == apbtch->lBatchno())
      {
		 if(apbtchd->iJournal() == iEntryno)
        {
	 apbtchd->deleteRec();
	 apbtchd->pack();
		  }
	 apbtchd->skip();
		}
	  HourGlassOff();
	 }

  }
  CurrentBatch(Msg);
  return;
};


void TAPInvoiceEntry::New(RTMessage)
{
 if(bTransaction)
  {
	 switch(MessageBox(HWindow,GetString(ID_COMMIT),szApp,MB_YESNOCANCEL |MB_ICONQUESTION))
	{
	 case IDCANCEL: return;
	 case IDNO    : SendMessage(HWindow,WM_COMMAND,CM_ROLLBACK,NULL); break;
	 case IDYES   : break;
	}
 }

 Clear();
 return;

}

void TAPInvoiceEntry::RollBack(RTMessage)
{
 HourGlassOn();
 apbtch->SetIndex(GetString(8140));
 long lBatchno = lGetField(ID_BATCHNO);
 if(apbtch->seek(lBatchno) == 0)
	apbtch->deleteRec();

 if(apbtchd->seek(lBatchno) == 0)
  while(apbtchd->lBatchno() == lBatchno)
  {
	 apbtchd->deleteRec();
	 apbtchd->skip();
  }

 apbtchd->pack();
 apbtch->pack();
 HourGlassOff();
 return;
}

void TAPInvoiceEntry::SetupWindow()
{

 TMainDialog::SetupWindow();
 HourGlassOn();
 bModify = FALSE;
 if(!CheckFile(GetString(2043),Parent->HWindow) ||
	 !CheckFile(GetString(2044),Parent->HWindow) ||
	 !CheckFile(GetString(2045),Parent->HWindow) ||
	 !CheckFile(GetString(2046),Parent->HWindow) ||
	 !CheckFile(GetString(2047),Parent->HWindow) ||
	 !CheckFile(GetString(2048),Parent->HWindow) ||
	 !CheckFile(GetString(2049),Parent->HWindow) ||
	 !CheckFile(GetString(2050),Parent->HWindow) ||
	 !CheckFile(GetString(2051),Parent->HWindow) ||
	 !CheckFile(GetString(2052),Parent->HWindow))
	{
	 cb.init();
	 ShutDownWindow();
	 return;
	}


 DebitTotal=0;
 CreditTotal=0;
 bTransaction = FALSE;
 apbtchd=  new Tapbtchd(&cb);
 apvendor= new Tapvendor(&cb);
 glacct =  new Tglaccts(&cb);
 gldept =  new Tgldept(&cb);
 gldist =  new Tgldist(&cb);
 apdist =  new Tapdist(&cb);
 prods  =  new Tprods(&cb);
 medreps = new Tglmedreps(&cb);
 drcodes = new Tgldrcodes(&cb);
 apbtch =  new Tapbtch(&cb);


 LoadDoctors();
 LoadMedReps();
 LoadProducts();

 SetPeriods(HWindow);

 apbtch->SetIndex(GetString(8140));
 apvendor->SetIndex("vendorno");



 GL_SETAT gl_setat;
 gl_setat.open(cb);
 gl_setat.SetIndex("gl_accts");


 if(gl_setat.seek("WTXI") == 0) {
  strcpy(szIndTaxAccount,gl_setat.account->str());
  strcpy(szIndTaxDept,gl_setat.dept->str());
 }
 else {
  strcpy(szIndTaxAccount,"");
  strcpy(szIndTaxDept,"");
 }

 if(gl_setat.seek("WTXC") == 0) {
  strcpy(szCompanyTaxAccount,gl_setat.account->str());
  strcpy(szCompanyTaxDept,gl_setat.dept->str());
 }
 else{
	strcpy(szCompanyTaxAccount,"");
	strcpy(szCompanyTaxDept,"");
 }


 if(gl_setat.seek("VATP") == 0) {
  strcpy(szVATAccount,gl_setat.account->str());
  strcpy(szVATDept,gl_setat.dept->str());
 }
 else {
  strcpy(szVATAccount,"");
  strcpy(szVATDept,"");
 }

 AcctList = new TAccountList(this,"",glacct);
 AcctList->Attr.Style |= WS_THICKFRAME;
 AcctList->EnableAutoCreate();
 GetApplication()->MakeWindow(AcctList);

 VendorList = new TCustomerList(this,"",apvendor);
 VendorList->Attr.Style |= WS_THICKFRAME;
 VendorList->EnableAutoCreate();
 GetApplication()->MakeWindow(VendorList);
 VendorList->SetCaption("Vendor List");


 SlipList  = new TBatchList(this,"",apbtch);
 SlipList->Attr.Style |= WS_THICKFRAME;
 SlipList->EnableAutoCreate();
 GetApplication()->MakeWindow(SlipList);


 //D = new TAPDistList(this,1024,apbtchd);
 D = new TAPDistList(this,GetString(8156),apbtchd);
 D->Attr.Style |= WS_THICKFRAME;
 D->EnableAutoCreate();
 GetApplication()->MakeWindow(D);



 long db=0;
 apbtch->bottom();
 db = apbtch->lBatchno();
 db++;
 IntSetDlgItemValue(HWindow,ID_BATCHNO,db);
 IntSetDlgItemValue(HWindow,ID_LINENO,1);



 RECT rc;
 RECT parent;
 int pos = 72;
 GetWindowRect(HWindow,&rc);
 GetWindowRect(Parent->HWindow,&parent);
 MoveWindow(HWindow,parent.left+3,parent.top+pos,rc.right-rc.left,rc.bottom-rc.top,TRUE);
 D->Show(SW_SHOW);
 SetBitmap(IDCANCEL);
 SetBitmap(ID_NEW);
 SetBitmap(IDOK);
 SetBitmap(ID_NEW);
 SetBitmap(ID_VIEW);
 SetBitmap(ID_SEARCHCUST);
 SetBitmap(ID_SLIPLIST);
 SetBitmap(ID_PRINT);
 IntSetDlgItemRange(HWindow,4065,0,100);
 IntSetDlgItemRange(HWindow,4073,0,100);
 IntSetDlgItemRange(HWindow,6086,0,365);
 HourGlassOff();
}


void TAPInvoiceEntry::ProductList(RTMessage Msg)
{

 char *szProduct = new char[8];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  long Indx = SendDlgItemMsg(5017,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(5017,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM) ((LPSTR) szProduct));
	 prods->seek(szProduct);
    SetField(5018,prods->fPcode);
	 SetFocus(HWindow);
	}
 }
 delete [] szProduct;
}



void TAPInvoiceEntry::SetVendor(RTMessage)
{
 SetField(ID_VENDORNO,apvendor->vendorno);
 SetField(ID_VENDORNAME,apvendor->vdname); 
 SetField(ID_APCONTROL,apvendor->account);
 SetField(ID_APDEPART,apvendor->depart);
 SetField(ID_REBATE,apvendor->rebate);
 SetField(ID_VATPERCENTAGE,apvendor->percentage);
 SetField(ID_TERMS,apvendor->terms);
 SetField(ID_VATACCT, apvendor->vat_acct);
 SetField(ID_VATDEPT, apvendor->vat_dept);
 if(strcmp(apvendor->type->str(),"C") == 0 )
  {
	SendDlgItemMsg(ID_COMPANY,BM_SETCHECK,TRUE,NULL);
	SendDlgItemMsg(ID_INDIVIDUAL,BM_SETCHECK,FALSE,NULL);
  }
  else
  {
	SendDlgItemMsg(ID_INDIVIDUAL,BM_SETCHECK,TRUE,NULL);
   SendDlgItemMsg(ID_COMPANY,BM_SETCHECK,FALSE,NULL);
  }
}




void TAPInvoiceEntry::Period(RTMessage Msg)
{
 char szPeriod[3];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  GL_PDS gl_pds;
  if(!gl_pds.open(cb) == 0)
  {
	MessageBox(HWindow,GetString(1038),szApp,MB_ICONINFORMATION);
	return;
  }
  gl_pds.SetIndex("PERIOD");
  long Indx = SendDlgItemMsg(ID_PERIOD,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(ID_PERIOD,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szPeriod);
	 if(gl_pds.seek(szPeriod) == 0){
		SetField(1084,gl_pds.pd_desc.str());
	  }
	}
  gl_pds.close();
 }
}


void TAPInvoiceEntry::CurrentBatch(RTMessage Msg)
{

  D->List->SetBool(SSB_REDRAW,FALSE);

  double lDebitTotal,lCreditTotal;
  char szDueDate[11],szInvDate[11];
  lDebitTotal=lCreditTotal=0;
  if(apbtch->recCount() == 0)
  {
	MessageBox(HWindow,GetString(8153),szApp,MB_ICONINFORMATION);
	return;
  }

  if(apvendor->seek(apbtch->vendorno->str()) == 0)
	SetVendor(Msg);

  SetField(ID_BATCHNO,apbtch->batchno);
  SetField(ID_DESCRIPT,apbtch->descript);
  SetField(ID_VENDORNO,apbtch->vendorno);
  SetField(ID_VENDORNAME,apbtch->vdname);
  SetField(ID_INVOICENO,apbtch->invoiceno);
  SetField(ID_CURRBAL,apbtch->orderno);
  SetField(ID_REFERENCE,apbtch->reference);
  SetField(ID_APCONTROL,apbtch->account);
  SetField(ID_APDEPART,apbtch->depart);
  SetField(ID_REBATE,apbtch->rebate);


  InvoiceDate.assign(apbtch->invdate->str(),"CCYYMMDD");
  InvoiceDate.format(szInvDate,GetString(4024));
  SetField(ID_INVDATE,szInvDate);

  DueDate.assign(apbtch->duedate->str(),"CCYYMMDD");
  DueDate.format(szDueDate,GetString(4024));
  SetField(ID_DUEDATE,szDueDate);

  SetField(ID_TERMS,apbtch->terms);
  SetField(ID_VATPERCENTAGE,apbtch->vatpercent);
  SetField(ID_REBATE,apbtch->rebate);
  SetField(ID_ACCOUNTS,"");
  SetField(ID_DEPART,"");
  SetField(ID_DEBIT,"");
  SetField(ID_CREDIT,"");
  SetField(ID_INVOICE,apbtch->payable);

  long i=1;

  apbtchd->SetIndex(GetString(8140));
  char *szLineNo = new char[3];

  D->List->ClearData(SS_ALLCOLS,SS_ALLROWS);

  if (apbtchd->seek(double(Str4ptr(apbtch->batchno->str()))) == 0){
	while (apbtch->lBatchno() == apbtchd->lBatchno() && !apbtch->eof() ){
	  if(apbtchd->dDebit() > 0){
		lDebitTotal += apbtchd->dDebit();
		ltoa(i,szLineNo,10);
		D->List->SetData(1,i,szLineNo);
		D->List->SetData(2,i,apbtchd->descript->str());
		D->List->SetData(3,i,apbtchd->account->str());
		D->List->SetData(4,i,apbtchd->dept->str());
		D->List->SetData(5,i,apbtchd->debit->str());
		D->List->SetData(6,i,apbtchd->credit->str());
		apbtchd->entryno->assignLong(i);
		apbtchd->flush();
		apbtchd->unlock();
		i++;
	  }
	  apbtchd->skip();
	}
 }

  //Set the credit amounts.
 if (apbtchd->seek(apbtch->lBatchno()) == 0){
	while (apbtch->lBatchno() == apbtchd->lBatchno() && !apbtch->eof()){
	  if(apbtchd->dCredit() > 0){
		lCreditTotal += apbtchd->dCredit();
		ltoa(i,szLineNo,10);
		D->List->SetData(1,i,szLineNo);
		D->List->SetData(2,i,apbtchd->descript->str());
		D->List->SetData(3,i,apbtchd->account->str());
		D->List->SetData(4,i,apbtchd->dept->str());
		D->List->SetData(5,i,apbtchd->debit->str());
		D->List->SetData(6,i,apbtchd->credit->str());
		apbtchd->entryno->assignLong(i);
		apbtchd->flush();
		apbtchd->unlock();
		i++;
	  }
	  apbtchd->skip();
	}

  }


	Str4large szDebitTotal;
	Str4large szCreditTotal;
	szDebitTotal.assignDouble(lDebitTotal,16,2);
	szCreditTotal.assignDouble(lCreditTotal,16,2);
	IntSetDlgItemValue(HWindow,ID_LINENO,i);
	i++;
	short iHeight = 0;
	RECT Drc;
	GetClientRect(D->HWindow,&Drc);
	D->List->GetRowHeightInPixels(1,&iHeight);
	int cRows = (Drc.bottom - Drc.top) / iHeight + 1;
	if(i < cRows)
	 D->iRows = cRows;
	else
	 D->iRows = i;

	D->List->SetMaxRows(D->iRows);
	D->List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
	if(lDebitTotal != lCreditTotal)
	{
	 D->List->SetData(2,i,"Voucher Out Of Balance");
	 D->List->SetColor(2,i,RGBCOLOR_WHITE,RGBCOLOR_RED);
	}
	else
	{
	 D->List->SetData(2,i,"Total");
	}
	D->List->SetData(5,i,szDebitTotal.str());
	D->List->SetData(6,i,szCreditTotal.str());

	apbtchd->iCurrentRecord = 0;
  delete szLineNo;

  if(bSlipList)
	SlipList->Show(SW_HIDE);


  if(strcmp(apbtch->mode->str(),"0") == 0)
	{
	 bRecovered = FALSE;
	}
  D->List->SetColor(5,i,RGB(192,220,192), RGBCOLOR_BLUE);
  D->List->SetColor(6,i,RGB(192,220,192), RGBCOLOR_BLUE);
  D->List->SetBool(SSB_REDRAW,TRUE);
  SetFocus(GetDlgItem(HWindow,ID_INVOICE));
}

void TAPInvoiceEntry::Automatic(RTMessage)
{
 if(SendDlgItemMessage(HWindow,ID_AUTO,BM_GETCHECK,NULL,NULL))
  {
	 SendDlgItemMessage(HWindow,ID_AUTO,BM_SETCHECK,TRUE,NULL);
	 bAutomaticCalc = TRUE;
  }
 else
  {
	 SendDlgItemMessage(HWindow,ID_AUTO,BM_SETCHECK,FALSE,NULL);
	 bAutomaticCalc = FALSE;
  }

}


void TAPInvoiceEntry::Ok (RTMessage Msg )
{

 //szVendorNo = strcpy(new char[apbtch->vendorno->len() + 1],GetField(ID_VENDORNO));
 string szVendorNo(GetField(ID_VENDORNO));

 if(strlen(StrTrim((char *)szVendorNo.c_str())) == 0)
	{
	 if(!IsWindowVisible(VendorList->HWindow))
	  VList(Msg);
	 else
	  SetFocus(VendorList->HWindow);
	 return;
	}

 if(apvendor->seek((char *)szVendorNo.c_str())!=0)
  {
	MessageBeep(MB_ICONEXCLAMATION);
	if(MessageBox(HWindow,GetString(6029),szApp,MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
	 D->Show(SW_HIDE);
	 AddVendor(Msg);
	 D->Show(SW_SHOW);
	 if(apvendor->seek((char *)szVendorNo.c_str()) == 0)
	  SetField(ID_VENDORNAME,apvendor->vdname);
	 VList(Msg);
	}
	return;
  }

 if(apbtch->New(lGetField(ID_BATCHNO)))
 {
  string szInvoiceno(GetField(ID_VENDORNO));
  szInvoiceno += GetField(ID_INVOICENO);
  if(apbtch->seek((char *)szInvoiceno.c_str()) == 0)
  {
	  MessageBox(HWindow,GetString(1039),szApp,MB_ICONINFORMATION);
	  CurrentBatch(Msg);
	  return;

  }
 }

 apbtch->SetIndex(GetString(8140));
 szAdj = strcpy(new char[apbtch->adj->len() + 1],"N");
 SendDlgItemMsg(ID_DISTLIST, WM_SETREDRAW, FALSE, 0L);
 iPeriodNo = (int)lGetField(ID_PERIOD);

 if(strlen(StrTrim((char *)szVendorNo.c_str())) == 0)
	{
	 CheckError(ID_VENDORNO);
	 return;
	}

 if(apvendor->seek((char *)szVendorNo.c_str())!=0)
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
	CheckError(ID_INVOICENO);
	return;
  }

 lInvoiceAmount = dGetField(ID_INVOICE);
 if(lInvoiceAmount == 0 )
	 {
	  CheckError(ID_INVOICE);
	  return;
	 }


 szInvoiceDate = strcpy(new char[apbtch->date->len() + 1],GetField(ID_INVDATE));
 if(*szInvoiceDate==0)
  {
	CheckError(ID_INVDATE);
	return;
  }
 InvoiceDate.assign(szInvoiceDate,GetString(4024));


 szDueDate = strcpy(new char[apbtch->date->len() + 1],GetField(ID_DUEDATE));
 if(*szDueDate==0)
	{
	 CheckError(ID_DUEDATE);
	 return;
	}
 DueDate.assign(szDueDate,GetString(4024));
 dTerms = dGetField(ID_TERMS);
 long lDueDate = long (InvoiceDate) + dTerms;
 DueDate.assign(lDueDate);

 szOrderNo = strcpy(new char[apbtch->orderno->len() + 1],GetField(ID_CURRBAL));

 szDepart = strcpy(new char[apbtch->depart->len() + 1],GetField(ID_DEPART));
 szAccounts = strcpy(new char[apbtch->account->len() + 1],GetField(ID_ACCOUNTS));
 char *szSeek = new char[apbtch->account->len() + apbtch->depart->len() + 1];
 strcpy(szSeek,szAccounts);
 strcat(szSeek,szDepart);

 if(strlen(StrTrim(szSeek))== 0)
  {
	Accountlist();
	MessageBeep(NULL);
	return;
  }


 if(!ValidateGL(szAccounts,szDepart))
  {
	Accountlist();
	SetFocus(GetDlgItem(HWindow,ID_ACCOUNTS));
	return;
  }


 if(strlen(StrTrim(GetField(ID_JDESCRIPT))) == 0)
 {
  CheckError(ID_JDESCRIPT);
  return;
 }

 if(strlen(StrTrim(GetField(ID_REFERENCE)))==0) {
	CheckError(ID_REFERENCE);
	return;
 }

 iLineno = lGetField(ID_LINENO);

 dVatPercentage = dGetField(ID_VATPERCENTAGE);

 if(dVatPercentage != 0 && strlen(StrTrim(GetField(ID_VATACCT))) == 0)
  {
	 MessageBox(HWindow,GetString(8157),szApp,MB_ICONINFORMATION);
	 SetFocus(GetDlgItem(HWindow,ID_VATACCT));
	 return;
  }

 dRebatePercentage = dGetField(ID_REBATE);
 iBatchno = (int)lGetField(ID_BATCHNO);

  SetFocus(HWindow);
  if(bModify)
	{
	 ModifyCurrentSlip();
	}
  else
	{
	 bSlipList = TRUE;
	 CreateAPBatchEntry(iBatchno);
	 bTransaction = TRUE;
	}
  bModify = FALSE;
  SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
  return;
}


BOOL TAPInvoiceEntry::GetWithHolding()
{
  char szWithHolding[2];
  GetPrivateProfileString(GetString(8150),"TAX","T",szWithHolding,sizeof(szWithHolding),"GL.INI");
  if(strcmp(szWithHolding,"T") == 0)
	return TRUE;
  else
	return FALSE;
}

void TAPInvoiceEntry::GetAPExpense(AP_EXPENSE* ex,
						double lInvoiceAmount,
						double dVatPercentage,
						double dRebatPercentage)
 {
	ex->REBATE  = 0.0;
	ex->VAT     = 0.0;
	ex->NEWAP      = apbtch->dPayable() + lInvoiceAmount;
	ex->OLDAP      = apbtch->dPayable();
	ex->APAMT      = lInvoiceAmount;

	if(!GetWithHolding())
	{
	 ex->OLDEXPENSE = ex->OLDAP - apbtch->dVatamt();
	 if(dVatPercentage != 1)
	  ex->VAT  = (ex->NEWAP - ((floor(ex->NEWAP/(1+dVatPercentage)*1000)/1000)));
	 ex->NEWEXPENSE  = ex->NEWAP - ex->OLDEXPENSE - ex->VAT;

	 if(dRebatePercentage != 1)
		 ex->REBATE  = -1*((ex->NEWAP)*dRebatePercentage);

	 ex->APAMT = ex->NEWAP + ex->REBATE;
	}
	else
	{
	 ex->OLDEXPENSE = ex->OLDAP - apbtch->dVatamt() - apbtch->dRebateamt();
	 double dVatRebateTotal = 1;
	 if(dRebatePercentage != 1)
		  dVatRebateTotal += dRebatePercentage;
	 if(dVatPercentage != 1)
		  dVatRebateTotal += dVatPercentage;

	 ex->NEWEXPENSE = lInvoiceAmount/dVatRebateTotal;
	 ex->VAT        = (floor(ex->NEWEXPENSE * dVatPercentage*1000)/1000) + apbtch->dVatamt();
	 ex->REBATE     = ex->NEWAP - ex->VAT - ex->NEWEXPENSE - ex->OLDEXPENSE;
	 ex->APAMT      = ex->NEWAP;
	}
}



void TAPInvoiceEntry::ModifyCurrentSlip()
{
  apbtch->date->assign(InvoiceDate.str());
  apbtch->period->assignDouble(GetPeriod(HWindow));
  apbtch->orderno->assign(GetField(ID_CURRBAL));
  apbtch->invoiceno->assign(GetField(ID_INVOICENO));
  apbtch->duedate->assign(DueDate.str());
  apbtch->invdate->assign(InvoiceDate.str());
  apbtch->reference->assign(GetField(ID_REFERENCE));
  apbtch->descript->assign(GetField(ID_JDESCRIPT));
  apbtch->account->assign(GetField(ID_APCONTROL));
  apbtch->depart->assign(GetField(ID_APDEPART));
  apbtch->payable->assignDouble(dGetField(ID_INVOICE));

	/*
	if(strcmp(StrTrim(GetField(ID_ACCOUNTS)),StrTrim(szIndTaxAccount)) == 0 ||
		strcmp(StrTrim(GetField(ID_ACCOUNTS)),StrTrim(szCompanyTaxAccount)) == 0)
	if(lDebit > 0)
	 apbtch->rebateamt->assignDouble(lDebit);
	else
	if(lCredit > 0)
	 apbtch->rebateamt->assignDouble(-lCredit);

	if(strcmp(StrTrim(GetField(ID_ACCOUNTS)),StrTrim(szVATAccount)) == 0)
	if(lDebit > 0)
	 apbtch->vatamt->assignDouble(lDebit);
	else
	if(lCredit > 0)
	 apbtch->vatamt->assignDouble(-lCredit);
	*/
	apbtch->entryno->assignLong(lGetField(ID_LINENO));
	apbtch->rebate->assignDouble(dGetField(ID_REBATE));
	apbtch->terms->assignLong(lGetField(ID_TERMS));
	apbtch->UpdateUserTime(szUser);
	apbtch->flush();
	apbtch->unlock();

	apbtchd->batchno->assignLong(iBatchno);
	apbtchd->account->assign(GetField(ID_ACCOUNTS));
	apbtchd->dept->assign(GetField(ID_DEPART));
	apbtchd->descript->assign(GetField(ID_JDESCRIPT));


	string szMedRep(GetMedRep());
	string szProduct(GetProduct());
	string szDoctor(GetDoctor());

	apbtchd->product->assign((char *) szMedRep.c_str());
	apbtchd->medreps->assign((char *) szProduct.c_str());
	apbtchd->drcodes->assign((char *) szDoctor.c_str());

	apbtchd->debit->assignDouble(dGetField(ID_DEBIT));
	apbtchd->credit->assignDouble(dGetField(ID_CREDIT));
	apbtchd->flush();
	apbtchd->unlock();
	return;
}



void TAPInvoiceEntry::SearchGL    (RTMessage)
  {
	AcctList->List->Seek(ID_ACCOUNTS);
	AcctList->Show(SW_SHOW);
	//Accounts.List->Seek(ID_ACCOUNTS);
	//Accounts.Show(SW_SHOW);
  // SetFocus(GetDlgItem(HWindow,ID_DESCRIPT));
  }



void TAPInvoiceEntry::Vendorlist  ()
{
 VendorList->List->Seek(ID_VENDORNO);
 VendorList->Show(SW_SHOW);
}


void TAPInvoiceEntry::Sliplist() //slip list search
{
 SlipList->List->Seek(ID_BATCHNO);
 SlipList->Show(SW_SHOW);
}



void TAPInvoiceEntry::Accountlist ()
{
  AcctList->List->Seek(ID_ACCOUNTS);
  AcctList->Show(SW_SHOW);
}
 

void TAPInvoiceEntry::Clear()
{
 apbtch->SetIndex(GetString(8140));
 apbtch->bottom();
 int iBatchno = (int) apbtch->lBatchno();
 iBatchno++;
 IntSetDlgItemValue(HWindow,ID_BATCHNO,iBatchno);
 IntSetDlgItemValue(HWindow,ID_LINENO,1);
 SetField(ID_DESCRIPT,"");
 SetField(ID_VENDORNO,"");
 SetField(ID_VENDORNAME,"");
 SetField(ID_INVOICENO,"");
 SetField(ID_CURRBAL,"");
 SetField(ID_REFERENCE,"");
 SetField(ID_JDESCRIPT,"");
 SetField(ID_APCONTROL,"");
 SetField(ID_APDEPART,"");
 SetField(ID_ACCOUNTS,"");
 SetField(ID_DEPART,"");
 SetField(ID_VATPERCENTAGE,"");
 SetField(ID_TERMS,"");
 SetField(ID_REBATE,"");
 SetField(ID_ACCOUNTS,"");
 SetField(ID_DEPART,"");
 SetField(ID_VATACCT,"");
 SetField(ID_VATDEPT,"");
 SetField(ID_VIN_NUM,"");


 D->List->SetColor(SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
 D->List->ClearData(SS_ALLCOLS,SS_ALLROWS);

 FloatSetDlgItemValue(HWindow,ID_DEBIT,0.00);
 FloatSetDlgItemValue(HWindow,ID_CREDIT,0.00);
 FloatSetDlgItemValue(HWindow,ID_INVOICE,0.00);
 DebitTotal=CreditTotal=0;
 bTransaction = FALSE;

 long Indx = SendDlgItemMsg(ID_PRODUCTS,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_PRODUCTS,CB_SETCURSEL,Indx,NULL);

 Indx = SendDlgItemMsg(ID_MEDREPS,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_MEDREPS,CB_SETCURSEL,Indx,NULL);

 Indx = SendDlgItemMsg(ID_DRCODES,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_DRCODES,CB_SETCURSEL,Indx,NULL);

 SetFocus(GetDlgItem(HWindow,ID_VENDORNO));
}


void TAPInvoiceEntry::Print(RTMessage)
{
 if(apbtch->seek(dGetField(ID_BATCHNO)) == 0)
  PrintSlip(D->GetReportName(),(int)dGetField(ID_BATCHNO));
}

void TAPInvoiceEntry::Cancel (RTMessage )
{
 if(bTransaction)
 {
  switch(MessageBox(HWindow,GetString(ID_COMMIT),szApp,MB_YESNOCANCEL |MB_ICONQUESTION))
	{
	 case IDCANCEL: return;
	 case IDNO    : SendMessage(HWindow,WM_COMMAND,CM_ROLLBACK,NULL); break;
	 case IDYES   : break;
	}
 }
 VendorList->List->SavePosition();
 AcctList->List->SavePosition();
 SlipList->List->SavePosition();
 D->SavePosition();
 cb.closeAll();
 cb.init();
 ShutDownWindow();
 return;
}

void TAPInvoiceEntry::HandleDepart (RTMessage ){}


void TAPInvoiceEntry::SetLineItem(RTMessage)
{
  bModify = TRUE;
  SetField(ID_JDESCRIPT,apbtchd->descript);
  SetField(ID_ACCOUNTS ,apbtchd->account);
  SetField(ID_DEPART   ,apbtchd->dept);
  SetField(ID_DEBIT    ,apbtchd->debit);
  SetField(ID_CREDIT   ,apbtchd->credit);
  SetField(ID_LINENO   ,apbtchd->entryno);
}



int TAPInvoiceEntry::CheckGLAccount(char *account)
{
 if(glacct->seek(account)==0)
 return TRUE;
 else
 return FALSE;
}


void TManilaGL::APBatch       (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPInvoiceEntry(this,1042));
 }

char *TAPInvoiceEntry::GetControlAcct(char *Account,char *Depart)
{
 static char szDescript[40];
 strcpy(szDescript,Account);
 strcat(szDescript,Depart);
 if(glacct->seek(szDescript) == 0)
 strcpy(szDescript,glacct->descript->str());
 else
 strcpy(szDescript,"");
 return szDescript;

}


int GetFontSize()
{
 char szSize[3];
 GetPrivateProfileString("FONT","SIZE","",szSize,sizeof(szSize),"GL.INI");
 //MessageBox(GetFocus(),szSize,"",MB_OK);
 return atoi(szSize);
}


void PrintSlip(LPSTR szReportName,int iBatchno)
{
  HourGlassOn();
  PEOpenEngine();
  char  szBatchNo[10];
  short JobNumber = PEOpenPrintJob(szReportName);

  if (JobNumber == 0)
  {
	HANDLE textHandle;
	short  iTextLen;
	PEGetErrorText(JobNumber,&textHandle,&iTextLen);
	char   *szErrorText = new char[iTextLen];
	PEGetHandleString(textHandle,szErrorText,iTextLen);
	MessageBox(GetFocus(),szErrorText,szApp,MB_ICONINFORMATION);
	delete szErrorText;
	return;
  }

	itoa((unsigned)iBatchno,szBatchNo,10);

	string szName;
	szName = "'";
	szName += GetCompany();
	szName += "'";

	PESetFormula (JobNumber,"Company Name",(char *)szName.c_str());

  if(!PESetFormula (JobNumber,"Batchno",szBatchNo))
	{
	 HANDLE textHandle;
	 short  iTextLen;
	 PEGetErrorText(JobNumber,&textHandle,&iTextLen);
	 char   *szErrorText = new char[iTextLen];
	 PEGetHandleString(textHandle,szErrorText,iTextLen);
	 MessageBox(GetFocus(),szErrorText,szApp,MB_ICONINFORMATION);
	 delete szErrorText;
	 PEClosePrintJob(JobNumber);
	 return ;
	}
  PEOutputToPrinter(JobNumber,1);
  PEStartPrintJob(JobNumber,TRUE);
  PEClosePrintJob(JobNumber);
  HourGlassOff();
}

void TAPDistList::SetField(int ID,Field4 *field)
{
  SendDlgItemMessage(Parent->HWindow,ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field->str());
  return ;
}

void TAPDistList::SetField(int ID, long field)
{
  IntSetDlgItemValue(Parent->HWindow,ID,field);
  return;
}

void TAPDistList::SetField(int ID, double field)
{
  FloatSetDlgItemValue(Parent->HWindow,ID,field);
  return;
}

void TAPDistList::SetField(int ID, char *field)
{
  SendDlgItemMessage(Parent->HWindow,ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field);
  return;
}

void TAPDistList::SetupWindow()
{
 TWindow::SetupWindow();
 RECT rc;
 HourGlassOn();
 SetListSize(8);

 List->GetColor(SS_ALLCOLS,SS_ALLROWS,&lpBack,&lpFore);

 List->SetMaxCols(6);
 List->GetClientRect(&rc);
 iLeft = iTop = iRight = iBottom = 0;
 string szClassName(GetClassName());
 iLeft    = GetPrivateProfileInt(szClassName.c_str(),"LEFT",0,PROFILE);
 iTop     = GetPrivateProfileInt(szClassName.c_str(),"TOP",0,PROFILE);
 iRight   = GetPrivateProfileInt(szClassName.c_str(),"RIGHT",0,PROFILE);
 iBottom  = GetPrivateProfileInt(szClassName.c_str(),"BOTTOM",0,PROFILE);
 if(iTop == 0 || iBottom == 0 || iLeft == 0 || iRight == 0)
 {
	iBottom = 100;
 }
 SetPosition();

 List->SetColWidth(1,3);
 List->SetColWidth(3,6);
 List->SetColWidth(4,5);
 List->SetColWidth(5,14);
 List->SetColWidth(6,14);

 List->GetColWidthInPixels(1,(LPSHORT)&d0Width);
 List->GetColWidthInPixels(3,(LPSHORT)&d2Width);
 List->GetColWidthInPixels(4,(LPSHORT)&d3Width);
 List->GetColWidthInPixels(5,(LPSHORT)&d4Width);
 List->GetColWidthInPixels(6,(LPSHORT)&d5Width);

 short ColWidth = (rc.right - rc.left)
		  - d0Width
		  - d2Width
		  - d3Width
		  - d4Width
		  - d5Width;


 List->SetColWidthInPixels(2,ColWidth);
 List->SetData(1,SS_HEADER, GetString(GetApplication()->hInstance,6067));
 List->SetData(2,SS_HEADER, GetString(GetApplication()->hInstance,6044));
 List->SetData(3,SS_HEADER, GetString(GetApplication()->hInstance,6068));
 List->SetData(4,SS_HEADER, GetString(GetApplication()->hInstance,6054));
 List->SetData(5,SS_HEADER, GetString(GetApplication()->hInstance,6069));
 List->SetData(6,SS_HEADER, GetString(GetApplication()->hInstance,6070));

 List->SetTypeEdit(&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 List->SetCellType(1, SS_ALLROWS, &CellType);
 List->SetCellType(2, SS_ALLROWS, &CellType);
 List->SetTypeEdit(&CellType,ES_RIGHT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 List->SetCellType(3, SS_ALLROWS, &CellType);
 List->SetCellType(4, SS_ALLROWS, &CellType);
 List->SetTypeFloat(&CellType,FS_SEPARATOR,15,2,0,9999999999999.99);
 List->SetCellType(5, SS_ALLROWS, &CellType);
 List->SetCellType(6, SS_ALLROWS, &CellType);

 HourGlassOff();
}

void TAPDistList::SetListSize (int Rows)
{

 LOGFONT lfFont;
 HFONT   hFont;

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
 strcpy (lfFont.lfFaceName, "HELV");
 hFont = CreateFontIndirect((LPLOGFONT) &lfFont);
 List->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 List->SetBool(SSB_AUTOSIZE,FALSE);
 List->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
 List->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 List->SetBool(SSB_NOBEEP,TRUE);
 List->SetBool(SSB_ALLOWUSERRESIZE,TRUE);
 List->SetBool(SSB_PROTECT,TRUE);
 List->SetBool(SSB_SHOWROWHEADERS,FALSE);
 List->SetBool(SSB_NOBEEP,TRUE);
 List->SetAutoSizeVisible(6,Rows);
}

void TAPDistList::SelectCell (RTMessage Msg)
{
  LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
  unsigned int nIndx = (unsigned int) Cell->Row;
  apbtd->SetIndex(GetString(GetApplication()->hInstance,8140));
  int iBatchno;
  IntGetValue(GetDlgItem(Parent->HWindow,ID_BATCHNO),(LPLONG)&iBatchno);
  apbtd->iCurrentRecord = 0;
  if(iBatchno > 0){
	if(apbtd->seek(iBatchno) == 0){
	  while(iBatchno == apbtd->lBatchno()){
		 if(nIndx == apbtd->iJournal()) {
			apbtd->iCurrentRecord = (int) apbtd->recNo();
			break;
		 }
		 apbtd->skip();
	 }
  }
 }
 if(apbtd->iCurrentRecord == 0)
	{
	 SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	 return;
	}
  List->SetColor(SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
  List->SetColor(SS_ALLCOLS,nIndx,RGB(192,220,192), RGBCOLOR_BLUE);
  SendMessage(Parent->HWindow,WM_COMMAND,CM_LINEITEM,NULL);
  return;
}

void TAPDistList::WMSize(RTMessage Msg)
{
  TWindow::WMSize(Msg);
  RECT rc;
  SetWindowPos(List->HWindow, 0, 0,0,LOWORD(Msg.LParam), HIWORD(Msg.LParam), SWP_NOZORDER);
  List->GetClientRect(&rc);
  List->GetColWidthInPixels(1,(LPSHORT)&d0Width);
  List->GetColWidthInPixels(3,(LPSHORT)&d2Width);
  List->GetColWidthInPixels(4,(LPSHORT)&d3Width);
  List->GetColWidthInPixels(5,(LPSHORT)&d4Width);
  List->GetColWidthInPixels(6,(LPSHORT)&d5Width);
  short ColWidth = (rc.right - rc.left)
		  - d0Width
		  - d2Width
		  - d3Width
		  - d4Width
		  - d5Width;
  List->SetColWidthInPixels(2,ColWidth);

  short iHeight = 0;
  List->GetRowHeightInPixels(1,&iHeight);
  int cRows = (rc.bottom - rc.top) / iHeight + 1;
  if(iRows < (cRows))
	List->SetMaxRows(cRows);
  else
	List->SetMaxRows(iRows);
}



void TAPDistList::SavePosition()
{
 RECT rc;
 GetWindowRect(HWindow,&rc);
 Str4ten szLeft;
 Str4ten szTop;
 Str4ten szRight;
 Str4ten szBottom;
 szLeft.assignLong(rc.left,5);
 szTop.assignLong(rc.top,5);
 szRight.assignLong(rc.right,5);
 szBottom.assignLong(rc.bottom,5);
 string szClassName(GetClassName());
 WritePrivateProfileString(szClassName.c_str(),"LEFT",szLeft.str(),PROFILE);
 WritePrivateProfileString(szClassName.c_str(),"TOP",szTop.str(),PROFILE);
 WritePrivateProfileString(szClassName.c_str(),"RIGHT",szRight.str(),PROFILE);
 WritePrivateProfileString(szClassName.c_str(),"BOTTOM",szBottom.str(),PROFILE);
 return;
}

BOOL TAPDistList::SetPosition()
{
 RECT rc;
 GetWindowRect(HWindow,&rc);
 if(iRight == 0)
 iRight = rc.right;
 if(iBottom == 0)
 iBottom = rc.bottom;
 return MoveWindow(HWindow,iLeft,iTop,iRight-iLeft,iBottom-iTop,TRUE);
}

