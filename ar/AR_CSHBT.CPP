#include "ar.h"
#include "db_dept.h"
#include "db_apbtc.h"
#include "db_cash.h"
#include "ar_class.h"
#include "ar_btch.h"
#include "ar_cashb.h"

#define ID_BANK 5012
#define ID_INVOICE 1030

void TAPCashEntry::GetBankAccountDescription(RTMessage Msg)
{
 DWORD Indx;
 char *szBankAccount = new char[20];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  Indx = SendDlgItemMsg(ID_CASH,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
   {
    SendDlgItemMsg(ID_CASH,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM) ((LPSTR) szBankAccount));
    apsetup->SetIndex("cash");
    apsetup->seek(szBankAccount);
    SetField(ID_BANK,apsetup->name1);
    SetFocus(HWindow);
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


static COLORREF lpBack, lpFore;

void TGlWindow::APCash   (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPCashEntry(this,"AR_CASH"));
 }

void TAPCashEntry::Top(RTMessage Msg)
{
 apcash->top();
 CurrentBatch(Msg);
};
void TAPCashEntry::Bottom(RTMessage Msg)
{
 apcash->bottom();
 CurrentBatch(Msg);
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
   }
 }
 else
 return;

};
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
  apcashd->SetIndex("batchno");
  if(apcashd->seek(apcash->lBatchno()) == 0)
  {
   HourGlassOn();
   if(DlgMsg(ID_DELETE,ID_BATCHNO,MB_YESNO | MB_ICONQUESTION) == IDYES)
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




void TAPCashDist::Print (RTMessage)
{
  HourGlassOn(); 
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
   MessageBox(HWindow,szErrorText,"AP Slip",MB_ICONINFORMATION);
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
    MessageBox(HWindow,szErrorText,"AP Slip",MB_ICONINFORMATION);
    delete szErrorText; 
    PEClosePrintJob(JobNumber);
    return ;
   }
  PEOutputToPrinter(JobNumber,1);
  PEStartPrintJob(JobNumber,TRUE);
  PEClosePrintJob(JobNumber);
  HourGlassOff();
}





void TAPCashEntry::CurrentBatch(RTMessage)
{
  SetField(IDOK,GetString(IDOK));
  double lDebitTotal,lCreditTotal,lDifference;
  char szDueDate[11],szInvDate[11];
  lDebitTotal=lCreditTotal=lDifference=0;
  if(apcash->recCount() == 0)
  {
   DlgMsg(ID_NOBATCH,ID_SLIP,MB_ICONINFORMATION);
   return;
  }

  HWND List = GetDlgItem(D->HWindow,ID_DISTLIST);
  SSSetBool(List,SSB_REDRAW,FALSE);

  SSClearData(List,SS_ALLCOLS,SS_ALLROWS);
  delete D->ListArray;
  
  D->ListArray = new TListArray(1000, 0 ,50);
  IntSetDlgItemValue(HWindow,ID_BATCHNO,int (Str4ptr(apcash->batchno->str())));
  SetField(ID_INVOICE,apcash->payable);
  SetField(ID_DESCRIPT,apcash->descript);
  SetField(ID_VENDORNO,apcash->vendorno);
  SetField(ID_VENDORNAME,apcash->vdname);
  SetField(ID_INVOICENO,apcash->invoiceno);
  SetField(ID_CURRBAL,apcash->orderno);
  SetField(ID_REFERENCE,apcash->reference);
  SetField(1031,apcash->account);
  SetField(ID_APDEPART,apcash->depart);
  SetField(ID_REBATE,apcash->rebate);
  SetField(ID_CHEQUE,apcash->chequeno);

  InvoiceDate.assign(apcash->invdate->str(),"CCYYMMDD");
  InvoiceDate.format(szInvDate,"MM/DD/YY");
  SetField(ID_INVDATE,szInvDate);

  DueDate.assign(apcash->duedate->str(),"CCYYMMDD");
  DueDate.format(szDueDate,"MM/DD/YY");
  SetField(ID_DUEDATE,szDueDate);
  
  SetField(ID_TERMS,apcash->terms);
  SetField(ID_VATPERCENTAGE,apcash->vatpercent);
  SetField(ID_ACCOUNTS,"");
  SetField(ID_DEPART,"");

  if(strcmp(apcash->mode->str(),"0") == 0)
   SetField(IDOK,GetString(ID_RECOVER));

  FloatSetDlgItemValue(HWindow,ID_INVOICE,double (Str4ptr(apcash->payable->str())));
  
  long i=1;
  apcashd->SetIndex("batchno");

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
      SSSetData(List,1,i,szLineNo);
      SSSetData(List,2,i,apcashd->descript->str());
      SSSetData(List,3,i,apcashd->account->str());
      SSSetData(List,4,i,apcashd->dept->str());
      SSSetData(List,5,i,apcashd->debit->str());
      SSSetData(List,6,i,apcashd->credit->str());
      apcashd->entryno->assignLong(i);
      apcashd->flush();
      DWORD nRec_no = apcashd->recNo();
      D->ListArray->add(* (new TListItem((int)i,(int)nRec_no)));
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
      SSSetData(List,1,i,szLineNo);
      SSSetData(List,2,i,apcashd->descript->str());
      SSSetData(List,3,i,apcashd->account->str());
      SSSetData(List,4,i,apcashd->dept->str());
      SSSetData(List,5,i,apcashd->debit->str());
      SSSetData(List,6,i,apcashd->credit->str());
      apcashd->entryno->assignLong(i);
      apcashd->flush();

      DWORD nRec_no = apcashd->recNo();
      D->ListArray->add(* (new TListItem((int)i,(int)nRec_no)));
      CreditTotal = lCreditTotal;
      i++;    
     }
     apcashd->skip();
   }
  
  }
  apcashd->unlock();
   lDifference = lDebitTotal - lCreditTotal;
   IntSetDlgItemValue(HWindow,ID_LINENO,i);
   FloatSetMask(GetDlgItem(D->HWindow,ID_DEBITTOTAL), "999999999.99");
   FloatSetMask(GetDlgItem(D->HWindow,ID_CREDITTOTAL),"999999999.99");
   FloatSetDlgItemValue(D->HWindow,ID_CREDITTOTAL,lCreditTotal);
   FloatSetDlgItemValue(D->HWindow,ID_DEBITTOTAL,lDebitTotal);
   FloatSetDlgItemValue(D->HWindow,ID_DIFFERENCE,lDifference);
   apcashd->iCurrentRecord = 0;
   if(i>8)
    SSSetMaxRows(List,i);
   else
    SSSetMaxRows(List,8);
  delete szLineNo;
  SSSetBool(List,SSB_REDRAW,TRUE);
  SSSetColor(GetDlgItem(D->HWindow,ID_DISTLIST), SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
  SetFocus(HWindow);
}

BOOL TAPCashEntry::Closed(void)
{
 if(strcmp(GetField(IDOK),GetString(ID_RECOVER)) == 0)
  {
   if(DlgMsg(4111,4112,MB_ICONQUESTION | MB_YESNO) == IDYES)
     {
      RecoverInvoice();
      SetField(IDOK,"&Add/Replace");
      SS_COORD Row,Col;
      SSGetActiveCell(GetDlgItem(SlipList->HWindow,ID_DISTLIST),&Col,&Row);
      SSSetColor(GetDlgItem(SlipList->HWindow,ID_DISTLIST),SS_ALLCOLS,Row,lpBack,lpFore);      
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

void TAPCashEntry::Ok (RTMessage Msg )
{
 if(Closed())
  return;

 szAdj = strcpy(new char[apcash->adj->len() + 1],"N");
 SendDlgItemMsg(ID_DISTLIST, WM_SETREDRAW, FALSE, 0L);
 iPeriodNo = (int)lGetField(ID_PERIOD);
 iBatchno  = (int) lGetField(ID_BATCHNO);
 // Check to see if the invoice has been entered previously.
 if(dGetField(ID_BATCHNO) == 0)
 if(PreviousInvoice())
  {
   CurrentBatch(Msg);
   return;
  }



 // Check to see if the batch number matches the invoice number.
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



 szVendorNo = strcpy(new char[apcash->vendorno->len() + 1],GetField(ID_VENDORNO));
 if(*szVendorNo==0)
   {
    CheckError(ID_VENDORNO);
    return;
   }

 if(*GetField(1031) == 0) // Check to make sure the bank account is okay.
 {
   CheckError(1031);
   return;
 }

 if(apvendor->seek(szVendorNo)!=0)
  {
   MessageBeep(MB_ICONEXCLAMATION);
   if(IsWindowVisible(VendorList->HWindow))
    VendorList->Show(SW_HIDE);
   else
    VendorList->Show(SW_SHOW);
   SetFocus(VendorList->HWindow);
   return;
  }

 szInvoiceNo = strcpy(new char[apcash->invoiceno->len() + 1], GetField(ID_INVOICENO));
 if(*szInvoiceNo==0)
  {
   CheckError(ID_INVOICENO);
   return;
  }

 szInvoiceDate = strcpy(new char[apcash->date->len() + 1],GetField(ID_INVDATE));
 if(*szInvoiceDate==0)
  {
   CheckError(ID_INVDATE);
   return;
  }
 InvoiceDate.assign(szInvoiceDate,"MM/DD/YY");

 DueDate.assign(szInvoiceDate,"MM/DD/YY");

 dTerms = dGetField(ID_TERMS);
 long lDueDate = long (DueDate) + dTerms;
 DueDate.assign(lDueDate);

 szOrderNo = strcpy(new char[apcash->orderno->len() + 1],GetField(ID_CURRBAL));

 szReference = strcpy(new char[apcash->reference->len() + 1],GetField(ID_REFERENCE));
 if(*szReference==0) {
   CheckError(ID_REFERENCE);
   return;
 }

 szDescript = strcpy(new char[apcash->descript->len() + 1],GetField(ID_JDESCRIPT));
 if(*szDescript == 0){CheckError(ID_JDESCRIPT);return;}



 szDepart = strcpy(new char[apcash->depart->len() + 1],GetField(ID_DEPART));
 szAccounts = strcpy(new char[apcash->account->len() + 1],GetField(ID_ACCOUNTS));
 char *szSeek = new char[apcash->account->len() + apcash->depart->len() + 1];
 strcpy(szSeek,szAccounts);
 strcat(szSeek,szDepart);
 if(*szAccounts==0){CheckError(ID_ACCOUNTS);return;}
 if(!CheckGLAccount(szSeek)){SearchGL(Msg);SetFocus(GetDlgItem(HWindow,ID_ACCOUNTS));return;}



 iLineno = lGetField(ID_LINENO);

 lInvoiceAmount = dGetField(ID_INVOICE);
 if(lInvoiceAmount == 0 ){CheckError(ID_INVOICE);return;}

 
 dVatPercentage = dGetField(ID_VATPERCENTAGE);
 dRebatePercentage = dGetField(ID_REBATE);

 apcash->SetIndex("batchno");


 if(iBatchno == 0)
 {

   if(apcash->lockTestFile() !=0)
      {
       SetField(IDD_STATUSBOX,GetString(ID_BATCHSTATUS));
      }

   if(apcash->lockAll() != 0)
      {       
       SetField(IDD_STATUSBOX,GetString(ID_TRYAGAIN));
       return;
      }     
    if(apcashd->lockAll() !=0 )
      {
      SetField(IDD_STATUSBOX,GetString(ID_TRYAGAIN));
       return;
      }

   apcash->bottom();
   iBatchno = (int)apcash->lBatchno();
   iBatchno++;

  if(bAutomaticCalc)
   { 
    if(dVatPercentage > 0 && dRebatePercentage == 0)
     {
      SetField(IDD_STATUSBOX,"Create Vat Slip");
      CreateVatSlip(apcash,apcashd,D);
     }
    else if(dVatPercentage > 0 && dRebatePercentage > 0)
     CreateVat_RebateSlip(apcash,apcashd,D);
    else if (dVatPercentage == 0 && dRebatePercentage > 0)
     CreateRebateSlip(apcash,apcashd,D);
    else
     CreateNewApSlip(apcash,apcashd,D);
     SetField(IDD_STATUSBOX,"");
     return;
   }
  else
   {
     if((dGetField(ID_DEBIT) == 0) && (dGetField(ID_CREDIT) == 0))
      {
	MessageBox(HWindow,"A debit or credit must be greater than 0.",GetString(ID_WARNING),MB_OK);
        return;
      }
     CreateLineItem(apcash,apcashd);
     return;
   } 
 }



  lCredit = dGetField(ID_CREDIT);
  lDebit  = dGetField(ID_DEBIT);
  if((lCredit == 0) && (lDebit == 0))
   {
     MessageBox(HWindow,"A debit or credit must be greater than 0.",GetString(ID_WARNING),MB_OK);
     return;
   }
  
  if((lCredit > 0) && (lDebit > 0))
   {
     MessageBox(HWindow,"Both debit and credit cannot be  greater than 0.",GetString(ID_WARNING),MB_OK);
     return;
   }

  ModifyCurrentSlip(apcash,apcashd,D);
  return;

}



void TAPCashEntry::ModifyCurrentSlip(PTapcash apcash,PTapcashd apcashd,PTAPCashDist D)
{  

   int iLine;
   IntGetDlgItemValue(HWindow,3033,(LPLONG)&iLine);

   FloatGetDlgItemValue(D->HWindow,ID_DEBITTOTAL,&DebitTotal);
   FloatGetDlgItemValue(D->HWindow,ID_CREDITTOTAL,&CreditTotal);  

 
   apcash->batchno->assignLong(iBatchno);
   apcash->mode->assign("1");
   apcash->date->assign(InvoiceDate.str());
   apcash->period->assignDouble(GetCurrentPeriodn());
   apcash->orderno->assign(szOrderNo);
   apcash->vendorno->assign(szVendorNo);
   apcash->vdname->assign(GetField(ID_VENDORNAME));
   apcash->invdate->assign(InvoiceDate.str());
   apcash->reference->assign(GetField(ID_REFERENCE));
   apcash->payable->assignDouble(lInvoiceAmount);
   apcash->chequeno->assign(GetField(ID_CHEQUE));
   apcash->invoiceno->assign(GetField(ID_INVOICENO));
   apcash->vatpercent->assignDouble(dGetField(ID_VATPERCENTAGE));
   apcash->flush();



   if(apcashd->seek(iBatchno) == 0)
   {
    if(apcashd->iCurrentRecord == 0)
    {
      apcashd->appendStart();
      DebitTotal = DebitTotal+ lDebit;
      CreditTotal = CreditTotal+ lCredit;
      apcashd->batchno->assignLong(iBatchno);
      apcashd->descript->assign(szDescript);
      apcashd->entryno->assignLong(iLine);
      apcashd->account->assign(szAccounts);  // AP account is always set to the default.
      apcashd->dept->assign(szDepart);        // AP department code is set to the default dept code.
      apcashd->debit->assignDouble(lDebit);
      apcashd->credit->assignDouble(lCredit);
      apcashd->append();
      apcashd->unlock();
      DWORD nRec_no = apcashd->recNo();
      D->ListArray->add(* (new TListItem((int)iLineno,(int)nRec_no)));
    }
    else
    {
	apcashd->go(apcashd->iCurrentRecord);
	DebitTotal = DebitTotal  + lDebit - double (Str4ptr(apcashd->debit->str()));
	CreditTotal = CreditTotal + lCredit - double (Str4ptr(apcashd->credit->str()));
	apcashd->batchno->assignLong(iBatchno);
	apcashd->descript->assign(szDescript);
	apcashd->entryno->assignLong(iLine);
	apcashd->account->assign(szAccounts);  // AP account is always set to the default.
	apcashd->dept->assign(szDepart);        // AP department code is set to the default dept code.
	apcashd->debit->assignDouble(lDebit);
	apcashd->credit->assignDouble(lCredit);
	apcashd->flush();
        apcashd->unlock();
    }
   }
   bSlipList = TRUE;
   SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
   return;
}

void TAPCashDist::SetupWindow()
{
 TDialog::SetupWindow();
 RECT rc;
 HourGlassOn();
 
 SendDlgItemMsg(ID_PRINT,SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)(LPSTR)"GLPRINT");
 FloatSetMask(GetDlgItem(HWindow,ID_DEBITTOTAL), "99999999999.99");
 FloatSetMask(GetDlgItem(HWindow,ID_CREDITTOTAL),"99999999999.99");
 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,&lpBack,&lpFore);
 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
 SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetBool(SHWindow,SSB_ALLOWUSERRESIZE,TRUE);
 SSSetBool(SHWindow,SSB_HORZSCROLLBAR,FALSE);
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);

 SSSetMaxCols(SHWindow, 6);
 SSGetClientRect(SHWindow,&rc);
 short d0Width,d2Width,d3Width,d4Width,d5Width;

 SSSetColWidth(SHWindow,1,3);
 SSSetColWidth(SHWindow,3,6);
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
 
 SSSetMaxRows(SHWindow, 10);
 LOGFONT 	lfFont;
 HFONT   	hFont;

 lfFont.lfHeight          = 8;
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








void TAPCashEntry::Sliplist() //slip list search
{
 if(bSlipList)
 {
  HWND SHWindow = GetDlgItem(SlipList->HWindow,ID_DISTLIST);
  SSClearData(SHWindow,SS_ALLCOLS,SS_ALLROWS);
  SSSetBool(SHWindow,SSB_REDRAW,FALSE);
  apcash->SetIndex("batchno");
  Relate4set *rQuery = new Relate4set(apcash);
  rQuery->querySet("MODE = '1'");
  int i = 1;
  for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
   SSSetData(SHWindow,1,i,StrTrim(apcash->batchno->str()));
   SSSetData(SHWindow,2,i,StrTrim(apcash->invoiceno->str()));
   SSSetData(SHWindow,3,i,apcash->reference->str());
   i++;
  } 
  rQuery->unlock();
  rQuery->free();
  SSSetBool(SHWindow,SSB_REDRAW,TRUE);
 }
 if(IsWindowVisible(SlipList->HWindow))
 SlipList->Show(SW_HIDE);
 else
 SlipList->Show(SW_SHOW);
}

void TAPCashEntry::New(RTMessage)
{
 delete D->ListArray;
 D->ListArray = new TListArray(1000, 0, 50);
 IntSetDlgItemValue(HWindow,ID_BATCHNO,0);
 IntSetDlgItemValue(HWindow,ID_LINENO,1);
 SetField(ID_DESCRIPT,"");
 SetField(ID_VENDORNO,"");
 SetField(ID_VENDORNAME,"");
 SetField(ID_INVOICENO,"");
 SetField(ID_CURRBAL,"");
 SetField(ID_REFERENCE,"");
 SetField(1031,"");
 SetField(ID_APDEPART,"");
 SetField(ID_ACCOUNTS,"");
 SetField(ID_DEPART,"");
 SetField(ID_BANK,"");


 SSClearData(GetDlgItem(D->HWindow,ID_DISTLIST),SS_ALLCOLS,SS_ALLROWS);

 FloatSetMask(GetDlgItem(HWindow,ID_DEBIT), "999999999.99");
 FloatSetMask(GetDlgItem(HWindow,ID_CREDIT), "999999999.99");
 FloatSetDlgItemValue(HWindow,ID_DEBIT,0.00);
 FloatSetDlgItemValue(HWindow,ID_CREDIT,0.00);
 FloatSetDlgItemValue(HWindow,ID_INVOICE,0.00);
 FloatSetDlgItemValue(D->HWindow,ID_DEBITTOTAL,0.00);
 FloatSetDlgItemValue(D->HWindow,ID_CREDITTOTAL,0.00);
 FloatSetDlgItemValue(D->HWindow,ID_DIFFERENCE,0.00);
 DebitTotal=CreditTotal=0;
 SetFocus(GetDlgItem(HWindow,2015));
}

void TAPCashEntry::SetupWindow()
{
 TDialog::SetupWindow();
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

 bAutomaticCalc = TRUE;
 SendDlgItemMessage(HWindow,ID_AUTO,BM_SETCHECK,TRUE,NULL);
 // Get the AP Control Accounts.
 apsetup = new Tapsetup(&cb);
 apsetup->SetIndex("name");
 apsetup->go(2L);
 szIndTaxAccount = strcpy(new char[apsetup->account->len()+1],apsetup->account->str());
 szIndTaxDept    = strcpy(new char[apsetup->depart->len()+1],apsetup->depart->str());
 apsetup->go(3L);
 szCompanyTaxAccount = strcpy(new char[apsetup->account->len()+1] ,apsetup->account->str());
 szCompanyTaxDept = strcpy(new char[apsetup->depart->len()+1],apsetup->depart->str());
 apsetup->go(4L);
 szVATAccount = strcpy(new char[apsetup->account->len()+1] ,apsetup->account->str());
 szVATDept    = strcpy(new char[apsetup->depart->len()+1],apsetup->depart->str());

 apsetup->SetIndex("cash");
 for (apsetup->top();!apsetup->eof();apsetup->skip())
  {
   SendDlgItemMsg(ID_CASH,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)apsetup->account->str());
  }

 
 VendorList = new TAPVendorList(this,"AR_LIST",apvendor);
 VendorList->EnableAutoCreate();
 GetApplication()->MakeWindow(VendorList);
 VendorList->SetCaption("Accounts Payable Vendor List");


 AcctList = new TGLAcctList(this,"AR_LIST",glacct);
 AcctList->EnableAutoCreate();
 GetApplication()->MakeWindow(AcctList);

 SlipList  = new TAPCashBatch(this,"AR_LIST",apcash);
 SlipList->EnableAutoCreate();
 GetApplication()->MakeWindow(SlipList);


 D = new TAPCashDist(this,"GL_DISTLIST",apcashd);
 D->EnableAutoCreate();
 GetApplication()->MakeWindow(D);


 RECT rc;
 RECT child;
 RECT parent;
 int pos = 30;
 GetWindowRect(HWindow,&rc);
 GetWindowRect(Parent->HWindow,&parent);
 MoveWindow(HWindow,parent.left+pos,parent.top+pos,rc.right-rc.left,rc.bottom-rc.top,TRUE);
 GetWindowRect(HWindow,&rc);
 GetWindowRect(D->HWindow,&child);
 MoveWindow(D->HWindow,rc.left+pos,rc.bottom+pos,child.right-child.left,child.bottom-child.top,TRUE);
 GetWindowRect(VendorList->HWindow,&child);
 MoveWindow(VendorList->HWindow,rc.right+pos,rc.top+pos,child.right-child.left,child.bottom-child.top,TRUE);
 pos = pos + 20;
 GetWindowRect(AcctList->HWindow,&child);
 MoveWindow(AcctList->HWindow,rc.right+pos,rc.top+pos,child.right-child.left,child.bottom-child.top,TRUE);
 pos = pos + 20;
 GetWindowRect(SlipList->HWindow,&child);
 MoveWindow(SlipList->HWindow,rc.right+pos,rc.top+pos,child.right-child.left,child.bottom-child.top,TRUE);

 long db=0;
 IntSetDlgItemValue(HWindow,ID_BATCHNO,db);
 IntSetDlgItemValue(HWindow,3032,GetCurrentPeriodn());

 IntSetDlgItemValue(HWindow,ID_LINENO,1);

 D->Show(SW_SHOW);
 FloatSetMask(GetDlgItem(HWindow,ID_CREDIT),"999999999.99");

 SendDlgItemMsg(ID_DEPART,WM_SETTEXT,NULL,(LONG)(LPSTR)"");

 SetFocus(GetDlgItem(HWindow,ID_DESCRIPT));
 HourGlassOff();

}

void TAPCashEntry::SetLineItem(RTMessage)
{
  SetField(IDOK,GetString(ID_MODIFY));
  SetField(ID_JDESCRIPT,apcashd->descript);
  SetField(ID_ACCOUNTS ,apcashd->account);
  SetField(ID_DEPART   ,apcashd->dept);
  SetField(ID_DEBIT    ,apcashd->debit);
  SetField(ID_CREDIT   ,apcashd->credit);
  SetField(ID_LINENO   ,apcashd->entryno);

}

void TAPCashDist::SelectCell (RTMessage Msg)
{
  int nIndx;
  int nRecord = 0;
  LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
  RArrayIterator ListIterator =(RArrayIterator)(ListArray->initIterator());
   while (int (ListIterator) != 0)
    {
      RObject AnObject = ListIterator++;
      nIndx = ((PTListItem)(&AnObject))->index;
      if( nIndx == Cell->Row)
       {
	nRecord = ((PTListItem)(&AnObject))->rec;
	break;
       }
    }
  if(nRecord == 0)
  {
   SSSetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
   return;
  }
  apcashd->go(nRecord);
  apcashd->iCurrentRecord = nRecord;

  SHWindow = GetDlgItem(HWindow,ID_DISTLIST);   
  
  SSSetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
  SSSetColor(SHWindow,SS_ALLCOLS,nIndx,RGB(192,220,192), RGBCOLOR_BLUE);
  SendMessage(Parent->HWindow,WM_COMMAND,CM_LINEITEM,NULL);
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
 rQuery->querySet("MODE = '1' .OR. MODE = '0'");
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
 apcash->SetIndex("batchno");
 if(apcash->seek(lRecord)==0)
 {
  SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
 }
 delete szBuffer;
}



void TAPCashEntry::InvoiceData()
{
/*
  double lDebitTotal,lCreditTotal,lDifference;
  lDebitTotal=lCreditTotal=lDifference=0;
  HWND List = GetDlgItem(D->HWindow,ID_DISTLIST);
  SSClearData(List,SS_ALLCOLS,SS_ALLROWS);
  delete D->ListArray;
  D->ListArray = new TListArray(1000, 0 ,50);
  IntSetDlgItemValue(HWindow,ID_BATCHNO,int (Str4ptr(apcash->batchno->str())));
  SendDlgItemMsg(ID_DESCRIPT,WM_SETTEXT,NULL,(LONG)(LPSTR)apcash->descript->str());
  IntSetDlgItemValue(HWindow,3032,GetCurrentPeriodn());
  int iLineNo = int (Str4ptr(apcash->entryno->str()) );
  iLineNo++;
  IntSetDlgItemValue(HWindow,3033,iLineNo);
  SendDlgItemMsg(ID_DESCRIPT,WM_SETTEXT,NULL,(LONG)(LPSTR)apcash->descript->str());

  apcashd->SetIndex("batchno");
  apcashd->seek(double (Str4ptr(apcash->batchno->str())));
  SendDlgItemMsg(4001,WM_SETTEXT,NULL,(LONG)(LPSTR)apcash->vendorno->str());
  SendDlgItemMsg(4003,WM_SETTEXT,NULL,(LONG)(LPSTR)apcash->vdname->str());
  SendDlgItemMsg(ID_INVOICENO,WM_SETTEXT,NULL,(LONG)(LPSTR)apcash->invoiceno->str());
  SendDlgItemMsg(2017,WM_SETTEXT,NULL,(LONG)(LPSTR)apcash->orderno->str());
  SendDlgItemMsg(ID_REFERENCE,WM_SETTEXT,NULL,(LONG)(LPSTR)apcash->reference->str());
  SendDlgItemMsg(ID_DESCRIPT,WM_SETTEXT,NULL,(LONG)(LPSTR)apcash->descript->str());
  SendDlgItemMsg(ID_INVDATE,WM_SETTEXT,NULL,(LONG)(LPSTR)apcash->invdate->str());
  SendDlgItemMsg(ID_DUEDATE,WM_SETTEXT,NULL,(LONG)(LPSTR)apcash->duedate->str());


  long i=1;
  while (int (Str4ptr(apcash->batchno->str())) == int (Str4ptr(apcashd->batchno->str())) )
   {
     lDebitTotal = lDebitTotal + double(Str4ptr(apcashd->debit->str()));
     lCreditTotal = lCreditTotal + double(Str4ptr(apcashd->credit->str()));
     SSSetData(List,1,i,apcashd->descript->str());
     SSSetData(List,2,i,apcashd->account->str());
     SSSetData(List,3,i,apcashd->dept->str());
     SSSetData(List,4,i,apcashd->debit->str());
     SSSetData(List,5,i,apcashd->credit->str());
     DWORD nRec_no = apcashd->recNo();
     D->ListArray->add(* (new TListItem((int)i,(int)nRec_no)));
     DebitTotal = lDebitTotal;
     CreditTotal = lCreditTotal;
     i=i+1;
     apcashd->skip(1);
   }
   lDifference = lDebitTotal - lCreditTotal;
   FloatSetMask(GetDlgItem(D->HWindow,ID_DEBITTOTAL), "999999999.99");
   FloatSetMask(GetDlgItem(D->HWindow,ID_CREDITTOTAL),"999999999.99");
   FloatSetDlgItemValue(D->HWindow,ID_CREDITTOTAL,lCreditTotal);
   FloatSetDlgItemValue(D->HWindow,ID_DEBITTOTAL,lDebitTotal);
   FloatSetDlgItemValue(D->HWindow,ID_DIFFERENCE,lDifference);
 */
}

