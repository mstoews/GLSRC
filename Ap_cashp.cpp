#include "std_inc.h"

//#include "ap.h"
#include <math.h>
#include "db_dist.h"
#include "db_apbtc.h"
#include "ap_ctrl.h"
#include "gl_dlg.h"
#include "ap_postb.h"
#include "gl_postb.h"




void TPostBatch::CashRedraw()
{
 CHourGlass wait;
 HWND hSS = GetDlgItem(HWindow,ID_DISTLIST);
 SSSetBool(hSS,SSB_REDRAW,FALSE);
 SSClearData(hSS,SS_ALLCOLS,SS_ALLROWS);

 long RecTotal=0;

 RecTotal = apcash.recCount();
 if(RecTotal < 15 )
 RecTotal = 15;

 apcash.SetIndex("batchno");
 long i=1;
 Field4 update(apcash,"update");
 Field4 user(apcash,"user");
 Relate4set rQuery(apcash);
 rQuery.querySet(GetQuery());
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
		SSSetData(hSS,1,i,apcash.batchno.str());
		SSSetData(hSS,2,i,apcash.descript.str());
		SSSetData(hSS,3,i,GetDate(update.str()));
		SSSetData(hSS,4,i,user.str());
		SSSetData(hSS,5,i,apcash.invoiceno.str());
		SSSetData(hSS,6,i,apcash.payable.str());
	  i++;
  }
 rQuery.unlock();
 rQuery.free();
 SSSetBool(hSS,SSB_REDRAW,TRUE);
}


BOOL TPostBatch::CASH_CheckBalance(long lBatchno)
{
  apcashd.SetIndex(GetString(8140));
  double dDebit  = 0 ;
  double dCredit = 0 ;
  if(apcashd.seek(lBatchno) == 0)
	{
	 while(apcashd.lBatchno() == lBatchno)
     {
		 dCredit  += apcashd.dCredit();
		 dDebit   += apcashd.dDebit();
		 apcashd.skip();
	  }
	}
  if(round(dCredit,2) == round(dDebit,2))
  {
	return TRUE;
  }
  else
  {
	Str4ten szBatchno;
	szBatchno.assignLong(lBatchno,10);
	szBatchno.trim();
	// The voucher number %s is out of balance and can not be posted.
	wsprintf(szMsg,GetString(1007),StrTrim(szBatchno.str()));
	MessageBox(HWindow,szMsg,((TAcctModule *)GetApplication())->GetAppName(),MB_ICONEXCLAMATION);
	return FALSE;
  }
}


void TPostBatch::CASH_PostAll()
{
  if(MessageBox(HWindow,GetString(6048),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDNO)
	return;
  CHourGlass wait;
  apcash.SetIndex("batchno");
  Relate4set rQuery(apcash);
  rQuery.querySet(GetQuery());
  for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  CASH_CreateGLSlip();
  rQuery.unlock();
  rQuery.free();
  CashRedraw();
  return;
}

/*
void TPostBatch::HandleListBox( RTMessage Msg)
{
 dAPControl = 0;
 if(MessageBox(HWindow,GetString(6048),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDNO)
  return;
 CHourGlass wait;
 char szBuffer[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);
 if(CheckCashBalance(lRecord))
 {
	apcash.SetIndex(GetString(8140));
	if(apcash.seek(lRecord)==0)
	{
	 long lBatchno = apcash.lBatchno();
	 AppendAPSummary(lBatchno);
	}

	SetField(ID_BATCHNO,"");
	SSClearData(SHWindow,SS_ALLCOLS,SS_ALLROWS);
	long i=1;
	apcash.top();
	while(!apcash.eof())
	{
	  if(StrCmp(apcash.mode.str(),"1")==0)
	  {
		SSSetData(SHWindow,1,i,apcash.batchno.str());
		SSSetData(SHWindow,2,i,apcash.descript.str());
		SSSetData(SHWindow,3,i,apcash.invoiceno.str());
		SSSetData(SHWindow,4,i,apcash.payable.str());
		ListArray->add(* (new TListItem((int)i,(int)apcash.recNo())));
	  i++;

	  }
	  apcash.skip(1);

	 }
 }
 return;
}
*/

void TPostBatch::CASH_CreateGLSlip(void)
{

  apcash.top();
  while(!apcash.eof())
  {
	long lBatchno = apcash.lBatchno();
	if(CASH_CheckBalance(lBatchno))
	{
	 if(apcash.Open())
	  CASH_AppendSummary(lBatchno);
	}
	 apcash.skip(1);
  }
 return;
}

BOOL Tapcash::Open()
{
 if(int(Str4ptr(mode.str())) == 1)
 return TRUE;
 else
 return FALSE;
}


void TPostBatch::CASH_AppendSummary(long lBatchno)
{
     if(apcashd.seek(lBatchno) == 0)
      {
       while (apcashd.lBatchno() == lBatchno)
        {
		  SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)apcashd.batchno.str());
		  apcash.mode.assign("0");
		  apcash.UpdateUserTime(szUser);
	     apcash.flush();
	     apjsum.appendStart();
		  apjsum.account.assign(apcashd.account.str());
		  apjsum.dept.assign(apcashd.dept.str());
		  apjsum.mode.assign("1");
		  apjsum.period.assign(apcash.period.str());
		  apjsum.date.assign(apcash.date.str());
		  apjsum.type.assign("CA");
		  apjsum.batchno.assignDouble(apcashd.lBatchno());
		  apjsum.reference.assign(apcashd.descript.str());
		  apjsum.descript.assign(apcashd.descript.str());
		  apjsum.debit.assignDouble(double(Str4ptr(apcashd.debit.str())));
		  apjsum.credit.assignDouble(double(Str4ptr(apcashd.credit.str())));
	     apjsum.append();
		  apcashd.skip();
		}
	  }
}

