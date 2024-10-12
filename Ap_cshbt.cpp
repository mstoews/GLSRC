#include "std_inc.h"


#define ID_BANK 5012
#define ID_INVOICE 1030
#define ID_COMMIT  1056
#define  ID_VATACCT 1034
#define  ID_VATDEPT 1035
#define  ID_DRCODES 1098

//static COLORREF lpBack, lpFore;

extern BOOL bNEW;

void TAPCashEntry::Clear()
{
 apcash.bottom();
 int iBatchno = (int) apcash.lBatchno();
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
 SSSetColor(GetDlgItem(D->HWindow,ID_DISTLIST),SS_ALLCOLS,SS_ALLCOLS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);

 long Indx = SendDlgItemMsg(ID_PRODUCTS,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_PRODUCTS,CB_SETCURSEL,(WORD)Indx,NULL);

 Indx = SendDlgItemMsg(ID_MEDREPS,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_MEDREPS,CB_SETCURSEL,(WORD)Indx,NULL);

 Indx = SendDlgItemMsg(ID_DRCODES,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_DRCODES,CB_SETCURSEL,(WPARAM)Indx,NULL);

 DebitTotal=CreditTotal=0;
 SetFocus(GetDlgItem(HWindow,ID_VENDORNO));
 D->List->ClearData(SS_ALLCOLS,SS_ALLROWS);
 D->List->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 bTransaction = FALSE;
 bNEW = TRUE;
}


void TAPCashEntry::CurrentBatch(RTMessage)
{


  bModify = FALSE;
  bNEW = FALSE;
  long CurrentRecord = apcash.recNo();
  int rc = apcash.lock(CurrentRecord);
  if(rc != 0)
  {
	 MessageBox(HWindow,GetString(1052),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	 return;
  }


  D->List->SetBool(SSB_REDRAW,FALSE);
  double lDebitTotal,lCreditTotal;
  char szDueDate[11],szInvDate[11];
  lDebitTotal=lCreditTotal=0;
  if(apcash.recCount() == 0)
  {
	MessageBox(HWindow,GetString(8153),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	return;
  }

  D->List->ClearData(SS_ALLCOLS,SS_ALLROWS);

  IntSetDlgItemValue(HWindow,ID_BATCHNO,int (Str4ptr(apcash.batchno.str())));
  SetField(ID_INVOICE,apcash.payable);
  SetField(ID_VATACCT,apcash.vatacct);
  SetField(ID_VATDEPT,apcash.vatdept);
  SetField(ID_DESCRIPT,apcash.descript);
  SetField(ID_VENDORNO,apcash.vendorno);
  SetField(ID_VENDORNAME,apcash.vdname);
  SetField(ID_INVOICENO,apcash.invoiceno);
  SetField(ID_CURRBAL,apcash.orderno);
  SetField(ID_REFERENCE,apcash.reference);
  SetField(ID_APCONTROL,apcash.account);
  SetField(ID_APDEPART,apcash.depart);
  SetField(ID_CHEQUE,apcash.chequeno);
  SetField(ID_REBATE,apcash.rebate);
  SetField(ID_VATPERCENTAGE,apcash.vatpercent);

  InvoiceDate.assign(apcash.invdate.str(),GetString(6115));
  InvoiceDate.format(szInvDate,GetString(4024));
  SetField(ID_INVDATE,szInvDate);

  DueDate.assign(apcash.duedate.str(),GetString(6115));
  DueDate.format(szDueDate,GetString(4024));
  SetField(ID_DUEDATE,szDueDate);

  SetField(ID_TERMS,apcash.terms);
  SetField(ID_ACCOUNTS,"");
  SetField(ID_DEPART,"");
  SetField(ID_INVOICE,apcash.dPayable());

  long i=1;
  apcashd.SetIndex(GetString(8140));
  char *szLineNo = new char[3];
  apcashd.lockFile();

  if (apcashd.seek(double(Str4ptr(apcash.batchno.str()))) == 0)
  {
	while (apcash.lBatchno() == apcashd.lBatchno() && !apcash.eof() )
	 {
	  if(apcashd.dDebit() > 0)
	  {
		lDebitTotal = lDebitTotal + double(Str4ptr(apcashd.debit.str()));
		ltoa(i,szLineNo,10);
		D->List->SetData(1,i,szLineNo);
		D->List->SetData(2,i,apcashd.descript.str());
		D->List->SetData(3,i,apcashd.account.str());
		D->List->SetData(4,i,apcashd.dept.str());
		D->List->SetData(5,i,apcashd.debit.str());
		D->List->SetData(6,i,apcashd.credit.str());

		apcashd.entryno.assignLong(i);
		apcashd.flush();
		DebitTotal = lDebitTotal;
		i++;
	  }
	  apcashd.skip();
	}
  }

  //Set the credit amounts.
  if (apcashd.seek(apcash.lBatchno()) == 0)
  {
	while (apcash.lBatchno() == apcashd.lBatchno() && !apcash.eof() )
	 {
	  if(apcashd.dCredit() > 0)
	  {
		lCreditTotal = lCreditTotal + double(Str4ptr(apcashd.credit.str()));
		ltoa(i,szLineNo,10);
		D->List->SetData(1,i,szLineNo);
		D->List->SetData(2,i,apcashd.descript.str());
		D->List->SetData(3,i,apcashd.account.str());
		D->List->SetData(4,i,apcashd.dept.str());
		D->List->SetData(5,i,apcashd.debit.str());
		D->List->SetData(6,i,apcashd.credit.str());
		apcashd.entryno.assignLong(i);
		apcashd.flush();
		CreditTotal = lCreditTotal;
		i++;
	  }
	  apcashd.skip();
	}

  }

	IntSetDlgItemValue(HWindow,ID_LINENO,i);
	apcashd.iCurrentRecord = 0;
	short iHeight = 0;
	RECT Drc;
	GetClientRect(D->HWindow,&Drc);
	D->List->GetRowHeightInPixels(1,&iHeight);
	long cRows = (Drc.bottom - Drc.top) / iHeight + 1;
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
	 D->List->SetData(2,i,GetString(5116));
	 D->List->SetColor(2,i,RGBCOLOR_WHITE,RGBCOLOR_RED);
	}
	else
	{
	 D->List->SetData(2,i,"Total");
	}
	D->List->SetData(5,i,szDebitTotal.str());
	D->List->SetData(6,i,szCreditTotal.str());

	D->List->SetColor(SS_ALLCOLS,SS_ALLCOLS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
	D->List->SetColor(5,i,RGB(192,220,192), RGBCOLOR_BLUE);
	D->List->SetColor(6,i,RGB(192,220,192), RGBCOLOR_BLUE);
	SetFocus(GetDlgItem(HWindow,ID_INVOICE));
	D->List->SetBool(SSB_REDRAW,TRUE);
}



void TAPCashEntry::SetupWindow()
{
 TMainDialog::SetupWindow();
 cb = &((TAcctModule *)GetApplication())->cb;
 bModify = FALSE;
 bTransaction = FALSE;
 CHourGlass wait;
 DebitTotal=0;
 CreditTotal=0;


 if(apcash.open(*cb) < 0)
  {
	ErrorClose(apcash.GetDbfName());
	return;
  }


 if(apcashd.open(*cb) < 0)
  {
	ErrorClose(apcashd.GetDbfName());
	return;
  }

  if(apvendor.open(*cb) < 0)
  {
	ErrorClose(apvendor.GetDbfName());
	return;
  }


 if(glacct.open(*cb) < 0)
  {
	ErrorClose(glacct.GetDbfName());
	return;
  }

 if(gldist.open(*cb) < 0)
  {
	ErrorClose(gldist.GetDbfName());
	return;
  }


 if(apdist.open(*cb) < 0)
  {
	ErrorClose(apdist.GetDbfName());
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

  if(drcodes.open(*cb) < 0)
  {
	ErrorClose(drcodes.GetDbfName());
	return;
  }

 gl_setat.open(*cb);
 gl_setat.SetIndex("gl_accts");


 if(gl_setat.seek("WTXI") == 0)
 {
	strIndTaxAccount = gl_setat.account.str();
	strIndTaxDept    = gl_setat.dept.str();
  //strcpy(szIndTaxAccount,gl_setat.account.str());
  //strcpy(szIndTaxDept,gl_setat.dept.str());
 }
 else
 {
	strIndTaxAccount = gl_setat.account.str();
	strIndTaxDept    = gl_setat.dept.str();

  //strcpy(szIndTaxAccount,"");
  //strcpy(szIndTaxDept,"");
 }

 if(gl_setat.seek("WTXC") == 0)
 {
  strCompanyTaxAccount = gl_setat.account.str();
  strCompanyTaxDept    += gl_setat.dept.str();
  //strcpy(szCompanyTaxAccount,gl_setat.account.str());
  //strcpy(szCompanyTaxDept,gl_setat.dept.str());
 }
 else
 {
	strCompanyTaxAccount = gl_setat.account.str();
	strCompanyTaxDept    = gl_setat.dept.str();
	//strcpy(szCompanyTaxAccount,"");
	//strcpy(szCompanyTaxDept,"");
 }


 if(gl_setat.seek("VATP") == 0)
 {
	strVATAccount = gl_setat.account.str();
	strVATDept    = gl_setat.dept.str();

  //strcpy(szVATAccount,gl_setat.account.str());
  //strcpy(szVATDept,gl_setat.dept.str());
 }
 else
 {
	strVATAccount = "";
	strVATDept    = "";

  //strcpy(szVATAccount,"");
  //strcpy(szVATDept,"");
 }


 LoadDoctors();
 LoadDepartments();
 LoadProducts();

 IntSetDlgItemRange(HWindow,4065,0,100);
 IntSetDlgItemRange(HWindow,4073,0,100);


 bAutomaticCalc = TRUE;
 SendDlgItemMessage(HWindow,ID_AUTO,BM_SETCHECK,TRUE,NULL);
 // Get the AP Control Accounts.


 gl_setat.open(*cb);
 gl_setat.SetIndex("gl_accts");


 gl_setat.SetIndex("cash");

 for (gl_setat.top();!gl_setat.eof();gl_setat.skip())
  {
	SendDlgItemMsg(ID_CASH,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)gl_setat.name.str());
  }

  VendorList = NULL;
  AcctList = NULL;
  SlipList = NULL;
 /*
 VendorList = new TCustomerList(this,"",apvendor);
 VendorList->EnableAutoCreate();
 GetApplication()->MakeWindow(VendorList);

 AcctList = new TAccountList(this,"",glacct);
 AcctList->EnableAutoCreate();
 GetApplication()->MakeWindow(AcctList);

 SlipList  = new TCashList(this,"",apcash);
 SlipList->EnableAutoCreate();
 GetApplication()->MakeWindow(SlipList);
 */

 D = new TAPCashList(this,GetString(8156),&apcashd);
 D->EnableAutoCreate();
 GetApplication()->MakeWindow(D);



 RECT rc;
 RECT parent;
 int pos = 72;
 GetWindowRect(HWindow,&rc);
 GetWindowRect(Parent->HWindow,&parent);
 MoveWindow(HWindow,parent.left+3,parent.top+pos,rc.right-rc.left,rc.bottom-rc.top,TRUE);


 long db=0;
 apcash.bottom();
 db = apcash.lBatchno();
 db++;
 IntSetDlgItemValue(HWindow,ID_BATCHNO,db);
 bNEW = TRUE;
 //IntSetDlgItemValue(HWindow,3032,GetCurrentPeriodn());
 SetPeriods(HWindow);

 IntSetDlgItemValue(HWindow,ID_LINENO,1);

 D->Show(SW_SHOW);
 FloatSetMask(GetDlgItem(HWindow,ID_CREDIT),GetString(6107));

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

}

#pragma argsused
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




void TAPCashBatch::SetupWindow()
 {
 TDialog::SetupWindow();
 CHourGlass wait;
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
 string strSlipno(GetString(6147));
 string strInvoiceno(GetString(6148));
 SSSetData(SHWindow,1,SS_HEADER,(LPSTR)strSlipno.c_str());
 SSSetData(SHWindow,2,SS_HEADER,(LPSTR)strInvoiceno.c_str());
 SSSetData(SHWindow,3,SS_HEADER,GetString(ID_SLIP_DESCRIPTION));
 string strQuery(GetString(6149));
 Relate4set rQuery(apcash);
 // "MODE = '1'"
 rQuery.querySet((LPSTR)strQuery.c_str());
 int i = 1;
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
 {
  SSSetData(SHWindow,1,i,StrTrim(apcash->batchno.str()));
  SSSetData(SHWindow,2,i,StrTrim(apcash->invoiceno.str()));
  SSSetData(SHWindow,3,i,apcash->reference.str());
  if(strcmp(apcash->mode.str(),"0") == 0)
  SSSetColor(SHWindow,SS_ALLCOLS,i,RGB(255,255,130), RGBCOLOR_BLUE);
  i++;
 }
 rQuery.unlock();
 rQuery.free();

}



void TAPCashBatch::HandleListBox (RTMessage Msg)
{
 char *szBuffer = new char[apcash->batchno.len()];
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


void TAPCashEntry::Ok (RTMessage Msg )
{
 CHourGlass wait;
 if(Closed())
  return;

 if(strlen(StrTrim(GetField(ID_VENDORNO)))== 0)
	{
	  VList(Msg);
	  return;
	}

 if(apvendor.seek(GetField(ID_VENDORNO))!=0)
  {
	MessageBeep(MB_ICONEXCLAMATION);
	if(MessageBox(HWindow,GetString(6029),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
	 D->Show(SW_HIDE);
	 AddVendor(Msg);
	 D->Show(SW_SHOW);
	 SetFocus(GetDlgItem(HWindow,ID_VENDORNO));
	}
	return;
  }



 //szAdj = strcpy(new char[apcash.adj.len() + 1],"N");
 SendDlgItemMsg(ID_DISTLIST, WM_SETREDRAW, FALSE, 0L);
 iPeriodNo = (int)lGetField(ID_PERIOD);


 char *szVendorNo = strcpy(new char[apcash.vendorno.len() + 1],GetField(ID_VENDORNO));
 if(*szVendorNo==0)
	{
	 //CheckError(ID_VENDORNO);
	 return;
	}
 delete szVendorNo;

 if(strlen(StrTrim(GetField(ID_APCONTROL))) == 0)
  {
	 //CheckError(ID_APCONTROL);
	 SetFocus(GetDlgItem(HWindow,ID_BANK));
	 return;
  }


// MessageBox(HWindow,"1","",MB_OK);

 char *szInvoiceNo = strcpy(new char[apcash.invoiceno.len() + 1], GetField(ID_INVOICENO));
 if(strlen(StrTrim(szInvoiceNo)) == 0)
  {
	//CheckError(ID_INVOICENO);
	SetFocus(GetDlgItem(HWindow,ID_INVOICENO));
	return;
  }
 delete szInvoiceNo;

// MessageBox(HWindow,"2","",MB_OK);

 if(dGetField(ID_INVOICE) == 0 ){CheckError(ID_INVOICE);return;}

 char *szInvoiceDate = strcpy(new char[apcash.date.len() + 1],GetField(ID_INVDATE));
 if(strlen(StrTrim(szInvoiceDate)) == 0)
  {
	//CheckError(ID_INVDATE);
	SetFocus(GetDlgItem(HWindow,ID_INVDATE));
	return;
  }
 InvoiceDate.assign(szInvoiceDate,GetString(4024));
 DueDate.assign(szInvoiceDate,GetString(4024));
 delete szInvoiceDate;

 dTerms = dGetField(ID_TERMS);
 long lDueDate = long (DueDate) + dTerms;
 DueDate.assign(lDueDate);

 if(strlen(StrTrim(GetField(ID_JDESCRIPT))) == 0)
 {
  SearchGL(Msg);
  return;
 }


 if(strlen(StrTrim(GetField(ID_REFERENCE)))==0)
 {
	//CheckError(ID_REFERENCE);
	SetFocus(GetDlgItem(HWindow,ID_REFERENCE));
	return;
 }

 //MessageBox(HWindow,"2.1","",MB_OK);


 if(*StrTrim(GetField(ID_ACCOUNTS)) == 0)
 {
  SearchGL(Msg);
  return;
 }

 //MessageBox(HWindow,"2.2","",MB_OK);

 string GLAccount(GetField(ID_ACCOUNTS));
 string APControl(GetField(ID_APCONTROL));

 if(GLAccount == APControl)
  {
	MessageBox(HWindow,GetString(8158),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
	return;
  }

 // MessageBox(HWindow,"2.3","",MB_OK);

 //szDepart = strcpy(new char[apcash.depart.len() + 1],GetField(ID_DEPART));
 //szAccounts = strcpy(new char[apcash.account.len() + 1],GetField(ID_ACCOUNTS));
 //char *szSeek = new char[apcash.account.len() + apcash.depart.len() + 1];
 //strcpy(szSeek,szAccounts);
 //strcat(szSeek,szDepart);
 //if(*szAccounts==0){CheckError(ID_ACCOUNTS);return;}
 string strSeek(GetField(ID_ACCOUNTS));
 strSeek += GetField(ID_DEPART);

 if(!CheckGLAccount((LPSTR)strSeek.c_str()))
 {
	SearchGL(Msg);
	SetFocus(GetDlgItem(HWindow,ID_ACCOUNTS));
	return;
 }

// MessageBox(HWindow,"2.4","",MB_OK);


 iLineno = lGetField(ID_LINENO);


 //dVatPercentage = dGetField(ID_VATPERCENTAGE);
 int dVat;
 IntGetValue(GetDlgItem(HWindow,ID_VATPERCENTAGE),(LPLONG)&dVat);

// MessageBox(HWindow,"3","",MB_OK);

 dVatPercentage = (double)dVat;


 if(strlen(StrTrim(GetField(ID_VATACCT))) == 0 && dVatPercentage > 0)
  {
	 MessageBox(HWindow,GetString(6097),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
	 SetFocus(GetDlgItem(HWindow,ID_VATACCT));
	 return;
  }

// MessageBox(HWindow,"4","",MB_OK);

 string VatAcct =  GetField(ID_VATACCT);
 string VatDept =  GetField(ID_VATDEPT);
 if(!ValidateGL((char *)VatAcct.c_str(),(char *)VatDept.c_str()))
  {
	MessageBox(HWindow,GetString(5124),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	SetFocus(GetDlgItem(HWindow,ID_VATACCT));
	return;
  }

  // MessageBox(HWindow,"5","",MB_OK);
 dRebatePercentage = dGetField(ID_REBATE);

 apcash.SetIndex(GetString(8140));

 // MessageBox(HWindow,"6","",MB_OK);

 if(bModify)
	{
	 ModifyCurrentSlip();
	 bTransaction = TRUE;
	}
 else
	{
	 if(bNEW)
	 {
	  apcash.bottom();
	  iBatchno = (long)apcash.lBatchno();
	  iBatchno++;
	 }
	 else
	 iBatchno = (long)dGetField(ID_BATCHNO);
	 AddToCurrentSlip(iBatchno);
	 bTransaction = TRUE;
	}
 CurrentBatch(Msg);
}

void TAPCashEntry::GetCashExpense(AP_EXPENSE* ex)
{

	if(!GetWithHolding())
	{
	 ex->OLDEXPENSE = ex->OLDAP - apcash.dVatamt();
	 if(dVatPercentage != 1)
	  ex->VAT  = ex->NEWAP - round(ex->NEWAP/(1+dVatPercentage),2);
	 ex->NEWEXPENSE  = ex->NEWAP - ex->OLDEXPENSE - ex->VAT;

	 if(dRebatePercentage != 1)
		 ex->REBATE  = -1*((ex->NEWEXPENSE+ex->OLDEXPENSE)*dRebatePercentage);

	 ex->APAMT = ex->NEWAP + ex->REBATE;
	 ex->NEWAP = ex->APAMT;
	}
	else
	{
	 if (ex->OLDAP == 0)
	  ex->OLDEXPENSE = 0;
	 else
	  ex->OLDEXPENSE = ex->OLDAP - apcash.dVatamt() - apcash.dRebateamt();

	 double dVatRebateTotal = 1;
	 if(dRebatePercentage != 1)
		  dVatRebateTotal += dRebatePercentage;
	 if(dVatPercentage != 1)
		  dVatRebateTotal += dVatPercentage;

	 ex->NEWEXPENSE = lInvoiceAmount/dVatRebateTotal;

	 if(dVatPercentage != 1)
	  ex->VAT        = round(ex->NEWEXPENSE * dVatPercentage,2) + apcash.dVatamt();
	 else
	  ex->VAT        = 0;

	 if(dRebatePercentage != 1)
	  ex->REBATE     = round(ex->NEWEXPENSE * dRebatePercentage,2) + apcash.dRebateamt();
	 else
	  ex->REBATE     = 0;

	  ex->NEWEXPENSE = ex->NEWAP - (ex->VAT + ex->REBATE + ex->OLDEXPENSE);
	  ex->APAMT      = ex->NEWAP;

	}

}


void TAPCashEntry::AddToCurrentSlip(long iBatchno)
{
	apcash.SetIndex(GetString(8140));
	apcashd.SetIndex(GetString(8140));

	//char *szSeekAccount  = new char [apcash.account.len() + apcash.depart.len() + 1];
	//char *szRebateAccount = new char [apcash.account.len()+1];
	//char *szRebateDepart  = new char [apcash.depart.len()+1];
	string szSeekAccount;
	string szRebateAccount;
	string szRebateDept;

	if(dGetField(ID_REBATE) > 0)
	{
	 if(SendDlgItemMsg(ID_INDIVIDUAL,BM_GETSTATE,NULL,NULL))
	  {
		szSeekAccount = strIndTaxAccount;
		szSeekAccount += strIndTaxDept;
		szRebateAccount = strIndTaxAccount;
		szRebateDept  = strIndTaxDept;
	  }
	 else if(SendDlgItemMsg(ID_COMPANY,BM_GETSTATE,NULL,NULL))
	 {
	  //strcpy(szSeekAccount,szCompanyTaxAccount);
	  //strcat(szSeekAccount,szCompanyTaxDept);
	  //strcpy(szRebateAccount,szCompanyTaxAccount);
	  //strcpy(szRebateDepart,szCompanyTaxDept);
	  szSeekAccount = strCompanyTaxAccount;
	  szSeekAccount += strCompanyTaxDept;
	  szRebateAccount = strCompanyTaxAccount;
	  szRebateDept  = strCompanyTaxDept;
	 }
	 else if(!SendDlgItemMsg(ID_COMPANY,BM_GETSTATE,NULL,NULL) && !SendDlgItemMsg(ID_INDIVIDUAL,BM_GETSTATE,NULL,NULL))
	 {
	  MessageBox(HWindow,GetString(4101),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	  //delete [] szSeekAccount;
	  //delete [] szRebateAccount;
	  //delete [] szRebateDepart;
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

	expense->REBATE_RATE    = dRebatePercentage;
	expense->VAT_RATE 		= dVatPercentage;
	expense->INVOICE_AMT 	= lInvoiceAmount;
	expense->REBATE  			= 0.0;
	expense->VAT     	  		= 0.0;
	expense->NEWAP  	 	  	= apbtch.dPayable() + lInvoiceAmount;
	expense->OLDAP   	  		= apbtch.dPayable();
	expense->APAMT   	  		= apbtch.dPayable() + lInvoiceAmount;
	expense->NEWEXPENSE 		= lInvoiceAmount;

	BOOL bNewBatch = FALSE;
	if(apcash.seek(iBatchno) != 0)
	 bNewBatch = TRUE;

	Date4 Today;
	Today.today();

	if(bNewBatch)
	{
	apcash.appendStart();
	apcash.batchno.assignLong(iBatchno);
	apcash.mode.assign("1");
	apcash.adj.assign("N");
	apcash.date.assign(Today.str());
	}

	GetCashExpense(expense);

	/*
	  IF you use the same account for the expenses as you do for the bank
	  account the total will come out wrong.
	*/


	apcash.period.assignDouble(GetPeriod());
	apcash.chequeno.assign(GetField(ID_CHEQUE));
	apcash.orderno.assign(GetField(ID_CURRBAL));
	apcash.vendorno.assign(GetField(ID_VENDORNO));
	apcash.vdname.assign(GetField(ID_VENDORNAME));
	apcash.invoiceno.assign(GetField(ID_INVOICENO));
	apcash.duedate.assign(DueDate.str());
	apcash.invdate.assign(InvoiceDate.str());
	apcash.reference.assign(GetField(ID_REFERENCE));
	apcash.descript.assign(GetField(ID_JDESCRIPT));
	apcash.account.assign(GetField(ID_APCONTROL));
	apcash.depart.assign(GetField(ID_APDEPART));
	apcash.vatacct.assign(GetField(ID_VATACCT));
	apcash.vatdept.assign(GetField(ID_VATDEPT));
	apcash.entryno.assignLong(lGetField(ID_LINENO));
	apcash.UpdateUserTime(szUser);
	apcash.payable.assignDouble(lInvoiceAmount + apcash.dPayable());
	apcash.vatamt.assignDouble(expense->VAT);
	apcash.rebateamt.assignDouble(expense->REBATE);
	apcash.vatpercent.assignDouble(dGetField(ID_VATPERCENTAGE));
	apcash.rebate.assignDouble(dGetField(ID_REBATE));
	//apcash.terms.assignDouble((long)dGetField(ID_TERMS));

	if(bNewBatch)
	 apcash.append();
	else
	 apcash.flush();


	char szDept[5];

	if(expense->VAT > 0)
	 {
	  strcpy(szDept,GetField(ID_VATDEPT));
	  UpdateBatchDetail(iBatchno,"",GetField(ID_VATACCT),szDept,expense->VAT);
	 }

	if(fabs(expense->REBATE) > 0)
	  UpdateBatchDetail(iBatchno,"",(LPSTR)szRebateAccount.c_str(),(LPSTR)szRebateDept.c_str(),expense->REBATE);


	// Update AP amount
	strcpy(szDept,GetField(ID_APDEPART));
	UpdateBatchDetail(iBatchno,"",GetField(ID_APCONTROL),szDept,-1*expense->NEWAP);

	// Add new expense

	strcpy(szDept,GetField(ID_DEPART));
	AddDetail(iBatchno,"",GetField(ID_ACCOUNTS),szDept,expense->NEWEXPENSE);

	delete [] expense;
	return;
}

void TAPCashEntry::UpdateBatchDetail(long iBatchno, char *szDescript, char *szAccount,
		char *szDepart,double dEntry)
{
 char szReference[41];
 if(strcmp(szDescript,"") == 0)
	{
	 char *szSeek = new char[12];
	 strcpy(szSeek,szAccount);
	 glacct.SetIndex(GetString(8145));
	 glacct.seek(strcat(szSeek,szDepart));
	 strcpy(szReference,glacct.descript.str());
	 delete szSeek;
	 }
  else
  strcpy(szReference,szDescript);

 apcashd.SetIndex(GetString(8140));
 if(apcashd.seek(iBatchno) == 0)
 {
		// If the entry for that Account and Department code has already been entered just update it.
		int iEntryno = 1;
	  while(iBatchno == apcashd.lBatchno())
		  {
			if(strcmp(StrTrim(szAccount),StrTrim(apcashd.account.str())) == 0
			  && strcmp(StrTrim(szDepart),StrTrim(apcashd.dept.str())) == 0 )
				{
				  apcashd.descript.assign(szReference);
				  if(dEntry > 0)
				  {
					 apcashd.debit.assignDouble(fabs(dEntry));
					 apcashd.credit.assignDouble(0.00);
				  }
					else if (dEntry < 0)
				  {
					 apcashd.debit.assignDouble(0.00);
					 apcashd.credit.assignDouble(fabs(dEntry));
				  }
					 apcashd.flush();

					 return;
				 }
			apcashd.skip();
			iEntryno++;
		  }
	  // if not found create a new line entry.
	  apcashd.appendStart();
	  apcashd.batchno.assignLong(iBatchno);
	  apcashd.descript.assign(szReference);
	  apcashd.account.assign(szAccount);
	  apcashd.dept.assign(szDepart);
	  apcashd.product.assign("");
	  apcashd.medreps.assign("");
	  apcashd.drcodes.assign("");

	  if(dEntry < 0)
		{
		 apcashd.debit.assignDouble(0.00);
		 apcashd.credit.assignDouble(fabs(dEntry));
		}
	  else if (dEntry > 0)
		{
		 apcashd.debit.assignDouble(fabs(dEntry));
		 apcashd.credit.assignDouble(0.00);
		}
		apcashd.append();

  }
 else
 {
	  apcashd.appendStart();
	  apcashd.batchno.assignLong(iBatchno);
	  apcashd.descript.assign(szReference);
	  apcashd.account.assign(szAccount);
	  apcashd.dept.assign(szDepart);
	  apcashd.product.assign("");
	  apcashd.medreps.assign("");
	  apcashd.drcodes.assign("");

	  if(dEntry < 0)
		{
		 apcashd.debit.assignDouble(0.00);
		 apcashd.credit.assignDouble(fabs(dEntry));
		}
	  else if (dEntry > 0)
		{
		 apcashd.debit.assignDouble(fabs(dEntry));
		 apcashd.credit.assignDouble(0.00);
		}
	  apcashd.append();

 }
 return;
}


