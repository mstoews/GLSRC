/*
	Accounts Payable  - (C) Copyright 1992 by Tegara Corporation
	Posting
	ap_post.cpp
*/
#include "std_inc.h"

/*
#include "tbowl.h"
#include "d4all.hpp"
#include <math.h>
#include "gl.h"
#include "ar.h"
#include "ar_ctrl.h"
#include "gl_dlg.h"
#include "ar_postb.h"
#include "db_dist.h"
#include "gl_postb.h"
*/




void TManilaGL::ARPostBatch   (RTMessage)
 {
  //GetApplication()->ExecDialog(new TARPostBatch(this,1061));
 }



void TPostBatch::AR_AppendPriorPeriod(long lBatchno)
{
  Tgldist gldist(cb);
  if(arbtch.seek(lBatchno) == 0)
	{
		  if(arbtchd.seek(arbtch.batchno.str())==0)
		  {
			while ((long)arbtchd.lBatchno() == (long)arbtch.lBatchno())
			 {
			  SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)arbtchd.batchno.str());
			  if(AR_UpdateGL(&arbtchd.account,&arbtchd.dept,arbtch.iPeriod()))
					{
						arbtch.mode.assign("4");
						arbtch.UpdateUserTime(szUser);
						gldist.appendStart();
						gldist.account.assign(arbtchd.account.str());
						gldist.type.assign("PR");
						gldist.dept.assign(arbtchd.dept.str());
						gldist.period.assign(arbtch.period.str());
						gldist.date.assign(arbtch.date.str());
						gldist.batchno.assignDouble(double(Str4ptr(arbtchd.batchno.str())));
						gldist.reference.assign(arbtch.reference.str());
						gldist.medrep.assign(arbtchd.depart.str());
						gldist.prod.assign(arbtchd.product.str());
						gldist.drcode.assign(arbtchd.drcode.str());
						gldist.descript.assign(arbtchd.descript.str());
						gldist.debit.assignDouble(double(Str4ptr(arbtchd.debit.str())));
						gldist.credit.assignDouble(double(Str4ptr(arbtchd.credit.str())));
						gldist.UpdateUserTime(szUser);
						gldist.append();
				 }
				 arbtchd.skip(1);
			  }
			}
	}
}

void TPostBatch::AR_UpdateRetainedEarnings(double dAdjustment)
{
  for(glaccts.top();!glaccts.eof();glaccts.skip())
	{
	  if(strcmp(glaccts.type.str(),"R") == 0)
		{
		  double dCurrentBalance = glaccts.dCurrbal();
		  dCurrentBalance += dAdjustment;
		  glaccts.curr_bal.assignDouble(dCurrentBalance);
		  glaccts.openbal.assignDouble(dCurrentBalance);
		  return;
		}
	}
}

#pragma argsused
BOOL TPostBatch::AR_UpdateGL(Field4 *account, Field4 *dept,int iPeriod)
{
		BOOL bValid = FALSE;
		//glaccts.SetIndexAccts();
		 if(glaccts.SetIndexAccts(1) < 0)
		 {
			ShutDownWindow();
			return bValid;
		 }

		string szSeek(account->str());
		szSeek += dept->str();
		if(glaccts.seek((LPSTR)szSeek.c_str()) == 0)
		{
				double credit = arbtchd.dCredit();
				double debit  = arbtchd.dDebit();
				double dNewOpenBal  = (glaccts.dCurrbal() - dCurrentYearTotal()) + debit - credit;
				double dCurrBal =  dNewOpenBal + dCurrentYearTotal();

			if(strcmp(glaccts.type.str(),"B") == 0)
			{
				glaccts.curr_bal.assignDouble(dCurrBal);
				double dOpenbal = glaccts.dOpenbal();
				dOpenbal += arbtchd.dDebit() - arbtchd.dCredit();
				glaccts.openbal.assignDouble(dOpenbal);
				bValid = TRUE;
			}
			else
			{
				double dPrevious12 = arbtchd.dDebit() - arbtchd.dCredit();
				AR_UpdateRetainedEarnings(dPrevious12);
				dPrevious12 += double(Str4ptr(glaccts.previous12.str()));
				glaccts.previous12.assignDouble(dPrevious12);
				bValid = TRUE;

			}
			glaccts.UpdateUserTime(szUser);
			arbtch.UpdateUserTime(szUser);
		}
		else
		  MessageBox(HWindow,GetString(3081),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
  return bValid;
}

BOOL TPostBatch::AR_CheckBalance(long lBatchno,double dPayable,char *szARAccount)
{
  double dDebit  = 0 ;
  double dCredit = 0 ;
  double dARBalance = 0 ;
  if(arbtchd.seek(lBatchno) == 0)
	{
	 while(arbtchd.lBatchno() == lBatchno)
	  {
		 dDebit  += arbtchd.dCredit();
		 dCredit += arbtchd.dDebit();
		 string szAccount(arbtchd.account.str());
		 szAccount += arbtchd.dept.str();
		 if (szAccount == szARAccount)
		  dARBalance += arbtchd.dDebit() - arbtchd.dCredit();
		 arbtchd.skip();
	  }
	}

  if(dARBalance != dPayable)
  {
	Str4ten szBatchno;
	szBatchno.assignLong(lBatchno,10);
	// The voucher number %s is out of balance and can not be posted.
	wsprintf(szMsg,GetString(1008),StrTrim(szBatchno.str()));
	MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	return FALSE;
  }

  if((round(dDebit,2) == round(dCredit,2)) && (round(dDebit,2) > 0 ))
  {
	return TRUE;
  }
  else
  {
	Str4ten szBatchno;
	szBatchno.assignLong(lBatchno,10);
	// The voucher number %s is out of balance and can not be posted.
	wsprintf(szMsg,GetString(1007),StrTrim(szBatchno.str()));
	MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	return FALSE;
  }
}

/* formally the select cell function
	each function has to be set individually based upon which posting is currently
	operating

void TPostBatch::AR_PostBatch( RTMessage Msg)
{
 dAPControl = 0;
 string strMsg;
 char szBuffer[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);
 wsprintf(szMsg,GetString(8129),szBuffer);
 if(MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDNO)
  return;
 CHourGlass wait;
 if(AR_CheckBalance(lRecord))
 {
	if(arbtch.seek(lRecord)==0)
	{
	  if(strcmp(arbtch.mode.str(),"3") == 0)
			{
				strMsg = GetString(3093);
				MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
				return;
			}

	 char *szInvoice = strcpy(new char[arbtch.vendorno.len()+arbtch.invoiceno.len()+1],arbtch.vendorno.str());
	 strcat(szInvoice,arbtch.invoiceno.str());
	 AR_AppendDist(szInvoice);
	 delete szInvoice;
	 AR_AppendSummary(lRecord);

	}
	SetField(ID_BATCHNO,"");
	SSClearData(SHWindow,SS_ALLCOLS,SS_ALLROWS);
	long i=1;
	arbtch.SetIndex("post_ar");
	arbtch.top();
	while(!arbtch.eof())
	{
		SSSetData(SHWindow,1,i,arbtch.batchno.str());
		SSSetData(SHWindow,2,i,arbtch.reference.str());
		SSSetData(SHWindow,3,i,arbtch.invoiceno.str());
		SSSetData(SHWindow,4,i,arbtch.payable.str());
		i++;
	  arbtch.skip(1);
	}
 }
 return;
}
*/

void TPostBatch::AR_PostAll()
{
 if(MessageBox(HWindow,GetString(6048),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDNO)
  return;
 CHourGlass wait;
 ardist.lockAll();
 arbtch.lockAll();
 arbtchd.lockAll();
 arbtch.SetIndex("batchno");
 Relate4set rQuery(arbtch);
 rQuery.querySet(GetQuery());
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
 {
	 char szInvoice[15];
	 long lBatchno = arbtch.lBatchno();
	 strcpy(szInvoice,arbtch.vendorno.str());
	 strcat(szInvoice,arbtch.invoiceno.str());
	 string szARAccount(arbtch.account.str());
	 szARAccount += arbtch.depart.str();
	 if(AR_CheckBalance(lBatchno,arbtch.dPayable(),(LPSTR)szARAccount.c_str()))
	  {
		AR_AppendDist(szInvoice);
		if(strcmp(arbtch.mode.str(),"3") != 0)
			{
				AR_AppendSummary(lBatchno);
				arbtch.mode.assign("0");
				arbtch.UpdateUserTime(szUser);
			}
		else
		 AR_AppendPriorPeriod(lBatchno);
	  }
 }
 rQuery.unlock();
 rQuery.free();
 ARRedraw();
 return;
}

/*
	 if(CheckAPBalance(lBatchno))
	  {
		if(strcmp(apbtch->mode.str(),"3") != 0)
			{
			 AppendAPDist(szInvoice);
			 AppendAPSummary(lBatchno);
			 apbtch->mode.assign("0");
			 apbtch->UpdateUserTime(szUser);
			}
		else
		AppendPriorPeriod(lBatchno);
	  }
*/

#pragma argsused
void TPostBatch::AR_AppendDist(char *szInvoice)
{
	 SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)arbtchd.batchno.str());
	 ardist.appendStart();
	 ardist.account.assign("");
	 ardist.dept.assign("");
	 ardist.apaccount.assign(arbtch.account.str());
	 ardist.apdepart.assign(arbtch.depart.str());
	 ardist.period.assignLong(long(Str4ptr(arbtch.period.str())));
	 ardist.date.assign(arbtch.invdate.str());
	 ardist.duedate.assign(arbtch.duedate.str());
	 ardist.batchno.assignDouble(double(Str4ptr(arbtch.batchno.str())));
	 ardist.paymentctl.assign("O");
	 ardist.chequeno.assign("");
	 ardist.datepaid.assign("");
	 ardist.invoiceno.assign(arbtch.invoiceno.str());
	 ardist.vendorno.assign(arbtch.vendorno.str());
	 ardist.reference.assign(arbtch.reference.str());
	 ardist.descript.assign(arbtch.descript.str());
	 ardist.amountpaid.assignDouble(0);
	 ardist.paymentreq.assignDouble(0);
	 ardist.amount.assignDouble(double(Str4ptr(arbtch.payable.str())));
	 ardist.remainder.assignDouble(double(Str4ptr(arbtch.payable.str())));
	 //ardist.rebateamt->assignDouble(double(Str4ptr(arbtch.rebateamt->str())));
	 ardist.vatamt.assignDouble(arbtch.dVatamt());
	 ardist.UpdateUserTime(szUser);
	 ardist.append();
}

// arjsum is really the summary database file to hold the entries that will be transferred to the GL.

void TPostBatch::AR_AppendSummary(long lBatchno)
{
  if(arbtchd.seek(lBatchno) == 0)
	{
	  while (arbtchd.lBatchno() == lBatchno)
		  {
		  SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)arbtchd.batchno.str());
		  arbtch.mode.assign("0");
		  arbtch.UpdateUserTime(szUser);
		  arjsum.appendStart();
		  arjsum.account.assign(arbtchd.account.str());
		  arjsum.dept.assign(arbtchd.dept.str());
		  arjsum.period.assign(arbtch.period.str());
		  arjsum.date.assign(arbtch.date.str());
		  arjsum.mode.assign("1");
		  arjsum.type.assign("AR");
		  arjsum.batchno.assignDouble(double(Str4ptr(arbtch.batchno.str())));
		  arjsum.reference.assign(arbtch.reference.str());
		  arjsum.descript.assign(arbtch.descript.str());
		  arjsum.debit.assignDouble(double(Str4ptr(arbtchd.debit.str())));
		  arjsum.credit.assignDouble(double(Str4ptr(arbtchd.credit.str())));
		  arjsum.depart.assign(arbtchd.depart.str());
		  arjsum.append();
		  arbtchd.skip();
		  }
	}
}





void TPostBatch::ARRedraw()
{
 CHourGlass wait;
 long RecTotal=0;
 HWND hSS = GetDlgItem(HWindow,ID_DISTLIST);
 SSSetBool(hSS,SSB_REDRAW,FALSE);
 SSClearData(hSS,SS_ALLCOLS,SS_ALLROWS);
 RecTotal = arbtch.recCount();
 if(RecTotal < 15 )
 RecTotal = 15;
 SSSetMaxRows(hSS,RecTotal);
 Field4 update(arbtch,"update");
 Field4 user(arbtch,"user");
 long i=1;

 TMainDialog test(this,1016);
 GetApplication()->MakeWindow(&test);
 test.SetField(ID_TEXT,GetString(337));
 test.Show(SW_SHOW);
 string strMsg1,strMsg;
 strMsg1 = GetString(335);


 arbtch.SetIndex("batchno");
 Relate4set rQuery(arbtch);
 rQuery.querySet(GetQuery());
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
 {
		SSSetData(hSS,1,i,arbtch.batchno.str());
		SSSetData(hSS,2,i,arbtch.reference.str());
		SSSetData(hSS,3,i,GetDate(update.str()));
		SSSetData(hSS,4,i,user.str());
		SSSetData(hSS,5,i,arbtch.invoiceno.str());
		SSSetData(hSS,6,i,arbtch.payable.str());
		strMsg += glbtch.batchno.str();
		test.SetField(ID_NUMBER,(LPSTR)strMsg.c_str());
		SetFocus(test.HWindow);

	  i++;
  }
 rQuery.unlock();
 rQuery.free();
 if(i > 15)
  SSSetMaxRows(hSS,i);
 else
  SSSetMaxRows(hSS,15);
  SSSetBool(hSS,SSB_REDRAW,TRUE);
}

