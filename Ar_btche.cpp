#include "std_inc.h"

/*
#include "tbowl.h"
#include "d4all.hpp"
#include "math.h"
#include "utildlg.h"
#include "gl.h"
#include "ar.h"
*/

#define ID_DEPT_DESC 3038
#define ID_PERIOD_USED 1073


#include "ar_id.h"
#include "ar_btch.h"
#include "ar_btchl.h"

//#include "gl_pds.hpp"


#define ID_NEXT	1071
#define ID_BOTTOM	4067
#define ID_TOP	4066

#define ID_REFERENCE	3036
#define ID_VIN_NO	1004

#define ID_VIN_NUM 1004
#define ID_UNITS   1005
#define ID_PROD    1003

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
#define ID_MEMOEDIT 3046
#define ID_ARPRIORPERIOD 8193
#define  ID_VATACCT 1034
#define  ID_VATDEPT 1035



static COLORREF lpBack, lpFore;

extern HINSTANCE hInst;
extern BOOL bNEW;


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
  int iRows = (int)ar_btch->recCount();

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
 if(strcmp(mode.str(),"1") == 0 || strcmp(mode.str(),"3") == 0)
  return TRUE;
 else
  return FALSE;
}

BOOL Tarbtch::Posted()
{
 if(strcmp(mode.str(),"2") == 0 || strcmp(mode.str(),"4") == 0)
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
  GL_PERIODS gl_pds;
  gl_pds.open(*cb);
  gl_pds.SetIndex("PERIOD");
  int Indx = (int)SendDlgItemMsg(ID_PERIOD,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(ID_PERIOD,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szPeriod);
	  if(gl_pds.seek(szPeriod) == 0){
		SetField(1084,gl_pds.pd_desc.str());
	  if(bNEW)
		SetField(ID_PERIOD_USED,szPeriod);
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

char  *TARInvoiceEntry::GetDept()
{
	static char szString[11];
	int Indx = (int)SendDlgItemMsg(ID_DEPTCODES,CB_GETCURSEL,NULL,NULL);
	if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(ID_DEPTCODES,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szString);
	}
  return szString;
}



void TARInvoiceEntry::Top(RTMessage Msg)
{
 CHourGlass wait;
 arbtch.SetIndex("POST_AR");
 arbtch.top();
 CurrentBatch(Msg);
 arbtch.SetIndexBatchno();
}
void TARInvoiceEntry::Bottom(RTMessage Msg)
{
 CHourGlass wait;

 arbtch.SetIndex("POST_AR");
 arbtch.bottom();
 while(!arbtch.Open())
 {
	arbtch.skip(-1);
	if(arbtch.bof())
	 break;
 }
 CurrentBatch(Msg);
 arbtch.SetIndexBatchno();
}

void TARInvoiceEntry::Next(RTMessage Msg)
{
 CHourGlass wait;

 int iBatchno = 0;
 IntGetDlgItemValue(HWindow,ID_BATCHNO,(LPLONG)&iBatchno);
 if(arbtch.seek(iBatchno) == 0)
 {
	arbtch.SetIndex("POST_AR");
	arbtch.skip();
	if(arbtch.eof())
	 return;
	else
	 CurrentBatch(Msg);
	 arbtch.SetIndexBatchno();
 }
 else
 return;
}


void TARInvoiceEntry::Previous(RTMessage Msg)
{
 /*
  1.	If there are no records and you go to the previous record
		you will be at the beginning of the file, therefore return.
  2.  If there are previous records but they are all posted return
		and tell the user that there are no more open records.
  3. 	If there are open records go to the first open record from your
		current position.
 */

 int iBatchno = 0;
 arbtch.SetIndex("POST_AR");
 IntGetDlgItemValue(HWindow,ID_BATCHNO,(LPLONG)&iBatchno);
 if(arbtch.seek(iBatchno) == 0)
 {

	arbtch.skip(-1);
	if(arbtch.bof())
	 return;
	while(!arbtch.Open())
	{
	  arbtch.skip(-1);
	  if(arbtch.bof())
		 break;
	}

	if(arbtch.Open())
	 CurrentBatch(Msg);

 }
 else
 {
	 arbtch.bottom();
	 while(!arbtch.Open())
	 {
		arbtch.skip(-1);
		if(arbtch.bof())
		 break;
	 }

	 if(arbtch.Open())
	  CurrentBatch(Msg);
 }
 arbtch.SetIndexBatchno();
 return;
}


void TARInvoiceEntry::SelectProduct(RTMessage Msg)
{

 char szProductCode[41];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  long Indx = SendDlgItemMsg(ID_PRODUCTS,CB_GETCURSEL,0,0);
  if(CB_ERR != Indx)
	{
	 SendDlgItemMsg(ID_PRODUCTS,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szProductCode);
	 prods.SetIndex("ffulln");
	 if(prods.seek(szProductCode) == 0)
	  {
		SetField(ID_PROD,prods.fPcode.str());
		SetField(1038,prods.fAvprice.str());
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
	 drcodes.SetIndex("rpname");
	 if(drcodes.seek(szDoctor) == 0)
		SetField(1008,drcodes.drcode.str());
	}
 }
}


/*
void TARInvoiceEntry::SelectMedRep(RTMessage Msg)
{
 char szMedRepCode[41];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  long Indx = SendDlgItemMsg(ID_MEDREP,CB_GETCURSEL,0,0);
  SendDlgItemMsg(ID_MEDREP,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR) szMedRepCode);
  if(Indx != CB_ERR)
	{
	 medreps.SetIndex("rpname");
	 if(medreps.seek(szMedRepCode) == 0)
		SetField(ID_REPS,medreps.repcode.str());
	}
 }
}
*/


void TARInvoiceEntry::Delete(RTMessage Msg)
{
  if(bModify == TRUE)
  {
	int iEntryno;
	IntGetValue(GetDlgItem(HWindow,ID_LINENO),(LPLONG)&iEntryno);
	arbtchd.SetIndex(GetString(8140));
	if(arbtchd.seek(arbtch.lBatchno()) == 0)
	{
		if(MessageBox(HWindow,GetString(6098),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			CHourGlass wait;
			  while(arbtchd.lBatchno() == arbtch.lBatchno())
				{
					if(arbtchd.iJournal() == iEntryno)
						 arbtchd.deleteRec();
					arbtchd.skip();
				}
				// arbtchd.pack(); COMMENT
				CurrentBatch(Msg);
				return;
		}
		else
		return;
	 }
  }
}


void TARInvoiceEntry::RollBack(RTMessage)
{
 CHourGlass wait;
 arbtch.SetIndex(GetString(8140));
 if(arbtch.seek(lGetField(ID_BATCHNO)) == 0)
  {
	arbtch.deleteRec();

  }
 if(arbtchd.seek(lGetField(ID_BATCHNO)) == 0)
  while(arbtchd.lBatchno() == arbtch.lBatchno()) {
	 arbtchd.deleteRec();
	 arbtchd.skip();
  }
 arbtchd.pack();
 arbtch.pack();

 Clear();
 return;
}



void TARMemo::Cut (RTMessage)
{
  SendMessage(GetDlgItem(HWindow,ID_MEMOEDIT),WM_CUT,0,0L);
}

void TARMemo::Copy (RTMessage)
{
 SendMessage(GetDlgItem(HWindow,ID_MEMOEDIT),WM_COPY,0,0L);
}

void TARMemo::Paste (RTMessage)
{
 SendMessage(GetDlgItem(HWindow,ID_MEMOEDIT),WM_PASTE,0,0L);
}

void TARMemo::Clear (RTMessage)
{
 SendMessage(GetDlgItem(HWindow,ID_MEMOEDIT),EM_SETSEL,0,MAKELONG(0,32767));
 SendMessage(GetDlgItem(HWindow,ID_MEMOEDIT),WM_CLEAR,0,0L);
}

void TARMemo::Undo(RTMessage)
{
 SendMessage(GetDlgItem(HWindow,ID_MEMOEDIT),WM_UNDO,0,0L);
}

void TARMemo::SetupWindow()
{
 RECT rc;
 GetClientRect(HWindow,&rc);
 MoveWindow(GetDlgItem(HWindow,ID_MEMOEDIT),rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,TRUE);
 //SetField(3046,arbtch->memo);
}

void TARMemo::Ok
 (RTMessage)
{
 CHourGlass wait;
 string strMemo(GetField(3046));
 //arbtch->memo.assign((LPSTR)strMemo.c_str());
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
 arbtch.SetIndex(GetString(8140));
 if(arbtch.seek(lGetField(ID_BATCHNO)) == 0)
  GetApplication()->ExecDialog(new TARMemo(this,1060,&arbtch));
 else
  MessageBox(HWindow,GetString(6088),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
}

/*
void TARInvoiceEntry::GoMemo(RTMessage)
{
  SendMessage(HWindow,WM_COMMAND,ID_MEMO,NULL);
  MessageBox(HWindow,"GO MEMO","",MB_OK);
}

*/

/*
void TARInvoiceEntry::LoadMedReps(void)
{
 SendDlgItemMsg(ID_MEDREP,CB_RESETCONTENT,NULL,NULL);

 for(medreps.top();!medreps.eof();medreps.skip())
	SendDlgItemMsg(ID_MEDREP,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)medreps.rpname.str());
	SendDlgItemMsg(ID_MEDREP,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)" ");
}
*/

void TARInvoiceEntry::LoadDepartments(void)
{
  gldept.SetIndex("cld");
  SendDlgItemMsg(ID_DEPTCODES,CB_RESETCONTENT,NULL,NULL);
  for(gldept.top();!gldept.eof();gldept.skip())
	 {
		SendDlgItemMsg(ID_DEPTCODES,CB_ADDSTRING,NULL,(LPARAM) (LPCSTR)gldept.dept.str());
	 }
  SendDlgItemMsg(ID_DEPTCODES,CB_ADDSTRING,NULL,(LPARAM) (LPCSTR)" ");

}

void TARInvoiceEntry::LoadProducts(void)
{
 SendDlgItemMsg(ID_PRODUCTS,CB_RESETCONTENT,NULL,NULL);
 prods.SetIndex("fpcode");
 prods.top();
 for(prods.top();!prods.eof();prods.skip())
	SendDlgItemMsg(ID_PRODUCTS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)prods.fFulln.str());
	SendDlgItemMsg(ID_PRODUCTS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)" ");
}


void TARInvoiceEntry::SetupWindow()
{
 TMainDialog::SetupWindow();
 mode = "1";
 bNEW = TRUE;
 bTransaction = FALSE;
 bModify = FALSE;
 bRedraw = FALSE;
 RECT button;
 GetWindowRect(HWindow,&rc);
 GetWindowRect(GetDlgItem(HWindow,IDOK),&button);
 //int width = rc.right - (rc.left + (button.right - button.left + 12));
 //MoveWindow(HWindow,rc.left,rc.top,width,rc.bottom-rc.top,TRUE);

 SendDlgItemMsg(ID_SEARCHCUST,SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)(LPSTR)"SEEK");

 CHourGlass wait;
 DebitTotal=0;
 CreditTotal=0;

  if(drcodes.open(*cb) < 0)
  {
	ErrorClose(drcodes.GetDbfName());
	return;
  }

 if(arcust.open(*cb) < 0)
  {
	ErrorClose(arcust.GetDbfName());
	return;
  }

 if(arcust.open(*cb) < 0)
  {
	ErrorClose(arcust.GetDbfName());
	return;
  }

 if(arbtch.open(*cb) < 0)
  {
	ErrorClose(arbtch.GetDbfName());
	return;
  }

 if(arbtchd.open(*cb) < 0)
  {
	ErrorClose(arbtchd.GetDbfName());
	return;
  }

 if(glacct.open(*cb) < 0)
  {
	ErrorClose(glacct.GetDbfName());
	return;
  }

  if(prods.open(*cb) < 0)
  {
	ErrorClose(prods.GetDbfName());
	return;
  }

 if(gldept.open(*cb) < 0)
  {
	ErrorClose(gldept.GetDbfName());
	return;
  }

  if(ardist.open(*cb) < 0)
  {
	ErrorClose(ardist.GetDbfName());
	return;
  }


 //LoadMedReps();
 LoadDepartments();
 LoadProducts();
 LoadDoctors();

 IntSetDlgItemRange(HWindow,6086,0,6086);
 IntSetDlgItemRange(HWindow,6086,0,4065);

 arbtch.SetIndex(GetString(8140));
 //arcust->SetIndex("vendorno");
 arcust.SetIndexVendorno();

 // Get the AP Control Accounts.

 /*
 AcctList = new TAccountList(this,"",glacct);
 AcctList->EnableAutoCreate();
 GetApplication()->MakeWindow(AcctList);


 VendorList = new TARCustomerList(this,"",arcust);
 VendorList->EnableAutoCreate();
 GetApplication()->MakeWindow(VendorList);


 SlipList = new TARBatchList(this,"",arbtch);
 SlipList->EnableAutoCreate();
 GetApplication()->MakeWindow(SlipList);
 */

 AcctList = NULL;
 VendorList = NULL;
 SlipList = NULL;


 bSlipList = TRUE;
 D = new TARDistList(this,GetString(8156),&arbtchd);
 D->EnableAutoCreate();
 GetApplication()->MakeWindow(D);

 long db=0;
 arbtch.bottom();
 db = arbtch.lBatchno();
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

 RECT rc;
 RECT parent;
 int pos = 72;
 GetWindowRect(HWindow,&rc);
 GetWindowRect(Parent->HWindow,&parent);
 MoveWindow(HWindow,parent.left+3,parent.top+pos,rc.right-rc.left,rc.bottom-rc.top,TRUE);


 SetBitmap(ID_SLIPLIST);
 SetBitmap(1121);
 SetBitmap(3043);
 SetBitmap(2000);
 SetBitmap(ID_PRINT);
 SetBitmap(1100);
 D->Show(SW_SHOW);
 char szPeriod[3];
 SetField(ID_PERIOD_USED,ltoa(GetPeriod(),szPeriod,10));
}


void TARInvoiceEntry::Print(RTMessage)
{
  //if(arbtch.seek(dGetField(ID_BATCHNO)) == 0)
  //	 PrintSlip(D->GetReportName(),(int)dGetField(ID_BATCHNO));
}


void TARInvoiceEntry::SetVendor(RTMessage)
{
	if(arcust.seek(GetField(ID_VENDORNO)) == 0)
	{
	SetField(ID_VENDORNO       , arcust.vendorno );
	SetField(ID_VENDORSHORT    , arcust.vdshort );
	SetField(ID_APCONTROL      , arcust.account );
	SetField(ID_APDEPART       , arcust.depart );
	SetField(ID_AR_ACCT	      , arcust.ar_acct );
	SetField(ID_AR_DEPT	      , arcust.ar_dept );
	SetField(ID_VATACCT	      , arcust.vat_acct );
	SetField(ID_VATDEPT        , arcust.vat_dept );
	SetField(ID_VATPERCENTAGE  , arcust.vat );
	SetField(ID_VENDORCONTACT  , arcust.contact );
	SetField(ID_TERMS          , arcust.terms );
	SetField(ID_VIN_NUM        , arcust.vin );
	SetFocus(GetDlgItem(HWindow,ID_INVOICENO));
	}
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

}
*/


void TARInvoiceEntry::CurrentBatch(RTMessage Msg)
{
 long CurrentRecord = arbtch.recNo();
 int rc = arbtch.lock(CurrentRecord);
 if(rc != 0)
  {
	 MessageBox(HWindow,GetString(1052),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	 return;
  }

  if(!arbtch.Open())
  {
	//char szMsg[255];
	//wsprintf(szMsg,GetString(8124),GetField(ID_INVOICENO));
	//MessageBox(HWindow,szMsg,GetString(8125),MB_ICONINFORMATION);
	return;
  }
  //D->Show(SW_SHOW);

  SetField(ID_DEBIT,"");
  SetField(ID_CREDIT,"");

  D->List->SetBool(SSB_REDRAW,FALSE);
  double lDebitTotal,lCreditTotal;
  char szDueDate[11],szInvDate[11];
  lDebitTotal=lCreditTotal=0;
  if(arbtch.recCount() == 0)
  {
	MessageBox(HWindow,GetString(ID_NOBATCH),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	return;
  }

  arcust.seek(arbtch.vendorno.str());

  IntSetDlgItemValue(HWindow,ID_BATCHNO,int (Str4ptr(arbtch.batchno.str())));
  SetField(ID_DESCRIPT,arbtch.descript);
  SetField(ID_VENDORNO,arbtch.vendorno);
  if(arcust.seek(arbtch.vendorno.str()) == 0)
	SetVendor(Msg);
  SetField(ID_INVOICENO,arbtch.invoiceno);
  SetField(ID_ORDERNO,arbtch.orderno);
  SetField(ID_REFERENCE,arbtch.reference);
  SetField(ID_APCONTROL,arbtch.account);
  SetField(ID_APDEPART,arbtch.depart);
  SetField(ID_AR_ACCT,arbtch.sales_acct);
  SetField(ID_AR_DEPT,arbtch.sales_dept);
  char szPeriod[3];
  SetField(ID_PERIOD_USED,ltoa(GetPeriod(),szPeriod,10));

  InvoiceDate.assign(arbtch.invdate.str(),GetString(6115));
  InvoiceDate.format(szInvDate,GetString(4024));
  SetField(ID_INVDATE,szInvDate);

  DueDate.assign(arbtch.duedate.str(),GetString(6115));
  DueDate.format(szDueDate,GetString(4024));
  SetField(ID_DUEDATE,szDueDate);

  SetField(ID_TERMS,arbtch.terms);
  SetField(ID_VATPERCENTAGE,arbtch.vatpercent);
  SetField(ID_ACCOUNTS,"");
  SetField(ID_DEPART,"");

  //FloatSetDlgItemValue(HWindow,1999,arbtch.dPayable());
  SetField(1999,arbtch.dPayable());
  SetField(1037,"");
  SetField(1038,"");


  long i=1;
  arbtchd.SetIndex(GetString(8140));

  char *szLineNo = new char[3];
  D->List->ClearData(SS_ALLCOLS,SS_ALLROWS);

  double dARAmount = 0;
  string szAccount(arbtch.account.str());
  szAccount += arbtch.depart.str();

  if (arbtchd.seek(double(Str4ptr(arbtch.batchno.str()))) == 0)
  {
	while (arbtch.lBatchno() == arbtchd.lBatchno() && !arbtch.eof() )
	 {
	  if(arbtchd.dDebit() > 0)
	  {
		lDebitTotal = lDebitTotal + double(Str4ptr(arbtchd.debit.str()));
		ltoa(i,szLineNo,10);
		 D->List->SetData(1,i,szLineNo);
		 D->List->SetData(2,i,arbtchd.descript.str());

		 string strAccount(arbtchd.account.str());
		 strAccount += arbtchd.dept.str();

		 D->List->SetData(3,i,(LPSTR)strAccount.c_str());
		 D->List->SetData(4,i,arbtchd.depart.str());

		 D->List->SetData(5,i,arbtchd.debit.str());
		 D->List->SetData(6,i,arbtchd.credit.str());
		 DebitTotal = lDebitTotal;
		 arbtchd.entryno.assignLong(i);
		 string szARAccount(arbtchd.account.str());
		 szARAccount += arbtchd.dept.str();
		 // Assumes credit is always zero if debit > 0
		 if(szARAccount == szAccount)
		  dARAmount += arbtchd.dDebit();
		i++;
	  }
	  arbtchd.skip();
	}

  }

  double dQty = 0;

  if (arbtchd.seek(arbtch.lBatchno()) == 0)
  {

	while (arbtch.lBatchno() == arbtchd.lBatchno() && !arbtch.eof() )
	 {
	  //if(arbtchd.dCredit() == 0 && arbtchd.dDebit() == 0)
	  if((arbtchd.dCredit() > 0) || (arbtchd.dCredit() == 0 && arbtchd.dDebit() == 0))
	  {
		lCreditTotal = lCreditTotal + double(Str4ptr(arbtchd.credit.str()));
		ltoa(i,szLineNo,10);
		 D->List->SetData(1,i,szLineNo);
		 D->List->SetData(2,i,arbtchd.descript.str());
		 string strAccount(arbtchd.account.str());
		 strAccount += arbtchd.dept.str();

		 D->List->SetData(3,i,(LPSTR)strAccount.c_str());
		 D->List->SetData(4,i,arbtchd.depart.str());

		 //D->List->SetData(3,i,arbtchd.account.str());
		 //D->List->SetData(4,i,arbtchd.dept.str());
		 D->List->SetData(5,i,arbtchd.debit.str());
		 D->List->SetData(6,i,arbtchd.credit.str());
		 dQty += atof(arbtchd.qty.str());
		 arbtchd.entryno.assignLong(i);
		 string szARAccount(arbtchd.account.str());
		 szARAccount += arbtchd.dept.str();
		 // Assumes credit is always zero if debit > 0
		 if(szARAccount == szAccount)
		  dARAmount += -(arbtchd.dCredit());
		i++;
	  }
	  arbtchd.skip();
	}
  }
	i++;
	arbtch.qty.assignDouble(dQty);
	arbtch.payable.assignDouble(dARAmount);
	// Update the AR account and the total amount receivable
	SetField(1999,arbtch.dPayable());

	IntSetDlgItemValue(HWindow,ID_LINENO,i);
	arbtchd.iCurrentRecord = 0;

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


  Str4large szDebitTotal;
  Str4large szCreditTotal;
  szDebitTotal.assignDouble(lDebitTotal,16,2);
  szCreditTotal.assignDouble(lCreditTotal,16,2);
  IntSetDlgItemValue(HWindow,ID_LINENO,i);

  D->iRows = D->iRows + 2;
  D->List->SetMaxRows(D->iRows);
  D->List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);

	if(round(szDebitTotal,2) != round(szCreditTotal,2))
	{
	 D->List->SetData(2,i,GetString(6113));
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
  SetFocus(GetDlgItem(HWindow,ID_PRODUCTS));
  bModify = FALSE;
  bNEW = FALSE;
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
  MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
  bError = TRUE;
 }
 SetFocus(GetDlgItem(HWindow,ID));
 return bError;
}

void TARInvoiceEntry::Ok (RTMessage)
{
 CHourGlass wait;
 SendDlgItemMsg(ID_DISTLIST, WM_SETREDRAW, FALSE, 0L);
 iPeriodNo = (int)lGetField(ID_PERIOD);


 if(arcust.seek(GetField(ID_VENDORNO))!=0)
  {
	MessageBeep(MB_ICONEXCLAMATION);
	SendMessage(HWindow,WM_COMMAND,ID_SEARCHCUST,NULL);
	return;
  }

  /*
		New Journal vouchers must have a unique vendor name + invoice number
		to be entered.
		If they have been previously entered they may be added to if the invoice number
		matches the batch number. If the invoice number and vendor number don't match the
		corresponding batch number return a message stating that the invoice number has already
		been used.
  */


  iBatchno = (long)lGetField(ID_BATCHNO);
  if(bModify == FALSE)
  {
	arbtch.SetIndex("invoiceno");
	string szInvoiceno(GetField(ID_VENDORNO));
	szInvoiceno += GetField(ID_INVOICENO);
	if(arbtch.seek((char *)szInvoiceno.c_str()) == 0)
	{
	  if(iBatchno != arbtch.lBatchno())
		{
			 if(strcmp(arbtch.mode.str(),"1") == 0)
			  {
				if(MessageBox(HWindow,GetString(1039),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
					 SendMessage(Parent->HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
			  }
			 return;
		}
	}
  }

 arbtch.SetIndex(GetString(8140));
 Str4large szSales;
 szSales.assign(GetField(ID_AR_ACCT));
 szSales.add(GetField(ID_AR_DEPT));

 if(!ValidGLAccount(szSales.str())){
	MessageBox(HWindow,GetString(6089),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	return;
 }


 Str4large szVat;
 szVat.assign(GetField(ID_VATACCT));
 szVat.add(GetField(ID_VATDEPT));

 if(!ValidGLAccount(szVat.str())){
  MessageBox(HWindow,GetString(6090),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
  return;
 }

 Str4large szAR;
 szAR.assign(GetField(ID_APCONTROL));
 szAR.add(GetField(ID_APDEPART));

 if(!ValidGLAccount(szAR.str())){
  MessageBox(HWindow,GetString(6091),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
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

 char *szDueDate = strcpy(new char [arbtch.duedate.len()+1],GetField(ID_DUEDATE));
 DueDate.assign(szDueDate,GetString(4024));
 DueDate.assign(lDueDate);
 delete [] szDueDate;

 if(szEnterError(ID_REFERENCE))
  return;

 iLineno = lGetField(ID_LINENO);
 //dVatPercentage = dGetField(ID_VATPERCENTAGE);
 //dRebatePercentage = dGetField(ID_REBATE);


 // if not a new invoice then update the current line or add a new line.
 if(bModify == FALSE)
 {
	if(dGetField(1037) == 0)
	  {
		SetFocus(GetDlgItem(HWindow,1037));
		return;
	  }


	if(dGetField(1038) == 0)
	  {
		SetFocus(GetDlgItem(HWindow,1038));
		return;
	  }
 }

 /*
 If the product price is zero the amount total must be greater than zero.
 if(dEnterError(1038))
  return;

 */
 if(arbtch.lockAll() == r4locked)
 {
	arbtch.bottom();
	MessageBox(HWindow,GetString(10062),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	return;
 }

 // This section adds a sample entry or an entry with no sales value but records
 // the quantity and product but no price.
 if(dGetField(1038) == 0 && dGetField(1030) == 0)
 {
  if(bModify == FALSE)
  if(dGetField(1030) == 0)
	{
	  if(MessageBox(HWindow,GetString(4030),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
	  {
		LPLINEITEM *Item = new LPLINEITEM;
		Item->iBatchno   = iBatchno;
		Item->iEntryno   = 0;
		Item->szAccount  = GetField(ID_AR_ACCT);
		Item->szDept     = GetField(ID_AR_DEPT);
		Item->szDescript = GetSalesDescription();
		Item->szMedrep   = GetDept();
		Item->szProduct  = GetField(ID_PROD);
		Item->szDoctor   = GetField(1008);
		Item->dEntry     = 0;
		Item->dQty       = dGetField(1037);
		Item->dPrice     = 0;
		AddBatchDetail(Item);
		delete Item;
	  }
	 SendMessage(HWindow,WM_COMMAND,CM_CURRENTBATCH,NULL);
	 return;
	}
 }


 /*
	 If the entry is not being modified and the batchno is a new batch number then
	 go to the bottom of the database to make sure that no one has entered another entry
	 before you have started this one and then create a new entry.
	 If there is already an entry created for that batchno and we are not modifying the entry
	 add to the present entry. If the entry is modified just modify the line.
 */
 if(bModify == FALSE && arbtch.seek(lGetField(ID_BATCHNO)) != 0)
  {
	  arbtch.bottom();
	  iBatchno = arbtch.lBatchno();
	  ++iBatchno;
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





char *TARInvoiceEntry::GetSalesDescription(void)
{
 string szDesc;
 string szSeek(GetField(ID_AR_ACCT));
 szSeek += GetField(ID_AR_DEPT);

 glacct.SetIndex("accounts");
 prods.SetIndex("fpcode");

 if(strlen(StrTrim(GetField(ID_PROD))) == 0)
	{
	 if(glacct.seek((char *)szSeek.c_str()) == 0)
		 szDesc = glacct.descript.str();
	 else
		 szDesc = "";
	}
 else
 if (prods.seek(GetField(ID_PROD)) == 0)
 {
	 szDesc = prods.fFulln.str();
 }
 return (char *)szDesc.c_str();
}



char *TARInvoiceEntry::GetDesc(int ID)
{
 if(strcmp(StrTrim(GetField(ID)),"") == 0)
  {
	char szSeek[12];
	strcpy(szSeek,GetField(ID_ACCOUNTS));
	strcat(szSeek,GetField(ID_DEPART));
	if(glacct.seek(szSeek) == 0)
	 return glacct.descript.str();
  }
 if(strcmp(GetField(ID_ACCOUNTS),GetField(ID_VATACCT)) == 0 || strcmp(GetField(ID_APCONTROL),GetField(ID_ACCOUNTS)) == 0)
  {
	char szSeek[12];
	strcpy(szSeek,GetField(ID_ACCOUNTS));
	strcat(szSeek,GetField(ID_DEPART));
	if(glacct.seek(szSeek) == 0)
	 return glacct.descript.str();
  }
 else
 prods.seek(GetField(ID));
 return prods.fFulln.str();
}

void TARDistList::SetupWindow()
{
 TWindow::SetupWindow();
 RECT rc;
 CHourGlass wait;

 HFONT   hFont;

 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
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
 iLeft    = GetPrivateProfileInt(szClassName.c_str(),"L",0,PROFILE);
 iTop     = GetPrivateProfileInt(szClassName.c_str(),"T",0,PROFILE);
 iRight   = GetPrivateProfileInt(szClassName.c_str(),"R",0,PROFILE);
 iBottom  = GetPrivateProfileInt(szClassName.c_str(),"B",0,PROFILE);
 if(iTop == 0 || iBottom == 0 || iLeft == 0 || iRight == 0)
 {
	iBottom = 100;
 }
 SetPosition();

 List->SetColWidth(1,3);
 List->SetColWidth(3,10);
 List->SetColWidth(4,10);
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
 //List->SetTypeEdit(&CellType,ES_RIGHT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 List->SetCellType(3, SS_ALLROWS, &CellType);
 List->SetCellType(4, SS_ALLROWS, &CellType);
 List->SetTypeFloat(&CellType,FS_SEPARATOR,15,2,0,atof(GetString(GetApplication()->hInstance,6107)));
 List->SetCellType(5, SS_ALLROWS, &CellType);
 List->SetCellType(6, SS_ALLROWS, &CellType);



}


void TARInvoiceEntry::SearchGL    (RTMessage){  Accountlist();}

void TARInvoiceEntry::InvoiceData()
{
}



void TARInvoiceEntry::Vendorlist  ()
{
	CHourGlass wait;
	if(VendorList == NULL)
	{
	VendorList = new TARCustomerList(this,"");
	VendorList->EnableAutoCreate();
	GetApplication()->MakeWindow(VendorList);
	}
	VendorList->List->Seek(ID_VENDORNO);
	VendorList->Show(SW_SHOW);
}


void TARInvoiceEntry::Sliplist() //slip list search
{
	CHourGlass wait;
	if(SlipList == NULL)
	{
	 SlipList = new TARBatchList(this,"",&arbtch);
	 SlipList->EnableAutoCreate();
	 GetApplication()->MakeWindow(SlipList);
	 bRedraw = FALSE;
	}
	if(bRedraw)
	SlipList->List->Redraw();
	SlipList->List->Seek(ID_BATCHNO);
	SlipList->Show(SW_SHOW);
}







void TARInvoiceEntry::Accountlist ()
{
 CHourGlass wait;
 if(AcctList == NULL)
 {
  AcctList = new TAccountList(this,"");
  AcctList->EnableAutoCreate();
  GetApplication()->MakeWindow(AcctList);
 }

  AcctList->List->Seek(ID_ACCOUNTS);
  AcctList->Show(SW_SHOW);
}

void TARInvoiceEntry::Clear()
{
  bTransaction = FALSE;
  arbtch.SetIndex(GetString(8140));
  arbtch.bottom();
  int iBatchno = (int) arbtch.lBatchno();
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
  SetField(ID_PROD,"");
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
  SendDlgItemMsg(ID_MEDREP,CB_SETCURSEL,(WPARAM)Indx,NULL);

  Indx = SendDlgItemMsg(ID_DRCODES,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
  SendDlgItemMsg(ID_DRCODES,CB_SETCURSEL,(WPARAM)Indx,NULL);
  bModify = FALSE;
  return;
}



void TARInvoiceEntry::New(RTMessage)
{
 if(bTransaction)
 {
  int rc = MessageBox(HWindow,GetString(ID_COMMIT),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNOCANCEL |MB_ICONQUESTION);
  if(rc == IDYES)
	{
	 Clear();
	 bRedraw = TRUE;
	 //if(SlipList != NULL)
	 // SlipList->List->Redraw();
	}
  else if (rc == IDNO)
	{
	 SendMessage(HWindow,WM_COMMAND,CM_ROLLBACK,NULL);
	}
  else if (rc == IDCANCEL)
  return;
 }
 else
 {
  Clear();
  bNEW = TRUE;
 }
 return;
}



void TARInvoiceEntry::Cancel (RTMessage )
{

 if(bTransaction)
 {
  switch(MessageBox(HWindow,GetString(ID_COMMIT),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNOCANCEL |MB_ICONQUESTION))
	{
	 case IDYES     : Clear(); break;
	 case IDNO      : SendMessage(HWindow,WM_COMMAND,CM_ROLLBACK,NULL); break;
	 case IDCANCEL  : return;
	}
 }
  if(VendorList != NULL)
  VendorList->List->SavePosition();
  if(SlipList != NULL)
  SlipList->List->SavePosition();
  if(AcctList != NULL)
  AcctList->List->SavePosition();
  D->SavePosition();
  // Close all the databases...
  glacct.close();
  ardist.close();
  arbtch.close();
  arbtchd.close();
  prods.close();
  gldept.close();
  drcodes.close();
  arcust.close();
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
 long Indx = SendMessage(GetDlgItem(Parent->HWindow,ID_DEPTCODES),CB_FINDSTRING,NULL,(LPARAM)(LPSTR)arbtchd->depart.str());
  if(Indx == CB_ERR )
 Indx = SendMessage(GetDlgItem(Parent->HWindow,ID_DEPTCODES),CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendMessage(GetDlgItem(Parent->HWindow,ID_DEPTCODES),CB_SETCURSEL,(WORD)Indx,NULL);


 List->SetColor(SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
 List->SetColor(SS_ALLCOLS,nIndx,RGB(192,220,192), RGBCOLOR_BLUE);
 SendMessage(Parent->HWindow,WM_COMMAND,CM_LINEITEM,NULL);
 return;
}

void TARInvoiceEntry::SetLineItem(RTMessage)
{
  SetField(ID_JDESCRIPT,arbtchd.descript);
  SetField(ID_ACCOUNTS ,arbtchd.account);
  SetField(ID_DEPART   ,arbtchd.dept);
  SetField(ID_DEBIT    ,arbtchd.debit);
  SetField(ID_CREDIT   ,arbtchd.credit);
  SetField(ID_LINENO   ,arbtchd.entryno);
  SetField(ID_DEPT_DESC,"");
  bModify = TRUE;
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
 if(glacct.seek(account)==0)
 return TRUE;
 else
 return FALSE;
}


void TManilaGL::ARBatch       (RTMessage)
 {
  GetApplication()->ExecDialog(new TARInvoiceEntry(this,1053));

 }



void TARPriorPeriod::SetupWindow()
{
  TARInvoiceEntry::SetupWindow();
  mode = "3";
  string strCaption(hInst,ID_ARPRIORPERIOD);
  SetCaption((LPSTR)strCaption.c_str());

}


void TManilaGL::ARPriorPeriod (RTMessage)
 {
  GetApplication()->ExecDialog(new TARPriorPeriod(this,1053));
 }



char *TARInvoiceEntry::GetControlAcct(char *Account,char *Depart)
{
 static char szDescript[40];
 strcpy(szDescript,Account);
 strcat(szDescript,Depart);
 if(glacct.seek(szDescript) == 0)
 strcpy(szDescript,glacct.descript.str());
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

/*

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

*/
