#include "ap.h"
#include "db_dept.h"
#include "db_apbtc.h"
#include "db_cash.h"
#include "db_itemm.h"
#include "db_medrp.h"
#include "ap_class.h"
#include "ap_btch.h"
//#include "gl_setat.hpp"
#include "ap_cashb.h"
#include "utildlg.h"
#include "math.h"


#define ID_BANK 5012
#define ID_INVOICE 1030
#define ID_COMMIT  1056
#define  ID_VATACCT 1034
#define  ID_VATDEPT 1035
#define  ID_DRCODES 1098

static COLORREF lpBack, lpFore;
extern char szApp[20];


void TAPCashEntry::Print(RTMessage)
{
  if(apcash->seek(dGetField(ID_BATCHNO)) == 0)
	 PrintSlip(D->GetReportName(),(int)dGetField(ID_BATCHNO));
}

void TAPCashEntry::GetBankAccountDescription(RTMessage Msg)
{

 char *szBankAccount = new char[40];
 gl_setat.SetIndex("name");
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  long Indx = SendDlgItemMsg(ID_CASH,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(ID_CASH,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM) ((LPSTR) szBankAccount));
    if(gl_setat.seek(szBankAccount) == 0)
	 SetField(ID_APCONTROL,gl_setat.account->str());
	 SetField(ID_APDEPART,gl_setat.dept->str());
	}
 }
 delete [] szBankAccount;
}

void TAPCashEntry::RecoverInvoice()
{
 char szAccountNo[13];
 long dBatchNo;

 IntGetValue(GetDlgItem(HWindow,ID_BATCHNO),(LPLONG)&dBatchNo);

 if(apcash->seek(dBatchNo) == 0)
 {
	  apcash->mode->assign("1");
	  apcash->UpdateUserTime(szUser);
     apcash->flush();
     apcash->unlock();
 }
 gldist->SetIndex("apdist");
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

void TAPCashEntry::Cancel (RTMessage )
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



void TManilaGL::APCash   (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPCashEntry(this,1044));
 }

void TAPCashEntry::Top(RTMessage Msg)
{
 apcash->top();
 CurrentBatch(Msg);
 //ChangeSize(FALSE);
};
void TAPCashEntry::Bottom(RTMessage Msg)
{
 apcash->bottom();
 CurrentBatch(Msg);
 //ChangeSize(FALSE);
};
void TAPCashEntry::Next(RTMessage Msg)
{
 int iBatchno = 0;
 IntGetDlgItemValue(HWindow,ID_BATCHNO,(LPLONG)&iBatchno);
 if(apcash->seek(iBatchno) == 0)
 { 
	if(!apcash->eof())
	{
    apcash->skip();
	 if(apcash->eof())
    apcash->bottom();
	 CurrentBatch(Msg);
    //ChangeSize(FALSE);
   }
 }
 else
 return;

};

void TAPCashEntry::RollBack(RTMessage)
{
 HourGlassOn();
 apcash->SetIndex(GetString(8140));
 long lBatchno = lGetField(ID_BATCHNO);
 if(apcash->seek(lBatchno) == 0)
	apcash->deleteRec();

 if(apcashd->seek(lBatchno) == 0)
  while(apcashd->lBatchno() == lBatchno)
  {
	 apcashd->deleteRec();
	 apcashd->skip();
  }

 apcashd->pack();
 apcash->pack();
 HourGlassOff();
 return;
}


void TAPCashEntry::Previous(RTMessage Msg)
{
 int iBatchno = 0;
 IntGetDlgItemValue(HWindow,ID_BATCHNO,(LPLONG)&iBatchno);
 if(apcash->seek(iBatchno) == 0)
 {
   if(!apcash->bof())
	{
	 apcash->skip(-1);
	 CurrentBatch(Msg);
   }
   else if (apcash->eof())
	{
    apcash->skip(-1);
	 CurrentBatch(Msg);
   }
 }
 else
 return;
};

void TAPCashEntry::Delete(RTMessage Msg)
{
  int iEntryno;
  IntGetValue(GetDlgItem(HWindow,ID_LINENO),(LPLONG)&iEntryno);
  apcashd->SetIndex(GetString(8140));
  if(apcashd->seek(apcash->lBatchno()) == 0)
  {
   HourGlassOn();
	if(MessageBox(HWindow,GetString(6098),szApp,MB_YESNO | MB_ICONQUESTION) == IDYES)
	 {
	  while(apcashd->lBatchno() == apcash->lBatchno())
		{
		 if(apcashd->iJournal() == iEntryno)
		  {
			apcashd->deleteRec();
			apcashd->pack();
         break;
        }
		  apcashd->skip();
      }

    }
   HourGlassOff();
  }
  CurrentBatch(Msg);
  return;
};



/*
void TAPCashDist::Print (RTMessage)
{

  HourGlassOn();
  PEOpenEngine();
  char  szBatchNo[10];
  long iBatchNo;
  IntGetDlgItemValue(Parent->HWindow,ID_BATCHNO,(LPLONG)&iBatchNo);

  short JobNumber = PEOpenPrintJob("ca_slips.rpt");

  if (JobNumber == 0)
  {
	HANDLE textHandle;
	short  iTextLen;
	PEGetErrorText(JobNumber,&textHandle,&iTextLen);
	char   *szErrorText = new char[iTextLen];
	PEGetHandleString(textHandle,szErrorText,iTextLen);
	MessageBox(HWindow,szErrorText,szApp,MB_ICONINFORMATION);
	delete szErrorText;
	return;
  }

  itoa((unsigned)iBatchNo,szBatchNo,10);

  if(!PESetFormula (JobNumber,"Batchno",szBatchNo))
	{
	 HANDLE textHandle;
	 short  iTextLen;
	 PEGetErrorText(JobNumber,&textHandle,&iTextLen);
	 char   *szErrorText = new char[iTextLen];
	 PEGetHandleString(textHandle,szErrorText,iTextLen);
	 MessageBox(HWindow,szErrorText,szApp,MB_ICONINFORMATION);
	 delete szErrorText;
    PEClosePrintJob(JobNumber);
    return ;
	}
  char szCompanyName[40];
  wsprintf(szCompanyName,"'%s'",GetCompany());
  PESetFormula(JobNumber,"Company Name",szCompanyName);
  PEOutputToPrinter(JobNumber,1);
  PEStartPrintJob(JobNumber,TRUE);
  PEClosePrintJob(JobNumber);
  HourGlassOff();
}

*/



void TAPCashEntry::CurrentBatch(RTMessage)
{
  bModify = FALSE;

  D->List->SetBool(SSB_REDRAW,FALSE);
  double lDebitTotal,lCreditTotal;
  char szDueDate[11],szInvDate[11];
  lDebitTotal=lCreditTotal=0;
  if(apcash->recCount() == 0)
  {
	MessageBox(HWindow,GetString(8153),szApp,MB_ICONINFORMATION);
	return;
  }

  D->List->ClearData(SS_ALLCOLS,SS_ALLROWS);

  IntSetDlgItemValue(HWindow,ID_BATCHNO,int (Str4ptr(apcash->batchno->str())));
  SetField(ID_INVOICE,apcash->payable);
  SetField(ID_DESCRIPT,apcash->descript);
  SetField(ID_VENDORNO,apcash->vendorno);
  SetField(ID_VENDORNAME,apcash->vdname);
  SetField(ID_INVOICENO,apcash->invoiceno);
  SetField(ID_CURRBAL,apcash->orderno);
  SetField(ID_REFERENCE,apcash->reference);
  SetField(ID_APCONTROL,apcash->account);
  SetField(ID_APDEPART,apcash->depart);
  SetField(ID_CHEQUE,apcash->chequeno);

  InvoiceDate.assign(apcash->invdate->str(),"CCYYMMDD");
  InvoiceDate.format(szInvDate,GetString(4024));
  SetField(ID_INVDATE,szInvDate);

  DueDate.assign(apcash->duedate->str(),"CCYYMMDD");
  DueDate.format(szDueDate,GetString(4024));
  SetField(ID_DUEDATE,szDueDate);

  SetField(ID_TERMS,apcash->terms);
  SetField(ID_ACCOUNTS,"");
  SetField(ID_DEPART,"");
  SetField(ID_INVOICE,apcash->dPayable());

  long i=1;
  apcashd->SetIndex(GetString(8140));
  char *szLineNo = new char[3];
  apcashd->lockFile();

  if (apcashd->seek(double(Str4ptr(apcash->batchno->str()))) == 0)
  {
	while (apcash->lBatchno() == apcashd->lBatchno() && !apcash->eof() )
	 {
	  if(apcashd->dDebit() > 0)
	  {
		lDebitTotal = lDebitTotal + double(Str4ptr(apcashd->debit->str()));
		ltoa(i,szLineNo,10);
		D->List->SetData(1,i,szLineNo);
		D->List->SetData(2,i,apcashd->descript->str());
		D->List->SetData(3,i,apcashd->account->str());
		D->List->SetData(4,i,apcashd->dept->str());
		D->List->SetData(5,i,apcashd->debit->str());
		D->List->SetData(6,i,apcashd->credit->str());

		apcashd->entryno->assignLong(i);
		apcashd->flush();
		DebitTotal = lDebitTotal;
		i++;
	  }
	  apcashd->skip();
	}
  }

  //Set the credit amounts.
  if (apcashd->seek(apcash->lBatchno()) == 0)
  {
	while (apcash->lBatchno() == apcashd->lBatchno() && !apcash->eof() )
	 {
	  if(apcashd->dCredit() > 0)
	  {
		lCreditTotal = lCreditTotal + double(Str4ptr(apcashd->credit->str()));
		ltoa(i,szLineNo,10);
		D->List->SetData(1,i,szLineNo);
		D->List->SetData(2,i,apcashd->descript->str());
		D->List->SetData(3,i,apcashd->account->str());
		D->List->SetData(4,i,apcashd->dept->str());
		D->List->SetData(5,i,apcashd->debit->str());
		D->List->SetData(6,i,apcashd->credit->str());
		apcashd->entryno->assignLong(i);
		apcashd->flush();
		CreditTotal = lCreditTotal;
		i++;
	  }
	  apcashd->skip();
	}

  }
	apcashd->unlock();
	IntSetDlgItemValue(HWindow,ID_LINENO,i);
	apcashd->iCurrentRecord = 0;
	short iHeight = 0;
	RECT Drc;
	GetClientRect(D->HWindow,&Drc);
	D->List->GetRowHeightInPixels(1,&iHeight);
	int cRows = (Drc.bottom - Drc.top) / iHeight + 1;
	if(i < cRows)
	 D->iRows = cRows;
	else
	 D->iRows = i;

	Str4large szDebitTotal;
	Str4large szCreditTotal;
	szDebitTotal.assignDouble(lDebitTotal,16,2);
	szCreditTotal.assignDouble(lCreditTotal,16,2);
	IntSetDlgItemValue(HWindow,ID_LINENO,i);
	i++;
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

	D->List->SetColor(SS_ALLCOLS,SS_ALLCOLS,RGB(255,255,255),RGB(0,0,0));
	D->List->SetColor(5,i,RGB(192,220,192), RGBCOLOR_BLUE);
	D->List->SetColor(6,i,RGB(192,220,192), RGBCOLOR_BLUE);
	SetFocus(GetDlgItem(HWindow,ID_INVOICE));
	D->List->SetBool(SSB_REDRAW,TRUE);
}

BOOL TAPCashEntry::Closed(void)
{
 if(strcmp(GetField(IDOK),GetString(ID_RECOVER)) == 0)
  {
	if(MessageBox(HWindow,GetString(4111),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
	  {
		RecoverInvoice();
      SS_COORD Row,Col;
		SSGetActiveCell(GetDlgItem(SlipList->HWindow,ID_DISTLIST),&Col,&Row);
      SSSetColor(GetDlgItem(SlipList->HWindow,ID_DISTLIST),SS_ALLCOLS,Row,RGB(255,255,255),RGB(0,0,0));
     }
   return TRUE;
  }
 return FALSE;
}

BOOL TAPCashEntry::PreviousInvoice(void)
 {
  char *szInvoiceno = strcpy(new char[apcash->invoiceno->len()+ apcash->vendorno->len() + 1],GetField(ID_VENDORNO));
  apcash->SetIndex("invoiceno");
  strcat(szInvoiceno,GetField(ID_INVOICENO));
  if(apcash->seek(szInvoiceno) == 0)
	 {
       SetField(IDD_STATUSBOX,GetString(ID_INVOICECREATED));
       return TRUE;
    }
  else
  return FALSE;
 }




void TAPCashEntry::ModifyCurrentSlip()
{
	apcash->mode->assign("1");
	apcash->date->assign(InvoiceDate.str());
	apcash->period->assignDouble(GetPeriod(HWindow));
	apcash->orderno->assign(szOrderNo);
	apcash->vendorno->assign(szVendorNo);
	apcash->vdname->assign(GetField(ID_VENDORNAME));
	apcash->invdate->assign(InvoiceDate.str());
	apcash->reference->assign(GetField(ID_REFERENCE));
	apcash->payable->assignDouble(lInvoiceAmount);
	apcash->chequeno->assign(GetField(ID_CHEQUE));
	apcash->invoiceno->assign(GetField(ID_INVOICENO));
	apcash->vatpercent->assignDouble(dGetField(ID_VATPERCENTAGE));
	apcash->UpdateUserTime(szUser);
	apcash->flush();
	apcashd->go(apcashd->iCurrentRecord);
	apcashd->batchno->assignLong(iBatchno);
	apcashd->descript->assign(GetField(ID_JDESCRIPT));
	apcashd->account->assign(szAccounts);  // AP account is always set to the default.
	apcashd->dept->assign(szDepart);        // AP department code is set to the default dept code.
	apcashd->debit->assignDouble(dGetField(ID_DEBIT));
	apcashd->credit->assignDouble(dGetField(ID_CREDIT));
	apcashd->flush();
	apcashd->unlock();
	bSlipList = TRUE;
	SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	return;
}

/*
void TAPCashDist::SetupWindow()
{
 TDialog::SetupWindow();
 RECT rc;
 HourGlassOn();
 SetListSize(8);

 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,&lpBack,&lpFore);

 SSSetMaxCols(SHWindow, 6);
 SSGetClientRect(SHWindow,&rc);
 short d0Width,d2Width,d3Width,d4Width,d5Width;

 SSSetColWidth(SHWindow,1,3);
 SSSetColWidth(SHWindow,3,6);
 SSSetColWidth(SHWindow,4,5);
 SSSetColWidth(SHWindow,5,14);
 SSSetColWidth(SHWindow,6,14);

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
 //SSSetData(SHWindow,1,SS_HEADER, "Ln");
 //SSSetData(SHWindow,2,SS_HEADER, "Description");
 //SSSetData(SHWindow,3,SS_HEADER, "Acct");
 //SSSetData(SHWindow,4,SS_HEADER, "Dept");
 //SSSetData(SHWindow,5,SS_HEADER, "Debit");
 //SSSetData(SHWindow,6,SS_HEADER, "Credit");
 SSSetData(SHWindow,1,SS_HEADER, GetString(GetApplication()->hInstance,6067));
 SSSetData(SHWindow,2,SS_HEADER, GetString(GetApplication()->hInstance,6044));
 SSSetData(SHWindow,3,SS_HEADER, GetString(GetApplication()->hInstance,6068));
 SSSetData(SHWindow,4,SS_HEADER, GetString(GetApplication()->hInstance,6054));
 SSSetData(SHWindow,5,SS_HEADER, GetString(GetApplication()->hInstance,6069));
 SSSetData(SHWindow,6,SS_HEADER, GetString(GetApplication()->hInstance,6070));


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

*/






void TAPCashEntry::Sliplist() //slip list search
{
 SlipList->List->Seek(ID_BATCHNO);
 SlipList->Show(SW_SHOW);
}

void TAPCashEntry::Clear()
{
 apcash->bottom();
 int iBatchno = (int) apcash->lBatchno();
 iBatchno++;
 IntSetDlgItemValue(HWindow,ID_BATCHNO,iBatchno);
 IntSetDlgItemValue(HWindow,ID_LINENO,1);
 SetField(ID_DESCRIPT,"");
 SetField(ID_VENDORNO,"");
 SetField(ID_VENDORNAME,"");
 SetField(ID_INVOICENO,"");
 SetField(ID_CURRBAL,"");
 SetField(ID_REFERENCE,"");
 SetField(1031,"");
 SetField(ID_JDESCRIPT,"");
 SetField(ID_APDEPART,"");
 SetField(ID_ACCOUNTS,"");
 SetField(ID_DEPART,"");
 SetField(ID_APCONTROL,"");
 SetField(ID_APDEPART,"");
 SetField(ID_VATACCT,"");
 SetField(ID_VATDEPT,"");
 SetField(ID_DEBIT,"");
 SetField(ID_CREDIT,"");
 SetField(ID_INVOICE,"");

 SSClearData(GetDlgItem(D->HWindow,ID_DISTLIST),SS_ALLCOLS,SS_ALLROWS);
 SSSetColor(GetDlgItem(D->HWindow,ID_DISTLIST),SS_ALLCOLS,SS_ALLCOLS,RGB(255,255,255),RGB(0,0,0));

 long Indx = SendDlgItemMsg(ID_PRODUCTS,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_PRODUCTS,CB_SETCURSEL,(WORD)Indx,NULL);

 Indx = SendDlgItemMsg(ID_MEDREPS,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_MEDREPS,CB_SETCURSEL,(WORD)Indx,NULL);

 Indx = SendDlgItemMsg(ID_DRCODES,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_DRCODES,CB_SETCURSEL,Indx,NULL);

 DebitTotal=CreditTotal=0;
 SetFocus(GetDlgItem(HWindow,ID_VENDORNO));
 D->List->ClearData(SS_ALLCOLS,SS_ALLROWS);
 D->List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 bTransaction = FALSE;
}

void TAPCashEntry::New(RTMessage)
{
 if(bTransaction)
 {
  int rc = MessageBox(HWindow,GetString(ID_COMMIT),szApp,MB_YESNOCANCEL |MB_ICONQUESTION);
  if(rc == IDYES)
	Clear();
  else if (rc == IDNO)
  {
	SendMessage(HWindow,WM_COMMAND,CM_ROLLBACK,NULL);
	Clear();
  }
  else if (rc == IDCANCEL)
  return;
 }
 else
 Clear();
 return;

}

void TAPCashEntry::SetupWindow()
{
 TMainDialog::SetupWindow();
  if(!CheckFile(GetString(2053),Parent->HWindow) ||
	 !CheckFile(GetString(2054),Parent->HWindow) ||
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


 bModify = FALSE;
 bTransaction = FALSE;
 HourGlassOn();
 DebitTotal=0;
 CreditTotal=0;

 apcash =  new Tapcash(&cb);
 apcashd=  new Tapcashd(&cb);
 apvendor= new Tapvendor(&cb);
 glacct =  new Tglaccts(&cb);
 gldept =  new Tgldept(&cb);
 gldist =  new Tgldist(&cb);
 apdist =  new Tapdist(&cb);
 prods  =  new Tprods(&cb);
 medreps = new Tglmedreps(&cb);
 drcodes = new Tgldrcodes(&cb);

 LoadDoctors();
 LoadMedReps();
 LoadProducts();

 IntSetDlgItemRange(HWindow,4065,0,100);
 IntSetDlgItemRange(HWindow,4073,0,100);


 bAutomaticCalc = TRUE;
 SendDlgItemMessage(HWindow,ID_AUTO,BM_SETCHECK,TRUE,NULL);
 // Get the AP Control Accounts.


 gl_setat.open(cb);
 gl_setat.SetIndex("gl_accts");

 if(gl_setat.seek("WTXI") == 0)
 {
  strcpy(szIndTaxAccount,gl_setat.account->str());
  strcpy(szIndTaxDept,gl_setat.dept->str());
 }
 else
 {
  strcpy(szIndTaxAccount,"");
  strcpy(szIndTaxDept,"");
 }

 if(gl_setat.seek("WTXC") == 0)
 {
  strcpy(szCompanyTaxAccount,gl_setat.account->str());
  strcpy(szCompanyTaxDept,gl_setat.dept->str());
 }
 else
 {
	strcpy(szCompanyTaxAccount,"");
	strcpy(szCompanyTaxDept,"");
 }

 if(gl_setat.seek("VATP") == 0)
 {
  strcpy(szVATAccount,gl_setat.account->str());
  strcpy(szVATDept,gl_setat.dept->str());
 }
 else
 {
  strcpy(szVATAccount,"");
  strcpy(szVATDept,"");

 }

 gl_setat.SetIndex("cash");

 for (gl_setat.top();!gl_setat.eof();gl_setat.skip())
  {
	SendDlgItemMsg(ID_CASH,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)gl_setat.name->str());
  }

 VendorList = new TCustomerList(this,"",apvendor);
 VendorList->EnableAutoCreate();
 GetApplication()->MakeWindow(VendorList);

 AcctList = new TAccountList(this,"",glacct);
 AcctList->EnableAutoCreate();
 GetApplication()->MakeWindow(AcctList);

 SlipList  = new TCashList(this,"",apcash);
 SlipList->EnableAutoCreate();
 GetApplication()->MakeWindow(SlipList);


 D = new TAPCashList(this,GetString(8156),apcashd);
 D->EnableAutoCreate();
 GetApplication()->MakeWindow(D);

 
 int pos = 20;
 GetWindowRect(HWindow,&rc);
 GetWindowRect(Parent->HWindow,&parent);
 ChangeSize(FALSE);
 GetWindowRect(HWindow,&rc);
 GetWindowRect(D->HWindow,&child);
 MoveWindow(D->HWindow,rc.left+pos,rc.bottom+pos,child.right-child.left,child.bottom-child.top,TRUE);

 long db=0;
 apcash->bottom();
 db = apcash->lBatchno();
 db++;
 IntSetDlgItemValue(HWindow,ID_BATCHNO,db);
 //IntSetDlgItemValue(HWindow,3032,GetCurrentPeriodn());
 SetPeriods(HWindow);

 IntSetDlgItemValue(HWindow,ID_LINENO,1);

 D->Show(SW_SHOW);
 FloatSetMask(GetDlgItem(HWindow,ID_CREDIT),"999999999.99");

 SendDlgItemMsg(ID_DEPART,WM_SETTEXT,NULL,(LONG)(LPSTR)"");
 SetFocus(GetDlgItem(HWindow,ID_DESCRIPT));

 SetBitmap(IDCANCEL);
 SetBitmap(ID_NEW);
 SetBitmap(IDOK);
 SetBitmap(ID_NEW);
 SetBitmap(ID_VIEW);
 SetBitmap(ID_SEARCHCUST);
 SetBitmap(ID_SLIPLIST);
 SetBitmap(ID_PRINT);
 HourGlassOff();

}

void TAPInvoiceEntry::ChangeSize(BOOL bFull)
{
 int pos = 50;
 /*
 if(bFull)
  {
	if(!bFullSize)
	 {
	  bFullSize = TRUE;
	  GetWindowRect(HWindow,&rc);
	  GetWindowRect(Parent->HWindow,&parent);
	  MoveWindow(HWindow,parent.left+pos,parent.top+pos,rc.right-rc.left,rc.bottom-rc.top+pos,TRUE);
	 }
	return;
  }
 else
  {
  if(bFullSize)
	{
 */
	 bFullSize = FALSE;
	 GetWindowRect(HWindow,&rc);
	 GetWindowRect(Parent->HWindow,&parent);
	 MoveWindow(HWindow,parent.left+pos,parent.top+pos,rc.right-rc.left,rc.bottom-rc.top,TRUE);
}



void TAPCashEntry::SetLineItem(RTMessage)
{
  bModify = TRUE;

  SetFocus(GetDlgItem(HWindow,IDOK));
  SetField(ID_JDESCRIPT,apcashd->descript);
  SetField(ID_ACCOUNTS ,apcashd->account);
  SetField(ID_DEPART   ,apcashd->dept);
  SetField(ID_DEBIT    ,apcashd->debit);
  SetField(ID_CREDIT   ,apcashd->credit);
  SetField(ID_LINENO   ,apcashd->entryno);
  //ChangeSize(TRUE);
}

void TAPCashList::SetupWindow()
{
 TWindow::SetupWindow();
 RECT rc;
 HourGlassOn();
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
 StrCpy (lfFont.lfFaceName, "HELV");
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
void TAPCashList::SelectCell (RTMessage Msg)
{
  LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
  unsigned int nIndx = (unsigned int) Cell->Row;
  apcashd->SetIndex(GetString(GetApplication()->hInstance,8140));
  int iBatchno;
  IntGetValue(GetDlgItem(Parent->HWindow,ID_BATCHNO),(LPLONG)&iBatchno);
  HWND hWin = GetDlgItem(HWindow,ID_DISTLIST);
  apcashd->iCurrentRecord = 0;
  if(iBatchno > 0){
	if(apcashd->seek(iBatchno) == 0){
	  while(iBatchno == apcashd->lBatchno()){
		 if(nIndx == apcashd->iJournal()){
		  apcashd->iCurrentRecord = (int) apcashd->recNo();
		  break;
		 }
		  apcashd->skip();
	  }
	 }
  }
  if(apcashd->iCurrentRecord == 0)
	{
	 //List->SetColor(hWin,SS_ALLCOLS,SS_ALLCOLS,RGB(255,255,255),RGB(0,0,0));
	 SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	 return;
	}
  List->SetColor(SS_ALLCOLS,SS_ALLCOLS,RGB(255,255,255),RGB(0,0,0));
  List->SetColor(SS_ALLCOLS,nIndx,RGB(192,220,192), RGBCOLOR_BLUE);
  SendMessage(Parent->HWindow,WM_COMMAND,CM_LINEITEM,NULL);
  return;

}

void TAPCashBatch::SetupWindow()
 {
 TDialog::SetupWindow();
 HourGlassOn();
 RECT rc;
 SS_CELLTYPE CellType;

 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);

 SetListSize(8);

 GetClientRect(HWindow,&rc);
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);
 SSSetColWidth(SHWindow,1,5);
 SSSetColWidth(SHWindow,2,10);
 short d1Width,d2Width;
 SSSetMaxCols(SHWindow,3);
 SSGetColWidthInPixels(SHWindow,1,(LPSHORT)&d1Width);
 SSGetColWidthInPixels(SHWindow,2,(LPSHORT)&d2Width);
 short ColWidth = (rc.right - rc.left) - d1Width - d2Width - GetSystemMetrics(SM_CXHSCROLL);
 SSSetColWidthInPixels(SHWindow,3,ColWidth);
 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,SS_ALLCOLS, SS_ALLROWS, &CellType);

 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_CENTER | SS_TEXT_SHADOW);
 SSSetCellType(SHWindow,SS_HEADER, SS_ALLROWS, &CellType);


 apcash->SetIndex("recNo");
 if(apcash->recCount()>10)
 SSSetMaxRows(SHWindow,apcash->recCount());
 else 
 SSSetMaxRows(SHWindow,10);
 SSSetData(SHWindow,1,SS_HEADER,"Slip");
 SSSetData(SHWindow,2,SS_HEADER,"Inv. No.");
 SSSetData(SHWindow,3,SS_HEADER,GetString(ID_SLIP_DESCRIPTION));

 Relate4set *rQuery = new Relate4set(apcash);
 //rQuery->querySet("MODE = '1' .OR. MODE = '0'");
 rQuery->querySet("MODE = '1'");
 int i = 1;
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
 {
  SSSetData(SHWindow,1,i,StrTrim(apcash->batchno->str()));
  SSSetData(SHWindow,2,i,StrTrim(apcash->invoiceno->str()));
  SSSetData(SHWindow,3,i,apcash->reference->str());
  if(strcmp(apcash->mode->str(),"0") == 0)
  SSSetColor(SHWindow,SS_ALLCOLS,i,RGB(255,255,130), RGBCOLOR_BLUE);
  i++;
 }
 rQuery->unlock();
 rQuery->free();
 HourGlassOff();
}



void TAPCashBatch::HandleListBox (RTMessage Msg)
{
 char *szBuffer = new char[apcash->batchno->len()];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);
 apcash->SetIndex(GetString(8140));
 if(apcash->seek(lRecord)==0)
 {
  SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
 }
 delete szBuffer;
}



void TAPCashEntry::InvoiceData()
{
}

/*
void TAPCashDist::SetListSize (int Rows)
{
 RECT rc,wd;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 short iHeight;
 LOGFONT lfFont;
 HFONT   hFont;
 //lfFont.lfHeight          = -11;
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
}
*/

void TAPCashEntry::Ok (RTMessage Msg )
{
 if(Closed())
  return;

 if(strlen(StrTrim(GetField(ID_VENDORNO)))== 0)
	{
	  VList(Msg);
	  return;
	}

 if(apvendor->seek(GetField(ID_VENDORNO))!=0)
  {
	MessageBeep(MB_ICONEXCLAMATION);
	if(MessageBox(HWindow,GetString(6029),szApp,MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
	 D->Show(SW_HIDE);
	 AddVendor(Msg);
	 D->Show(SW_SHOW);
	 SetFocus(GetDlgItem(HWindow,ID_VENDORNO));
	}
	return;
  }



 szAdj = strcpy(new char[apcash->adj->len() + 1],"N");
 SendDlgItemMsg(ID_DISTLIST, WM_SETREDRAW, FALSE, 0L);
 iPeriodNo = (int)lGetField(ID_PERIOD);
 iBatchno  = (int) lGetField(ID_BATCHNO);

 // Check to see if the invoice has been entered previously.
 /*

 if(dGetField(ID_BATCHNO) == 0)
 if(PreviousInvoice())
  {
	CurrentBatch(Msg);
	return;
  }
*/


 // Check to see if the batch number matches the invoice number.
 /*
 if(lGetField(ID_BATCHNO) > 0 && strcmp(GetField(IDOK),GetString(ID_MODIFY)) != 0)
 {
  apcash->SetIndex("invoiceno");
  char *szInvoiceno = strcpy(new char[apcash->invoiceno->len()+ apcash->vendorno->len() + 1],GetField(ID_VENDORNO));
  strcat(szInvoiceno,GetField(ID_INVOICENO));
  if(apcash->seek(szInvoiceno) == 0 )
  {
	 if(DlgMsg(ID_INVOICECREATED,ID_ADJUSTMENT,MB_YESNO | MB_ICONQUESTION) == IDNO)
	 {

	  CurrentBatch(Msg);
     return;
	 }
  }
 }
 */


 szVendorNo = strcpy(new char[apcash->vendorno->len() + 1],GetField(ID_VENDORNO));
 if(*szVendorNo==0)
	{
	 CheckError(ID_VENDORNO);
	 return;
	}

 if(strlen(StrTrim(GetField(ID_APCONTROL))) == 0)
  {
	 CheckError(ID_APCONTROL);
	 SetFocus(GetDlgItem(HWindow,ID_BANK));
	 return;
  }




 szInvoiceNo = strcpy(new char[apcash->invoiceno->len() + 1], GetField(ID_INVOICENO));
 if(strlen(StrTrim(szInvoiceNo)) == 0)
  {
	CheckError(ID_INVOICENO);
	return;
  }

 lInvoiceAmount = dGetField(ID_INVOICE);
 if(lInvoiceAmount == 0 ){CheckError(ID_INVOICE);return;}

 szInvoiceDate = strcpy(new char[apcash->date->len() + 1],GetField(ID_INVDATE));
 if(*szInvoiceDate==0)
  {
	CheckError(ID_INVDATE);
	return;
  }
 InvoiceDate.assign(szInvoiceDate,GetString(4024));

 DueDate.assign(szInvoiceDate,GetString(4024));

 dTerms = dGetField(ID_TERMS);
 long lDueDate = long (DueDate) + dTerms;
 DueDate.assign(lDueDate);

 szOrderNo = strcpy(new char[apcash->orderno->len() + 1],GetField(ID_CURRBAL));

 if(strlen(StrTrim(GetField(ID_JDESCRIPT))) == 0)
 {
  SearchGL(Msg);
  return;
 }

 
 if(strlen(StrTrim(GetField(ID_REFERENCE)))==0)
 {
   CheckError(ID_REFERENCE);
	return;
 }



 if(*StrTrim(GetField(ID_ACCOUNTS)) == 0)
 {
  SearchGL(Msg);
  return;
 }

 if(strcmp(GetField(ID_ACCOUNTS),GetField(ID_APCONTROL)) == 0)
  {
	MessageBox(HWindow,GetString(8158),szApp,MB_OK);
	return;
  }


 szDepart = strcpy(new char[apcash->depart->len() + 1],GetField(ID_DEPART));
 szAccounts = strcpy(new char[apcash->account->len() + 1],GetField(ID_ACCOUNTS));
 char *szSeek = new char[apcash->account->len() + apcash->depart->len() + 1];
 strcpy(szSeek,szAccounts);
 strcat(szSeek,szDepart);
 if(*szAccounts==0){CheckError(ID_ACCOUNTS);return;}
 if(!CheckGLAccount(szSeek)){SearchGL(Msg);SetFocus(GetDlgItem(HWindow,ID_ACCOUNTS));return;}



 iLineno = lGetField(ID_LINENO);


 dVatPercentage = dGetField(ID_VATPERCENTAGE);

 if(strlen(StrTrim(GetField(ID_VATACCT))) == 0 && dVatPercentage > 0)
  {
	 MessageBox(HWindow,GetString(6097),szApp,MB_OK);
	 SetFocus(GetDlgItem(HWindow,ID_VATACCT));
	 return;
  }

 dRebatePercentage = dGetField(ID_REBATE);

 apcash->SetIndex(GetString(8140));

 if(bModify)
	{
	 ModifyCurrentSlip();
	 bTransaction = TRUE;
	}
 else
	{
	 AddToCurrentSlip(iBatchno);
	 bTransaction = TRUE;
	}
 CurrentBatch(Msg);
}

void TAPCashEntry::GetCashExpense(AP_EXPENSE* ex,
											 double lInvoiceAmount,
											 double dVatPercentage,
											 double dRebatePercentage)
{
	ex->REBATE  = 0.0;
	ex->VAT     = 0.0;
	ex->NEWAP      = apcash->dPayable() + lInvoiceAmount;
	ex->OLDAP      = apcash->dPayable();
	ex->APAMT      = lInvoiceAmount;

	if(!GetWithHolding())
	{
	 ex->OLDEXPENSE = ex->OLDAP - apcash->dVatamt();
	 if(dVatPercentage != 1)
	  ex->VAT  = ex->NEWAP - (floor(ex->NEWAP/(1+dVatPercentage)*1000)/1000);
	 ex->NEWEXPENSE  = ex->NEWAP - ex->OLDEXPENSE - ex->VAT;

	 if(dRebatePercentage != 1)
		 ex->REBATE  = -1*((ex->NEWAP)*dRebatePercentage);

	 ex->APAMT = ex->NEWAP + ex->REBATE;
	 ex->NEWAP = ex->APAMT;
	}
	else
	{
	 ex->OLDEXPENSE = ex->OLDAP - apcash->dVatamt() - apcash->dRebateamt();
	 double dVatRebateTotal = 1;
	 if(dRebatePercentage != 1)
		  dVatRebateTotal += dRebatePercentage;
	 if(dVatPercentage != 1)
		  dVatRebateTotal += dVatPercentage;

	 ex->NEWEXPENSE = lInvoiceAmount/dVatRebateTotal;
	 ex->VAT        = (floor(ex->NEWEXPENSE * dVatPercentage*1000)/1000) + apcash->dVatamt();
	 ex->REBATE     = ex->NEWAP - ex->VAT - ex->NEWEXPENSE - ex->OLDEXPENSE;
	}

}


void TAPCashEntry::AddToCurrentSlip(long iBatchno)
{
	apcash->SetIndex(GetString(8140));
	apcashd->SetIndex(GetString(8140));

	char *szSeekAccount  = new char [apcash->account->len() + apcash->depart->len() + 1];
	char *szRebateAccount = new char [apcash->account->len()+1];
	char *szRebateDepart  = new char [apcash->depart->len()+1];

	if(dGetField(ID_REBATE) > 0)
	{
	 if(SendDlgItemMsg(ID_INDIVIDUAL,BM_GETSTATE,NULL,NULL))
	  {
		strcpy(szSeekAccount,szIndTaxAccount);
		strcat(szSeekAccount,szIndTaxDept);
		strcpy(szRebateAccount,szIndTaxAccount);
		strcpy(szRebateDepart,szIndTaxDept);
	  }
	 else if(SendDlgItemMsg(ID_COMPANY,BM_GETSTATE,NULL,NULL))
	 {
	  strcpy(szSeekAccount,szCompanyTaxAccount);
	  strcat(szSeekAccount,szCompanyTaxDept);
	  strcpy(szRebateAccount,szCompanyTaxAccount);
	  strcpy(szRebateDepart,szCompanyTaxDept);
	 }
	 else if(!SendDlgItemMsg(ID_COMPANY,BM_GETSTATE,NULL,NULL) && !SendDlgItemMsg(ID_INDIVIDUAL,BM_GETSTATE,NULL,NULL))
	 {
	  MessageBox(HWindow,GetString(4101),szApp,MB_ICONEXCLAMATION);
	  delete [] szSeekAccount;
	  delete [] szRebateAccount;
	  delete [] szRebateDepart;
	  return;
	 }
	}



	if(dGetField(ID_REBATE) > 0)
		 dRebatePercentage = dGetField(ID_REBATE)/100;
	else
		 dRebatePercentage = 1;


	if(dGetField(ID_VATPERCENTAGE) > 0)
		{
		 dVatPercentage = dGetField(ID_VATPERCENTAGE)/100;
		}
	else
		dVatPercentage = 1;



	AP_EXPENSE* expense = new AP_EXPENSE;

	lInvoiceAmount = dGetField(ID_INVOICE);




	BOOL bNewBatch = FALSE;
	if(apcash->seek(iBatchno) != 0)
	 bNewBatch = TRUE;

	Date4 Today;
	Today.today();

	if(bNewBatch)
	{
	apcash->appendStart();
	apcash->batchno->assignLong(iBatchno);
	apcash->mode->assign("1");
	apcash->adj->assign("N");
	apcash->date->assign(Today.str());
	}

	GetCashExpense(expense,lInvoiceAmount,dVatPercentage,dRebatePercentage);

	/*
	  IF you use the same account for the expenses as you do for the bank
	  account the total will come out wrong.
	*/


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
	apcash->entryno->assignLong(lGetField(ID_LINENO));
	apcash->UpdateUserTime(szUser);
	apcash->payable->assignDouble(lInvoiceAmount + apcash->dPayable());
	apcash->vatamt->assignDouble(expense->VAT);
	apcash->rebateamt->assignDouble(expense->REBATE);

	if(bNewBatch)
	 apcash->append();
	else
	 apcash->flush();

	apcash->unlock();
	char szDept[5];

	if(expense->VAT > 0)
	 {
	  strcpy(szDept,GetField(ID_VATDEPT));
	  UpdateBatchDetail(iBatchno,"",GetField(ID_VATACCT),szDept,expense->VAT);
	 }

	if(fabs(expense->REBATE) > 0)
	  UpdateBatchDetail(iBatchno,"",szRebateAccount,szRebateDepart,expense->REBATE);


	// Update AP amount
	strcpy(szDept,GetField(ID_APDEPART));
	UpdateBatchDetail(iBatchno,"",GetField(ID_APCONTROL),szDept,-1*expense->NEWAP);

	// Add new expense

	strcpy(szDept,GetField(ID_DEPART));
	AddDetail(iBatchno,"",GetField(ID_ACCOUNTS),szDept,expense->NEWEXPENSE);

	delete [] szSeekAccount;
	delete [] szRebateAccount;
	delete [] szRebateDepart;
	delete [] expense;
	return;
}

void TAPCashEntry::UpdateBatchDetail(long iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry)
{
 char szReference[41];
 if(strcmp(szDescript,"") == 0)
	{
	 char *szSeek = new char[12];
	 strcpy(szSeek,szAccount);
	 glacct->SetIndex(GetString(8145));
	 glacct->seek(strcat(szSeek,szDepart));
	 strcpy(szReference,glacct->descript->str());
	 delete szSeek;
	 }
  else
  strcpy(szReference,szDescript);

 apcashd->SetIndex(GetString(8140));
 if(apcashd->seek(iBatchno) == 0)
 {
		// If the entry for that Account and Department code has already been entered just update it.
		int iEntryno = 1;
	  while(iBatchno == apcashd->lBatchno())
		  {
			if(strcmp(StrTrim(szAccount),StrTrim(apcashd->account->str())) == 0
			  && strcmp(StrTrim(szDepart),StrTrim(apcashd->dept->str())) == 0 )
				{
				  apcashd->descript->assign(szReference);
				  if(dEntry > 0)
				  {
					 apcashd->debit->assignDouble(fabs(dEntry));
					 apcashd->credit->assignDouble(0.00);
				  }
					else if (dEntry < 0)
				  {
					 apcashd->debit->assignDouble(0.00);
					 apcashd->credit->assignDouble(fabs(dEntry));
				  }
					 apcashd->flush();
					 apcashd->unlock();
					 return;
				 }
			apcashd->skip();
			iEntryno++;
		  }
	  // if not found create a new line entry.
	  apcashd->appendStart();
	  apcashd->batchno->assignLong(iBatchno);
	  apcashd->descript->assign(szReference);
	  apcashd->account->assign(szAccount);
	  apcashd->dept->assign(szDepart);
	  apcashd->product->assign(GetProduct());
	  apcashd->medreps->assign(GetMedRep());
	  apcashd->drcodes->assign(GetDoctor());

	  if(dEntry < 0)
		{
		 apcashd->debit->assignDouble(0.00);
		 apcashd->credit->assignDouble(fabs(dEntry));
		}
	  else if (dEntry > 0)
		{
		 apcashd->debit->assignDouble(fabs(dEntry));
		 apcashd->credit->assignDouble(0.00);
		}
		apcashd->append();
		apcashd->unlock();
  }
 else
 {
	  apcashd->appendStart();
	  apcashd->batchno->assignLong(iBatchno);
	  apcashd->descript->assign(szReference);
	  apcashd->account->assign(szAccount);
	  apcashd->dept->assign(szDepart);
	  apcashd->product->assign(GetProduct());
	  apcashd->medreps->assign(GetMedRep());
	  apcashd->drcodes->assign(GetDoctor());

	  if(dEntry < 0)
		{
		 apcashd->debit->assignDouble(0.00);
		 apcashd->credit->assignDouble(fabs(dEntry));
		}
	  else if (dEntry > 0)
		{
		 apcashd->debit->assignDouble(fabs(dEntry));
		 apcashd->credit->assignDouble(0.00);
		}
	  apcashd->append();
	  apcashd->unlock();
 }
 return;
}


void TAPCashEntry::AddDetail(long iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry)
{
 Str4large szReference;
 szReference.setLen(51);
 if(strcmp(StrTrim(szDescript),"") == 0)
   {
    char *szSeek = new char[12];
    strcpy(szSeek,szAccount);
    strcat(szSeek,szDepart);
    glacct->SetIndex("accounts");
    if(glacct->seek(szSeek) == 0)
     szReference.assign(glacct->descript->str());
    delete  [] szSeek;
    }
  else
  {
    szReference.assign(szDescript);
  }

 apcashd->appendStart();
 apcashd->batchno->assignLong(iBatchno);
 apcashd->descript->assign(szReference.str());
 apcashd->account->assign(szAccount);
 apcashd->dept->assign(szDepart);
 apcashd->product->assign(GetProduct());
 apcashd->medreps->assign(GetMedRep());
 apcashd->drcodes->assign(GetDoctor());

 if(dEntry < 0)
   {
    apcashd->debit->assignDouble(0.00);
    apcashd->credit->assignDouble(fabs(dEntry));
	}
 else if (dEntry > 0)
   {
    apcashd->debit->assignDouble(fabs(dEntry));
    apcashd->credit->assignDouble(0.00);
   }
 apcashd->append();
 apcashd->unlock(); 
 return;
}



	 /*
	 if(dVatPercentage > 0 && dRebatePercentage == 0)
	  {
		CreateVatSlip(apcash,apcashd,D);
	  }
	 else if(dVatPercentage > 0 && dRebatePercentage > 0)
	  CreateVat_RebateSlip(apcash,apcashd,D);
	 else if (dVatPercentage == 0 && dRebatePercentage > 0)
	  CreateRebateSlip(apcash,apcashd,D);
	 else
	  CreateNewApSlip(apcash,apcashd,D);
	  SetField(IDD_STATUSBOX,"");
	  bTransaction = TRUE;
	  return;
	}
  else if (!bModify)
	{
	  if(dGetField(ID_INVOICE) == 0)
		{
		  SetFocus(GetDlgItem(HWindow,ID_INVOICE));
		  return;
		}
	  AddToCurrentSlip(iBatchno);
	  SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	  return;
	}
  */

