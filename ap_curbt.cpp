
#include "std_inc.h"

#include "ap_vendo.h"

#define ID_INVOICE 1030
#define ID_COMMIT 1056
#define  ID_VATACCT 1034
#define  ID_VATDEPT 1035
#define  ID_VIN_NUM 1004
#define PROFILE "GL.INI"
#define  NEWBATCH  10
#define  OLDBATCH  11
#define  MODBATCH  12
#define ID_CENTURYDATE 6115
#define ID_PRIORPERIOD 8192
#define ID_DEPT_DESC 3038
#define ID_PERIOD_USED 1037

extern HINSTANCE hInst;

extern BOOL bNEW;

void TAPInvoiceEntry::CurrentBatch(RTMessage)
{

 apbtch.SetIndexBatchno();
 if(apbtch.seek(GetField(ID_BATCHNO)) != 0)
	return;


 long headerNo = apbtch.recNo();
 int rc = apbtch.lock(headerNo);
 if(rc != 0)
  {
	 MessageBox(HWindow,GetString(1052),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	 return;
  }
  headerNo = apbtch.lBatchno();

  D->List->SetBool(SSB_REDRAW,FALSE);
  double lDebitTotal,lCreditTotal;
  lDebitTotal = lCreditTotal = 0;
  char szDueDate[11],szInvDate[11];
  if(apbtch.recCount() == 0)
  {
	MessageBox(HWindow,GetString(8153),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	return;
  }

  if(apvendor.seek(apbtch.vendorno.str()) == 0)
	SetVendorAcct(apbtch.vendorno.str());

  SetField(ID_BATCHNO,apbtch.batchno);
  SetField(ID_DESCRIPT,apbtch.descript);
  SetField(ID_VENDORNO,apbtch.vendorno);
  SetField(ID_VENDORNAME,apbtch.vdname);
  SetField(ID_INVOICENO,apbtch.invoiceno);
  SetField(ID_CURRBAL,apbtch.orderno);
  SetField(ID_REFERENCE,apbtch.reference);
  SetField(ID_APCONTROL,apbtch.account);
  SetField(ID_APDEPART,apbtch.depart);
  SetField(ID_REBATE,apbtch.rebate);


  InvoiceDate.assign(apbtch.invdate.str(),GetString(ID_CENTURYDATE));
  InvoiceDate.format(szInvDate,GetString(4024));
  SetField(ID_INVDATE,szInvDate);

  DueDate.assign(apbtch.duedate.str(),GetString(ID_CENTURYDATE));
  DueDate.format(szDueDate,GetString(4024));
  SetField(ID_DUEDATE,szDueDate);

  SetField(ID_TERMS,apbtch.terms);
  SetField(ID_VATPERCENTAGE,apbtch.vatpercent);
  SetField(ID_REBATE,apbtch.rebate);
  SetField(ID_ACCOUNTS,"");
  SetField(ID_DEPART,"");
  SetField(ID_DEBIT,"");
  SetField(ID_CREDIT,"");
  SetField(ID_AMOUNT,apbtch.payable);
  SetField(ID_PERIOD_USED,apbtch.period);

  int i=1;

  apbtchd.SetIndex("batchno");
  apbtch.SetIndexBatchno();
  LPSTR szLineNo = new char[5];

  D->List->ClearData(SS_ALLCOLS,SS_ALLROWS);
  double dAPAmount = 0;
  string szAccount(apbtch.account.str());
  szAccount += apbtch.depart.str();



  if (apbtchd.seek(double(Str4ptr(apbtch.batchno.str()))) == 0){
	while (apbtch.lBatchno() == apbtchd.lBatchno() && !apbtchd.eof() ){
	  if(apbtchd.dDebit() > 0){
		lDebitTotal += apbtchd.dDebit();
		ltoa(i,szLineNo,10);
		D->List->SetData(1,i,szLineNo);
		D->List->SetData(2,i,apbtchd.descript.str());

		string strAccount(apbtchd.account.str());
		strAccount += apbtchd.dept.str();

		//D->List->SetData(3,i,apbtchd.account.str());
		D->List->SetData(3,i,(LPSTR)strAccount.c_str());

		//D->List->SetData(4,i,apbtchd.dept.str());
		D->List->SetData(4,i,apbtchd.depart.str());
		D->List->SetData(5,i,apbtchd.debit.str());
		D->List->SetData(6,i,apbtchd.credit.str());
		apbtchd.entryno.assignLong(i);
		// Assumes credit is always zero if debit > 0
		if(strAccount == szAccount) // Compare current account to AP account.
		  dAPAmount += -(apbtchd.dDebit());

		i++;
	  }
	  apbtchd.skip();
	}
 }

  //Set the credit amounts.
 long detailNo = 0;

 if (apbtchd.seek(headerNo) == 0){
	 detailNo = apbtchd.lBatchno();

	while ( detailNo ==  headerNo)
	 {
	  if(apbtchd.dCredit() > 0){
		lCreditTotal += apbtchd.dCredit();
		ltoa(i,szLineNo,10);
		D->List->SetData(1,i,szLineNo);
		D->List->SetData(2,i,apbtchd.descript.str());

		string strAccount(apbtchd.account.str());
		strAccount += apbtchd.dept.str();

		D->List->SetData(3,i,(LPSTR)strAccount.c_str());
		D->List->SetData(4,i,apbtchd.depart.str());

		//D->List->SetData(3,i,apbtchd.account.str());
		//D->List->SetData(4,i,apbtchd.dept.str());
		D->List->SetData(5,i,apbtchd.debit.str());
		D->List->SetData(6,i,apbtchd.credit.str());
		apbtchd.entryno.assignLong(i);

		// Assumes credit is always zero if debit > 0
		if(strAccount == szAccount) // Compare current account to AP account 
		  dAPAmount += (apbtchd.dCredit());
		i++;
	  }
	  apbtchd.skip();
	  detailNo = apbtchd.lBatchno();
	}

  }
	apbtch.payable.assignDouble(dAPAmount);
	SetField(ID_AMOUNT,apbtch.payable);

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
	if(round(szCreditTotal,2) == round(szDebitTotal,2))
	{
	 D->List->SetData(2,i,GetString(6114));
	}
	else
	{
	 D->List->SetColor(2,i,RGBCOLOR_WHITE,RGBCOLOR_RED);
	 D->List->SetData(2,i,GetString(6113));
	}

	D->List->SetData(5,i,szDebitTotal.str());
	D->List->SetData(6,i,szCreditTotal.str());

	apbtchd.iCurrentRecord = 0;
	delete szLineNo;

	if(bSlipList && SlipList != NULL)
   	SlipList->Show(SW_HIDE);


  if(strcmp(apbtch.mode.str(),"0") == 0)
	{
	 bRecovered = FALSE;
	}
  D->List->SetColor(5,i,RGB(192,220,192), RGBCOLOR_BLUE);
  D->List->SetColor(6,i,RGB(192,220,192), RGBCOLOR_BLUE);
  D->List->SetBool(SSB_REDRAW,TRUE);
  SetFocus(GetDlgItem(HWindow,ID_AMOUNT));
  bNEW = FALSE;
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


void TAPInvoiceEntry::SetVendor(RTMessage)
{
 if(apvendor.seek((LPSTR)VendorList->strKey.c_str()) == 0)
 {
	SetField(ID_VENDORNO,apvendor.vendorno);
	SetField(ID_VENDORNAME,apvendor.vdname);
	SetField(ID_APCONTROL,apvendor.account);
	SetField(ID_APDEPART,apvendor.depart);
	SetField(ID_REBATE,apvendor.rebate);
	SetField(ID_VATPERCENTAGE,apvendor.percentage);
	SetField(ID_TERMS,apvendor.terms);
	SetField(ID_VATACCT, apvendor.vat_acct);
	SetField(ID_VATDEPT, apvendor.vat_dept);
	if(strcmp(apvendor.type.str(),"C") == 0 )
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
}

void TAPInvoiceEntry::SetVendorAcct(char *szVendorCode)
{
	SetField(ID_VENDORNO,szVendorCode);
	SetField(ID_VENDORNAME,apvendor.vdname);
	SetField(ID_VATACCT, apvendor.vat_acct);
	SetField(ID_VATDEPT, apvendor.vat_dept);
	if(strcmp(apvendor.type.str(),"C") == 0 )
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


