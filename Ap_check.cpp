#include "std_inc.h"

#include "ap_verif.h"
#include "db_glcsh.h"
#include "ap_check.h"

//#include "sql.h"
//#include "sqlext.h"
//#include "ttab.h"

#define ID_CENTURYDATE 6115
#define APTAB 1500
#define APVENDOR 1501
#define APPAYMENT 1502
#define APBANKREC 1503


LPSTR   strTrim (LPSTR   Str)
{
	return StrLTrim (StrRTrim (Str));
}

/*
 *    Remove Leading blanks
 */
LPSTR   strLTrim(LPSTR Str)
{
	LPSTR   s = Str;

	while (isspace (*Str))
		Str++;

	return strcpy (s, Str);
}

/*
 *              Remove trailing blanks
 */
LPSTR   strRTrim(LPSTR Str)
{
	LPSTR   s = Str,p;
	int     Len;

	Len = strlen(Str);
	p = Str+Len-1;

	while(Len-- && isspace (*p))
		p--;

	p[1] = '\0';

	return s;
}




BOOL bOK;



char* TAPCreatePayment::GetDescription (char* szAccount)
{

	if( glaccts.seek(szAccount) == 0)
	 return glaccts.descript.str();
	else
	 return "Unknown";
}


class TAPHoldForce: public TMainDialog
{
 // Dialog No. 1066
 public:
 Tapdist *apdist;
 BOOL Hold;
 BOOL Force;
 TAccountList* List;
 TAPHoldForce(PTWindowsObject AParent, int AName): TMainDialog(AParent,AName)
 {
	List = NULL;
 }
 virtual void SetupWindow();
 virtual void Ok            (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel        (RTMessage Msg) = [ID_FIRST + IDCANCEL] {ShutDownWindow();}
 virtual void Holding       (RTMessage Msg) = [ID_FIRST + 4107]{SetType('H');}
 virtual void Clear         (RTMessage Msg) = [ID_FIRST + 4111]{SetType('C');}
 virtual void Open          (RTMessage Msg) = [ID_FIRST + 4108]{SetType('O');}
 virtual void Forcing       (RTMessage Msg) = [ID_FIRST + 4106]{SetType('F');}
 virtual void Paid          (RTMessage Msg) = [ID_FIRST + 4109]{SetType('P');}
 virtual void Verified      (RTMessage Msg) = [ID_FIRST + 4110]{SetType('V');}
 virtual void Search        (RTMessage Msg) = [ID_FIRST + 1039];
 void SetType(char Flag);
 ~TAPHoldForce()
 {
	if(List != NULL)
	 delete List;
 }
};

void TAPHoldForce::Search (RTMessage)
{
  if (List == NULL)
  {
	  List = new TAccountList(this,"");
	  List->SetAccountID(1038,1040,1041);
	  List->Attr.Style |= WS_THICKFRAME;
	  List->EnableAutoCreate();
	  GetApplication()->MakeWindow(List);
	  List->List->Redraw();
  }
  List->Show(SW_SHOW);
}

void TAPHoldForce::SetType(char Flag)
 {
	switch(Flag)
	{
	  case 'F' : SendDlgItemMsg(4106,BM_SETCHECK,TRUE,NULL);break;
	  case 'H' : SendDlgItemMsg(4107,BM_SETCHECK,TRUE,NULL);break;
	  case 'O' : SendDlgItemMsg(4108,BM_SETCHECK,TRUE,NULL); break;
	  case 'P' : SendDlgItemMsg(4109,BM_SETCHECK,TRUE,NULL);break;
	  case 'V' : SendDlgItemMsg(4110,BM_SETCHECK,TRUE,NULL);break;
	  case 'C' : SendDlgItemMsg(4111,BM_SETCHECK,TRUE,NULL);break;
	}
 }


void CreatePayment(void){}


void TAPCreatePayment::Verify(RTMessage Msg)
 {
	bVerified = FALSE;
	GetApplication()->ExecDialog(new TVerifyCheques(this,1157));
	if(bVerified)
	{
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
			SetDate(Msg);
	 }
 }

void TAPCreatePayment::GetBankAccount(RTMessage Msg)
{

 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  GL_SETAT gl_setat;
  gl_setat.open(((TAcctModule *)GetApplication())->cb);
  gl_setat.SetIndex("name");
  int Indx;
  Indx = (int)SendDlgItemMsg(ID_DESCRIPT,CB_GETCURSEL,0,0);
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
 gl_setat.open(((TAcctModule *)GetApplication())->cb);
 gl_setat.SetIndex("cash");
 for(gl_setat.top();!gl_setat.eof();gl_setat.skip())
  {
	 SendDlgItemMsg(ID_DESCRIPT,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)gl_setat.name.str());
  }
 gl_setat.close();
}




#pragma argsused
void TAPCreatePayment::Ok (RTMessage Msg)
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

  char *szSeek = new char [12];
  strcpy(szSeek,GetField(ID_ACCOUNTS));
  strcat(szSeek,GetField(ID_DEPART));

  if(glaccts.seek(szSeek) != 0)
  {
	MessageBox(HWindow,GetString(4007),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	return;
  }
  else
	delete [] szSeek;

  apdist.SetIndexVendorno();

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
	 apdist.SetIndex("Payment");
	 apdist.top();
	 strcpy(szVendorNo,apdist.vendorno.str());
	 while(!apdist.eof())
	  {
		Date4 dDueDate(apdist.duedate.str());
		if(dDueDate <= dPayment)
			{
			  // The new vendor number is copied back from the UpdateAPVendor.
			  // Then when we go to update the check numbers the cheque number
			  // stays the same for all the vendors.
			  strcpy(szVendorNo,UpdateAPVendor(szVendorNo,iStartingReq,dDatepaid));
			}
		iStartingReq++;
	  }
		apdist.SetIndexVendorno();
		//CreateGLEntry();
	 }

  //string strMsg(hInst,4038);
  //MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
  //string szQuery(GetString(4006));
  //apdist.SetIndexInvoiceno();
  //bHasInvoices = DistList((char *) szQuery.c_str());
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
	//SetDate(Msg);
 }
 else
 MessageBox(HWindow,GetString(4079),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
 return;
}

void TAPCreatePayment::CreateCashTracing( long   lBatchno,
														string *strApaccount,
														string *strApdept,
														string *strCashAccount,
														string *strCashDept,
														long lPeriod)
{
  string strAccount;
  strAccount = strApaccount->c_str();
  strAccount += strApdept->c_str();

  int i = 0;

  if(apbtchd.seek(lBatchno) == 0)
	{
	  long lAp_batchno = apbtchd.lBatchno();
	  while(lBatchno == lAp_batchno)
		{
			string strCompare;
			strCompare = apbtchd.account.str();
			strCompare += apbtchd.dept.str();
			if(strAccount != strCompare)
			{
				gl_cash.appendStart();
				gl_cash.batchno.assignLong(lBatchno);
				gl_cash.account.assign(apbtchd.account.str());
				gl_cash.apaccount.assign((LPSTR)strCashAccount->c_str());
				gl_cash.apdept.assign((LPSTR)strCashDept->c_str());
				gl_cash.dept.assign(apbtchd.dept.str());
				gl_cash.amount.assignDouble(-apbtchd.dCredit()+apbtchd.dDebit());
				gl_cash.period.assignLong(lPeriod);
				gl_cash.UpdateUserTime(szUser);
				gl_cash.append();

			}
		  apbtchd.skip();
		  lAp_batchno = apbtchd.lBatchno();
		  ++i;
		  if(i > 10)
			break;
		}

	}
}

char* TAPCreatePayment::UpdateAPVendor(char* szVendorno,double iStartingReq,Date4 dDatepaid)
 {
	string reference,date,chequeno,apaccount,apdept,account,dept,ex_acct,ex_dept;
	long lapbatchno;
	iStartingReq = fabs(iStartingReq);
	string strVendor(strTrim(szVendorno));
	string strCompare(strTrim(szVendorno));
	double dAmount = 0;
	double dExGainLoss = 0;
	while(strVendor == strCompare)
	 {
		dAmount += apdist.dAmount();
		dExGainLoss += apdist.dGainOrLoss();
		apdist.paymentctl.assign("P");
		apdist.chequeno.assignDouble(iStartingReq);
		apdist.account.assign(GetField(ID_ACCOUNTS));
		apdist.dept.assign(GetField(ID_DEPART));
		apdist.amountpaid.assignDouble(apdist.dAmount());
		apdist.datepaid.assign(dDatepaid.str());
		reference = apdist.reference.str();
		date = apdist.date.str();
		chequeno  = apdist.chequeno.str();
		apaccount = apdist.apaccount.str();
		apdept    = apdist.apdepart.str();
		account   = GetField(ID_ACCOUNTS);
		dept      = GetField(ID_DEPART);
		ex_acct   = apdist.ex_acct.str();
		ex_dept   = apdist.ex_dept.str();
		lapbatchno  = long (Str4ptr(apdist.batchno.str()));
		apdist.skip();
		strCompare = strTrim(apdist.vendorno.str());
	 }

  CreateCashTracing(lapbatchno,&apaccount,&apdept,&account,&dept,lPeriod);

  glbtch.bottom();
  long lBatchno = glbtch.Batchno();
  ++lBatchno;

  /*
	 If the ex_amount is greater than zero. You must add a an extra entry to
	 record the gain or loss in exchange rate.

  */

  AppendHeader(lBatchno,dAmount,&date,&reference,&chequeno);
  AppendDetail(lBatchno,dAmount,&date,&reference,&chequeno,&apaccount,&apdept); // AP


  if(dExGainLoss != 0)
  {
	 AppendDetail(lBatchno,-(dAmount+dExGainLoss),&date,&reference,&chequeno,&account,&dept);  // CASH
	 AppendDetail(lBatchno, dExGainLoss,&date,&reference,&chequeno,&ex_acct,&ex_dept);  // CASH
  }
  else
	 AppendDetail(lBatchno,-(dAmount),&date,&reference,&chequeno,&account,&dept);  // CASH


  return (LPSTR)strCompare.c_str();
}

void TAPCreatePayment::AppendHeader(long lBatchno,
												double dAmount,
												string* date,
												string* reference,
												string* chequeno)
{
	 glbtch.appendStart();
	 glbtch.batchno.assignLong(lBatchno);
	 glbtch.mode.assign("1");
	 glbtch.date.assign((LPSTR)date->c_str());          //glbtch.date.assign(apdist.date.str());
	 glbtch.type.assign("AP");
	 glbtch.period.assignLong(lPeriod);
	 glbtch.descript.assign((LPSTR)reference->c_str()); //glbtch.descript.assign(apdist.descript.str());
	 glbtch.debit.assignDouble(dAmount);
	 glbtch.credit.assignDouble(dAmount);
	 glbtch.cheque.assign((LPSTR)chequeno->c_str());    //glbtch.cheque.assign(apdist.chequeno.str());
	 glbtch.UpdateUserTime(szUser);
	 glbtch.append();
}


void TAPCreatePayment::AppendDetail(long lBatchno,
												double dAmount,
												string* date,
												string* reference,
												string* chequeno,
												string* account,
												string* dept)
{
	 glbtchd.appendStart();
	 glbtchd.batchno.assignLong(lBatchno);
	 glbtchd.mode.assign("0");
	 glbtchd.journal.assignLong(1);
	 glbtchd.period.assignLong(lPeriod);
	 glbtchd.sourcecode.assign("");
	 glbtchd.date.assign((LPSTR)date->c_str());
	 glbtchd.reference.assign((LPSTR)reference->c_str());
	 glbtchd.cheque.assign((LPSTR)StrTrim((LPSTR)chequeno->c_str()));
	 glbtchd.account.assign((LPSTR)account->c_str());
	 glbtchd.dept.assign((LPSTR)dept->c_str());

	 string szSeek;
	 szSeek = (LPSTR)account->c_str();
	 szSeek += (LPSTR)dept->c_str();
	 glbtchd.descript.assign(GetDescription((LPSTR)szSeek.c_str()));

	 if(dAmount > 0)
		glbtchd.debit.assignDouble(dAmount);
	 else
		glbtchd.debit.assignDouble(0);

	 if(dAmount < 0)
		glbtchd.credit.assignDouble(fabs(dAmount));
	 else
		glbtchd.credit.assignDouble(0);
	 glbtchd.append();
}



BOOL TAPCreatePayment::CheckVerifiedCheckPrinting()
{
	BOOL bVerified = TRUE;
	//apdist.SetIndex("vendorno");
	apdist.SetIndexVendorno();

	Relate4set rQuery(apdist);
	//string szQuery("PAYMENTCTL = 'P'");
	string szQuery(GetApplication()->hInstance,6109);
	rQuery.querySet((LPSTR) szQuery.c_str());
	long iCounter = 0;
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
 // Dialog No. 1066
 char szDueDate[11],szInvDate[11];
 bOK = FALSE;
 Date4 InvoiceDate;
 Date4 DueDate;

 TDialog::SetupWindow();
 SetField(ID_VENDORNO,apdist->vendorno);
 SetField(ID_INVOICENO,apdist->invoiceno);
 SetField(ID_CHEQUE,strTrim(apdist->chequeno.str()));
 SetField(1030,apdist->amount);        // Amount of orginal invoice
 SetField(1031,apdist->szAmountDue()); // Amount Due
 SetField(1033,apdist->vatamt);        // VAT Amount
 SetField(1034,apdist->rebateamt);     // Withholding Tax Amount
 SetField(1035,apdist->amountpaid);    // Amount Paid so far...
 SetField(1036,apdist->adjustment);    // Discount or adjustment
 SetField(1037,apdist->ex_amount);
 SetField(1038,apdist->ex_acct);
 SetField(1040,apdist->ex_dept);

 string seek(apdist->ex_acct.str());
 seek += apdist->ex_dept.str();

 Data4 glaccts(((TAcctModule *)GetApplication())->cb,"glaccts");
 if(glaccts.isValid())
 {
  Field4 desc(glaccts,"Descript");
  if ( glaccts.seek((LPSTR)seek.c_str()) == 0)
	SetField(1041,desc.str());
	glaccts.close();
 }

 InvoiceDate.assign(apdist->datepaid.str(),GetString(ID_CENTURYDATE));
 InvoiceDate.format(szInvDate,GetString(4024));
 SetField(ID_INVDATE,szInvDate);

 DueDate.assign(apdist->duedate.str(),GetString(ID_CENTURYDATE));
 DueDate.format(szDueDate,GetString(4024));
 SetField(ID_DUEDATE,szDueDate);

 char* controltype;
 controltype = apdist->paymentctl.str();
 switch(*controltype)
	{
	  case 'F' : SendDlgItemMsg(4106,BM_SETCHECK,TRUE,NULL);break;
	  case 'H' : SendDlgItemMsg(4107,BM_SETCHECK,TRUE,NULL);break;
	  case 'O' : SendDlgItemMsg(4108,BM_SETCHECK,TRUE,NULL); break;
	  case 'P' : SendDlgItemMsg(4109,BM_SETCHECK,TRUE,NULL);break;
	  case 'V' : SendDlgItemMsg(4110,BM_SETCHECK,TRUE,NULL);break;
	  case 'C' : SendDlgItemMsg(4111,BM_SETCHECK,TRUE,NULL);break;
	  default:break;
	}

}

void TAPHoldForce::Ok(RTMessage)
  {
	CHourGlass wait;
	char* controltype;
	controltype = apdist->paymentctl.str();
	switch(*controltype)
	{
	  case 'F' :
	  case 'H' :
	  case 'O' :
					  {
						if(SendDlgItemMsg(4106,BM_GETSTATE,NULL,NULL))
							  apdist->paymentctl.assign("F");
						else if (SendDlgItemMsg(4107,BM_GETSTATE,NULL,NULL))
							  apdist->paymentctl.assign("H");
						else if (SendDlgItemMsg(4108,BM_GETSTATE,NULL,NULL))
							  apdist->paymentctl.assign("O");
							  break;
					  }
	  case 'P' :
	  case 'V' :
	  case 'C' :
					  {
						if (SendDlgItemMsg(4109,BM_GETSTATE,NULL,NULL))
							  apdist->paymentctl.assign("P");
						else if (SendDlgItemMsg(4110,BM_GETSTATE,NULL,NULL))
							  apdist->paymentctl.assign("V");
						else if (SendDlgItemMsg(4111,BM_GETSTATE,NULL,NULL))
							  apdist->paymentctl.assign("C");
							  break;
					  }

	  default:break;
	}

	apdist->chequeno.assignDouble(atof(GetField(ID_CHEQUE)));
	apdist->adjustment.assignDouble(dGetField(1036));
	apdist->amount.assignDouble(dGetField(1030));
	apdist->amountpaid.assignDouble(dGetField(1035));
	apdist->vatamt.assignDouble(dGetField(1033));
	apdist->rebateamt.assignDouble(dGetField(1034));
	apdist->datepaid.assign(GetDate(ID_INVDATE));
	apdist->duedate.assign(GetDate(4075));
	apdist->ex_acct.assign(GetField(1038));
	apdist->ex_dept.assign(GetField(1040));
	apdist->ex_amount.assignDouble(dGetField(1037));

	apdist->UpdateUserTime(szUser);
	bOK = TRUE;
	ShutDownWindow();

}


void TAPCreatePayment::SelectCell(RTMessage Msg)
{
 string strInv,strVendorno;
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,3,Cell->Row,szMsg);
 strVendorno = szMsg;
 SSGetData(SHWindow,4,Cell->Row,szMsg);
 strInv = szMsg;
 strVendorno += strInv;
 apdist.SetIndexInvoiceno();

 if(apdist.seek((LPSTR)strVendorno.c_str()) == 0)
 {
  TAPHoldForce *HoldForce = new TAPHoldForce(this,1066);
  HoldForce->apdist = &apdist;
  GetApplication()->ExecDialog(HoldForce);
  if(bOK == TRUE)
  {
	InvoiceList->SetData(2,Cell->Row,apdist.paymentctl.str());
	InvoiceList->SetData(6,Cell->Row,apdist.amount.str());
	InvoiceList->SetData(7,Cell->Row,apdist.vatamt.str());
	InvoiceList->SetData(8,Cell->Row,apdist.rebateamt.str());
	double dAmountDue = apdist.dAmount() - apdist.dAmountpaid();
	Str4large szAmount;
	szAmount.assignDouble(dAmountDue,16,2);
	InvoiceList->SetData(9,Cell->Row,szAmount.str()); // Adjustment
	if(atol(apdist.chequeno.str()) > 0)
	 InvoiceList->SetData(10,Cell->Row,strTrim(apdist.chequeno.str()));

	char *controltype = apdist.paymentctl.str();
	switch(*controltype)
		  {
			case 'O' : InvoiceList->SetColor(SS_ALLCOLS,Cell->Row,RGB(192,220,192), RGBCOLOR_BLUE);break;
			//case 'H' : InvoiceList->SetColor(SS_ALLCOLS,Cell->Row,RGBCOLOR_WHITE, RGBCOLOR_BLUE);break;
			case 'H' : InvoiceList->SetColor(SS_ALLCOLS,Cell->Row,RGBCOLOR_WHITE, RGBCOLOR_BLACK);break;
			case 'F' : InvoiceList->SetColor(SS_ALLCOLS,Cell->Row,RGBCOLOR_WHITE, RGBCOLOR_BLUE);break;
			case 'P' : InvoiceList->SetColor(SS_ALLCOLS,Cell->Row,RGB(0,255,200), RGBCOLOR_BLUE);break;
			case 'C' : InvoiceList->SetColor(SS_ALLCOLS,Cell->Row,RGB(0,255,200), RGBCOLOR_BLUE);break;
			case 'V' : InvoiceList->SetColor(SS_ALLCOLS,Cell->Row,RGB(0,255,200), RGBCOLOR_BLUE);break;
		  }
  }
  }
  else
  {
	string strMsg(hInst,62);
	strMsg += strVendorno;
	MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
  }
}



void TAPCreatePayment::CreateGLEntry()
{

  glbtch.bottom();
  long lBatchno = glbtch.Batchno();
  ++lBatchno;

  apdist.SetIndex("paymentreq");
  Relate4set rQuery(apdist);
  string strQuery = GetString(6146);
  rQuery.querySet((LPSTR)strQuery.c_str());

  double dAmount = 0 ;
  int qc = rQuery.top();
  double compare =  double (Str4ptr(apdist.chequeno.str()));

  while(qc != r4eof)
	{
	  while(compare == double (Str4ptr(apdist.chequeno.str())))
		 {
			apdist.paymentctl.assign("P");
			dAmount += apdist.dAmountpaid();
			qc = rQuery.skip();
		 }
		//AppendHeader(lBatchno,dAmount);
		//AppendDetail(lBatchno,dAmount);
		//AppendDetail(lBatchno,-(dAmount));
		++lBatchno;
		dAmount = 0;
		compare = double (Str4ptr(apdist.chequeno.str()));
	}

	rQuery.unlock();
	rQuery.free();

	// apdist.SetIndex("paymentreq");
	// PAYMENTCTL = 'N'
	/*
	  Relate4set rQuery(apdist);
	  string strQuery = GetString(6146);

	  Check to make sure the ap_jsum database is being updated correctly.
	  CHECK

	rQuery.querySet((LPSTR)strQuery.c_str());
	for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
	 {
		 apdist.paymentctl.assign("P");
		 apjsum.appendStart();
		 apjsum.account.assign      (apdist.apaccount.str());
		 apjsum.dept.assign         (apdist.apdepart.str());
		 apjsum.period.assign       (apdist.period.str());
		 apjsum.mode.assign         ("1");
		 apjsum.date.assign         (apdist.date.str());
		 apjsum.type.assign         ("AP");
		 apjsum.batchno.assignDouble(double(Str4ptr(apdist.batchno.str())));
		 apjsum.reference.assign    (apdist.reference.str());
		 apjsum.descript.assign     (apdist.descript.str());
		 apjsum.debit.assignDouble  (apdist.dAmountpaid());
		 apjsum.credit.assignDouble (0.0);
		 apjsum.append();

		 apjsum.appendStart();
		 apjsum.account.assign      (apdist.account.str());
		 apjsum.dept.assign         (apdist.dept.str());
		 apjsum.period.assign       (apdist.period.str());
		 apjsum.date.assign         (apdist.date.str());
		 apjsum.mode.assign         ("1");
		 apjsum.type.assign         ("AP");
		 apjsum.batchno.assignDouble(double(Str4ptr(apdist.batchno.str())));
		 apjsum.reference.assign    (apdist.reference.str());
		 apjsum.descript.assign     (apdist.descript.str());
		 apjsum.debit.assignDouble  (0);
		 apjsum.credit.assignDouble (apdist.dAmountpaid());
		 apjsum.append();
	 }
	rQuery.unlock();
	rQuery.free();
	*/
}





class TCreatePaymentWindow : public TWindow
{
 public:
 char szlpAWindow[10];
 TAPCreatePayment* Form;
 TCreatePaymentWindow(TWindowsObject *parent,LPSTR ATitle) : TWindow(parent,""){}
 virtual void SetupWindow();
 virtual void WMSize(RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual BOOL CanClose();
 BOOL SetWindowPosition();
 BOOL SaveWindowPosition();
 int iLeft;
 int iTop;
 int iRight;
 int iBottom;
};

BOOL TCreatePaymentWindow::SetWindowPosition()
{
 RECT rc,rcDesktop;
 GetWindowRect(GetDesktopWindow(),&rcDesktop);
 GetWindowRect(HWindow,&rc);
 if(iRight == 0 || iRight > rcDesktop.right)
	iRight = rc.right;
 if(iBottom == 0 || iBottom > rcDesktop.bottom)
 iBottom = rc.bottom;
 int check = iBottom - iTop;
 if(check < 0)
  {
	 iBottom = 600;
	 iTop = 200;
  }
 //return MoveWindow(Parent->HWindow,iLeft,iTop,rc.right-rc.left,rc.bottom-rc.top,TRUE);
 return MoveWindow(HWindow,0,0,iRight-iLeft,iBottom-iTop,TRUE);

}

BOOL TCreatePaymentWindow::SaveWindowPosition()
{
 RECT rc;
 GetWindowRect(HWindow,&rc);
 Str4ten szLeft;
 Str4ten szTop;
 Str4ten szRight;
 Str4ten szBottom;
 szLeft.assignLong(3,5);
 szTop.assignLong(rc.top,5);
 szRight.assignLong(rc.right,5);
 szBottom.assignLong(rc.bottom,5);
 WritePrivateProfileString(szlpAWindow,"L",szLeft.str(),PROFILE);
 WritePrivateProfileString(szlpAWindow,"T",szTop.str(),PROFILE);
 WritePrivateProfileString(szlpAWindow,"R",szRight.str(),PROFILE);
 WritePrivateProfileString(szlpAWindow,"B",szBottom.str(),PROFILE);
 return TRUE;

}



BOOL TCreatePaymentWindow::CanClose()
{
  return TRUE;
}

void TCreatePaymentWindow::WMSize(RTMessage Msg)
{
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	 Could add could here to move the window to the size of the form
	 when ever it gets too small for the form.
	 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  int TAB_HEIGHT = 0;
  int HEADER_HEIGHT = 190;
  TWindow::WMSize(Msg);
  RECT rc;
  GetClientRect(HWindow,&rc);
  MoveWindow(Form->HWindow,0,0,LOWORD(Msg.LParam),HIWORD(Msg.LParam)-TAB_HEIGHT,FALSE);
  MoveWindow(GetDlgItem(Form->HWindow,ID_DISTLIST),rc.left,rc.top+HEADER_HEIGHT,LOWORD(Msg.LParam),
				 rc.bottom-(rc.top+HEADER_HEIGHT+TAB_HEIGHT),TRUE);


 short d1Width,d2Width,d3Width,d4Width,d5Width,d6Width,d7Width,d8Width,d9Width,d10Width;


 Form->InvoiceList->SetColWidth(2,3);
 Form->InvoiceList->SetColWidth(3,6);
 Form->InvoiceList->SetColWidth(4,9);
 Form->InvoiceList->SetColWidth(5,9);
 Form->InvoiceList->SetColWidth(6,10);
 Form->InvoiceList->SetColWidth(7,9);
 Form->InvoiceList->SetColWidth(8,10);
 Form->InvoiceList->SetColWidth(9,10);
 Form->InvoiceList->SetColWidth(10,10);

 Form->InvoiceList->GetColWidthInPixels(2,(LPSHORT)&d2Width);
 Form->InvoiceList->GetColWidthInPixels(3,(LPSHORT)&d3Width);
 Form->InvoiceList->GetColWidthInPixels(4,(LPSHORT)&d4Width);
 Form->InvoiceList->GetColWidthInPixels(5,(LPSHORT)&d5Width);
 Form->InvoiceList->GetColWidthInPixels(6,(LPSHORT)&d6Width);
 Form->InvoiceList->GetColWidthInPixels(7,(LPSHORT)&d7Width);
 Form->InvoiceList->GetColWidthInPixels(8,(LPSHORT)&d8Width);
 Form->InvoiceList->GetColWidthInPixels(9,(LPSHORT)&d9Width);
 Form->InvoiceList->GetColWidthInPixels(10,(LPSHORT)&d10Width);

 Form->InvoiceList->GetClientRect(&rc);

 short width = rc.right - rc.left;

 d1Width = width-  ( d2Width
						  + d3Width
						  + d4Width
						  + d5Width
						  + d6Width
						  + d7Width
						  + d8Width
						  + d9Width
						  + d10Width);


 Form->InvoiceList->SetColWidthInPixels(1,d1Width);

}

void TCreatePaymentWindow::SetupWindow()
{
  //Tab  = new TTabControl (this,"TAB");
  //Tab->EnableAutoCreate();
  //GetApplication()->MakeWindow(Tab);
  //Tab->Show(SW_SHOW);
 strcpy(szlpAWindow,"AP_CHECK");
 iLeft = iTop = iRight = iBottom = 0;
 iLeft    = GetPrivateProfileInt(szlpAWindow,"L",0,PROFILE);
 iTop     = GetPrivateProfileInt(szlpAWindow,"T",0,PROFILE);
 iRight   = GetPrivateProfileInt(szlpAWindow,"R",0,PROFILE);
 iBottom  = GetPrivateProfileInt(szlpAWindow,"B",0,PROFILE);

 Form = new TAPCreatePayment(this,1162);
 Form->EnableAutoCreate();
 GetApplication()->MakeWindow(Form);
 RECT rcForm;
 GetWindowRect(Form->HWindow,&rcForm);
 int width = rcForm.right - rcForm.left;
 int height = rcForm.bottom - rcForm.top;

 RECT rcDesktop;
 GetWindowRect(GetDesktopWindow(),&rcDesktop);
 if((iBottom - iTop)  > (rcDesktop.bottom - rcDesktop.top) ||  (iRight -iLeft) > (rcDesktop.right - rcDesktop.left))
	 {
	  iTop = 0;
	  iBottom = 0;
	  iLeft = 0;
	  iRight = width;
	 }


 if(iTop == 0 || iBottom == 0 || iLeft == 0 || iRight == 0)
 {
	iBottom = height;
 }

  Form->Show(SW_SHOW);
  SetCaption(GetString(hInst,3094));
  SetWindowPosition();
}

void TManilaGL::APCreateGLSlip(RTMessage)
{
	//GetApplication()->ExecDialog(new TAPCreatePayment(this,1162));
	TCreatePaymentWindow *CreatePaymentWindow = new TCreatePaymentWindow(this,"");
	GetApplication()->MakeWindow(CreatePaymentWindow);
}

void TAPCreatePayment::Print(RTMessage)
{
  if(MessageBox(HWindow,GetString(4067),((TAcctModule *)GetApplication())->GetAppName(),MB_YESNO|MB_ICONQUESTION) == IDYES)
	 PrintPaymentList();
}


void TAPCreatePayment::SetDate (RTMessage)
{
 //"PAYMENTCTL = 'H' .OR. PAYMENTCTL = 'F' .OR. PAYMENTCTL = 'O'"
 string szQuery(GetString(4011));
 apdist.SetIndexInvoiceno();
 if(strlen(strTrim(GetField(ID_VENDORNO))) > 0)
 {
		//szQuery += " .AND. (VENDORNO = '";
		szQuery += GetString(1090);
		szQuery += strTrim(GetField(ID_VENDORNO));
		szQuery += "')";
 }


 // Invoice No.
 if(strlen(strTrim(GetField(ID_INVOICENO))) > 0)
 {
		// szQuery += " .AND. (INVOICENO = '";
		szQuery += GetString(1091);
		szQuery += strTrim(GetField(ID_INVOICENO));
		szQuery += "')";
 }

 bHasInvoices = DistList((char *)szQuery.c_str());
 if(!bHasInvoices)
  MessageBox(HWindow,GetString(4028),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
}

void TAPCreatePayment::ViewAll(RTMessage)
{
 apdist.SetIndexVendorno();
 static string strQuery;
 static string OneCheck;
 static string TwoCheck;

 strQuery = GetString(6145);
 /*
 Str4large dS;
 dS.assignDouble(dGetField(1068),18);
 Str4large dE;
 dE.assignDouble(dGetField(1069),18);

 string dStarting(dS.str());
 string dEnding(dE.str());
 */
 string dStarting(GetField(1068));
 string dEnding(GetField(1069));

 // Replace all the text into the resource file.

 if(strlen(strTrim((LPSTR)dStarting.c_str())) > 0)
 {
	apdist.SetIndexChequeno();
	OneCheck =  GetString(1094);
	//OneCheck =  GetString(); // "CHEQUENO = ";
	OneCheck += dStarting;
	strQuery = OneCheck;

	if(strlen(strTrim((LPSTR)dEnding.c_str())) > 0)
	  {
		 TwoCheck =  GetString(1095);
		 TwoCheck += dStarting;
		 TwoCheck += GetString(1096);
		 TwoCheck += dEnding;

	  }
 }



 // Vendor No.
 if(strlen(strTrim(GetField(ID_VENDORNO))) > 0)
 {
		strQuery += GetString(1098);
		strQuery += strTrim(GetField(ID_VENDORNO));
		strQuery += "')";
 }


 // Invoice No.
 if(strlen(strTrim(GetField(ID_INVOICENO))) > 0)
 {
		strQuery += GetString(1099);
		strQuery += strTrim(GetField(ID_INVOICENO));
		strQuery += "')";
 }

 if(strlen(strTrim((LPSTR)TwoCheck.c_str())) > 0)
  {
	DistList((LPSTR)TwoCheck.c_str());
	return;
  }

 DistList((LPSTR)strQuery.c_str());
}

void TAPCreatePayment::SetupWindow()
{

 TMainDialog::SetupWindow();

 bSetVendors = TRUE;
 CHourGlass wait;
 InvoiceList->SetMaxCols(10);
 lPeriod = ((TGlWindow *)Parent)->iGetPeriod();
 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
 InvoiceList->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 InvoiceList->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
 InvoiceList->SetBool(SSB_PROTECT,TRUE);
 InvoiceList->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
 InvoiceList->SetBool(SSB_NOBEEP,TRUE);
 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 InvoiceList->SetBool(SSB_VERTSCROLLBAR,TRUE);

 InvoiceList->SetMaxRows(2000);

 InvoiceList->SetData(1,SS_HEADER, GetString(ID_VENDORNAME));
 InvoiceList->SetData(2,SS_HEADER, GetString(ID_CONTROL));
 InvoiceList->SetData(3,SS_HEADER, GetString(ID_VENDORCODE));
 InvoiceList->SetData(4,SS_HEADER, GetString(ID_INVOICENO));
 InvoiceList->SetData(5,SS_HEADER, GetString(ID_DUEDATE));
 InvoiceList->SetData(6,SS_HEADER, GetString(ID_AMOUNT));
 InvoiceList->SetData(7,SS_HEADER, GetString(ID_VAT));
 InvoiceList->SetData(8,SS_HEADER, GetString(ID_REBATE));
 InvoiceList->SetData(9,SS_HEADER, GetString(ID_AMOUNTDUE));
 InvoiceList->SetData(10,SS_HEADER, "Cheque No.");

 InvoiceList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
 InvoiceList->SetTypeEdit(&CellType,ES_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);
 InvoiceList->SetCellType(1, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeEdit(&CellType,ES_LEFT,10,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 InvoiceList->SetCellType(2, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(3, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(4, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeEdit(&CellType,ES_RIGHT,10,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 InvoiceList->SetCellType(5, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(10, SS_ALLROWS, &CellType);
 InvoiceList->SetTypeFloat(&CellType,FS_SEPARATOR,9,2,0,atof(GetString(6107)));
 InvoiceList->SetCellType(6, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(7, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(8, SS_ALLROWS, &CellType);
 InvoiceList->SetCellType(9, SS_ALLROWS, &CellType);

 // Open databases...
  if(apjsum.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(apjsum.GetDbfName());
	return;
  }


  if(apdist.open(((TAcctModule *)GetApplication())->cb)< 0)
  {
	ErrorClose(apdist.GetDbfName());
	return;
  }

  if(apvendor.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(apvendor.GetDbfName());
	return;
  }
 apvendor.init();

 if(apbtch.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(apbtch.GetDbfName());
	return;
  }



 if(apbtchd.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(apbtchd.GetDbfName());
	return;
  }
  apbtchd.SetIndex("batchno");


 if(gl_cash.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(gl_cash.GetDbfName());
	return;
  }
 gl_cash.SetIndex("no");


 if(glbtch.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(glbtch.GetDbfName());
	return;
  }

 if(glbtchd.open(((TAcctModule *)GetApplication())->cb) < 0)
  {
	ErrorClose(glbtchd.GetDbfName());
	return;
  }

  if(glaccts.open(((TAcctModule *)GetApplication())->cb) < 0 )
  {
	ErrorClose(glaccts.GetDbfName());
	return;
  }
  glaccts.SetIndex("accounts");

  bHasInvoices = FALSE;

 SetCashAccounts();
 SetChequeNo();
 //SetVendorNames();
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
 //apvendor.SetIndex("vendorno");
 apvendor.SetIndexVendor();
 Relate4set rQuery(apvendor);
 string szQuery(GetString(4069));
 rQuery.querySet((LPSTR)szQuery.c_str());
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip())
	SendDlgItemMsg(ID_VENDORNO,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)apvendor.vendorno.str());
 rQuery.unlock();
 rQuery.free();
}

void TAPCreatePayment::GetVendorCode(RTMessage Msg)
{
 if(bSetVendors)
  {
	SetVendorNames();
	bSetVendors = FALSE;
  }
 int Indx;
 string szVendorCode;
 if(Msg.LP.Hi == CBN_SELCHANGE)
 {
  Indx = (int) SendDlgItemMsg(ID_VENDORNO,CB_GETCURSEL,0,0);
  if(Indx >= 0)
	{
	 SendDlgItemMsg(ID_VENDORNO,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM) ((LPSTR) szVendorCode.c_str()));
	 if(apvendor.seek((char *)szVendorCode.c_str()) == 0)
		SetField(ID_VENDORNAME,apvendor.vdname.str());
	 else
	  {
		SetField(ID_VENDORNAME,GetString(4076));
		string strNotDeleted(GetApplication()->hInstance,6145);
		apvendor.SetIndexVendor();
		DistList((LPSTR)strNotDeleted.c_str());
		return;
	  }
	 SetFocus(GetDlgItem(HWindow,ID_VENDORNO));
	 string szVendorQuery(GetString(4066));
	 szVendorQuery += szVendorCode.c_str();
	 szVendorQuery += "'";
	 apvendor.SetIndexVendor();
	 DistList((char *) szVendorQuery.c_str());

	 // The string value 4066 is as follows:
	 // "(PAYMENTCTL = 'H' .OR. PAYMENTCTL = 'O' .OR. PAYMENTCTL = 'F') .AND. VENDORNO = '"
	 //
	}
 }
}




void TAPCreatePayment::Cancel  (RTMessage)
{
 SetField(ID_VENDORNO,"");
 SetField(ID_INVOICENO,"");
 SetFocus(GetDlgItem(HWindow,ID_ACCOUNTS));
 apdist.close();
 apvendor.close();
 apbtch.close();
 apjsum.close();
 glbtch.close();
 glbtchd.close();
 glaccts.close();
 apbtchd.close();
 gl_cash.close();
 ((TCreatePaymentWindow *) Parent)->SaveWindowPosition();
 ShutDownWindow();
 Parent->ShutDownWindow();
}
BOOL TAPCreatePayment::DistList(char *szQuery)
{
 CHourGlass wait;
 long l = 0;
 SetField(IDD_STATUSBOX,GetString(4026));
 Date4 dPayment;
 dPayment.assign(GetField(ID_DUEDATE),GetString(4024));
 dPayment.format(GetString(4023));
 InvoiceList->SetBool(SSB_REDRAW,FALSE);
 InvoiceList->SetMaxRows(apdist.recCount());
 InvoiceList->ClearData(SS_ALLCOLS,SS_ALLROWS);
 InvoiceList->SetColor(SS_ALLCOLS,SS_ALLROWS,RGBCOLOR_WHITE,RGBCOLOR_BLACK);
 Date4 dInvoiceDueDate;
 double dPageTotal = 0;
 InvoiceList->SetMaxRows(apdist.recCount());
 Relate4set rQuery(apdist);
 rQuery.querySet(szQuery);

 char *controltype;
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip())
  {

	  Date4 dDueDate(apdist.duedate.str());
	  if( dDueDate <= dPayment )
		 {
		 l++;
		 apvendor.seek(apdist.vendorno.str());
		 InvoiceList->SetData(1,l,apvendor.vdshort.str());  // Vendor Name
		 controltype = apdist.paymentctl.str();
		 switch(*controltype)
		  {
			case 'O' : InvoiceList->SetColor(SS_ALLCOLS,l,RGB(192,220,192), RGBCOLOR_BLUE);break;
			case 'H' : InvoiceList->SetColor(SS_ALLCOLS,l,RGBCOLOR_WHITE, RGBCOLOR_BLACK);break;
			case 'F' : InvoiceList->SetColor(SS_ALLCOLS,l,RGBCOLOR_WHITE, RGBCOLOR_BLUE);break;
			case 'P' : InvoiceList->SetColor(SS_ALLCOLS,l,RGB(0,255,200), RGBCOLOR_BLUE);break;
			case 'C' : InvoiceList->SetColor(SS_ALLCOLS,l,RGB(0,255,200), RGBCOLOR_BLUE);break;
			case 'V' : InvoiceList->SetColor(SS_ALLCOLS,l,RGB(0,255,200), RGBCOLOR_BLUE);break;
		  }
		 InvoiceList->SetData(2,l,apdist.paymentctl.str()); // Payment Control
		 InvoiceList->SetData(3,l,apdist.vendorno.str());  // Vendor Code
		 InvoiceList->SetData(4,l,strTrim(apdist.invoiceno.str())); // Invoice Number
		 InvoiceList->SetData(5,l,GetDate(apdist.duedate.str()));   // Due Date
		 double dTotal = 0;
		 if(dTotal > 0)
		  dTotal = apdist.dAmount() + fabs(apdist.dRebateamt());
		 else
		  dTotal = apdist.dAmount() - fabs(apdist.dRebateamt());
		 Str4large szTotal;
		 szTotal.assignDouble(dTotal,16,2);
		 InvoiceList->SetData(6,l,szTotal.str());
		 InvoiceList->SetData(7,l,apdist.vatamt.str()); // Adjustment
		 InvoiceList->SetData(8,l,apdist.rebateamt.str()); // Adjustment
		 double dAmountDue = apdist.dAmount() - apdist.dAmountpaid();
		 Str4large szAmount;
		 szAmount.assignDouble(dAmountDue,16,2);
		 InvoiceList->SetData(9,l,szAmount.str()); // Adjustment

		 InvoiceList->SetData(10,l,strTrim(apdist.chequeno.str()));
		 dPageTotal += dAmountDue;
		 }
  }
  rQuery.unlock();
  rQuery.free();
 long pagerow = l + 2;

  if(pagerow >30)
  InvoiceList->SetMaxRows(pagerow);
 else
  InvoiceList->SetMaxRows(31);


 Str4large szPageTotal;
 szPageTotal.assignDouble(dPageTotal,16,2);
 string strPage(hInst,6144);
 InvoiceList->SetData(1,pagerow,(LPSTR)strPage.c_str());
 InvoiceList->SetData(9,pagerow,szPageTotal.str());


 InvoiceList->SetBool(SSB_REDRAW,TRUE);
 SetChequeNo();
 string strSearchCompleted(GetApplication()->hInstance,6141);
 SetField(IDD_STATUSBOX,(LPSTR)strSearchCompleted.c_str());

 if(l>0)
 return TRUE;
 else
 return TRUE;
}


void TAPCreatePayment::SetChequeNo()
 {
  //apdist.SetIndex("lastcheqno");
  apdist.SetIndexVerifiedCleared();
  apdist.bottom();
  double iNewRequestNumber =1;
  iNewRequestNumber = double (Str4ptr(apdist.chequeno.str())) + 1;
  FloatSetDlgItemValue(HWindow,ID_APCONTROL,iNewRequestNumber);
  //apdist.SetIndex("invoiceno");
  apdist.SetIndexInvoiceno();
 }


void TAPCreatePayment::PrintPaymentList(void)
{
  ((TGlWindow *)Parent)->PrintReport("ap_reqst.rpt",GetApplication()->MainWindow->HWindow);
  return;
}
