#include "std_inc.h"
/*
#include "ap.h"
#include "gl_dlg.h"
#include "db_dist.h"
#include "db_vend.h"
#include "db_apdst.h"
#include "db_apbtc.h"
#include "db_jsum.h"
#include "db_btchd.h"
#include "db_btch.h"
#include "db_accts.h"
#include "db_glcsh.h"
#include "utildlg.h"
#include "math.h"
#include "ap_verif.h"
#include "ap_check.h"
*/


#define ID_DATA_ERROR 60


#define ID_INVOICE 1030
#define ID_CHEQUENO 1087
BOOL bCleared;

_CLASSDEF (TAPPayment)
class TAPPayment : public TMainDialog
{
 public:
  SS_CELLTYPE 	CellType;
  HFONT   	hFont;
  HWND 		SHWindow;

  BOOL         bHasInvoices;
  Tglaccts     glaccts;
  Tapvendor    apvendor;
  Tapdist      apdist;
  TSpread     *InvoiceList;
  double dAPControl;
  TAPPayment(PTWindowsObject AParent, int ATitle):TMainDialog(AParent,ATitle)
  {
	InvoiceList = new TSpread(this,ID_DISTLIST);
  }

  virtual void SetupWindow();
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void ViewAll        (RTMessage Msg) = [ID_FIRST + ID_VIEW];
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual void SetAll         (RTMessage Msg) = [ID_FIRST + ID_ALL];
  virtual BOOL LoadDistributionList(BOOL bALL);
  void         PrintPaymentList(void);
  void         APAccountUpdate();
  ~TAPPayment()
  {
	delete InvoiceList;
  }
};

_CLASSDEF (TAPSetChequeNumber)
class TAPSetChequeNumber: public TMainDialog
{
 public:
 Tapdist     *apdist;
 char        *szAmount;
 TAPSetChequeNumber(PTWindowsObject AParent,int ATitle,PTapdist reqst,char *Amount)
 :TMainDialog(AParent,ATitle)
  {
	apdist = reqst;
	szAmount = strcpy(new char[10],Amount);
  }
 ~TAPSetChequeNumber()
 {
  delete szAmount;
 }
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL]{ShutDownWindow();}
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
};


void TAPPayment::SetAll(RTMessage)
{
  if(SendDlgItemMsg(ID_ALL,BM_GETCHECK,NULL,NULL))
	SendDlgItemMsg(ID_ALL,BM_SETCHECK,FALSE,NULL);
  else
	SendDlgItemMsg(ID_ALL,BM_SETCHECK,TRUE,NULL);
}


void TAPSetChequeNumber::SetupWindow()
{
 TDialog::SetupWindow();
 Date4 dDatepaid(apdist->datepaid.str());
 SetField(ID_CHEQUE,StrTrim(apdist->chequeno.str()));
 SetField(ID_INVOICE,szAmount);
 SetField(ID_ACCOUNTS,apdist->account);
 SetField(ID_DEPART,apdist->dept);
 SetField(ID_DATE,dDatepaid.str());
}

void TAPSetChequeNumber::Ok (RTMessage)
{
 CHourGlass wait;
 if(apdist->seek(GetField(ID_CHEQUE)) == 0)
 {
	while( strcmp( StrTrim(apdist->chequeno.str()) , StrTrim(GetField(ID_CHEQUE))) == 0)
	{
		apdist->paymentctl.assign("C");
		apdist->UpdateUserTime(szUser);
		bCleared = TRUE;
		apdist->skip();
	}
 }
 ShutDownWindow();
}


void TVerifyCheques::Cancel (RTMessage)
{
  apdist.close();
  ShutDownWindow();
}

void TVerifyCheques::Verify (RTMessage Msg)
{
 CHourGlass wait;
 Tapdist APDIST;

 APDIST.open(*cb);
 if(!APDIST.isValid())
  {
	ErrorClose(APDIST.GetDbfName());
	return;
  }
 APDIST.SetIndex("cheques");

 //if(MessageBox(HWindow,GetString(3079),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO)==IDYES)
 //{
	 for(APDIST.top();!APDIST.eof();APDIST.skip()){
		 if(strcmp(StrTrim(APDIST.paymentctl.str()),"P") == 0)
			 APDIST.paymentctl.assign("V");
 //}
	 APDIST.UpdateUserTime(szUser);
	 ((TAPCreatePayment *)Parent)->bVerified = TRUE;
 }
 APDIST.close();
 Cancel(Msg);
}

void TVerifyCheques::SetupWindow()
{
  CHourGlass wait;
  TDialog::SetupWindow();
  cb = &((TAcctModule *)GetApplication())->cb;
  apdist.open(*cb);
  if(!apdist.isValid())
  {
	ErrorClose(apdist.GetDbfName());
	return;
  }

  SetChequeNumbers();
  SetBitmap(1031);
  SetBitmap(IDOK);
  SetBitmap(IDCANCEL);
}

void TVerifyCheques::SetChequeNumbers()
{
  //apdist->SetIndex("lastcheqno");
  apdist.SetIndexVerifiedCleared();
  apdist.bottom();
  double iNewRequestNumber =1;
  iNewRequestNumber = double (Str4ptr(apdist.chequeno.str())) + 1;
  FloatSetDlgItemValue(HWindow,ID_CHEQUENO,iNewRequestNumber);
  //apdist.SetIndex("invoiceno");
  apdist.SetIndexInvoiceno();

}

void TVerifyCheques::Ok (RTMessage)
{
 CHourGlass wait;
 Date4 dPayment;
 Date4 dDatepaid;
 dPayment.assign(GetField(ID_DUEDATE),GetString(4024));
 dDatepaid.assign(GetField(ID_DATE),GetString(4024));
 dPayment.format(GetString(4023));
 dDatepaid.format(GetString(4023));
 double  iStartingReq = 0;
 FloatGetDlgItemValue(HWindow,ID_APCONTROL,(LPDOUBLE)&iStartingReq);
 char szVendorNo[7];
 strcpy(szVendorNo,apdist.vendorno.str());
 double dChequeAmount = 0;
 apdist.SetIndex("assignno");
 apdist.top();
 while(!apdist.eof())
	{
		 dChequeAmount += apdist.dAmount();
		 if(dChequeAmount > 0 || dChequeAmount == 0)
			{
			  UpdateAPVendor(iStartingReq);
			}
		 dChequeAmount = 0;
		 iStartingReq++;
  }
 MessageBox(HWindow,GetString(2014),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);

}


void TVerifyCheques::UpdateAPVendor(double iStartingReq)
 {
	iStartingReq = fabs(iStartingReq);
	string strVendor(StrTrim(apdist.vendorno.str()));
	string strCompare(strVendor);
	while(strVendor == strCompare)
	 {
		apdist.chequeno.assignDouble(iStartingReq);
		apdist.skip();
		strCompare = StrTrim(apdist.vendorno.str());
	 }
}



#pragma argsused
void TVerifyCheques::UpdateCheque(double dAmount,int dChequeNo)
{
  apdist.UpdateUserTime(szUser);
  apdist.chequeno.assignLong(dChequeNo);
  apdist.datepaid.assign(GetDate(4075));
  return;
}

void TVerifyCheques::AddCheque(double dChequeNo)
{
  apdist.UpdateUserTime(szUser);
  apdist.chequeno.assignDouble(dChequeNo);
  apdist.datepaid.assign(GetDate(4075));
}


void TManilaGL::APPayment(RTMessage)
{
 GetApplication()->ExecDialog(new TAPPayment(this,1046));
}

void TManilaGL::APChequeAP (RTMessage)
{
 GetApplication()->ExecDialog(new TVerifyCheques(this,1157));
}


void TAPPayment::SelectCell (RTMessage Msg)
{
 CHourGlass wait;
 string szRequest;
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD) Msg.LParam;
 InvoiceList->GetData(4,Cell->Row,szRequest);
 bCleared = FALSE;
 if(strlen(StrTrim((LPSTR)szRequest.c_str())) == 0)
	return;

 if(apdist.seek((LPSTR)szRequest.c_str()) == 0)
  {
	 if(strcmp(StrTrim(apdist.paymentctl.str()),"C") == 0)
	  {
		string chequeno = StrTrim( apdist.chequeno.str());
		string compare  = StrTrim(apdist.chequeno.str());
		while(chequeno == compare)
		{
			apdist.paymentctl.assign("V");
			apdist.UpdateUserTime(szUser);
			bCleared = TRUE;
			apdist.skip();
			compare  = StrTrim(apdist.chequeno.str());
		}
		InvoiceList->SetColor(SS_ALLCOLS,Cell->Row,RGBCOLOR_WHITE, RGBCOLOR_BLUE);
	  }
	 else
	  {
		string chequeno = StrTrim(apdist.chequeno.str());
		string compare  = StrTrim(apdist.chequeno.str());
		while(chequeno == compare)
		{
			apdist.paymentctl.assign("C");
			apdist.UpdateUserTime(szUser);
			bCleared = TRUE;
			apdist.skip();
			compare  = StrTrim(apdist.chequeno.str());
		}
		InvoiceList->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);
	 }
  }
 else
 MessageBox(HWindow,GetString(5111),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
}

void TAPPayment::Print (RTMessage)
{
 if(MessageBox(HWindow,GetString(8138),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
  {
	PrintPaymentList();
  }
 return;
}


void TAPPayment::SetupWindow()
{
 TMainDialog::SetupWindow();
 CHourGlass wait;
 RECT rc;
 InvoiceList->GetClientRect(&rc);
 short d1Width,d0Width,d2Width,d3Width,d4Width;

 if(apdist.open(*cb) < 0)
  {
	ErrorClose(apdist.GetDbfName());
	return;
  }

 if(apvendor.open(*cb) < 0)
  {
	ErrorClose(apvendor.GetDbfName());
	return;
  }

 if(glaccts.open(*cb) < 0)
  {
	ErrorClose(glaccts.GetDbfName());
	return;
  }

 InvoiceList->SetMaxCols(5);
 InvoiceList->SetColWidth(1,apdist.vendorno.len());
 InvoiceList->SetColWidth(2,apdist.account.len());
 InvoiceList->SetColWidth(3,apdist.dept.len());
 InvoiceList->SetColWidth(4,apdist.chequeno.len());

 InvoiceList->GetColWidthInPixels(0,(LPSHORT)&d0Width);
 InvoiceList->GetColWidthInPixels(1,(LPSHORT)&d1Width);
 InvoiceList->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 InvoiceList->GetColWidthInPixels(3,(LPSHORT)&d3Width);
 InvoiceList->GetColWidthInPixels(4,(LPSHORT)&d4Width);


 short ColWidth = (rc.right - rc.left)
		  - d0Width
		  - d1Width
		  - d2Width
		  - d3Width
		  - d4Width;


 InvoiceList->SetColWidthInPixels(5,ColWidth);

 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
 InvoiceList->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 InvoiceList->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
 InvoiceList->SetBool(SSB_PROTECT,TRUE);
 InvoiceList->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 InvoiceList->SetBool(SSB_NOBEEP,TRUE);
 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);


 InvoiceList->SetMaxRows(5000);
 InvoiceList->SetData(1,SS_HEADER, GetString(4001));
 InvoiceList->SetData(2,SS_HEADER, GetString(4034));
 InvoiceList->SetData(3,SS_HEADER, GetString(4037));
 InvoiceList->SetData(4,SS_HEADER, GetString(4035));
 InvoiceList->SetData(5,SS_HEADER, GetString(4036) ) ;

 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 InvoiceList->SetTypeEdit(&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 InvoiceList->SetCellType(1, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(2, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(3, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(4, SS_ALLROWS, &CellType);


 InvoiceList->SetTypeFloat(&CellType,FS_SEPARATOR,9,2,0,atof(GetString(6107)));
 InvoiceList->SetCellType(5, SS_ALLROWS, &CellType);

 // Open databases...
 cb->safety = 0;

 //apdist.SetIndex("invoiceno");
 apdist.SetIndexInvoiceno();
 SetBitmap(IDCANCEL);
 SetBitmap(ID_VIEW);
 SetBitmap(ID_PRINT);
 //bHasInvoices  = LoadDistributionList(FALSE);
}

void TAPPayment::Cancel  (RTMessage)
{
 //cb.closeAll();
 glaccts.close();
 apvendor.close();
 apdist.close();
 ShutDownWindow();
}

#pragma argsused
BOOL TAPPayment::LoadDistributionList(BOOL bAll)
{
 CHourGlass wait;
 int l = 1;
 char szPaymentDate[9];
 GetDlgItemText(HWindow,ID_DATE,szPaymentDate,sizeof(szPaymentDate));
 Date4 dPayment(szPaymentDate);
 //InvoiceList->SetBool(SSB_REDRAW,FALSE);
 InvoiceList->ClearData(SS_ALLCOLS,SS_ALLROWS);
 long  dCheque;
 Str4large dAmountDue;
 InvoiceList->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 InvoiceList->SetMaxRows(apdist.recCount());

 string szQuery;

 if(bAll)
  apdist.SetIndexVerifiedCleared();
 else
  apdist.SetIndexVerified();



 apdist.top();
 while (!apdist.eof())
 {
		InvoiceList->SetData(1,l,apdist.vendorno.str());  // Vendor Code
		InvoiceList->SetData(2,l,apdist.account.str());
		InvoiceList->SetData(3,l,apdist.dept.str());
		InvoiceList->SetData(4,l,StrTrim(apdist.chequeno.str()));  // Cheque Number
		if(strcmp(StrTrim(apdist.paymentctl.str()),"C") == 0)
			InvoiceList->SetColor(SS_ALLCOLS,l,RGB(192,220,192), RGBCOLOR_BLUE);
		dAmountDue.assignDouble(apdist.dAmountpaid()+ double (Str4ptr(dAmountDue.str())),16,6);
		InvoiceList->SetData(5,l,dAmountDue.str()); // Adjustment
		dCheque = long (Str4ptr(apdist.chequeno.str()));
		apdist.skip();
		if(dCheque != long (Str4ptr(apdist.chequeno.str())))
		{
		 l++;
		 dAmountDue.assignDouble(0);
		}
		InvoiceList->ShowCell(SS_ALLCOLS,l,SS_SHOW_NEAREST);
 }

 if(l>9)
  InvoiceList->SetMaxRows(l-1);
 else
  InvoiceList->SetMaxRows(9);

 if(l>1)
 return TRUE;
 else
 return FALSE;
}


void TAPPayment::PrintPaymentList(void)
{
  ((TGlWindow *)Parent)->PrintReport("ap_bkrec.rpt",((TAcctModule *)GetApplication())->GetMainWin()	);
  ShutDownWindow();
  return;
}

void TAPPayment::ViewAll (RTMessage)
{
  if(SendDlgItemMsg(ID_ALL,BM_GETCHECK,NULL,NULL))
	LoadDistributionList(TRUE);
  else
	LoadDistributionList(FALSE);
}



