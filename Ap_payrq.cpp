#include "ap.h"
#include "gl_dlg.h"
#include "db_apbtc.h"
#include "db_vend.h"
#include "db_jsum.h"
#include "math.h"
#include "db_apdst.h"
#include "db_dist.h"
#include "gl_setat.hpp"
#include "utildlg.h"
#include "ap_verif.h"






_CLASSDEF (TAPCreatePayment)
class TAPCreatePayment : public TMainDialog
{
 public:
  BOOL          bHasInvoices;
  SS_CELLTYPE 	CellType;
  LOGFONT 	lfFont;
  HFONT   	hFont;
  HWND 		SHWindow;
  Tapdist*      apdist;
  Tapvendor*    apvendor;
  Tapbtch*	    apbtch;
  Tapreqst*     apreqst;
  Tapjsum*      apjsum;
  TSpread*      InvoiceList;

  TAPCreatePayment(PTWindowsObject AParent, int ATitle):TMainDialog(AParent,ATitle)
  {
  InvoiceList = new TSpread(this,ID_DISTLIST);
  bHasInvoices = FALSE;
  }
  ~TAPCreatePayment()
  {
	delete InvoiceList;
  }
  virtual void SetupWindow();
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  virtual void SetDate        (RTMessage Msg) = [ID_FIRST + ID_SETDATE];
  virtual void Ok	            (RTMessage Msg) = [ID_FIRST + IDOK]; // Create the request.
  virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual void ViewAll        (RTMessage Msg) = [ID_FIRST + ID_VIEW];
  virtual void GetBankAccount (RTMessage Msg) = [ID_FIRST + ID_DESCRIPT];
  virtual void GetVendorCode  (RTMessage Msg) = [ID_FIRST + ID_VENDORNO];
  virtual void Verify         (RTMessage Msg) = [ID_FIRST + 1031];
  virtual BOOL DistList(char *szQuery);
  virtual void SetCashAccounts();
  virtual void SetVendorNames();
  void PrintPaymentList(void);
  void SetChequeNo();
  char *UpdateAPVendor(LPSTR szVendorno,double iStartingReq,Date4 dDatepaid);
  void CreateGLEntry(void);
  BOOL CheckVerifiedCheckPrinting();
};

class TAPHoldForce: public TMainDialog
{
 public:
 PTapdist apdist;
 BOOL Hold;
 BOOL Force;
 TAPHoldForce(PTWindowsObject AParent, int AName,PTapdist a): TMainDialog(AParent,AName)
 {
  apdist = a;
 }
 virtual void SetupWindow();
 virtual void Ok            (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel        (RTMessage Msg) = [ID_FIRST + IDCANCEL] {ShutDownWindow();}
};

void TAPCreatePayment::Verify(RTMessage)
 {
	GetApplication()->ExecDialog(new TPrintCheques(this,1157));
	apdist   =  new Tapdist(&cb);
	 if(!apdist->isValid())
	 {
		ErrorClose(apdist->GetDbfName());
		return;
	 }

	apvendor =  new Tapvendor(&cb);
	 if(!apvendor->isValid())
  {
	ErrorClose(apvendor->GetDbfName());
	return;
  }

	apreqst  =  new Tapreqst(&cb);
	 if(!apreqst->isValid())
	  {
		ErrorClose(apreqst->GetDbfName());
		return;
	  }

	apbtch   =  new Tapbtch(&cb);
	 if(!apbtch->isValid())
	  {
		ErrorClose(apbtch->GetDbfName());
		return;
	  }

	apjsum   =  new Tapjsum(&cb);
	if(!apjsum->isValid())
	{
	ErrorClose(apjsum->GetDbfName());
	return;
	}

	bHasInvoices = FALSE;
	SetCashAccounts();
	SetVendorNames();
	SetChequeNo();
	if(!CheckVerifiedCheckPrinting())
	{
	 ShowWindow(GetDlgItem(HWindow,1031),SW_SHOW);
	 ShowWindow(GetDlgItem(HWindow,IDOK),SW_HIDE);
	}
	else
	{
	 ShowWindow(GetDlgItem(HWindow,1031),SW_HIDE);
	 ShowWindow(GetDlgItem(HWindow,IDOK),SW_SHOW);
	}
 }

void TAPCreatePayment::GetBankAccount(RTMessage Msg)
{

 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  GL_SETAT gl_setat;
  gl_setat.open(cb);
  gl_setat.SetIndex("name");
  DWORD Indx;
  Indx = SendDlgItemMsg(ID_DESCRIPT,CB_GETCURSEL,0,0);
  if(Indx >= 0)
	{
	 char *szBankAccount = new char[gl_setat.name.len()+1];
	 SendDlgItemMsg(ID_DESCRIPT,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM) ((LPSTR) szBankAccount));
	 if(gl_setat.seek(szBankAccount) == 0)
	  {
		SetField(ID_ACCOUNTS,gl_setat.account.str());
		SetField(ID_DEPART,gl_setat.dept.str());
	  }
	  SetFocus(GetDlgItem(HWindow,2017));
	 delete [] szBankAccount;
	}
  gl_setat.close();
 }
}

void TAPCreatePayment::SetCashAccounts()
{
 GL_SETAT gl_setat;
 gl_setat.open(cb);
 gl_setat.SetIndex("cash");
 for(gl_setat.top();!gl_setat.eof();gl_setat.skip())
  {
	 SendDlgItemMsg(ID_DESCRIPT,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)gl_setat.name.str());
  }
 gl_setat.close();
}





void TAPCreatePayment::Ok (RTMessage)
{
  CHourGlass wait;
  double  iStartingReq = 0;
  FloatGetDlgItemValue(HWindow,ID_APCONTROL,(LPDOUBLE)&iStartingReq);
	if(!CheckVerifiedCheckPrinting())
	{
	 ShowWindow(GetDlgItem(HWindow,1031),SW_SHOW);
	 ShowWindow(GetDlgItem(HWindow,IDOK),SW_HIDE);
	}
	else
	{
	 ShowWindow(GetDlgItem(HWindow,1031),SW_HIDE);
	 ShowWindow(GetDlgItem(HWindow,IDOK),SW_SHOW);
	}


  if(iStartingReq  == 0)
  {
	MessageBox(HWindow,GetString(ID_REQUESTNO),((TAcctModule *)GetApplication())->GetAppName(),MB_OK |MB_ICONINFORMATION);   // If no request number return.
	return;
  }

  Data4 glaccts(&cb,"glaccts");
  if(!glaccts.isValid())
  {
	ErrorClose(glaccts.GetDbfName());
	return;
  }

  glaccts.SetIndex("accounts");
  //glaccts.SetIndexAccts();


  char *szSeek = new char [12];
  strcpy(szSeek,GetField(ID_ACCOUNTS));
  strcat(szSeek,GetField(ID_DEPART));

  if(glaccts.seek(szSeek) != 0)
  {
	MessageBox(HWindow,GetString(4007),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	glaccts.close();
	return;
  }
  else
  glaccts.close();
  delete [] szSeek;

  //apdist->SetIndex("vendorno");
  apdist->SetIndexVendorno();

  Date4 dPayment;
  Date4 dDatepaid;
  dPayment.assign(GetField(ID_DUEDATE),GetString(4024));
  dDatepaid.assign(GetField(ID_DATE),GetString(4024));
  dPayment.format(GetString(4023));
  dDatepaid.format(GetString(4023));
  if(bHasInvoices)
  {
	if(MessageBox(HWindow,GetString(4009),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
	 CHourGlass wait;

	 char szVendorNo[7];
	 apdist->SetIndex("Payment");
	 apdist->top();
	 strcpy(szVendorNo,apdist->vendorno.str());
	 while(!apdist->eof())
	  {
		Date4 dDueDate(apdist->duedate.str());
		if(dDueDate <= dPayment)
			 UpdateAPVendor(szVendorNo,iStartingReq,dDatepaid);
		iStartingReq++;
	  }
	//apdist->SetIndex("vendorno");
		apdist->SetIndexVendorno();
		CreateGLEntry();
	 }
  string szQuery(GetString(4006));

  bHasInvoices = DistList((char *) szQuery.c_str());
	if(!CheckVerifiedCheckPrinting())
	{
	 ShowWindow(GetDlgItem(HWindow,1031),SW_SHOW);
	 ShowWindow(GetDlgItem(HWindow,IDOK),SW_HIDE);
	}
	else
	{
	 ShowWindow(GetDlgItem(HWindow,1031),SW_HIDE);
	 ShowWindow(GetDlgItem(HWindow,IDOK),SW_SHOW);
	}
 }
 else
 MessageBox(HWindow,GetString(4079),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
 return;
}

char* TAPCreatePayment::UpdateAPVendor(char* szVendorno,double iStartingReq,Date4 dDatepaid)
 {
   apdist->SetIndex("payment");
	iStartingReq = fabs(iStartingReq);
	string strVendor(StrTrim(szVendorno));
	string strCompare(StrTrim(szVendorno));
	while(strVendor == strCompare)
	 {
		apdist->paymentctl.assign("P");
		apdist->chequeno.assignDouble(iStartingReq);
		apdist->account.assign(GetField(ID_ACCOUNTS));
		apdist->dept.assign(GetField(ID_DEPART));
		apdist->amountpaid.assignDouble(apdist->dAmount());
		apdist->datepaid.assign(dDatepaid.str());
		apdist->skip();
		strCompare = StrTrim(apdist->vendorno.str());
	 }
  return (LPSTR)strCompare.c_str();
}


BOOL TAPCreatePayment::CheckVerifiedCheckPrinting()
{
	BOOL bVerified = TRUE;
	//apdist->SetIndex("vendorno");
	apdist->SetIndexVendorno();

	Relate4set rQuery(apdist);
	//string szQuery("PAYMENTCTL = 'P'");
	string szQuery(GetApplication()->hInstance,6109);
	rQuery.querySet((LPSTR) szQuery.c_str());
	int iCounter = 0;
	for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
		++iCounter;
	rQuery.unlock();
	rQuery.free();
	if (iCounter > 0)
	bVerified = FALSE;
	return bVerified;
}


void TAPHoldForce::SetupWindow()
{
 TDialog::SetupWindow();
 if(strcmp(apdist->paymentctl.str(),"R") == 0 || strcmp(apdist->paymentctl.str(),"P") == 0)
  {
	MessageBox(HWindow,GetString(4010),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	ShutDownWindow();
  }
 SetField(ID_VENDORNO,apdist->vendorno);
 SetField(ID_INVOICENO,apdist->invoiceno);
 SetBitmap(IDCANCEL);
 SetBitmap(IDOK);
}

void TAPHoldForce::Ok(RTMessage)
  {
   CHourGlass wait;
	if(SendDlgItemMsg(ID_FORCE,BM_GETSTATE,NULL,NULL))
	 {
	  apdist->paymentctl.assign("F");
	  apdist->UpdateUserTime(szUser);

	 }
	else if (SendDlgItemMsg(ID_HOLD,BM_GETSTATE,NULL,NULL))
	 {
	  apdist->UpdateUserTime(szUser);
	  apdist->paymentctl.assign("H");

	 }
	else
	 {
	  apdist->UpdateUserTime(szUser);
	  apdist->paymentctl.assign("O");

	 }
	ShutDownWindow();
}


void TAPCreatePayment::SelectCell(RTMessage Msg)
{
 char szVendorno[21];
 char szInvoiceno[11];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,3,Cell->Row,szVendorno);
 SSGetData(SHWindow,4,Cell->Row,szInvoiceno);
 strcat(szVendorno,szInvoiceno);
 //apdist->SetIndex("invoiceno");
 apdist->SetIndexInvoiceno();
 if(apdist->seek(szVendorno)==0)
 {
  GetApplication()->ExecDialog(new TAPHoldForce(this,1066,apdist));
 }
 SSSetData(SHWindow,2,Cell->Row,apdist->paymentctl.str());
 char *controltype = apdist->paymentctl.str();
 switch(*controltype)
  {
	case 'O' :
	case 'F' : SSSetColor(SHWindow,SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);break;
	case 'H' : SSSetColor(SHWindow,SS_ALLCOLS,Cell->Row,RGBCOLOR_WHITE ,RGBCOLOR_BLACK);break;
	case 'P' : SSSetColor(SHWindow,SS_ALLCOLS,Cell->Row,RGB(0,255,200) ,RGBCOLOR_BLUE);break;
  }
}

void TAPCreatePayment::CreateGLEntry()
{
	apdist->SetIndex("paymentreq");
	Relate4set rQuery(apdist);
	string strQuery = GetString(6146);
	/*
	  Check to make sure the ap_jsum database is being updated correctly.
	  CHECK	
	*/


	rQuery.querySet((LPSTR)strQuery.c_str());
	for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
	 {
	 apjsum->appendStart();
	 apjsum->account.assign      (apdist->apaccount.str());
	 apjsum->dept.assign         (apdist->apdepart.str());
	 apjsum->period.assign       (apdist->period.str());
	 apjsum->mode.assign         ("1");
	 apjsum->date.assign         (apdist->date.str());
	 apjsum->type.assign         ("AP");
	 apjsum->batchno.assignDouble(double(Str4ptr(apdist->batchno.str())));
	 apjsum->reference.assign    (apdist->reference.str());
	 apjsum->descript.assign     (apdist->descript.str());
	 apjsum->debit.assignDouble  (apdist->dAmountpaid());
	 apjsum->credit.assignDouble (0.0);
	 apjsum->append();

	 apjsum->appendStart();
	 apjsum->account.assign      (apdist->account.str());
	 apjsum->dept.assign         (apdist->dept.str());
	 apjsum->period.assign       (apdist->period.str());
	 apjsum->date.assign         (apdist->date.str());
	 apjsum->mode.assign         ("1");
	 apjsum->type.assign         ("AP");
	 apjsum->batchno.assignDouble(double(Str4ptr(apdist->batchno.str())));
	 apjsum->reference.assign    (apdist->reference.str());
	 apjsum->descript.assign     (apdist->descript.str());
	 apjsum->debit.assignDouble  (0);
	 apjsum->credit.assignDouble (apdist->dAmountpaid());
	 apjsum->append();
	 }
   rQuery.unlock();
   rQuery.free();	          
}



void TAPCreatePayment::Print(RTMessage)
{
  if(MessageBox(HWindow,GetString(4067),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO|MB_ICONQUESTION) == IDYES)
  PrintPaymentList();
}

void TManilaGL::APCreateGLSlip(RTMessage)
{
 //GetApplication()->ExecDialog(new TAPCreatePayment(this,1045));
 GetApplication()->ExecDialog(new TAPCreatePayment(this,1162));
}


void TAPCreatePayment::SetDate (RTMessage)
{
 //"PAYMENTCTL = 'H' .OR. PAYMENTCTL = 'F' .OR. PAYMENTCTL = 'O'"
 string szQuery(GetString(4011));
 bHasInvoices = DistList((char *)szQuery.c_str());
 if(!bHasInvoices)
  MessageBox(HWindow,GetString(4028),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
}

void TAPCreatePayment::ViewAll(RTMessage)
{
 // ".NOT. DELETED()"
 //DistList(GetString(4069));
 string strNotDeleted(GetApplication()->hInstance,6145);
 DistList((LPSTR)strNotDeleted.c_str());
}

void TAPCreatePayment::SetupWindow()
{
 TDialog::SetupWindow();
 CHourGlass wait;
 RECT rc;
 InvoiceList->GetClientRect(&rc);
 short d0Width,d2Width,d3Width,d4Width,d5Width,d6Width,d7Width,d8Width,d9Width;
 InvoiceList->SetMaxCols(9);
 InvoiceList->SetColWidth(2,3);
 InvoiceList->SetColWidth(3,6);
 InvoiceList->SetColWidth(4,9);
 InvoiceList->SetColWidth(5,9);
 InvoiceList->SetColWidth(6,10);
 InvoiceList->SetColWidth(7,9);
 InvoiceList->SetColWidth(8,10);
 InvoiceList->SetColWidth(9,10);


 InvoiceList->GetColWidthInPixels(0,(LPSHORT)&d0Width);
 InvoiceList->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 InvoiceList->GetColWidthInPixels(3,(LPSHORT)&d3Width);
 InvoiceList->GetColWidthInPixels(4,(LPSHORT)&d4Width);
 InvoiceList->GetColWidthInPixels(5,(LPSHORT)&d5Width);
 InvoiceList->GetColWidthInPixels(6,(LPSHORT)&d6Width);
 InvoiceList->GetColWidthInPixels(7,(LPSHORT)&d7Width);
 InvoiceList->GetColWidthInPixels(8,(LPSHORT)&d8Width);
 InvoiceList->GetColWidthInPixels(9,(LPSHORT)&d9Width);


 short ColWidth = (rc.right - rc.left)
		  - d0Width
		  - d2Width
		  - d3Width
		  - d4Width
		  - d5Width
		  - d6Width
		  - d7Width
		  - d8Width
		  - d9Width;

 InvoiceList->SetColWidthInPixels(1,ColWidth);

 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
 InvoiceList->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 InvoiceList->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
 InvoiceList->SetBool(SSB_PROTECT,TRUE);
 InvoiceList->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 InvoiceList->SetBool(SSB_NOBEEP,TRUE);
 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 InvoiceList->SetBool(SSB_VERTSCROLLBAR,TRUE);

 InvoiceList->SetMaxRows(1000);

 InvoiceList->SetData(1,SS_HEADER, GetString(ID_VENDORNAME));
 InvoiceList->SetData(2,SS_HEADER, GetString(ID_CONTROL));
 InvoiceList->SetData(3,SS_HEADER, GetString(ID_VENDORCODE));
 InvoiceList->SetData(4,SS_HEADER, GetString(ID_INVOICENO));
 InvoiceList->SetData(5,SS_HEADER, GetString(ID_DUEDATE));
 InvoiceList->SetData(6,SS_HEADER, GetString(ID_AMOUNT));
 InvoiceList->SetData(7,SS_HEADER, GetString(ID_VAT));
 InvoiceList->SetData(8,SS_HEADER, GetString(ID_REBATE));
 InvoiceList->SetData(9,SS_HEADER, GetString(ID_AMOUNTDUE));

 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 InvoiceList->SetTypeEdit(&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 InvoiceList->SetCellType(1, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeEdit(&CellType,ES_LEFT,10,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 InvoiceList->SetCellType(2, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(3, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(4, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeEdit(&CellType,ES_RIGHT,10,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 InvoiceList->SetCellType(5, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeFloat(&CellType,FS_SEPARATOR,9,2,0,atof(GetString(6107)));
 InvoiceList->SetCellType(6, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(7, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(8, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(9, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(10, SS_ALLROWS, &CellType);



 // Open databases...

 apdist   =  new Tapdist(&cb);
  if(!apdist->isValid())
  {
	ErrorClose(apdist->GetDbfName());
	return;
  }

 apvendor =  new Tapvendor(&cb);
  if(!apvendor->isValid())
  {
	ErrorClose(apvendor->GetDbfName());
	return;
  }

 apreqst  =  new Tapreqst(&cb);
  if(!apreqst->isValid())
  {
	ErrorClose(apreqst->GetDbfName());
	return;
  }

 apbtch   =  new Tapbtch(&cb);
  if(!apbtch->isValid())
  {
	ErrorClose(apbtch->GetDbfName());
	return;
  }

 apjsum   =  new Tapjsum(&cb);
  if(!apjsum->isValid())
  {
	ErrorClose(apjsum->GetDbfName());
	return;
  }


 bHasInvoices = FALSE;

 SetCashAccounts();
 SetChequeNo();
 SetVendorNames();
 SetBitmap(IDCANCEL);
 SetBitmap(IDOK);
 SetBitmap(ID_VIEW);
 SetBitmap(ID_PRINT);
 SetBitmap(ID_SETDATE);
 SetBitmap(1031);
 if(!CheckVerifiedCheckPrinting())
 {
  MessageBox(HWindow,GetString(3055),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
  ShowWindow(GetDlgItem(HWindow,1031),SW_SHOW);
  ShowWindow(GetDlgItem(HWindow,IDOK),SW_HIDE);
 }
 else
 {
  ShowWindow(GetDlgItem(HWindow,1031),SW_HIDE);
  ShowWindow(GetDlgItem(HWindow,IDOK),SW_SHOW);
 }

}


void TAPCreatePayment::SetVendorNames()
{
 SendDlgItemMsg(ID_VENDORNO,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)GetString(4076));
 //apvendor->SetIndex("vendorno");
 apvendor->SetIndexVendor();
 Relate4set rQuery(apvendor);
 string szQuery(GetString(4069));
 rQuery.querySet((LPSTR)szQuery.c_str());
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip())
	SendDlgItemMsg(ID_VENDORNO,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)apvendor->vendorno.str());
 rQuery.unlock();
 rQuery.free();
}

void TAPCreatePayment::GetVendorCode(RTMessage Msg)
{
 DWORD Indx;
 string szVendorCode;
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  Indx = SendDlgItemMsg(ID_VENDORNO,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(ID_VENDORNO,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM) ((LPSTR) szVendorCode.c_str()));
	 if(apvendor->seek((char *)szVendorCode.c_str()) == 0)
		SetField(ID_VENDORNAME,apvendor->vdname.str());
	 else
	  {
		SetField(ID_VENDORNAME,GetString(4076));
		string strNotDeleted(GetApplication()->hInstance,6145);
		DistList((LPSTR)strNotDeleted.c_str());
		return;
	  }
	 SetFocus(GetDlgItem(HWindow,ID_VENDORNO));
	 string szVendorQuery(GetString(4066));
	 szVendorQuery += szVendorCode.c_str();
	 szVendorQuery += "'";
	 DistList((char *) szVendorQuery.c_str());

	 // The string value 4066 is as follows:
	 // "(PAYMENTCTL = 'H' .OR. PAYMENTCTL = 'O' .OR. PAYMENTCTL = 'F') .AND. VENDORNO = '"
	 //
	}
 }
}




void TAPCreatePayment::Cancel  (RTMessage)
{

 //cb.init();
 ShutDownWindow();
}
BOOL TAPCreatePayment::DistList(char *szQuery)
{
CHourGlass wait;
 int l = 1;
 char *szControl = new char [2];
 // "Checking for invoices that are due."
 SetField(IDD_STATUSBOX,GetString(4026));
 Date4 dPayment;
 dPayment.assign(GetField(ID_DUEDATE),GetString(4024));
 dPayment.format(GetString(4023));
 InvoiceList->SetBool(SSB_REDRAW,FALSE);
 InvoiceList->ClearData(SS_ALLCOLS,SS_ALLROWS);
 InvoiceList->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 //apdist->SetIndex("invoiceno");
 apdist->SetIndexInvoiceno();
 char szDate[11];
 Date4 dInvoiceDueDate;
 double dPageTotal = 0;
 Relate4set rQuery(apdist);
 rQuery.querySet(szQuery);
 char *controltype;
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip())
  {
	  Date4 dDueDate(apdist->duedate.str());
	  if( dDueDate <= dPayment )
		 {
		 apvendor->seek(apdist->vendorno.str());
		 InvoiceList->SetData(1,l,apvendor->vdshort.str());  // Vendor Name
		 controltype = apdist->paymentctl.str();
		 switch(*controltype)
		  {
			case 'O' :
			case 'F' : InvoiceList->SetColor(SS_ALLCOLS,l,RGB(192,220,192), RGBCOLOR_BLUE);break;
			case 'P' : InvoiceList->SetColor(SS_ALLCOLS,l,RGB(0,255,200), RGBCOLOR_BLUE);break;
		  }

		 InvoiceList->SetData(2,l,apdist->paymentctl.str()); // Payment Control
		 InvoiceList->SetData(3,l,apdist->vendorno.str());  // Vendor Code
		 InvoiceList->SetData(4,l,StrTrim(apdist->invoiceno.str())); // Invoice Number
		 Date4 dInvoiceDueDate(apdist->duedate.str());
		 dInvoiceDueDate.format(szDate,GetString(4024));
		 InvoiceList->SetData(5,l,szDate);   // Due Date
		 double dTotal = 0;
		 if(dTotal > 0)
		  dTotal = apdist->dAmount() + fabs(apdist->dRebateamt());
		 else
		  dTotal = apdist->dAmount() - fabs(apdist->dRebateamt());
		 Str4large szTotal;
		 szTotal.assignDouble(dTotal,16,2);
		 InvoiceList->SetData(6,l,szTotal.str());
		 InvoiceList->SetData(7,l,apdist->vatamt.str()); // Adjustment
		 InvoiceList->SetData(8,l,apdist->rebateamt.str()); // Adjustment
		 double dAmountDue = apdist->dAmount() - apdist->dAmountpaid();
		 Str4large szAmount;
		 szAmount.assignDouble(dAmountDue,16,2);
		 InvoiceList->SetData(9,l,szAmount.str()); // Adjustment
		 dPageTotal += dAmountDue;
		 l++;
		 }
  }
 rQuery.unlock();
 rQuery.free();
 l++;

 if(dPageTotal !=0)
 {
  Str4large szPageTotal;
  szPageTotal.assignDouble(dPageTotal,16,2);
  InvoiceList->SetData(1,l,GetString(6144));
  InvoiceList->SetData(9,l,szPageTotal.str());
 }

 if(l>15)
  InvoiceList->SetMaxRows(l);
 else
  InvoiceList->SetMaxRows(15);
 InvoiceList->SetBool(SSB_REDRAW,TRUE);
 SetChequeNo();
 string strSearchCompleted(GetApplication()->hInstance,6141);
 SetField(IDD_STATUSBOX,(LPSTR)strSearchCompleted.c_str());
 delete szControl;

 if(l>1)
 return TRUE;
 else
 return FALSE;
}


void TAPCreatePayment::SetChequeNo()
 {
  //apdist->SetIndex("lastcheqno");
  apdist->SetIndexVerifiedCleared();
  apdist->bottom();
  double iNewRequestNumber =1;
  iNewRequestNumber = double (Str4ptr(apdist->chequeno.str())) + 1;
  FloatSetDlgItemValue(HWindow,ID_APCONTROL,iNewRequestNumber);
  //apdist->SetIndex("invoiceno");
  apdist->SetIndexInvoiceno();
 }


void TAPCreatePayment::PrintPaymentList(void)
{
  ((TGlWindow *)Parent)->PrintReport("ap_reqst.rpt",GetApplication()->MainWindow->HWindow);
  return;
}
