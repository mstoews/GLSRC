#include "tbowl.h"
#include "d4all.hpp"

#include "ar.h"
#include "db_dept.h"
#include "db_arbtc.h"
#include "db_itemm.h"
#include "db_medrp.h"
#include "db_cash.h"
#include "ar_class.h"
#include "gl_acctl.h"
#include "ar_custl.h"
#include "ar_btchl.h"
#include "ar_btch.h"

#include "math.h"
#include "gldbf.h"

#define ID_VIN_NUM 1004
#define ID_UNITS   1005
#define ID_INVOICE 1030
#define ID_INVOICENO  4016
#define ID_COMMIT     1056

#define ID_EDITOR     3046
#define ID_UNDO       3075
#define ID_CUT        3076
#define ID_COPY       3077
#define ID_PASTE      3078
#define ID_CLEAR      3090

static COLORREF lpBack, lpFore;
extern char szApp[20];

//TCustomerList *VendorList;

class TARMemo: public TMainDialog
{
 public:
 Tapbtch *arbtch;
 TARMemo(PTWindowsObject AParent,LPSTR ATitle,Tapbtch *ar):TMainDialog(AParent,ATitle)
 {
  arbtch = ar;
 }
 virtual void SetupWindow();
 virtual void Ok     (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Save   (RTMessage Msg) = [CM_FIRST + IDOK]{Ok(Msg);}
 virtual void Quit   (RTMessage Msg) = [CM_FIRST + IDCANCEL]{Cancel(Msg);}
 virtual void Undo   (RTMessage Msg) = [CM_FIRST + ID_UNDO];
 virtual void Copy   (RTMessage Msg) = [CM_FIRST + ID_COPY];
 virtual void Paste  (RTMessage Msg) = [CM_FIRST + ID_PASTE];
 virtual void Cut    (RTMessage Msg) = [CM_FIRST + ID_CUT];
 virtual void Clear  (RTMessage Msg) = [CM_FIRST + ID_CLEAR];
};


BOOL Tapbtch::Open()
{
 if(strcmp(mode->str(),"1") == 0)
  return TRUE;
 else
  return FALSE;
}

BOOL Tapbtch::Posted()
{
 if(strcmp(mode->str(),"2") == 0)
  return TRUE;
 else
  return FALSE;
}

BOOL Tapbtch::New(long iBatchno)
{
 SetIndex("batchno");
 if(seek(iBatchno) != 0)
  return TRUE;
 else
  return FALSE; 
}

void TAPInvoiceEntry::AddVendor(RTMessage)
{
 SendMessage(Parent->HWindow,WM_COMMAND,ID_APADDVENDORS,NULL);
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
    bFullSize = FALSE;
    GetWindowRect(HWindow,&rc);
    GetWindowRect(Parent->HWindow,&parent);
    //MoveWindow(HWindow,parent.left+pos,parent.top+pos,rc.right-rc.left,rc.bottom-rc.top-pos,TRUE);
   }
  }
 */
 GetWindowRect(HWindow,&rc);
 GetWindowRect(Parent->HWindow,&parent);
 MoveWindow(HWindow,parent.left+pos,parent.top+pos,rc.right-rc.left,rc.bottom-rc.top,TRUE);
 GetWindowRect(D->HWindow,&parent);
 GetWindowRect(HWindow,&rc);
 MoveWindow(D->HWindow,rc.left+pos,rc.bottom-50,parent.right-parent.left,parent.bottom-parent.top,TRUE);
}




void TAPInvoiceEntry::Top(RTMessage Msg)
{
 apbtch->SetIndex("batchno");
 apbtch->top();
 while(apbtch->Posted())
  apbtch->skip();
  CurrentBatch(Msg);
};
void TAPInvoiceEntry::Bottom(RTMessage Msg)
{
 apbtch->SetIndex("batchno");
 apbtch->bottom();
 CurrentBatch(Msg);
};

void TAPInvoiceEntry::Next(RTMessage Msg)
{
 apbtch->SetIndex("batchno");
 int iBatchno = 0;
 IntGetDlgItemValue(HWindow,ID_BATCHNO,(LPLONG)&iBatchno);
 if(apbtch->seek(iBatchno) == 0)
 { 
   if(!apbtch->eof())
	{
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
 apbtch->SetIndex("batchno");
 int iBatchno = 0;
 IntGetDlgItemValue(HWindow,ID_BATCHNO,(LPLONG)&iBatchno);
 if(apbtch->seek(iBatchno) == 0)
 { 
   if(!apbtch->bof())
   {
    apbtch->skip(-1);
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


void TAPInvoiceEntry::SelectProduct(RTMessage Msg)
{
 char szProductCode[41];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  DWORD Indx = SendDlgItemMsg(ID_PRODUCTS,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
   {
	 SendDlgItemMsg(ID_PRODUCTS,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szProductCode);
    prods->SetIndex("ffulln");
    if(prods->seek(szProductCode) == 0)
     {
		SetField(1003,prods->fPcode->str());
      SetField(1038,prods->fAvprice->str());
     }    
   }
 }
}

void TAPInvoiceEntry::SelectDoctor(RTMessage Msg)
{ 
 char szDoctor[41];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  DWORD Indx = SendDlgItemMsg(ID_DRCODES,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
   {
    SendDlgItemMsg(ID_DRCODES,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szDoctor);
    drcodes->SetIndex("rpname");
    if(drcodes->seek(szDoctor) == 0)
      SetField(1008,drcodes->drcode->str());
   }
 }
}


void TAPInvoiceEntry::SelectMedRep(RTMessage Msg)
{
 char szMedRepCode[41];

 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  DWORD Indx = SendDlgItemMsg(ID_MEDREPS,CB_GETCURSEL,0,0);
  SendDlgItemMsg(ID_MEDREPS,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR) szMedRepCode);
  if(Indx != CB_ERR)
   {   
    medreps->SetIndex("rpname");
    if(medreps->seek(szMedRepCode) == 0)
      SetField(1002,medreps->repcode->str());
   }
 }
}


void TAPInvoiceEntry::Delete(RTMessage Msg)
{
  if(bModify)
  {
   int iEntryno;
   IntGetValue(GetDlgItem(HWindow,ID_LINENO),(LPLONG)&iEntryno);  
   apbtchd->SetIndex("batchno");
   if(apbtchd->seek(apbtch->lBatchno()) == 0)
   {   
   if(DlgMsg(ID_DELETE,ID_BATCHNO,MB_YESNO | MB_ICONQUESTION) == IDYES)
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
 }
 return;
};


void TAPInvoiceEntry::RollBack(RTMessage Msg)
{
 HourGlassOn();
 apbtch->SetIndex("batchno");
 if(apbtch->seek(lGetField(ID_BATCHNO)) == 0)
  {
   apbtch->deleteRec();
 
  }
 if(apbtchd->seek(lGetField(ID_BATCHNO)) == 0)
  while(apbtchd->lBatchno() == apbtch->lBatchno()) {
	 apbtchd->deleteRec();
	 apbtchd->skip();
  }
 apbtchd->pack();
 apbtch->pack();
 HourGlassOff();
 Clear();
 return;
}



void TARMemo::Cut (RTMessage)
{
  SendMessage(GetDlgItem(HWindow,ID_EDITOR),WM_CUT,0,0L);
}

void TARMemo::Copy (RTMessage)
{
 SendMessage(GetDlgItem(HWindow,ID_EDITOR),WM_COPY,0,0L);
}

void TARMemo::Paste (RTMessage)
{
 SendMessage(GetDlgItem(HWindow,ID_EDITOR),WM_PASTE,0,0L);
}

void TARMemo::Clear (RTMessage)
{
 SendMessage(GetDlgItem(HWindow,ID_EDITOR),EM_SETSEL,0,MAKELONG(0,32767));
 SendMessage(GetDlgItem(HWindow,ID_EDITOR),WM_CLEAR,0,0L);
}

void TARMemo::Undo(RTMessage)
{
 SendMessage(GetDlgItem(HWindow,ID_EDITOR),WM_UNDO,0,0L);
}

void TARMemo::SetupWindow()
{
 SetField(ID_EDITOR,arbtch->memo);
}

void TARMemo::Ok (RTMessage)
{
 arbtch->memo->assign(GetField(ID_EDITOR));
 arbtch->flush();
 ShutDownWindow();
 return;
}


void TAPInvoiceEntry::Memo(RTMessage)
{
 apbtch->SetIndex("batchno");
 if(apbtch->seek(lGetField(ID_BATCHNO)) == 0)
  GetApplication()->ExecDialog(new TARMemo(this,"AR_MEMO",apbtch));
 else
  MessageBox(HWindow,"Add the new invoice before adding a memo.",szApp,MB_ICONINFORMATION);
}


void TAPInvoiceEntry::LoadMedReps(void)
{
 SendDlgItemMsg(ID_MEDREPS,CB_RESETCONTENT,NULL,NULL);
 medreps->top();
 while(!medreps->eof())
  {
   SendDlgItemMsg(ID_MEDREPS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)medreps->rpname->str());
   medreps->skip();
  }

}

void TAPInvoiceEntry::LoadProducts(void)
{
 SendDlgItemMsg(ID_PRODUCTS,CB_RESETCONTENT,NULL,NULL);
 prods->SetIndex("fpcode");
 prods->top();
 while(!prods->eof())
  {
   SendDlgItemMsg(ID_PRODUCTS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)prods->fFulln->str());
   prods->skip();
  }
}


void TAPInvoiceEntry::SetupWindow()
{
 TMainDialog::SetupWindow();
 bTransaction = FALSE;
 bModify = FALSE;

 RECT button;
 GetWindowRect(HWindow,&rc);
 GetWindowRect(GetDlgItem(HWindow,IDOK),&button);
 int width = rc.right - (rc.left + (button.right - button.left + 12));
 MoveWindow(HWindow,rc.left,rc.top,width,rc.bottom-rc.top,TRUE);

 SendDlgItemMsg(ID_SEARCHCUST,SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)(LPSTR)"SEEK");

 HourGlassOn(); 
 DebitTotal=0;
 CreditTotal=0;
 apbtch =  new Tapbtch(&cb);
 apbtchd=  new Tapbtchd(&cb);
 apvendor= new Tapvendor(&cb);
 glacct =  new Tglaccts(&cb);
 gldept =  new Tgldept(&cb);
 prods  =  new Tprods(&cb);
 medreps = new Tglmedreps(&cb);
 drcodes = new Tgldrcodes(&cb);


 LoadMedReps();
 LoadProducts();
 LoadDoctors();

 apbtch->SetIndex("batchno");
 apvendor->SetIndex("vendorno");

 // Get the AP Control Accounts.
 
 AcctList = new TAccountList(this,"",glacct);
 AcctList->EnableAutoCreate();
 GetApplication()->MakeWindow(AcctList);


 VendorList = new TCustomerList(this,"",apvendor);
 VendorList->EnableAutoCreate();
 GetApplication()->MakeWindow(VendorList);

 
 SlipList = new TBatchList(this,"",apbtch);
 SlipList->EnableAutoCreate();
 GetApplication()->MakeWindow(SlipList);

 bSlipList = TRUE;
 
 D = new TAPDistList(this,"GL_DISTLIST",apbtchd);
 D->EnableAutoCreate();
 GetApplication()->MakeWindow(D);

 ChangeSize(FALSE);

 long db=0;
 apbtch->bottom();
 db = apbtch->lBatchno();
 db++;
 IntSetDlgItemValue(HWindow,ID_BATCHNO,db);
 //IntSetDlgItemValue(HWindow,ID_PERIOD,GetCurrentPeriodn());
 SetPeriods(HWindow);
 SetCaption(GetString(ID_APADD));

 IntSetDlgItemValue(HWindow,ID_LINENO,1);
 bFullSize = TRUE;

 

 ToolBox = new TARToolBox(this,"ToolBox");
 ToolBox->Attr.Style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT | WS_POPUPWINDOW;
 ToolBox->EnableAutoCreate();
 GetApplication()->MakeWindow(ToolBox);

 FloatSetMask(GetDlgItem(HWindow,ID_CREDIT),"999999999.99");
 SendDlgItemMsg(ID_DEPART,WM_SETTEXT,NULL,(LONG)(LPSTR)"");
 SetFocus(GetDlgItem(HWindow,ID_DESCRIPT));

 //D->Show(SW_SHOW);
 ToolBox->Show(SW_SHOW);

 HourGlassOff();
}

void TAPInvoiceEntry::SetVendor(RTMessage)
{
   SetField(ID_VENDORNO       , apvendor->vendorno );
   SetField(ID_VENDORSHORT    , apvendor->vdshort );
   SetField(ID_APCONTROL      , apvendor->account );
   SetField(ID_APDEPART       , apvendor->depart );
   SetField(ID_AR_ACCT	      , apvendor->ar_acct );
   SetField(ID_AR_DEPT	      , apvendor->ar_dept );
   SetField(ID_VATACCT	      , apvendor->vat_acct );
   SetField(ID_VATDEPT      , apvendor->vat_dept );
   SetField(ID_VATPERCENTAGE  , apvendor->vat );
   SetField(ID_VENDORCONTACT  , apvendor->contact );
   SetField(ID_TERMS          , apvendor->terms );
   SetField(ID_VIN_NUM        , apvendor->vin );

   
   SetFocus(GetDlgItem(HWindow,ID_INVOICENO));
   return;
}

void TAPDistList::Print (RTMessage)
{
  HourGlassOn(); 
  char  szBatchNo[10];
  long iBatchNo;
  IntGetDlgItemValue(Parent->HWindow,ID_BATCHNO,(LPLONG)&iBatchNo);

  short JobNumber = PEOpenPrintJob("ar_slips.rpt");

  if (JobNumber == 0)
  {
   HANDLE textHandle;   
   short  iTextLen;
   PEGetErrorText(JobNumber,&textHandle,&iTextLen);
   char   *szErrorText = new char[iTextLen];
   PEGetHandleString(textHandle,szErrorText,iTextLen);
   MessageBox(HWindow,szErrorText,"AR Slip",MB_ICONINFORMATION);
   delete [] szErrorText;   
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
    delete [] szErrorText; 
    PEClosePrintJob(JobNumber);
    return ;
   }
  PEOutputToPrinter(JobNumber,1);
  PEStartPrintJob(JobNumber,TRUE);
  PEClosePrintJob(JobNumber);
  HourGlassOff();
}



void TAPInvoiceEntry::CurrentBatch(RTMessage Msg)
{  

  if(!apbtch->Open())
  {
   //char szMsg[255];
   //wsprintf(szMsg,GetString(8124),GetField(ID_INVOICENO));
   //MessageBox(HWindow,szMsg,GetString(8125),MB_ICONINFORMATION);
   return;
  }
  D->Show(SW_SHOW);

  SetField(IDOK,GetString(IDOK));
  SetCaption(GetString(ID_APMODIFY));
  SetField(ID_DEBIT,"");
  SetField(ID_CREDIT,"");
  HWND List = GetDlgItem(D->HWindow,ID_DISTLIST);
  SSSetBool(List,SSB_REDRAW,FALSE);
  double lDebitTotal,lCreditTotal,lDifference;
  char szDueDate[11],szInvDate[11];
  lDebitTotal=lCreditTotal=lDifference=0;
  if(apbtch->recCount() == 0)
  {
   DlgMsg(ID_NOBATCH,ID_SLIP,MB_ICONINFORMATION);
   return;
  } 

  apvendor->seek(apbtch->vendorno->str());
  
  IntSetDlgItemValue(HWindow,ID_BATCHNO,int (Str4ptr(apbtch->batchno->str())));
  SetField(ID_DESCRIPT,apbtch->descript);
  SetField(ID_VENDORNO,apbtch->vendorno);
  if(apvendor->seek(apbtch->vendorno->str()) == 0)
   SetVendor(Msg);
  SetField(ID_INVOICENO,apbtch->invoiceno);
  SetField(ID_ORDERNO,apbtch->orderno);
  SetField(ID_REFERENCE,apbtch->reference);
  SetField(ID_APCONTROL,apbtch->account);
  SetField(ID_APDEPART,apbtch->depart);
  SetField(ID_AR_ACCT,apvendor->ar_acct);
  SetField(ID_AR_DEPT,apvendor->ar_dept);

  InvoiceDate.assign(apbtch->invdate->str(),"CCYYMMDD");
  InvoiceDate.format(szInvDate,"MM/DD/YY");
  SetField(ID_INVDATE,szInvDate);

  DueDate.assign(apbtch->duedate->str(),"CCYYMMDD");
  DueDate.format(szDueDate,"MM/DD/YY");
  SetField(ID_DUEDATE,szDueDate);
  
  SetField(ID_TERMS,apbtch->terms);
  SetField(ID_VATPERCENTAGE,apbtch->vatpercent);
  SetField(ID_ACCOUNTS,"");
  SetField(ID_DEPART,"");

  FloatSetDlgItemValue(HWindow,1999,apbtch->dPayable());
  SetField(1037,"");
  SetField(1038,"");

  
  long i=1;
  apbtchd->SetIndex("batchno");

  char *szLineNo = new char[3];
  SSClearData(List,SS_ALLCOLS,SS_ALLROWS);
  
  if (apbtchd->seek(double(Str4ptr(apbtch->batchno->str()))) == 0)
  {  
   while (apbtch->lBatchno() == apbtchd->lBatchno() && !apbtch->eof() )
    {
     if(apbtchd->dDebit() > 0)
     {
      lDebitTotal = lDebitTotal + double(Str4ptr(apbtchd->debit->str()));
      ltoa(i,szLineNo,10);
      SSSetData(List,1,i,szLineNo);
      SSSetData(List,2,i,apbtchd->descript->str());
      SSSetData(List,3,i,apbtchd->account->str());
      SSSetData(List,4,i,apbtchd->dept->str());
      SSSetData(List,5,i,apbtchd->debit->str());
      SSSetData(List,6,i,apbtchd->credit->str());
      DebitTotal = lDebitTotal;
      apbtchd->entryno->assignLong(i);
      apbtchd->flush();
      apbtchd->unlock();
      i++;
     }
     apbtchd->skip();
   }
  
  }

  //Set the credit amounts.
  if (apbtchd->seek(apbtch->lBatchno()) == 0)
  {  

	while (apbtch->lBatchno() == apbtchd->lBatchno() && !apbtch->eof() )
    {
     if(apbtchd->dCredit() > 0)
     {
      lCreditTotal = lCreditTotal + double(Str4ptr(apbtchd->credit->str()));
      ltoa(i,szLineNo,10);
      SSSetData(List,1,i,szLineNo);
      SSSetData(List,2,i,apbtchd->descript->str());
      SSSetData(List,3,i,apbtchd->account->str());
      SSSetData(List,4,i,apbtchd->dept->str());
      SSSetData(List,5,i,apbtchd->debit->str());
      SSSetData(List,6,i,apbtchd->credit->str());
      CreditTotal = lCreditTotal;
      apbtchd->entryno->assignLong(i);
      apbtchd->flush();
      apbtchd->unlock();

      i++;    
     }
     apbtchd->skip();
   }
  
  }


   lDifference = lDebitTotal - lCreditTotal;
   IntSetDlgItemValue(HWindow,ID_LINENO,i);
   FloatSetDlgItemValue(D->HWindow,ID_CREDITTOTAL,lCreditTotal);
   FloatSetDlgItemValue(D->HWindow,ID_DEBITTOTAL,lDebitTotal);
   FloatSetDlgItemValue(D->HWindow,ID_DIFFERENCE,lDifference);
   apbtchd->iCurrentRecord = 0;
   if(i>8)
    SSSetMaxRows(List,i);
   else
    SSSetMaxRows(List,8);

   delete [] szLineNo;

  if(bSlipList)
   SlipList->Show(SW_HIDE);

  SSSetColor(GetDlgItem(D->HWindow,ID_DISTLIST), SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
  if(strcmp(apbtch->mode->str(),"0") == 0)
   SetField(IDOK,GetString(ID_RECOVER));

 SetFocus(GetDlgItem(HWindow,ID_PRODUCTS));
 //ChangeSize(FALSE);
 SSSetBool(List,SSB_REDRAW,TRUE);
 return;
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

BOOL TAPInvoiceEntry::EnterError(LPSTR szValid,UINT ID)
{
 BOOL bError = FALSE;
 if(strlen(StrTrim(szValid)) == 0)
 {
  char szMsg[254];
  char szField[40];
  strcpy(szField,GetString(ID));
  wsprintf(szMsg,GetString(ID_BLANKFIELD),szField);
  MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);
  bError = TRUE;
 }
 SetFocus(GetDlgItem(HWindow,ID));
 return bError;
}

void TAPInvoiceEntry::Ok (RTMessage Msg )
{

 apbtch->SetIndex("batchno");

 SendDlgItemMsg(ID_DISTLIST, WM_SETREDRAW, FALSE, 0L);
 iPeriodNo = (int)lGetField(ID_PERIOD);

 if(szEnterError(ID_VENDORNO))
 {
  MessageBeep(MB_ICONEXCLAMATION);
  SendMessage(HWindow,WM_COMMAND,ID_SEARCHCUST,NULL);
  return;
 }
 

 if(apvendor->seek(GetField(ID_VENDORNO))!=0)
  {
   MessageBeep(MB_ICONEXCLAMATION);
   SendMessage(HWindow,WM_COMMAND,ID_SEARCHCUST,NULL);
   return;
  }

 Str4large szSales;
 szSales.assign(GetField(ID_AR_ACCT));
 szSales.add(GetField(ID_AR_DEPT));

 if(!ValidGLAccount(szSales.str())){
   MessageBox(HWindow,"Sales GL Account not found.",szApp,MB_ICONINFORMATION);
   return;
 }


 Str4large szVat;
 szVat.assign(GetField(ID_VATACCT));
 szVat.add(GetField(ID_VATDEPT));

 if(!ValidGLAccount(szVat.str())){
  MessageBox(HWindow,"VAT GL Account not found.",szApp,MB_ICONINFORMATION);
  return;
 }

 Str4large szAR; 
 szAR.assign(GetField(ID_APCONTROL));
 szAR.add(GetField(ID_APDEPART));

 if(!ValidGLAccount(szAR.str())){
  MessageBox(HWindow,"AR GL Account not found.",szApp,MB_ICONINFORMATION);
  return;
 }
 
 if(szEnterError(ID_INVOICENO))
  return;

 if(szEnterError(ID_INVDATE))
   return;
 
 if(szEnterError(ID_DUEDATE))
    return;



 InvoiceDate.assign(GetDate(ID_INVDATE));
 

 dTerms = dGetField(ID_TERMS);
 long lDueDate = long (InvoiceDate) + dTerms;

 char *szDueDate = strcpy(new char [apbtch->duedate->len()+1],GetField(ID_DUEDATE)); 
 DueDate.assign(szDueDate,"MM/DD/YY");
 DueDate.assign(lDueDate);
 delete [] szDueDate;

 if(szEnterError(ID_REFERENCE))
  return;

 iLineno = lGetField(ID_LINENO);
 //dVatPercentage = dGetField(ID_VATPERCENTAGE);
 //dRebatePercentage = dGetField(ID_REBATE);
 iBatchno = (int)lGetField(ID_BATCHNO);

 // if not a new invoice then update the current line or add a new line.
 if(!bModify)
 if(dEnterError(1037))
  return;

 /*
 If the product price is zero the amount total must be greater than zero.
 if(dEnterError(1038))
  return;
 */

 if(dGetField(1038) == 0 && dGetField(1030) == 0)
 {  
  if(!bModify)
  if(dEnterError(1030))
   return;
 }


 if(apbtch->New(iBatchno))
   {     
       bSlipList = TRUE;
       CreateAPBatchEntry(iBatchno);
       bTransaction = TRUE;
       SetField(IDD_STATUSBOX,"");       
  }
  else
   ModifyCurrentSlip(iBatchno);
 SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
 return;
}




void TAPInvoiceEntry::ModifyCurrentSlip(int iBatchno)
{ 
	int iLine;
   IntGetDlgItemValue(HWindow,3033,(LPLONG)&iLine);

   FloatGetDlgItemValue(D->HWindow,ID_DEBITTOTAL,&DebitTotal);
   FloatGetDlgItemValue(D->HWindow,ID_CREDITTOTAL,&CreditTotal);
   apbtch->batchno->assignLong(iBatchno);
   apbtch->mode->assign("1");
   apbtch->date->assign(InvoiceDate.str());
   apbtch->period->assignDouble(GetCurrentPeriodn());
   apbtch->orderno->assign(GetField(ID_ORDERNO));
   apbtch->vendorno->assign(GetField(ID_VENDORNO));
   apbtch->invoiceno->assign(GetField(ID_INVOICENO));
   apbtch->duedate->assign(DueDate.str());
   apbtch->invdate->assign(InvoiceDate.str());
   apbtch->reference->assign(GetField(ID_REFERENCE));
   apbtch->descript->assign(GetSalesDescription());
   apbtch->account->assign(GetField(ID_APCONTROL));
   apbtch->depart->assign(GetField(ID_APDEPART));
   apbtch->entryno->assignLong(lGetField(ID_LINENO));
   apbtch->terms->assignLong(lGetField(ID_TERMS));   
   
   apbtch->flush();
   apbtch->unlock();

   double dVatPercentage = 1;

   if(!bModify)
    {
      //Add a new line item.
      BOOL bVat = TRUE;
      double dCurrentPayable = apbtch->dPayable();
      double dCurrentVat = apbtch->dVatamt();
      lInvoiceAmount = dGetField(1037)*dGetField(1038);

    
      double dVatPercent = dGetField(ID_VATPERCENTAGE);
       if((int)dVatPercent > (int)0)
        {
	dVatPercentage = (100+dVatPercent)/100;
	 }
	else
	 bVat = FALSE;


		double AR_AMT = dCurrentPayable + lInvoiceAmount;
		double SALES  = floor(lInvoiceAmount/dVatPercentage*1000)/1000;
		double VAT1   = lInvoiceAmount - SALES;
		double VAT    = dCurrentVat + VAT1;


		apbtch->payable->assignDouble(apbtch->dPayable()+lInvoiceAmount);
		apbtch->vatamt->assignDouble(VAT);
		apbtch->flush();
		apbtch->unlock();

	 

	 char szDept[5];
	 char szAccount[8];
	 char szVatDept[5];
	 char szVatAccount[8];
	 char szSaleDept[5];
	 char szSaleAccount[8];


	 strcpy(szDept,GetField(ID_APDEPART));
	 strcpy(szVatAccount,GetField(ID_VATACCT));
	 strcpy(szSaleAccount,GetField(ID_AR_ACCT));


	 UpdateAR(iBatchno,"",GetField(ID_APCONTROL),szDept,AR_AMT);
	 if(bVat)
	 {
	  strcpy(szDept,GetField(ID_VATDEPT));
	  UpdateVAT(iBatchno,"",GetField(ID_VATACCT),szDept,-1*VAT);
	 }

	  strcpy(szSaleDept,GetField(ID_AR_DEPT));
	  char szSalesDescription[51];
	  strcpy(szSalesDescription,GetSalesDescription());

	  AddBatchDetail(iBatchno,szSalesDescription,GetField(ID_AR_ACCT),szSaleDept,(-1*SALES));

	  bTransaction = TRUE;
	  SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	  return;
	 }
	 else
	 {
		lCredit = dGetField(ID_CREDIT);
		lDebit  = dGetField(ID_DEBIT);

		if((lCredit == 0) && (lDebit == 0) || (lCredit > 0) && (lDebit > 0))
		 {
		 bModify = FALSE;
		 MessageBox(HWindow,GetString(4113),szApp,MB_OK);
		 return;
		 }
	// Modify an existing record.
	  apbtchd->go(apbtchd->iCurrentRecord);
	  DebitTotal = DebitTotal  + lDebit - double (Str4ptr(apbtchd->debit->str()));
	  CreditTotal = CreditTotal + lCredit - double (Str4ptr(apbtchd->credit->str()));
	 if(strcmp(StrTrim(GetField(ID_ACCOUNTS)),StrTrim(GetField(ID_VATACCT))) == 0)
	   {
	    if(lDebit > 0)
	     apbtch->vatamt->assignDouble(-lDebit);
	    else
	    if(lCredit > 0)
	     apbtch->vatamt->assignDouble(lCredit);
	   }

	  apbtch->descript->assign(GetField(ID_JDESCRIPT));
	  apbtch->flush();
	  apbtch->unlock();
	  apbtchd->batchno->assignLong(iBatchno);
	  apbtchd->entryno->assignLong(iLine);
	  apbtchd->account->assign(GetField(ID_ACCOUNTS));
	  apbtchd->dept->assign(GetField(ID_DEPART));
	  if(strcmp(StrTrim(GetField(ID_ACCOUNTS)),StrTrim(GetField(ID_AR_ACCT))) == 0)
	   apbtchd->descript->assign(GetSalesDescription());
	  else
           apbtchd->descript->assign(GetDesc(ID_ACCOUNTS));
	  apbtchd->drcode->assign(GetField(1008));
	  apbtchd->product->assign(GetField(1003));
	  apbtchd->medreps->assign(GetField(1005));
	  apbtchd->debit->assignDouble(dGetField(ID_DEBIT));
	  apbtchd->credit->assignDouble(dGetField(ID_CREDIT));
	  apbtchd->flush();
	  apbtchd->unlock();
	 }
	bModify = FALSE;
	return;
}

char *TAPInvoiceEntry::GetSalesDescription(void)
{

 char *szDesc = new char [51];
 char *szSeek = new char[12];
 strcpy(szSeek,GetField(ID_AR_ACCT));
 strcat(szSeek,GetField(ID_AR_DEPT));

 glacct->SetIndex("accounts");
 prods->SetIndex("fpcode");
 
 if(strlen(StrTrim(GetField(1003))) == 0)
   {
    if(glacct->seek(szSeek) == 0)
	strcpy(szDesc,glacct->descript->str());
    else
	strcpy(szDesc,"");
   }
 else
 if (prods->seek(GetField(1003)) == 0)
 {        
    strcpy(szDesc,prods->fFulln->str());
 } 
 return szDesc; 
}



char *TAPInvoiceEntry::GetDesc(int ID)
{
 if(strcmp(StrTrim(GetField(ID)),"") == 0)
  {
	char szSeek[12];
	strcpy(szSeek,GetField(ID_ACCOUNTS));
	strcat(szSeek,GetField(ID_DEPART));
	if(glacct->seek(szSeek) == 0)
	 return glacct->descript->str();
  }
 if(strcmp(GetField(ID_ACCOUNTS),GetField(ID_VATACCT)) == 0 || strcmp(GetField(ID_APCONTROL),GetField(ID_ACCOUNTS)) == 0)
  {
	char szSeek[12];
	strcpy(szSeek,GetField(ID_ACCOUNTS));
	strcat(szSeek,GetField(ID_DEPART));
   if(glacct->seek(szSeek) == 0)
    return glacct->descript->str();
  }
 else
 prods->seek(GetField(ID));
 return prods->fFulln->str();
}

void TAPDistList::SetupWindow()
{
 TDialog::SetupWindow();
 RECT rc;
 HourGlassOn();
 SetListSize(8);
 SendDlgItemMsg(ID_PRINT,SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)(LPSTR)"GLPRINT");
 FloatSetMask(GetDlgItem(HWindow,ID_DEBITTOTAL), "99999999999.99");
 FloatSetMask(GetDlgItem(HWindow,ID_CREDITTOTAL),"99999999999.99");
 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetColor(SHWindow,SS_ALLCOLS,SS_ALLROWS,&lpBack,&lpFore);

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

 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,9,2,0,999999999.99);
 SSSetCellType(SHWindow,5, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,6, SS_ALLROWS, &CellType);

 HourGlassOff();
}




char *GetString(HINSTANCE hInstance, int ID)
{
 static char szString[85];
 LoadString(hInstance,ID,szString,sizeof(szString));
 return szString;
}

void TAPInvoiceEntry::SearchGL    (RTMessage){  Accountlist();}

void TAPInvoiceEntry::InvoiceData()
{
  double lDebitTotal,lCreditTotal,lDifference;
  lDebitTotal=lCreditTotal=lDifference=0;
  HWND List = GetDlgItem(D->HWindow,ID_DISTLIST);
  SSClearData(List,SS_ALLCOLS,SS_ALLROWS);


  IntSetDlgItemValue(HWindow,ID_BATCHNO,int (Str4ptr(apbtch->batchno->str())));
  SendDlgItemMsg(ID_DESCRIPT,WM_SETTEXT,NULL,(LONG)(LPSTR)apbtch->descript->str());
  IntSetDlgItemValue(HWindow,3032,GetCurrentPeriodn());
  int iLineNo = int (Str4ptr(apbtch->entryno->str()) );
  iLineNo++;
  IntSetDlgItemValue(HWindow,3033,iLineNo);
  SendDlgItemMsg(ID_DESCRIPT,WM_SETTEXT,NULL,(LONG)(LPSTR)apbtch->descript->str());

  apbtchd->SetIndex("batchno");
  apbtchd->seek(double (Str4ptr(apbtch->batchno->str())));
  SendDlgItemMsg(4001,WM_SETTEXT,NULL,(LONG)(LPSTR)apbtch->vendorno->str());
  SendDlgItemMsg(ID_INVOICENO,WM_SETTEXT,NULL,(LONG)(LPSTR)apbtch->invoiceno->str());
  SendDlgItemMsg(2017,WM_SETTEXT,NULL,(LONG)(LPSTR)apbtch->orderno->str());
  SendDlgItemMsg(ID_REFERENCE,WM_SETTEXT,NULL,(LONG)(LPSTR)apbtch->reference->str());
  SendDlgItemMsg(ID_DESCRIPT,WM_SETTEXT,NULL,(LONG)(LPSTR)apbtch->descript->str());
  SendDlgItemMsg(ID_INVDATE,WM_SETTEXT,NULL,(LONG)(LPSTR)apbtch->invdate->str());
  SendDlgItemMsg(ID_DUEDATE,WM_SETTEXT,NULL,(LONG)(LPSTR)apbtch->duedate->str());


  long i=1;
  while (int (Str4ptr(apbtch->batchno->str())) == int (Str4ptr(apbtchd->batchno->str())) )
   {
     lDebitTotal = lDebitTotal + double(Str4ptr(apbtchd->debit->str()));
     lCreditTotal = lCreditTotal + double(Str4ptr(apbtchd->credit->str()));
     SSSetData(List,1,i,apbtchd->descript->str());
     SSSetData(List,2,i,apbtchd->account->str());
     SSSetData(List,3,i,apbtchd->dept->str());
     SSSetData(List,4,i,apbtchd->debit->str());
     SSSetData(List,5,i,apbtchd->credit->str());
     DebitTotal = lDebitTotal;
     CreditTotal = lCreditTotal;
     i=i+1;
     apbtchd->skip(1);
   }
   lDifference = lDebitTotal - lCreditTotal;
   FloatSetMask(GetDlgItem(D->HWindow,ID_DEBITTOTAL), "999999999.99");
   FloatSetMask(GetDlgItem(D->HWindow,ID_CREDITTOTAL),"999999999.99");
	FloatSetDlgItemValue(D->HWindow,ID_CREDITTOTAL,lCreditTotal);
   FloatSetDlgItemValue(D->HWindow,ID_DEBITTOTAL,lDebitTotal);
   FloatSetDlgItemValue(D->HWindow,ID_DIFFERENCE,lDifference);
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
  bTransaction = FALSE;
  SetField(IDOK,GetString(IDOK));
  SetCaption(GetString(ID_APADD));
  apbtch->SetIndex("batchno");
  apbtch->bottom();
  int iBatchno = (int) apbtch->lBatchno();
  iBatchno++;
  IntSetDlgItemValue(HWindow,ID_BATCHNO,iBatchno);
  IntSetDlgItemValue(HWindow,ID_LINENO,1);
  SetField(ID_VENDORNO,"");
  SetField(ID_VENDORSHORT,"");
  SetField(ID_INVOICENO,"");
  SetField(ID_ORDERNO,"");
  SetField(ID_REFERENCE,"");
  SetField(ID_APCONTROL,"");
  SetField(ID_APDEPART,"");
  SetField(ID_ACCOUNTS,"");
  SetField(ID_DEPART,"");
  SetField(ID_VATPERCENTAGE,"");
  SetField(ID_TERMS,"");
  SetField(ID_VATACCT,"");
  SetField(ID_VATDEPT,"");
  SetField(ID_VIN_NUM,"");
 
  SetField(1037,"");
  SetField(1038,"");
  SetField(1030,"");
  SetField(1999,"");
  SetField(ID_PRODUCTS,"");
  SetField(ID_MEDREPS,"");
  SetField(1002,"");
  SetField(1003,"");
  SetField(1008,"");
  SSSetColor(GetDlgItem(D->HWindow,ID_DISTLIST), SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
  SSClearData(GetDlgItem(D->HWindow,ID_DISTLIST),SS_ALLCOLS,SS_ALLROWS);
  FloatSetDlgItemValue(HWindow,ID_DEBIT,0.00);
  FloatSetDlgItemValue(HWindow,ID_CREDIT,0.00);
  FloatSetDlgItemValue(HWindow,ID_INVOICE,0.00);
  FloatSetDlgItemValue(D->HWindow,ID_DEBITTOTAL,0.00);
  FloatSetDlgItemValue(D->HWindow,ID_CREDITTOTAL,0.00);
  FloatSetDlgItemValue(D->HWindow,ID_DIFFERENCE,0.00);
  DebitTotal=CreditTotal=0;
  SetFocus(GetDlgItem(HWindow,ID_VENDORNO));
  LoadDoctors();
  LoadMedReps();
  LoadProducts();
  return;
}



void TAPInvoiceEntry::New(RTMessage)
{
 if(bTransaction)
 {
  int rc = MessageBox(HWindow,GetString(ID_COMMIT),szApp,MB_YESNOCANCEL |MB_ICONQUESTION);
  if(rc == IDYES)
   {  
    Clear();
    SlipList->List->Redraw("ar_btch");
   }     
  else if (rc == IDNO)
   {
    SendMessage(HWindow,WM_COMMAND,CM_ROLLBACK,NULL);
   }
  else if (rc == IDCANCEL)
  return;
 }
 else
 Clear();
 D->Show(SW_HIDE);
 return;
}



void TAPInvoiceEntry::Cancel (RTMessage )
{

 if(bTransaction)
 {
  switch(MessageBox(HWindow,GetString(ID_COMMIT),szApp,MB_YESNOCANCEL |MB_ICONQUESTION))
	{
	 case IDYES     : Clear(); break;
	 case IDNO      : SendMessage(HWindow,WM_COMMAND,CM_ROLLBACK,NULL); break;
	 case IDCANCEL  : return;
	}
 }

  VendorList->List->SavePosition();
  SlipList->List->SavePosition();
  AcctList->List->SavePosition();

  cb.closeAll();
  cb.initUndo();
  cb.init();
  ShutDownWindow();
  return;
}

void TAPInvoiceEntry::HandleDepart (RTMessage ){}

void TAPDistList::SelectCell (RTMessage Msg)
{
  //Check

  LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
  unsigned int nIndx = (unsigned int) Cell->Row;
  apbtd->SetIndex("batchno");
  int iBatchno;
  IntGetValue(GetDlgItem(Parent->HWindow,ID_BATCHNO),(LPLONG)&iBatchno);
  HWND hWin = GetDlgItem(HWindow,ID_DISTLIST);
  apbtd->iCurrentRecord = 0;
  if(iBatchno > 0)
  {
   if(apbtd->seek(iBatchno) == 0)
    {
     while(iBatchno == apbtd->lBatchno())
      {
	if(nIndx == apbtd->iJournal())
	 {
	  apbtd->iCurrentRecord = (int) apbtd->recNo();
	  break;
	 }
        apbtd->skip();
      }
    }
   }
  if(apbtd->iCurrentRecord == 0)
   {   
    SSSetColor(hWin,SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
    return;
   }

  SSSetColor(hWin,SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
  SSSetColor(SHWindow,SS_ALLCOLS,nIndx,RGB(192,220,192), RGBCOLOR_BLUE);
  SendMessage(Parent->HWindow,WM_COMMAND,CM_LINEITEM,NULL);  
  return;
}

void TAPInvoiceEntry::SetLineItem(RTMessage)
{
  SetField(IDOK,GetString(ID_MODIFY));
  SetField(ID_JDESCRIPT,apbtchd->descript);
  SetField(ID_ACCOUNTS ,apbtchd->account);
  SetField(ID_DEPART   ,apbtchd->dept);
  SetField(ID_DEBIT    ,apbtchd->debit);
  SetField(ID_CREDIT   ,apbtchd->credit);
  SetField(ID_LINENO   ,apbtchd->entryno);
  bModify = TRUE;
  //ChangeSize(TRUE);
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


int TAPInvoiceEntry::CheckGLAccount(char *account)
{
 if(glacct->seek(account)==0)
 return TRUE;
 else
 return FALSE;
}




void TPickList::SetListSize(int Rows)
{
 RECT rc,wd;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 LOGFONT lfFont;
 HFONT   hFont;
 lfFont.lfHeight          = -11;
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
 short iHeight,iHeader;
 SSGetRowHeightInPixels(SHWindow,1,&iHeight);
 SSGetRowHeightInPixels(SHWindow,0,&iHeader); 
 int Height =  GetSystemMetrics(SM_CYCAPTION)+ 2*GetSystemMetrics(SM_CXDLGFRAME) + ((Rows-1)*iHeight) + iHeader + 1 ;
 GetWindowRect(HWindow,&wd);
 GetClientRect(HWindow,&rc); 
 MoveWindow(HWindow,wd.left,wd.top,wd.right-wd.left,Height,TRUE);
 InflateRect(&rc, GetSystemMetrics(SM_CXBORDER),GetSystemMetrics(SM_CYBORDER));
 MoveWindow(SHWindow,rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
 
 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,TRUE);
 SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetBool(SHWindow,SSB_ALLOWUSERRESIZE,FALSE);
 SSSetBool(SHWindow,SSB_AUTOSIZE,TRUE);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);

 
}


void TGlWindow::APBatch       (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPInvoiceEntry(this,"AR_BATCH"));
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

void TAPDistList::SetListSize (int Rows)
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

#pragma argsused
void TAPInvoiceEntry::CreateLineItem(PTapbtch apbtch,PTapbtchd apbtchd)
{
}

void TARToolBox::WMActivate      (RTMessage )
{
 GetClientRect(HWindow,&rc);

 rc.bottom = rc.top + GetSystemMetrics(SM_CYCAPTION) /2;
 InvalidateRect(HWindow,&rc,FALSE);
}
void TARToolBox::WMPaint         (RTMessage )
{
 hCaptionFont = CreateFont(-1*(GetSystemMetrics(SM_CYCAPTION)/2),
			   0,0,0, 400, FALSE, FALSE, FALSE,
			   ANSI_CHARSET,OUT_CHARACTER_PRECIS,
			   CLIP_DEFAULT_PRECIS,PROOF_QUALITY,
                           VARIABLE_PITCH | FF_SWISS, (LPSTR) "Small Font");

 WORD wCaptionHeight = GetSystemMetrics(SM_CYCAPTION) /2;
 hDC = BeginPaint(HWindow,&ps);
 hOldFont = (HFONT)SelectObject(hDC, hCaptionFont);
 GetClientRect(HWindow,&rc);
 rc.bottom = wCaptionHeight;
 rc.left   = wCaptionHeight;
 GetWindowText(HWindow,szCaption,sizeof(szCaption));
 WORD wWidth = LOWORD (GetTextExtent(hDC,szCaption,strlen(szCaption)));
 SetBkMode(hDC,TRANSPARENT);
 if (GetFocus() == HWindow)
  {
   hCaptionBrush = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION));
   SetTextColor(hDC,GetSysColor(COLOR_CAPTIONTEXT));
  }
 else
  {
   hCaptionBrush = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION));
   SetTextColor(hDC,GetSysColor(COLOR_INACTIVECAPTIONTEXT));
  }
 FillRect(hDC,&rc,hCaptionBrush);
 DeleteObject(hCaptionBrush);
 ExtTextOut(hDC,rc.left + (rc.right-rc.left-wWidth)/2, -1, ETO_CLIPPED, &rc, szCaption,strlen(szCaption),NULL);
 SelectObject(hDC,hOldFont);
 rc.right = rc.left;
 rc.left = 0;
 SetBkColor (hDC,RGB(192,192,192));

 ExtTextOut(hDC,0,0,ETO_OPAQUE | ETO_CLIPPED, &rc, NULL, 0 ,NULL);
 GetClientRect(HWindow,&rc);
 hOldPen = (HPEN)SelectObject(hDC, GetStockObject(BLACK_PEN));
 MoveTo(hDC, ((wCaptionHeight+2)/4)-1, (wCaptionHeight/2)-1);
 LineTo(hDC, (3*((wCaptionHeight+2)/4))-1, (wCaptionHeight/2)-1);
 MoveTo(hDC, wCaptionHeight, 0);
 LineTo(hDC, wCaptionHeight,wCaptionHeight);
 MoveTo(hDC, 0, wCaptionHeight-1);
 LineTo(hDC, rc.right+1, wCaptionHeight-1);
 SelectObject(hDC, hOldPen);
 EndPaint(HWindow,&ps);
 
}

void TARToolBox::WMNchittest     (RTMessage Msg)
{
 
      GetClientRect (HWindow, &rc );

      // Figure the height of the fake caption
      rc.bottom = rc.top + GetSystemMetrics ( SM_CYCAPTION )/2;

      // remove out the area where the fake system menu is
      rc.left   = GetSystemMetrics ( SM_CYCAPTION )/2;

      pt.x = (int) LOWORD (Msg.LParam);
      pt.y = (int) HIWORD (Msg.LParam);
      ScreenToClient (HWindow, &pt );
      if (PtInRect (&rc, pt))
	Msg.Result  = HTCAPTION;
      else
	DefWndProc (Msg);

}

void TARToolBox::SetupWindow()
{
 RECT rect;
 GetWindowRect(Parent->HWindow,&rect);

 int NUMBER_OF_ROWS = 6;
 int NUMBER_OF_COLS = 2;
 int BUTTON_WIDTH   = 30;
 int BUTTON_HEIGHT  = 25;

 int iWidth = NUMBER_OF_COLS*BUTTON_WIDTH;
 int iHeight = NUMBER_OF_ROWS*BUTTON_HEIGHT+GetSystemMetrics(4)/2;

 MoveWindow(HWindow,rect.right-20,rect.top+20,iWidth,iHeight,0);

 bSearchAccounts = new TSuperButton(this,ID_VIEW,"",10,10,10,10);
 bBottom         = new TSuperButton(this,ID_BOTTOM,"",10,10,10,10);
 bTop            = new TSuperButton(this,ID_TOP,"",10,10,10,10);
 bNext           = new TSuperButton(this,ID_NEXT,"",10,10,10,10);
 bPrevious       = new TSuperButton(this,ID_PREVIOUS,"",10,10,10,10);
 bNew            = new TSuperButton(this,ID_NEW,"",10,10,10,10);
 bSlips          = new TSuperButton(this,ID_SLIPLIST,"",10,10,10,10);
 bOk             = new TSuperButton(this,IDOK,"",10,10,10,10);
 bDelete         = new TSuperButton(this,ID_DELETE,"",10,10,10,10);
 bCancel         = new TSuperButton(this,IDCANCEL,"",10,10,10,10);
 bMemo           = new TSuperButton(this,ID_SEARCHCUST,"",10,10,10,10);
 bCustomers      = new TSuperButton(this,ID_MEMO,"",10,10,10,10);


 GetApplication()->MakeWindow(bSearchAccounts);
 GetApplication()->MakeWindow(bBottom);
 GetApplication()->MakeWindow(bTop);
 GetApplication()->MakeWindow(bNext);
 GetApplication()->MakeWindow(bPrevious);
 GetApplication()->MakeWindow(bNew);
 GetApplication()->MakeWindow(bSlips);
 GetApplication()->MakeWindow(bOk);
 GetApplication()->MakeWindow(bDelete);
 GetApplication()->MakeWindow(bCancel);
 GetApplication()->MakeWindow(bMemo);
 GetApplication()->MakeWindow(bCustomers);

 bSearchAccounts->Attr.Style |= SBS_TEXTBITMAP | SBS_NOPOINTERFOCUS;
 bBottom->Attr.Style 	     |= SBS_TEXTBITMAP | SBS_NOPOINTERFOCUS;
 bTop->Attr.Style            |= SBS_TEXTBITMAP | SBS_NOPOINTERFOCUS;
 bNext->Attr.Style           |= SBS_TEXTBITMAP | SBS_NOPOINTERFOCUS;
 bPrevious->Attr.Style       |= SBS_TEXTBITMAP | SBS_NOPOINTERFOCUS;
 bNew->Attr.Style            |= SBS_TEXTBITMAP | SBS_NOPOINTERFOCUS;
 bSlips->Attr.Style          |= SBS_TEXTBITMAP | SBS_NOPOINTERFOCUS;
 bOk->Attr.Style             |= SBS_TEXTBITMAP | SBS_NOPOINTERFOCUS;
 bDelete->Attr.Style         |= SBS_TEXTBITMAP | SBS_NOPOINTERFOCUS;
 bCancel->Attr.Style         |= SBS_TEXTBITMAP | SBS_NOPOINTERFOCUS;


 SetBitmap(ID_TOP	,"TOP",1);
 SetBitmap(ID_BOTTOM	,"BOTTOM",2);
 SetBitmap(ID_NEXT	,"NEXT",3);
 SetBitmap(ID_PREVIOUS	,"PREVIOUS",4);
 SetBitmap(ID_NEW	,"DEFAULT",5);      

 SetBitmap(ID_SLIPLIST	,"INSERT",6);
 SetBitmap(IDOK		,"ADDLINE",7);
 SetBitmap(ID_DELETE	,"ERASE",8);
 SetBitmap(ID_VIEW	,"LOOK",9); 
 SetBitmap(ID_SEARCHCUST,"CUSTOMERS",10);
 SetBitmap(ID_MEMO      ,"MEMO",11);
 SetBitmap(IDCANCEL	,"QUIT",12);


}

void TARToolBox::SetBitmap(int ID, char *szBitmap,int iPosition)
{
 RECT rect;
 GetClientRect(HWindow,&rect); 
 int iWidth = 30;
 int iHeight = 25;
 int left = rect.left;
 int top  = rect.top+10;
 switch (iPosition)
   {
    case 1: break;
    case 2: left = left + iWidth;break;
    case 3: top = top + iHeight;break;
    case 4: left = left + iWidth;top = top + iHeight;break;
    case 5: top = top + 2*iHeight;break;
    case 6: left = left + iWidth;top = top + 2*iHeight;break;
    case 7: top = top + 3*iHeight;break;
    case 8: left = left + iWidth;top = top + 3*iHeight;break;
    case 9: top = top + 4*iHeight;break;
    case 10: left = left + iWidth;top = top + 4*iHeight;break;
    case 11: top = top + 5*iHeight;break;
    case 12: left = left + iWidth;top = top + 5*iHeight;break;
   }
 MoveWindow(GetDlgItem(HWindow,ID),left,top,iWidth,iHeight,0);
 SendMessage(GetDlgItem(HWindow,ID),SBM_SETSHADOWSIZE,1,NULL);
 SendMessage(GetDlgItem(HWindow,ID),SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)(LPSTR)szBitmap);

}
