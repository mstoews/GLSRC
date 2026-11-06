#include "tbowl.h"
#include "d4all.hpp"
#include "math.h"
#include "utildlg.h"

#include "ar.h"
#include "ar_btch.h"
#include "gl_pds.hpp"


#define ID_NEXT	1071
#define ID_BOTTOM	4067
#define ID_TOP	4066

#define ID_REFERENCE	3036
#define ID_VIN_NO	1004

#define ID_VIN_NUM 1004
#define ID_UNITS   1005
#define ID_INVOICE 1030

#define ID_COMMIT     1056

#define ID_UNDO       3075
#define ID_CUT        3076
#define ID_COPY       3077
#define ID_PASTE      3078
#define ID_CLEAR      3090
#define AR_BATCH	1053
#define ID_VENDORNO	4001
#define ID_VENDORSHORT	4002
#define ID_PERIOD	1091
#define ID_INVDATE	4074
#define ID_AR_BATCHENTRY 3037
#define ID_EDITOR 3046 


static COLORREF lpBack, lpFore;
extern char szApp[20];
extern HINSTANCE hInst;
extern BOOL crpeOpen;

//TARCustomerList *VendorList;

class TARMemo: public TMainDialog
{
 public:
 Tarbtch *arbtch;
 TARMemo(PTWindowsObject AParent,int ATitle,Tarbtch *ar):TMainDialog(AParent,ATitle)
 {
  arbtch = ar;
 }
 virtual void SetupWindow();
 virtual void Ok     (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Save   (RTMessage Msg) = [CM_FIRST + IDOK]{Ok(Msg);}
 virtual void Quit   (RTMessage Msg) = [CM_FIRST + IDCANCEL];
 virtual void Undo   (RTMessage Msg) = [CM_FIRST + ID_UNDO];
 virtual void Copy   (RTMessage Msg) = [CM_FIRST + ID_COPY];
 virtual void Paste  (RTMessage Msg) = [CM_FIRST + ID_PASTE];
 virtual void Cut    (RTMessage Msg) = [CM_FIRST + ID_CUT];
 virtual void Clear  (RTMessage Msg) = [CM_FIRST + ID_CLEAR];
};

		//TARBatchList
void TARBatchList::WMSize(RTMessage Msg)
{
	 // NEW
  TWindow::WMSize(Msg);
  int iRows = ar_btch->recCount();

  SetWindowPos(List->HWindow, 0, -1, -1, LOWORD(Msg.LParam)+2,
  HIWORD(Msg.LParam)+2, SWP_NOZORDER);
  List->SetBool(SSB_REDRAW,FALSE);
  RECT rc;
  short d1Width,d2Width,iHeight = 0;
  List->GetClientRect(&rc);
  List->GetRowHeightInPixels(1,&iHeight);
  int cRows = ((rc.bottom - rc.top) / iHeight) + 1;

  if(iRows < (cRows))
	List->SetMaxRows(cRows);
  else
	List->SetMaxRows(iRows);

  List->GetColWidthInPixels(1,(LPSHORT)&d1Width);
  List->GetColWidthInPixels(2,(LPSHORT)&d2Width);
  short ColWidth = ((rc.right - rc.left) - d1Width - d2Width);
  List->SetColWidthInPixels(3,ColWidth);
  List->SetBool(SSB_REDRAW,TRUE);

}



BOOL Tarbtch::Open()
{
 if(strcmp(mode->str(),"1") == 0)
  return TRUE;
 else
  return FALSE;
}

BOOL Tarbtch::Posted()
{
 if(strcmp(mode->str(),"2") == 0)
  return TRUE;
 else
  return FALSE;
}

BOOL Tarbtch::New(long iBatchno)
{
 SetIndex(GetString(hInst,8140));
 if(seek(iBatchno) != 0)
  return TRUE;
 else
  return FALSE;
}

void TARInvoiceEntry::AddVendor(RTMessage)
{
 SendMessage(Parent->HWindow,WM_COMMAND,ID_APADDVENDORS,NULL);
}

long TARInvoiceEntry::GetPeriod()
{
 char szPeriod[3];
 DWORD Indx = SendDlgItemMsg(ID_PERIOD,CB_GETCURSEL,0,0);
 SendDlgItemMsg(ID_PERIOD,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szPeriod);
 return atol(szPeriod);
}

void TARInvoiceEntry::Period(RTMessage Msg)
{
 char szPeriod[3];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  GL_PDS gl_pds;
  gl_pds.open(cb);
  gl_pds.SetIndex("PERIOD");
  int Indx = (int)SendDlgItemMsg(ID_PERIOD,CB_GETCURSEL,0,0);
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

#pragma argsused
void TARInvoiceEntry::ChangeSize(BOOL bFull)
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




void TARInvoiceEntry::Top(RTMessage Msg)
{
 arbtch->SetIndex(GetString(8140));
 arbtch->top();
 while(arbtch->Posted())
  arbtch->skip();
  CurrentBatch(Msg);
};
void TARInvoiceEntry::Bottom(RTMessage Msg)
{
 arbtch->SetIndex(GetString(8140));
 arbtch->bottom();
 CurrentBatch(Msg);
};

void TARInvoiceEntry::Next(RTMessage Msg)
{
 arbtch->SetIndex(GetString(8140));
 int iBatchno = 0;
 IntGetDlgItemValue(HWindow,ID_BATCHNO,(LPLONG)&iBatchno);
 if(arbtch->seek(iBatchno) == 0)
 {
	if(!arbtch->eof())
	{
	 arbtch->skip();
	 if(arbtch->eof())
	 arbtch->bottom();
	 CurrentBatch(Msg);
	}
 }
 else
 return;

};


void TARInvoiceEntry::Previous(RTMessage Msg)
{
 arbtch->SetIndex(GetString(8140));
 int iBatchno = 0;
 IntGetDlgItemValue(HWindow,ID_BATCHNO,(LPLONG)&iBatchno);
 if(arbtch->seek(iBatchno) == 0)
 { 
   if(!arbtch->bof())
	{
	 arbtch->skip(-1);
    CurrentBatch(Msg);
	}
 }
 else if (arbtch->eof())
 {
	 arbtch->bottom();
	 CurrentBatch(Msg);
 }
 else
 return;
};


void TARInvoiceEntry::SelectProduct(RTMessage Msg)
{

 char szProductCode[41];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  long Indx = SendDlgItemMsg(ID_PRODUCTS,CB_GETCURSEL,0,0);
  if(CB_ERR != Indx)
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

void TARInvoiceEntry::SelectDoctor(RTMessage Msg)
{
 char szDoctor[41];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  long Indx = SendDlgItemMsg(ID_DRCODES,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(ID_DRCODES,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szDoctor);
	 drcodes->SetIndex("rpname");
	 if(drcodes->seek(szDoctor) == 0)
		SetField(1008,drcodes->drcode->str());
	}
 }
}


void TARInvoiceEntry::SelectMedRep(RTMessage Msg)
{
 char szMedRepCode[41];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  long Indx = SendDlgItemMsg(ID_MEDREP,CB_GETCURSEL,0,0);
  SendDlgItemMsg(ID_MEDREP,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR) szMedRepCode);
  if(Indx != CB_ERR)
	{
	 medreps->SetIndex("rpname");
	 if(medreps->seek(szMedRepCode) == 0)
		SetField(ID_REPS,medreps->repcode->str());
	}
 }
}


void TARInvoiceEntry::Delete(RTMessage Msg)
{
  if(bModify)
  {
   int iEntryno;
   IntGetValue(GetDlgItem(HWindow,ID_LINENO),(LPLONG)&iEntryno);  
	arbtchd->SetIndex(GetString(8140));
	if(arbtchd->seek(arbtch->lBatchno()) == 0)
	{
	if(MessageBox(HWindow,GetString(6098),szApp,MB_YESNO | MB_ICONQUESTION) == IDYES)
	 {
	  HourGlassOn();
	  while(arbtchd->lBatchno() == arbtch->lBatchno())
      {
		 if(arbtchd->iJournal() == iEntryno)
        {
	 arbtchd->deleteRec();
	 arbtchd->pack();
		  }
	 arbtchd->skip();
		}
	  HourGlassOff();
	 }

  }
  CurrentBatch(Msg);
 }
 return;
};


void TARInvoiceEntry::RollBack(RTMessage)
{
 HourGlassOn();
 arbtch->SetIndex(GetString(8140));
 if(arbtch->seek(lGetField(ID_BATCHNO)) == 0)
  {
	arbtch->deleteRec();

  }
 if(arbtchd->seek(lGetField(ID_BATCHNO)) == 0)
  while(arbtchd->lBatchno() == arbtch->lBatchno()) {
	 arbtchd->deleteRec();
	 arbtchd->skip();
  }
 arbtchd->pack();
 arbtch->pack();
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
 RECT rc;
 GetClientRect(HWindow,&rc);
 MoveWindow(GetDlgItem(HWindow,ID_EDITOR),rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,TRUE);
 SetField(3046,arbtch->memo);
}

void TARMemo::Ok (RTMessage)
{
 arbtch->memo->assign(GetField(3046));
 arbtch->flush();
 ShutDownWindow();
 return;
}

void TARMemo::Quit (RTMessage)
{
 arbtch->flush();
 ShutDownWindow();
 return;
}


void TARInvoiceEntry::Memo(RTMessage)
{
 arbtch->SetIndex(GetString(8140));
 if(arbtch->seek(lGetField(ID_BATCHNO)) == 0)
  GetApplication()->ExecDialog(new TARMemo(this,1060,arbtch));
 else
  MessageBox(HWindow,GetString(6088),szApp,MB_ICONINFORMATION);
}

/*
void TARInvoiceEntry::GoMemo(RTMessage)
{
  SendMessage(HWindow,WM_COMMAND,ID_MEMO,NULL);
  MessageBox(HWindow,"GO MEMO","",MB_OK);
}

*/

void TARInvoiceEntry::LoadMedReps(void)
{
 SendDlgItemMsg(ID_MEDREP,CB_RESETCONTENT,NULL,NULL);

 for(medreps->top();!medreps->eof();medreps->skip())
	SendDlgItemMsg(ID_MEDREP,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)medreps->rpname->str());
	SendDlgItemMsg(ID_MEDREP,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)" ");

}

void TARInvoiceEntry::LoadProducts(void)
{
 SendDlgItemMsg(ID_PRODUCTS,CB_RESETCONTENT,NULL,NULL);
 prods->SetIndex("fpcode");
 prods->top();
 for(prods->top();!prods->eof();prods->skip())
	SendDlgItemMsg(ID_PRODUCTS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)prods->fFulln->str());
	SendDlgItemMsg(ID_PRODUCTS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)" ");
}


void TARInvoiceEntry::SetupWindow()
{
 TMainDialog::SetupWindow();

 if(!CheckFile(GetString(2057),Parent->HWindow) ||
	 !CheckFile(GetString(2058),Parent->HWindow) ||
	 !CheckFile(GetString(2045),Parent->HWindow) ||
	 !CheckFile(GetString(2046),Parent->HWindow) ||
	 !CheckFile(GetString(2047),Parent->HWindow) ||
	 !CheckFile(GetString(2048),Parent->HWindow) ||
	 !CheckFile(GetString(2049),Parent->HWindow) ||
	 !CheckFile(GetString(2050),Parent->HWindow) ||
	 !CheckFile(GetString(2051),Parent->HWindow) ||
	 !CheckFile(GetString(2052),Parent->HWindow))
	{
	 ShutDownWindow();
	 return;
	}



 bTransaction = FALSE;
 bModify = FALSE;

 RECT button;
 GetWindowRect(HWindow,&rc);
 GetWindowRect(GetDlgItem(HWindow,IDOK),&button);
 int width = rc.right - (rc.left + (button.right - button.left + 12));
 //MoveWindow(HWindow,rc.left,rc.top,width,rc.bottom-rc.top,TRUE);

 SendDlgItemMsg(ID_SEARCHCUST,SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)(LPSTR)"SEEK");

 HourGlassOn();
 DebitTotal=0;
 CreditTotal=0;

 drcodes =  new Tgldrcodes(&cb);
 arcust  =  new Tarcust(&cb);
 arbtch  =  new Tarbtch(&cb);
 arbtchd =  new Tarbtchd(&cb);
 glacct  =  new Tglaccts(&cb);
 prods   =  new Tprods(&cb);
 medreps =  new Tglmedreps(&cb);
 ardist  =  new Tardist(&cb);

 LoadMedReps();
 LoadProducts();
 LoadDoctors();

 IntSetDlgItemRange(HWindow,6086,0,6086);
 IntSetDlgItemRange(HWindow,6086,0,4065);

 arbtch->SetIndex(GetString(8140));
 arcust->SetIndex("vendorno");

 // Get the AP Control Accounts.

 AcctList = new TAccountList(this,"",glacct);
 AcctList->EnableAutoCreate();
 GetApplication()->MakeWindow(AcctList);


 VendorList = new TARCustomerList(this,"",arcust);
 VendorList->EnableAutoCreate();
 GetApplication()->MakeWindow(VendorList);


 SlipList = new TARBatchList(this,"",arbtch);
 SlipList->EnableAutoCreate();
 GetApplication()->MakeWindow(SlipList);

 bSlipList = TRUE;
 D = new TARDistList(this,GetString(8156),arbtchd);
 D->EnableAutoCreate();
 GetApplication()->MakeWindow(D);

 long db=0;
 arbtch->bottom();
 db = arbtch->lBatchno();
 db++;
 IntSetDlgItemValue(HWindow,ID_BATCHNO,db);
 bFullSize = TRUE;
 //ChangeSize(FALSE);
 IntSetDlgItemValue(HWindow,ID_LINENO,1);
 SetPeriods(HWindow);
 SetCaption(GetString(ID_AR_BATCHENTRY));
 //ToolBox = new TARToolBox(this,"ToolBox");
 //ToolBox->Attr.Style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT | WS_POPUPWINDOW;
 //ToolBox->EnableAutoCreate();
 //GetApplication()->MakeWindow(ToolBox);
 //ToolBox->Show(SW_SHOW);
 SetBitmap(ID_SLIPLIST);
 SetBitmap(1121);
 SetBitmap(3043);
 SetBitmap(2000);
 SetBitmap(ID_PRINT);
 SetBitmap(1100);
 D->Show(SW_SHOW);
 HourGlassOff();
}


void TARInvoiceEntry::Print(RTMessage)
{

  if(arbtch->seek(dGetField(ID_BATCHNO)) == 0)
	 PrintSlip(D->GetReportName(),(int)dGetField(ID_BATCHNO));
}


void TARInvoiceEntry::SetVendor(RTMessage)
{
	SetField(ID_VENDORNO       , arcust->vendorno );
	SetField(ID_VENDORSHORT    , arcust->vdshort );
	SetField(ID_APCONTROL      , arcust->account );
	SetField(ID_APDEPART       , arcust->depart );
	SetField(ID_AR_ACCT	      , arcust->ar_acct );
	SetField(ID_AR_DEPT	      , arcust->ar_dept );
	SetField(ID_VATACCT	      , arcust->vat_acct );
	SetField(ID_VATDEPT        , arcust->vat_dept );
   SetField(ID_VATPERCENTAGE  , arcust->vat );
   SetField(ID_VENDORCONTACT  , arcust->contact );
	SetField(ID_TERMS          , arcust->terms );
	SetField(ID_VIN_NUM        , arcust->vin );

   
	SetFocus(GetDlgItem(HWindow,ID_INVOICENO));
   return;
}

/*
void TARDistList::Print (RTMessage)
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
*/


void TARInvoiceEntry::CurrentBatch(RTMessage Msg)
{

  if(!arbtch->Open())
  {
	//char szMsg[255];
	//wsprintf(szMsg,GetString(8124),GetField(ID_INVOICENO));
   //MessageBox(HWindow,szMsg,GetString(8125),MB_ICONINFORMATION);
   return;
  }
  D->Show(SW_SHOW);


  
  SetField(ID_DEBIT,"");
  SetField(ID_CREDIT,"");

  D->List->SetBool(SSB_REDRAW,FALSE);
  double lDebitTotal,lCreditTotal,lDifference;
  char szDueDate[11],szInvDate[11];
  lDebitTotal=lCreditTotal=lDifference=0;
  if(arbtch->recCount() == 0)
  {
	MessageBox(HWindow,GetString(ID_NOBATCH),szApp,MB_ICONINFORMATION);
	return;
  }

  arcust->seek(arbtch->vendorno->str());

  IntSetDlgItemValue(HWindow,ID_BATCHNO,int (Str4ptr(arbtch->batchno->str())));
  SetField(ID_DESCRIPT,arbtch->descript);
  SetField(ID_VENDORNO,arbtch->vendorno);
  if(arcust->seek(arbtch->vendorno->str()) == 0)
	SetVendor(Msg);
  SetField(ID_INVOICENO,arbtch->invoiceno);
  SetField(ID_ORDERNO,arbtch->orderno);
  SetField(ID_REFERENCE,arbtch->reference);
  SetField(ID_APCONTROL,arbtch->account);
  SetField(ID_APDEPART,arbtch->depart);
  SetField(ID_AR_ACCT,arcust->ar_acct);
  SetField(ID_AR_DEPT,arcust->ar_dept);

  InvoiceDate.assign(arbtch->invdate->str(),"CCYYMMDD");
  InvoiceDate.format(szInvDate,GetString(4024));
  SetField(ID_INVDATE,szInvDate);

  DueDate.assign(arbtch->duedate->str(),"CCYYMMDD");
  DueDate.format(szDueDate,GetString(4024));
  SetField(ID_DUEDATE,szDueDate);
  
  SetField(ID_TERMS,arbtch->terms);
  SetField(ID_VATPERCENTAGE,arbtch->vatpercent);
  SetField(ID_ACCOUNTS,"");
  SetField(ID_DEPART,"");

  FloatSetDlgItemValue(HWindow,1999,arbtch->dPayable());
  SetField(1037,"");
  SetField(1038,"");


  long i=1;
  arbtchd->SetIndex(GetString(8140));

  char *szLineNo = new char[3];
  D->List->ClearData(SS_ALLCOLS,SS_ALLROWS);

  if (arbtchd->seek(double(Str4ptr(arbtch->batchno->str()))) == 0)
  {
	while (arbtch->lBatchno() == arbtchd->lBatchno() && !arbtch->eof() )
	 {
	  if(arbtchd->dDebit() > 0)
	  {
		lDebitTotal = lDebitTotal + double(Str4ptr(arbtchd->debit->str()));
		ltoa(i,szLineNo,10);
		 D->List->SetData(1,i,szLineNo);
		 D->List->SetData(2,i,arbtchd->descript->str());
		 D->List->SetData(3,i,arbtchd->account->str());
		 D->List->SetData(4,i,arbtchd->dept->str());
		 D->List->SetData(5,i,arbtchd->debit->str());
		 D->List->SetData(6,i,arbtchd->credit->str());
		DebitTotal = lDebitTotal;
		arbtchd->entryno->assignLong(i);
		arbtchd->flush();
		arbtchd->unlock();
		i++;
	  }
	  arbtchd->skip();
	}

  }
  if (arbtchd->seek(arbtch->lBatchno()) == 0)
  {

	while (arbtch->lBatchno() == arbtchd->lBatchno() && !arbtch->eof() )
	 {
	  //if(arbtchd->dCredit() == 0 && arbtchd->dDebit() == 0)
	  if((arbtchd->dCredit() > 0) || (arbtchd->dCredit() == 0 && arbtchd->dDebit() == 0))
	  {
		lCreditTotal = lCreditTotal + double(Str4ptr(arbtchd->credit->str()));
		ltoa(i,szLineNo,10);
		 D->List->SetData(1,i,szLineNo);
		 D->List->SetData(2,i,arbtchd->descript->str());
		 D->List->SetData(3,i,arbtchd->account->str());
		 D->List->SetData(4,i,arbtchd->dept->str());
		 D->List->SetData(5,i,arbtchd->debit->str());
		 D->List->SetData(6,i,arbtchd->credit->str());
		arbtchd->entryno->assignLong(i);
		arbtchd->flush();
		arbtchd->unlock();
		i++;
	  }
	  arbtchd->skip();
	}
  }
	IntSetDlgItemValue(HWindow,ID_LINENO,i);
	arbtchd->iCurrentRecord = 0;

   short iHeight = 0;
	RECT Drc;
	GetClientRect(D->HWindow,&Drc);
	D->List->GetRowHeightInPixels(1,&iHeight);
	int cRows = (Drc.bottom - Drc.top) / iHeight + 1;
	if(i < cRows)
	 D->iRows = cRows;
	else
	 D->iRows = i;


	delete [] szLineNo;

  if(bSlipList)
	SlipList->Show(SW_HIDE);


  SetFocus(GetDlgItem(HWindow,ID_PRODUCTS));
  lDifference = lDebitTotal - lCreditTotal;
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
	D->List->SetColor(5,i,RGB(192,220,192), RGBCOLOR_BLUE);
	D->List->SetColor(6,i,RGB(192,220,192), RGBCOLOR_BLUE);
	D->List->SetBool(SSB_REDRAW,TRUE);
  return;
}

void TARInvoiceEntry::Automatic(RTMessage)
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

BOOL TARInvoiceEntry::EnterError(LPSTR szValid,UINT ID)
{
 BOOL bError = FALSE;
 if(strlen(StrTrim(szValid)) == 0)
 {

  char szField[40];
  strcpy(szField,GetString(ID));
  wsprintf(szMsg,GetString(ID_BLANKFIELD),szField);
  MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);
  bError = TRUE;
 }
 SetFocus(GetDlgItem(HWindow,ID));
 return bError;
}

void TARInvoiceEntry::Ok (RTMessage)
{


 SendDlgItemMsg(ID_DISTLIST, WM_SETREDRAW, FALSE, 0L);
 iPeriodNo = (int)lGetField(ID_PERIOD);


 if(arcust->seek(GetField(ID_VENDORNO))!=0)
  {
	MessageBeep(MB_ICONEXCLAMATION);
	SendMessage(HWindow,WM_COMMAND,ID_SEARCHCUST,NULL);
	return;
  }

 Str4large szInvoiceNumber;
 szInvoiceNumber.setLen(arbtch->vendorno->len() + arbtch->invoiceno->len());
 szInvoiceNumber.set(' ');
 szInvoiceNumber.replace(Str4ptr(GetField(ID_VENDORNO)));
 szInvoiceNumber.replace(Str4ptr(GetField(ID_INVOICENO)),arbtch->vendorno->len());

 ardist->SetIndex("invoiceno");
 if(ardist->seek(szInvoiceNumber.str()) == 0)
  {
	MessageBox(HWindow,"Invoice number has been used previously.",szApp,MB_OK);
	return;
  }

 arbtch->SetIndex(GetString(8140));
 Str4large szSales;
 szSales.assign(GetField(ID_AR_ACCT));
 szSales.add(GetField(ID_AR_DEPT));

 if(!ValidGLAccount(szSales.str())){
	MessageBox(HWindow,GetString(6089),szApp,MB_ICONINFORMATION);
	return;
 }


 Str4large szVat;
 szVat.assign(GetField(ID_VATACCT));
 szVat.add(GetField(ID_VATDEPT));

 if(!ValidGLAccount(szVat.str())){
  MessageBox(HWindow,GetString(6090),szApp,MB_ICONINFORMATION);
  return;
 }

 Str4large szAR;
 szAR.assign(GetField(ID_APCONTROL));
 szAR.add(GetField(ID_APDEPART));

 if(!ValidGLAccount(szAR.str())){
  MessageBox(HWindow,GetString(6091),szApp,MB_ICONINFORMATION);
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

 char *szDueDate = strcpy(new char [arbtch->duedate->len()+1],GetField(ID_DUEDATE));
 DueDate.assign(szDueDate,GetString(4024));
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
  if(dGetField(1030) == 0)
	{
	  if(MessageBox(HWindow,"Enter Sample Entry?",szApp,MB_YESNO | MB_ICONQUESTION) == IDYES)
	  {
		char szDept[5];
		strcpy(szDept,GetField(ID_AR_DEPT));
		char *szSalesDescription = strcpy(new char[arbtchd->descript->len()+1],GetSalesDescription());
		AddBatchDetail(iBatchno,szSalesDescription,GetField(ID_AR_ACCT),szDept,0);
		delete [] szSalesDescription;
	  }
	 SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	 return;
	}
 }


 if(arbtch->New(iBatchno))
	{
		 bSlipList = TRUE;
		 arbtch->bottom();
		 iBatchno = arbtch->lBatchno();
		 ++iBatchno;
		 CreateAPBatchEntry(iBatchno);
		 bTransaction = TRUE;
		 SetField(IDD_STATUSBOX,"");
  }
  else
	ModifyCurrentSlip(iBatchno);
 SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
 return;
}




void TARInvoiceEntry::ModifyCurrentSlip(int iBatchno)
{
	int iLine;
	IntGetDlgItemValue(HWindow,3033,(LPLONG)&iLine);

	FloatGetDlgItemValue(D->HWindow,ID_DEBITTOTAL,&DebitTotal);
	FloatGetDlgItemValue(D->HWindow,ID_CREDITTOTAL,&CreditTotal);
	//arbtch->batchno->assignLong(iBatchno);
	arbtch->mode->assign("1");
	arbtch->date->assign(InvoiceDate.str());
	arbtch->period->assignDouble(GetCurrentPeriodn());
	arbtch->orderno->assign(GetField(ID_ORDERNO));
	//arbtch->vendorno->assign(GetField(ID_VENDORNO));
	//arbtch->invoiceno->assign(GetField(ID_INVOICENO));
	arbtch->duedate->assign(DueDate.str());
	arbtch->invdate->assign(InvoiceDate.str());
	arbtch->reference->assign(GetField(ID_REFERENCE));
	arbtch->descript->assign(GetSalesDescription());
	arbtch->account->assign(GetField(ID_APCONTROL));
	arbtch->depart->assign(GetField(ID_APDEPART));
	arbtch->entryno->assignLong(lGetField(ID_LINENO));
	arbtch->terms->assignLong(lGetField(ID_TERMS));
	arbtch->UpdateUserTime(szUser);
	arbtch->flush();
	arbtch->unlock();

	double dVatPercentage = 1;

	if(!bModify)
	 {
		//Add a new line item.
		BOOL bVat = TRUE;
		double dCurrentPayable = arbtch->dPayable();
		double dCurrentVat = arbtch->dVatamt();
		lInvoiceAmount = dGetField(1037)*dGetField(1038);


		double dVatPercent = dGetField(ID_VATPERCENTAGE);
		 if((int)dVatPercent > (int)0)
		  {
		  dVatPercentage = (100+dVatPercent)/100;
		  }
		 else
		 bVat = FALSE;

		double AR_AMT = 0;
		if(dGetField(1038) == 0 && dGetField(1030) != 0)
		 AR_AMT = dGetField(1030);
		else
		 AR_AMT  = dGetField(1037)*dGetField(1038);  //AR Total


		double SALES  = floor(lInvoiceAmount/dVatPercentage*1000)/1000;
		double VAT1   = lInvoiceAmount - SALES;
		double VAT    = dCurrentVat + VAT1;
		AR_AMT += dCurrentPayable;

		arbtch->payable->assignDouble(AR_AMT);
		arbtch->vatamt->assignDouble(VAT);
		arbtch->UpdateUserTime(szUser);
		arbtch->flush();
		arbtch->unlock();


	  char szDescript[255];
	  if(strcmp(StrTrim(GetField(ID_PRODUCTS)),"") == 0)
		strcpy(szDescript,"");
	  else
		strcpy(szDescript,GetField(ID_PRODUCTS));

	  char szDept[5];
	  char szMedrep[6];
	  char szDoctor[6];
	  char szProduct[11];

	  strcpy(szMedrep,GetField(ID_REPS));
	  strcpy(szProduct,GetField(1003));
	  strcpy(szDoctor,GetField(1008));


	 strcpy(szDept,GetField(ID_AR_DEPT));
	 //UpdateBatchDetail(iBatchno,szDescript,GetField(ID_AR_ACCT),szDept,szProduct,szMedrep,szDoctor,(-1*SALES));
	 char *szSalesDescription = strcpy(new char[arbtchd->descript->len()+1],GetSalesDescription());
	 AddBatchDetail(iBatchno,szSalesDescription,GetField(ID_AR_ACCT),szDept,(-1*SALES));
	 delete [] szSalesDescription;
	 //VAT if any.
	 if(dVatPercentage != 1)
	 {
	  strcpy(szDescript,"");
	  strcpy(szDept,GetField(ID_VATDEPT));
	  UpdateBatchDetail(iBatchno,szDescript,GetField(ID_VATACCT),szDept,"","","",(-1*VAT)); // creates the VAT line
	 }

	 strcpy(szDept,GetField(ID_AR_DEPT));
	 strcpy(szDescript,"");
	 UpdateBatchDetail(iBatchno,szDescript,GetField(ID_APCONTROL),szDept,szProduct,szMedrep,szDoctor,AR_AMT);
	 bTransaction = TRUE;
	 //SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
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
	  arbtchd->go(arbtchd->iCurrentRecord);
	  DebitTotal = DebitTotal  + lDebit - double (Str4ptr(arbtchd->debit->str()));
	  CreditTotal = CreditTotal + lCredit - double (Str4ptr(arbtchd->credit->str()));
	 if(strcmp(StrTrim(GetField(ID_ACCOUNTS)),StrTrim(GetField(ID_VATACCT))) == 0)
		{
		 if(lDebit > 0)
		  arbtch->vatamt->assignDouble(-lDebit);
		 else
		 if(lCredit > 0)
		  arbtch->vatamt->assignDouble(lCredit);
		}

	  arbtch->descript->assign(GetField(ID_JDESCRIPT));
	  arbtch->UpdateUserTime(szUser);
	  arbtch->flush();
	  arbtch->unlock();
	  arbtchd->batchno->assignLong(iBatchno);
	  arbtchd->entryno->assignLong(iLine);
	  arbtchd->account->assign(GetField(ID_ACCOUNTS));
	  arbtchd->dept->assign(GetField(ID_DEPART));
	  if(strcmp(StrTrim(GetField(ID_ACCOUNTS)),StrTrim(GetField(ID_AR_ACCT))) == 0)
		arbtchd->descript->assign(GetSalesDescription());
	  else
		arbtchd->descript->assign(GetDesc(ID_ACCOUNTS));
	  arbtchd->drcode->assign(GetField(1008));
	  arbtchd->product->assign(GetField(1003));
	  arbtchd->medreps->assign(GetField(ID_REPS));
	  arbtchd->debit->assignDouble(dGetField(ID_DEBIT));
	  arbtchd->credit->assignDouble(dGetField(ID_CREDIT));
	  arbtchd->flush();
	  arbtchd->unlock();
	 }
	bModify = FALSE;
	return;
}

char *TARInvoiceEntry::GetSalesDescription(void)
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



char *TARInvoiceEntry::GetDesc(int ID)
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

void TARDistList::SetupWindow()
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


void TARInvoiceEntry::SearchGL    (RTMessage){  Accountlist();}

void TARInvoiceEntry::InvoiceData()
{
  double lDebitTotal,lCreditTotal,lDifference;
  lDebitTotal=lCreditTotal=lDifference=0;
  HWND List = GetDlgItem(D->HWindow,ID_DISTLIST);
  SSClearData(List,SS_ALLCOLS,SS_ALLROWS);


  IntSetDlgItemValue(HWindow,ID_BATCHNO,int (Str4ptr(arbtch->batchno->str())));
  SendDlgItemMsg(ID_DESCRIPT,WM_SETTEXT,NULL,(LONG)(LPSTR)arbtch->descript->str());
  IntSetDlgItemValue(HWindow,3032,GetCurrentPeriodn());
  int iLineNo = int (Str4ptr(arbtch->entryno->str()) );
  iLineNo++;
  IntSetDlgItemValue(HWindow,3033,iLineNo);
  SendDlgItemMsg(ID_DESCRIPT,WM_SETTEXT,NULL,(LONG)(LPSTR)arbtch->descript->str());

  arbtchd->SetIndex(GetString(8140));
  arbtchd->seek(double (Str4ptr(arbtch->batchno->str())));
  SendDlgItemMsg(4001,WM_SETTEXT,NULL,(LONG)(LPSTR)arbtch->vendorno->str());
  SendDlgItemMsg(ID_INVOICENO,WM_SETTEXT,NULL,(LONG)(LPSTR)arbtch->invoiceno->str());
  SendDlgItemMsg(2017,WM_SETTEXT,NULL,(LONG)(LPSTR)arbtch->orderno->str());
  SendDlgItemMsg(ID_REFERENCE,WM_SETTEXT,NULL,(LONG)(LPSTR)arbtch->reference->str());
  SendDlgItemMsg(ID_DESCRIPT,WM_SETTEXT,NULL,(LONG)(LPSTR)arbtch->descript->str());
  SendDlgItemMsg(ID_INVDATE,WM_SETTEXT,NULL,(LONG)(LPSTR)arbtch->invdate->str());
  SendDlgItemMsg(ID_DUEDATE,WM_SETTEXT,NULL,(LONG)(LPSTR)arbtch->duedate->str());


  long i=1;
  while (int (Str4ptr(arbtch->batchno->str())) == int (Str4ptr(arbtchd->batchno->str())) )
   {
     lDebitTotal = lDebitTotal + double(Str4ptr(arbtchd->debit->str()));
     lCreditTotal = lCreditTotal + double(Str4ptr(arbtchd->credit->str()));
     SSSetData(List,1,i,arbtchd->descript->str());
     SSSetData(List,2,i,arbtchd->account->str());
	  SSSetData(List,3,i,arbtchd->dept->str());
	  SSSetData(List,4,i,arbtchd->debit->str());
	  SSSetData(List,5,i,arbtchd->credit->str());
	  DebitTotal = lDebitTotal;
	  CreditTotal = lCreditTotal;
	  i=i+1;
	  arbtchd->skip(1);
	}
	lDifference = lDebitTotal - lCreditTotal;
	FloatSetMask(GetDlgItem(D->HWindow,ID_DEBITTOTAL), "999999999.99");
	FloatSetMask(GetDlgItem(D->HWindow,ID_CREDITTOTAL),"999999999.99");
	FloatSetDlgItemValue(D->HWindow,ID_CREDITTOTAL,lCreditTotal);
	FloatSetDlgItemValue(D->HWindow,ID_DEBITTOTAL,lDebitTotal);
	FloatSetDlgItemValue(D->HWindow,ID_DIFFERENCE,lDifference);
}



void TARInvoiceEntry::Vendorlist  ()
{
	VendorList->List->Seek(ID_VENDORNO);
	VendorList->Show(SW_SHOW);
}


void TARInvoiceEntry::Sliplist() //slip list search
{
	SlipList->List->Seek(ID_BATCHNO);
   SlipList->Show(SW_SHOW);
}



void TARInvoiceEntry::Accountlist ()
{
  AcctList->List->Seek(ID_ACCOUNTS);
  AcctList->Show(SW_SHOW);
}

void TARInvoiceEntry::Clear()
{
  bTransaction = FALSE;
  arbtch->SetIndex(GetString(8140));
  arbtch->bottom();
  int iBatchno = (int) arbtch->lBatchno();
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
  SetField(ID_AR_ACCT,"");
  SetField(ID_AR_DEPT,"");
  SetField(ID_VIN_NUM,"");
  SetField(1037,"");
  SetField(1038,"");
  SetField(1030,"");
  SetField(1999,"");
  SetField(ID_PRODUCTS,"");
  SetField(ID_MEDREPS,"");
  SetField(ID_REPS,"");
  SetField(1003,"");
  SetField(1008,"");
  D->List->SetBool(SSB_REDRAW,FALSE);
  D->List->SetColor(SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
  D->List->ClearData(SS_ALLCOLS,SS_ALLROWS);
  D->List->SetBool(SSB_REDRAW,TRUE);
  FloatSetDlgItemValue(HWindow,ID_DEBIT,0.00);
  FloatSetDlgItemValue(HWindow,ID_CREDIT,0.00);
  FloatSetDlgItemValue(HWindow,ID_INVOICE,0.00);
  SetFocus(GetDlgItem(HWindow,ID_VENDORNO));

  long Indx = SendDlgItemMsg(ID_PRODUCTS,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
  SendDlgItemMsg(ID_PRODUCTS,CB_SETCURSEL,(WPARAM)Indx,NULL);

  Indx = SendDlgItemMsg(ID_MEDREP,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
  SendDlgItemMsg(ID_MEDREP,CB_SETCURSEL,Indx,NULL);

  Indx = SendDlgItemMsg(ID_DRCODES,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
  SendDlgItemMsg(ID_DRCODES,CB_SETCURSEL,Indx,NULL);
  return;
}



void TARInvoiceEntry::New(RTMessage)
{
 if(bTransaction)
 {
  int rc = MessageBox(HWindow,GetString(ID_COMMIT),szApp,MB_YESNOCANCEL |MB_ICONQUESTION);
  if(rc == IDYES)
   {  
    Clear();
    SlipList->List->Redraw();
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
 return;
}



void TARInvoiceEntry::Cancel (RTMessage )
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
  D->SavePosition();
  cb.closeAll();
  cb.initUndo();
  cb.init();
  ShutDownWindow();
  return;
}

void TARInvoiceEntry::HandleDepart (RTMessage ){}

void TARDistList::SelectCell (RTMessage Msg)
{
  LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
  unsigned int nIndx = (unsigned int) Cell->Row;
  arbtchd->SetIndex(GetString(hInst,8140));
  int iBatchno;
  IntGetValue(GetDlgItem(Parent->HWindow,ID_BATCHNO),(LPLONG)&iBatchno);
  arbtchd->iCurrentRecord = 0;
  if(iBatchno > 0){
	if(arbtchd->seek(iBatchno) == 0){
	  while(iBatchno == arbtchd->lBatchno()){
		 if(nIndx == arbtchd->iJournal()){
			arbtchd->iCurrentRecord = (int) arbtchd->recNo();
			break;
		 }
		  arbtchd->skip();
	 }
	}
 }
 if(arbtchd->iCurrentRecord == 0){
	 SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	 return;
 }

 List->SetColor(SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
 List->SetColor(SS_ALLCOLS,nIndx,RGB(192,220,192), RGBCOLOR_BLUE);
 SendMessage(Parent->HWindow,WM_COMMAND,CM_LINEITEM,NULL);
 return;
}

void TARInvoiceEntry::SetLineItem(RTMessage)
{

  SetField(ID_JDESCRIPT,arbtchd->descript);
  SetField(ID_ACCOUNTS ,arbtchd->account);
  SetField(ID_DEPART   ,arbtchd->dept);
  SetField(ID_DEBIT    ,arbtchd->debit);
  SetField(ID_CREDIT   ,arbtchd->credit);
  SetField(ID_LINENO   ,arbtchd->entryno);
  bModify = TRUE;
  //ChangeSize(TRUE);
}

/*
void TARDistList::SetFields(UINT ID,Field4 *field)
{
  SendDlgItemMessage(Parent->HWindow,ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field->str());
  return ;
}

void TARDistList::SetFields(UINT ID, long field)
{
  IntSetDlgItemValue(Parent->HWindow,ID,field);
  return;
}

void TARDistList::SetFields(UINT ID, double field)
{
  FloatSetDlgItemValue(Parent->HWindow,ID,field);
  return;
}

void TARDistList::SetFields(UINT ID, char *field)
{
  SendDlgItemMessage(Parent->HWindow,ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field);
  return;
}

*/
int TARInvoiceEntry::CheckGLAccount(char *account)
{
 if(glacct->seek(account)==0)
 return TRUE;
 else
 return FALSE;
}


void TManilaGL::ARBatch       (RTMessage)
 {
  TARInvoiceEntry* AREntry = new TARInvoiceEntry(this,1053);
  GetApplication()->MakeWindow(AREntry);
  AREntry->Show(SW_SHOW);
  //GetApplication()->ExecDialog(new TARInvoiceEntry(this,1053));

 }

char *TARInvoiceEntry::GetControlAcct(char *Account,char *Depart)
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

/*
void TARDistList::SetListSize (int Rows)
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
*/

#pragma argsused
void TARInvoiceEntry::CreateLineItem(PTarbtch arbtch,PTarbtchd arbtchd)
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
 hCaptionBrush = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION));
 if (GetFocus() == HWindow)
  {
	SetTextColor(hDC,GetSysColor(COLOR_CAPTIONTEXT));
  }
 else
  {
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
 DeleteObject(hOldPen);
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


 SetBitmap(ID_TOP	,1);
 SetBitmap(ID_BOTTOM	,2);
 SetBitmap(ID_NEXT	,3);
 SetBitmap(ID_PREVIOUS	,4);
 SetBitmap(ID_NEW	,5);

 SetBitmap(ID_SLIPLIST	,6);
 SetBitmap(IDOK		,7);
 SetBitmap(ID_DELETE	,8);
 SetBitmap(ID_VIEW	,9);
 SetBitmap(ID_SEARCHCUST,10);
 SetBitmap(ID_MEMO    ,11);
 SetBitmap(IDCANCEL	,12);


}

void TARToolBox::SetBitmap(int ID,int iPosition)
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
 SendMessage(GetDlgItem(HWindow,ID),SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)ID);

}
