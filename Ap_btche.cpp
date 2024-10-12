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

BOOL bNEW;



void TManilaGL::APBatch       (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPInvoiceEntry(this,1042));
  //GetApplication()->ExecDialog(new TCreateVoucher(this,1049));
 }



void TAPInvoiceEntry::LoadDoctors(void)
 {
  SendDlgItemMsg(ID_DRCODES,CB_RESETCONTENT,NULL,NULL);
  drcodes.SetIndex("drcode");
  for(drcodes.top();!drcodes.eof();drcodes.skip())
	 SendDlgItemMsg(ID_DRCODES,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)drcodes.rpname.str());
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


void TAPInvoiceEntry::ChangeDept(RTMessage Msg)
{
 char szDept[11];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  int Indx = (int)SendDlgItemMsg(ID_DEPTCODES,CB_GETCURSEL,0,0);

	//
	 if(Indx != CB_ERR){
	  SendDlgItemMsg(ID_DEPTCODES,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szDept);

			if(gldept.seek(szDept) == 0){
			SetField(3038,gldept.descript.str());
			}
	 }

 }
}

char  *TAPInvoiceEntry::GetDept()
{
	static char szString[11];
	int Indx = (int)SendDlgItemMsg(ID_DEPTCODES,CB_GETCURSEL,NULL,NULL);
	if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(ID_DEPTCODES,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szString);
	}
  return szString;
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


void TAPInvoiceEntry::LoadDepartments(void)
{
  /*
  SendDlgItemMsg(ID_MEDREPS,CB_RESETCONTENT,NULL,NULL);
  medreps.SetIndex("rpname");
  for(medreps.top();!medreps.eof();medreps.skip())
	 SendDlgItemMsg(ID_MEDREPS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)medreps.rpname.str());
	 SendDlgItemMsg(ID_MEDREPS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)" ");
  */
  gldept.SetIndex("cld");
  SendDlgItemMsg(ID_DEPTCODES,CB_RESETCONTENT,NULL,NULL);
  for(gldept.top();!gldept.eof();gldept.skip())
	 {
		SendDlgItemMsg(ID_DEPTCODES,CB_ADDSTRING,NULL,(LPARAM) (LPCSTR)gldept.dept.str());
	 }
  SendDlgItemMsg(ID_DEPTCODES,CB_ADDSTRING,NULL,(LPARAM) (LPCSTR)" ");

}

void TAPInvoiceEntry::LoadProducts(void)
{
  SendDlgItemMsg(ID_PRODUCTS,CB_RESETCONTENT,NULL,NULL);
  prods.SetIndex("ffulln");
  for(prods.top();!prods.eof();prods.skip())
	SendDlgItemMsg(ID_PRODUCTS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)prods.fFulln.str());
	SendDlgItemMsg(ID_PRODUCTS,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)" ");
}



static COLORREF lpBack, lpFore;


BOOL Tapbtch::Posted()
{
 if(strcmp(mode.str(),"0") == 0)
  return TRUE;
 else
  return FALSE;
}

void Tapbtch::SetIndexBatchno()
{
 SetIndex("batchno");
}

BOOL Tapbtch::Open()
{
 if(strcmp(mode.str(),"1") == 0)
  return TRUE;
 else
  return FALSE;
}

BOOL Tapbtch::Closed()
{
 if(strcmp(mode.str(),"0") == 0)
  return TRUE;
 else
  return FALSE;
}

BOOL Tapbtch::New(long iBatchno)
{
 CHourGlass wait;
 SetIndexBatchno();
 if(seek(iBatchno) != 0)
  return TRUE;
 else
  return FALSE;
}

void TAPInvoiceEntry::AddVendor(RTMessage)
{
 GetApplication()->ExecDialog(new TAPVendorBatchAdd(this,1041,GetField(ID_VENDORNO)));
 return;
}


void TAPInvoiceEntry::Top(RTMessage Msg)
{
 CHourGlass wait;
 apbtch.SetIndex("POST_AP");
 apbtch.top();
 SetField(ID_BATCHNO,apbtch.batchno);
 CurrentBatch(Msg);
 //apbtch.SetIndexBatchno();
}

void TAPInvoiceEntry::Bottom(RTMessage Msg)
{
 CHourGlass wait;
 apbtch.SetIndex("POST_AP");
 apbtch.bottom();
 SetField(ID_BATCHNO,apbtch.batchno);
 CurrentBatch(Msg);
 //apbtch.SetIndexBatchno();
}

void TAPInvoiceEntry::Next(RTMessage Msg)
{
 CHourGlass wait;
 apbtch.SetIndex("POST_AP");
 apbtch.skip();
 if(apbtch.eof())
	apbtch.bottom();
 SetField(ID_BATCHNO,apbtch.batchno);
 CurrentBatch(Msg);
 //apbtch.SetIndexBatchno();
}

void TAPInvoiceEntry::Previous(RTMessage Msg)
{
 CHourGlass wait;
 apbtch.SetIndex("POST_AP");
 if(apbtch.seek(GetField(ID_BATCHNO)) == 0)
 {
  apbtch.skip(-1);
  if(apbtch.bof())
	apbtch.top();
  SetField(ID_BATCHNO,apbtch.batchno);
 }
 else
  apbtch.bottom();
   
 CurrentBatch(Msg);
 //apbtch.SetIndexBatchno();
}



void TAPInvoiceEntry::Delete(RTMessage Msg)
{
  CHourGlass wait;
  if(apbtch.recCount() != 0 || bModify)
  {
	int iEntryno;
	IntGetValue(GetDlgItem(HWindow,ID_LINENO),(LPLONG)&iEntryno);
	string strBatchno(GetApplication()->hInstance,8140);
	apbtchd.SetIndex(strBatchno);
	if(apbtchd.seek(apbtch.lBatchno()) == 0)
	{
	 if(MessageBox(HWindow,GetString(6098),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO | MB_ICONQUESTION) == IDYES)
	 {

		while(apbtchd.lBatchno() == apbtch.lBatchno())
		{
		 if(apbtchd.iJournal() == iEntryno)
		  {
				apbtchd.deleteRec();
				//apbtchd.pack();
		  }
			apbtchd.skip();
		}
	  }
	}
  }
  CurrentBatch(Msg);
  return;
}


void TAPInvoiceEntry::New(RTMessage)
{
 if(bTransaction)
  {
	switch(MessageBox(HWindow,GetString(ID_COMMIT),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNOCANCEL |MB_ICONQUESTION))
	{
	 case IDCANCEL: return;
	 case IDNO    : SendMessage(HWindow,WM_COMMAND,CM_ROLLBACK,NULL); break;
	 case IDYES   : {
							 if(SlipList != NULL)
									SlipList->List->SetFields();
							 else
								 {
									CHourGlass wait;
									SlipList  = new TBatchList(this,"");
									SlipList->EnableAutoCreate();
									GetApplication()->MakeWindow(SlipList);
								 }
						 }
						 break;
	}
 }
 Clear();
 bNEW = TRUE;
 bModify = FALSE;
 return;
}

void TAPInvoiceEntry::RollBack(RTMessage)
{
 CHourGlass wait;

 apbtch.SetIndex("POST_AP");
 long lBatchno = lGetField(ID_BATCHNO);
 if(apbtch.seek(lBatchno) == 0)
	apbtch.deleteRec();

 if(apbtchd.seek(lBatchno) == 0)
  while(apbtchd.lBatchno() == lBatchno)
  {
	 apbtchd.deleteRec();
	 apbtchd.skip();
  }
 apbtchd.pack();
 apbtch.pack();
 return;
}




void TAPInvoiceEntry::ProductList(RTMessage Msg)
{
 string strProduct;
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  long Indx = SendDlgItemMsg(5017,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(5017,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM) ((LPSTR) strProduct.c_str()));
	 if(prods.seek((LPSTR)strProduct.c_str()) == 0)
		SetField(5018,prods.fPcode);
	 else
		MessageBox(HWindow,(LPSTR)strProduct.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
	 SetFocus(HWindow);
	}
 }
}


int TAPInvoiceEntry::SetBatchNumber()
{
 //apbtch.SetIndex(GetString(8140));
 apbtch.SetIndexBatchno();
 int db=0;
 apbtch.bottom();
 db = (int)apbtch.lBatchno();
 db++;
 return db;
}




void TAPInvoiceEntry::Period(RTMessage Msg)
{
 char szPeriod[3];
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  GL_PERIODS gl_pds;
  if(!gl_pds.open(((TAcctModule *)GetApplication())->cb) == 0)
  {
	MessageBox(HWindow,GetString(1038),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	return;
  }
  gl_pds.SetIndex("PERIOD");
  long Indx = SendDlgItemMsg(ID_PERIOD,CB_GETCURSEL,0,0);
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





BOOL TAPInvoiceEntry::GetWithHolding()
{
  char szWithHolding[2];
  GetPrivateProfileString(GetString(8150),"TAX","T",szWithHolding,sizeof(szWithHolding),PROFILE);
  if(strcmp(szWithHolding,"T") == 0)
	return TRUE;
  else
	return FALSE;
}

//#pragma argsused


void TAPInvoiceEntry::ModifyCurrentSlip()
{

  string szAPAccount(apbtch.account.str());
  string szAccount(apbtchd.account.str());
  string szNewAccount(GetField(ID_ACCOUNTS));
  string szInd = strIndTaxAccount;
  string szCo = strCompanyTaxAccount;
  string szVat(GetField(1034));

  if(szAPAccount == szAccount)
  {
	 //MessageBox(HWindow,(LPSTR)szNewAccount.c_str(),"",MB_OK);
	 if(szAccount != szNewAccount)
	  apbtch.account.assign((LPSTR)szNewAccount.c_str());
	 apbtch.payable.assignDouble(dGetField(ID_AMOUNT));
  }

  apbtch.account.assign(GetField(1087));
  apbtch.date.assign(InvoiceDate.str());
  	 /****************************************************************************
	 If the current open periods does not equal the period which the invoice was
	 created in ask the user whether they would like to modify the current
	 period to match the open period or keep the origal period.
	 ****************************************************************************/

	  if(GetPeriod() != (long) dGetField(ID_PERIOD_USED))
		{
		 char szPeriod[10];
		 long iPeriod = (long) dGetField(ID_PERIOD_USED);
		 long iCurrentPd = GetPeriod();

		 string strMsg(GetString(113));
		 ltoa(iPeriod,szPeriod,10);
		 strMsg += szPeriod;
		 strMsg += " to ";
		 ltoa(iCurrentPd,szPeriod,10);
		 strMsg += szPeriod;
		 strMsg += "?";

		 if(MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO) == IDNO)
		  apbtch.period.assignLong(dGetField(ID_PERIOD_USED));
		 else
		  apbtch.period.assignLong(iCurrentPd);
		}
	 else
	  apbtch.period.assignLong(GetPeriod());

   //apbtch.period.assignDouble(GetPeriod(HWindow));
  apbtch.orderno.assign(GetField(ID_CURRBAL));
  apbtch.invoiceno.assign(GetField(ID_INVOICENO));
  apbtch.duedate.assign(DueDate.str());
  apbtch.invdate.assign(InvoiceDate.str());
  apbtch.reference.assign(GetField(ID_REFERENCE));
  apbtch.descript.assign(GetField(ID_JDESCRIPT));
  apbtch.depart.assign(GetField(ID_APDEPART));
  apbtch.payable.assignDouble(dGetField(ID_AMOUNT));



  if(szInd == szAccount || szCo == szAccount)
	{
	 if(dGetField(ID_DEBIT) > 0)
	  apbtch.rebateamt.assignDouble(dGetField(ID_DEBIT));
	 else
	 if(dGetField(ID_CREDIT) > 0)
	  apbtch.rebateamt.assignDouble(-dGetField(ID_CREDIT));
	}

  if(szVat == szAccount)
  {
	if(dGetField(ID_DEBIT) > 0)
	  apbtch.vatamt.assignDouble(dGetField(ID_DEBIT));
	else
	if(dGetField(ID_CREDIT) > 0)
	 apbtch.vatamt.assignDouble(-dGetField(ID_CREDIT));
  }

  if(szAPAccount == szAccount)
  {
   apbtch.payable.assignDouble(dGetField(ID_AMOUNT));
  }

	apbtch.entryno.assignLong(lGetField(ID_LINENO));
	apbtch.rebate.assignDouble(dGetField(ID_REBATE));
	apbtch.terms.assignLong(lGetField(ID_TERMS));
	apbtch.vatpercent.assignDouble(lGetField(ID_VATPERCENTAGE));
	apbtch.UpdateUserTime(szUser);
	apbtch.flush();

	apbtchd.batchno.assignLong(iBatchno);
	apbtchd.account.assign(GetField(ID_ACCOUNTS));
	apbtchd.dept.assign(GetField(ID_DEPART));
	apbtchd.descript.assign(GetField(ID_JDESCRIPT));

	string szMedRep(GetDept());
	string szProduct(GetProduct());
	string szDoctor(GetDoctor());
	//UpdateBatchSupp(iBatchno,&szMedRep,&szProduct,&szDoctor);
	apbtchd.product.assign((LPSTR) szProduct.c_str());
	apbtchd.depart.assign((LPSTR) szMedRep.c_str());
	apbtchd.drcodes.assign((LPSTR) szDoctor.c_str());

	apbtchd.debit.assignDouble(dGetField(ID_DEBIT));
	apbtchd.credit.assignDouble(dGetField(ID_CREDIT));
	apbtchd.flush();

	bModify = FALSE;
	return;
}



void TAPInvoiceEntry::SearchGL    (RTMessage)
  {
	CHourGlass Wait;
	if(AcctList == NULL)
	{
	 AcctList = new TAccountList(this,"");
	 AcctList->EnableAutoCreate();
	 GetApplication()->MakeWindow(AcctList);
	}
	AcctList->List->Seek(ID_ACCOUNTS);
	AcctList->Show(SW_SHOW);
  }



void TAPInvoiceEntry::Vendorlist  ()
{
	CHourGlass wait;

 if(VendorList == NULL)
 {
  VendorList = new TCustomerList(this,"");
  VendorList->EnableAutoCreate();
  GetApplication()->MakeWindow(VendorList);
  VendorList->SetCaption(GetString(8182));
 }
  VendorList->List->Seek(ID_VENDORNO);
  VendorList->Show(SW_SHOW);
}

void TAPInvoiceEntry::Sliplist() //slip list search
{
  CHourGlass wait;

	if(SlipList == NULL)
	{
		 SlipList  = new TBatchList(this,"");
		 SlipList->EnableAutoCreate();
		 GetApplication()->MakeWindow(SlipList);
	}
	SlipList->List->Seek(ID_BATCHNO);
	SlipList->Show(SW_SHOW);
}

long TAPInvoiceEntry::GetPeriod()
{
 char szPeriod[3];
 DWORD Indx = SendDlgItemMsg(ID_PERIOD,CB_GETCURSEL,0,0);
 SendDlgItemMsg(ID_PERIOD,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szPeriod);
 return atol(szPeriod);
}


void TAPInvoiceEntry::Accountlist ()
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


void TAPInvoiceEntry::Clear()
{
 CHourGlass wait;
 IntSetDlgItemValue(HWindow,ID_BATCHNO,SetBatchNumber());
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
 SetField(3038,"");
 char szPeriod[3];
 ltoa(GetPeriod(),szPeriod,10);
 SetField(ID_PERIOD_USED,szPeriod);
 DebitTotal=CreditTotal=0;
 bTransaction = FALSE;


 D->List->SetColor(SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
 D->List->ClearData(SS_ALLCOLS,SS_ALLROWS);
 SetField(ID_DEBIT,"");
 SetField(ID_CREDIT,"");
 SetField(ID_AMOUNT,"");


 //FloatSetDlgItemValue(HWindow,ID_DEBIT,0.00);
 //FloatSetDlgItemValue(HWindow,ID_CREDIT,0.00);
 //FloatSetDlgItemValue(HWindow,ID_INVOICE,0.00);

 long Indx = SendDlgItemMsg(ID_PRODUCTS,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_PRODUCTS,CB_SETCURSEL,(WPARAM)Indx,NULL);

 Indx = SendDlgItemMsg(ID_MEDREPS,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_MEDREPS,CB_SETCURSEL,(WPARAM)Indx,NULL);

 Indx = SendDlgItemMsg(ID_DRCODES,CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
 SendDlgItemMsg(ID_DRCODES,CB_SETCURSEL,(WPARAM)Indx,NULL);

 SetFocus(GetDlgItem(HWindow,ID_VENDORNO));
}


void TAPInvoiceEntry::Print(RTMessage)
{
 //if(apbtch.seek(dGetField(ID_BATCHNO)) == 0)
 // PrintSlip(D->GetReportName(),(int)dGetField(ID_BATCHNO));
}


void TAPInvoiceEntry::HandleDepart (RTMessage ){}


void TAPInvoiceEntry::SetLineItem(RTMessage)
{
  bModify = TRUE;
  SetField(ID_JDESCRIPT,apbtchd.descript);
  SetField(ID_ACCOUNTS ,apbtchd.account);
  SetField(ID_DEPART   ,apbtchd.dept);
  SetField(ID_DEBIT    ,apbtchd.debit);
  SetField(ID_CREDIT   ,apbtchd.credit);
  SetField(ID_LINENO   ,apbtchd.entryno);
  SetField(ID_DEPT_DESC,"");
}

int TAPInvoiceEntry::CheckGLAccount(LPSTR account)
{
 if(glacct.seek(account)==0)
 return TRUE;
 else
 return FALSE;
}






void TManilaGL::APPriorPeriod (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPPriorPeriod(this,1042));
 }

void TAPPriorPeriod::SetupWindow()
{
  TAPInvoiceEntry::SetupWindow();
  string strCaption(hInst,ID_PRIORPERIOD);
  SetCaption((LPSTR)strCaption.c_str());
  mode = "3";
}

 LPSTR TAPInvoiceEntry::GetControlAcct(LPSTR Account,LPSTR Depart)
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


int GetFontSize()
{
 char szSize[3];
 string strFont(hInst,8183);
 string strSize(hInst,8184);

 GetPrivateProfileString((LPSTR)strFont.c_str(),(LPSTR)strSize.c_str(),"",szSize,sizeof(szSize),"GL.INI");
 //MessageBox(GetFocus(),szSize,"",MB_OK);
 return atoi(szSize);
}

int GetFontStyle()
{
 string strFont(hInst,8183);
 string strStyle(hInst,8185);
 char szSize[4];
 GetPrivateProfileString((LPSTR)strFont.c_str(),(LPSTR)strStyle.c_str(),"",szSize,sizeof(szSize),"GL.INI");
 return atoi(szSize);
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

void TAPDistList::SetField(int ID, LPSTR field)
{
  SendDlgItemMessage(Parent->HWindow,ID,WM_SETTEXT,NULL,(LPARAM)(LPSTR)field);
  return;
}

void TAPDistList::SetupWindow()
{
 TWindow::SetupWindow();
 RECT rc;
 CHourGlass wait;
 SetListSize(8);

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
 List->SetColWidth(4,8);
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
 List->SetCellType(3, SS_ALLROWS, &CellType);
 List->SetCellType(4, SS_ALLROWS, &CellType);

 //List->SetTypeEdit(&CellType,ES_RIGHT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 //List->SetCellType(3, SS_ALLROWS, &CellType);
 //List->SetCellType(4, SS_ALLROWS, &CellType);


 List->SetTypeFloat(&CellType,FS_SEPARATOR,15,2,0,atof(GetString(hInst,6107)));
 List->SetCellType(5, SS_ALLROWS, &CellType);
 List->SetCellType(6, SS_ALLROWS, &CellType);


}

void TAPDistList::SetListSize (int Rows)
{
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
  ((PTAPInvoiceEntry)Parent)->bModify = FALSE;
  if(iBatchno > 0)
  {
	  if(apbtd->seek(iBatchno) == 0)
	  {
		 while(iBatchno == apbtd->lBatchno())
		 {
		  if(nIndx == apbtd->iJournal())
			{
			 apbtd->iCurrentRecord = (int)apbtd->recNo();
			 ((PTAPInvoiceEntry)Parent)->bModify = TRUE;
			 break;
			}
		  apbtd->skip();
		 }
	  }
	}

  long Indx = SendMessage(GetDlgItem(Parent->HWindow,ID_DEPTCODES),CB_FINDSTRING,NULL,(LPARAM)(LPSTR)apbtd->depart.str());
  if(Indx == CB_ERR )
	Indx = SendMessage(GetDlgItem(Parent->HWindow,ID_DEPTCODES),CB_FINDSTRING,NULL,(LPARAM)(LPSTR)" ");
  SendMessage(GetDlgItem(Parent->HWindow,ID_DEPTCODES),CB_SETCURSEL,(WORD)Indx,NULL);

  if(((PTAPInvoiceEntry)Parent)->bModify)
  {
	List->SetColor(SS_ALLCOLS,SS_ALLROWS,lpBack,lpFore);
	List->SetColor(SS_ALLCOLS,nIndx,RGB(192,220,192), RGBCOLOR_BLUE);
	SendMessage(Parent->HWindow,WM_COMMAND,CM_LINEITEM,NULL);
  }
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
 WritePrivateProfileString(szClassName.c_str(),"L",szLeft.str(),PROFILE);
 WritePrivateProfileString(szClassName.c_str(),"T",szTop.str(),PROFILE);
 WritePrivateProfileString(szClassName.c_str(),"R",szRight.str(),PROFILE);
 WritePrivateProfileString(szClassName.c_str(),"B",szBottom.str(),PROFILE);
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

