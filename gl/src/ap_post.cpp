/*
	Accounts Payable  - (C) Copyright 1992 by Tegara Corporation
	Posting
	ap_post.cpp
*/

#include "ap.h"
#include <math.h>
#include "db_dist.h"
#include "db_apbtc.h"
#include "ap_ctrl.h"
#include "gl_dlg.h"
#include "ap_postb.h"

extern Code4 cb;
extern char szApp[20];
extern HINSTANCE hInst;


void TManilaGL::APPostBatch   (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPPostBatch(this,1047));
 }

void TManilaGL::APPostCash   (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPCashPost(this,1047));
 }

void TManilaGL::APPostAdj   (RTMessage){}


void TAPPostBatch::HandleListBox( RTMessage Msg)
{
 dAPControl = 0;
 char szBuffer[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);
 apbtch->SetIndex("batchno");
 if(apbtch->seek(lRecord)==0)
 {
  wsprintf(szMsg,GetString(8129),StrTrim(apbtch->batchno->str()));
  if(MessageBox(HWindow,szMsg,szApp,MB_YESNO | MB_ICONQUESTION) == IDYES)
  {
	if(CheckAPBalance(lRecord))
	{
	 char *szInvoice = strcpy(new char[apbtch->vendorno->len()+apbtch->invoiceno->len()+1],apbtch->vendorno->str());
	 strcat(szInvoice,apbtch->invoiceno->str());
	 AppendAPDist(szInvoice);
	 AppendGLDist(apbtch);
	 delete [] szInvoice;
	}
  }
 }

	SetField(ID_BATCHNO,"");
	SSClearData(SHWindow,SS_ALLCOLS,SS_ALLROWS);
	long i=1;
	Relate4set *rQuery = new Relate4set(apbtch);
	rQuery->querySet("MODE = '1'");
	for(int qc = rQuery->top();qc!=r4eof;qc=rQuery->skip(1L) )
	{
		SSSetData(SHWindow,1,i,apbtch->batchno->str());
		SSSetData(SHWindow,2,i,apbtch->descript->str());
		SSSetData(SHWindow,3,i,apbtch->invoiceno->str());
		SSSetData(SHWindow,4,i,apbtch->payable->str());
	  i++;
	}
	rQuery->unlock();
	rQuery->free();
 return;

}



void TAPPostBatch::Ok             (RTMessage )
{
 if(MessageBox(HWindow,GetString(8130),szApp,MB_ICONQUESTION | MB_YESNO) == IDNO)
  return;
 apdist->lockAll();
 apbtch->lockAll();
 apbtchd->lockAll();
 apdist->SetIndex("invoiceno");
 Relate4set *rQuery = new Relate4set(apbtch);
 rQuery->querySet("MODE = '1'");
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
	 char szInvoice[15];
	 long lBatchno = apbtch->lBatchno();
	 strcpy(szInvoice,apbtch->vendorno->str());
	 strcat(szInvoice,apbtch->invoiceno->str());
	 if(CheckAPBalance(lBatchno))
	  {
		AppendAPDist(szInvoice);
		AppendAPSummary(lBatchno);
	  }
  }
 rQuery->unlock();
 rQuery->free();

 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

BOOL TAPPostBatch::CheckAPBalance(long lBatchno)
{
  apbtchd->SetIndex("batchno");
  double dDebit  = 0 ;
  double dCredit = 0 ;
  if(apbtchd->seek(lBatchno) == 0)
	{
	 while(apbtchd->lBatchno() == lBatchno)
	  {
		 dDebit  += apbtchd->dCredit();
		 dCredit += apbtchd->dDebit();
		 apbtchd->skip();
	  }
	}

  if(dCredit == dDebit)
  {
	return TRUE;
  }
  else
  {
	Str4ten szBatchno;
	szBatchno.assignLong(lBatchno,10);
	// The voucher number %s is out of balance and can not be posted.
	wsprintf(szMsg,GetString(1007),StrTrim(szBatchno.str()));
	MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);
	return FALSE;
  }
}

void TAPPostBatch::AppendAPDist(char *szInvoice)
{
	if(apdist->seek(szInvoice) != 0)
	{
	 SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)apbtchd->batchno->str());
	 apdist->appendStart();
	 apdist->apaccount->assign(apbtch->account->str());
	 apdist->apdepart->assign(apbtch->depart->str());
	 apdist->period->assignLong(long(Str4ptr(apbtch->period->str())));
	 apdist->date->assign(apbtch->invdate->str());
	 apdist->duedate->assign(apbtch->duedate->str());
	 apdist->batchno->assignDouble(double(Str4ptr(apbtch->batchno->str())));
	 apdist->paymentctl->assign("H");
	 apdist->chequeno->assign("");
	 apdist->datepaid->assign("");
	 apdist->invoiceno->assign(apbtch->invoiceno->str());
	 apdist->vendorno->assign(apbtch->vendorno->str());
	 apdist->reference->assign(apbtch->reference->str());
	 apdist->descript->assign(apbtch->descript->str());
	 apdist->amountpaid->assignDouble(0);
	 apdist->paymentreq->assignDouble(0);
	 double dAmountdue = 0;
	 if(apbtch->dRebateamt() < 0)
	  dAmountdue = apbtch->dPayable() + apbtch->dRebateamt();
	 else
	  dAmountdue = apbtch->dPayable();
	 apdist->amount->assignDouble(dAmountdue);
	 apdist->rebateamt->assignDouble(double(Str4ptr(apbtch->rebateamt->str())));
	 apdist->vatamt->assignDouble(apbtch->dVatamt());
	 apdist->UpdateUserTime(szUser);
	 apdist->append();
	}
}

// apjsum is really the summary database file to hold the entries that will be transferred to the GL.

void TAPPostBatch::AppendAPSummary(long lBatchno)
{
	  if(apbtchd->seek(lBatchno) == 0)
		{
       while (apbtchd->lBatchno() == lBatchno)
        {
		  SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)apbtchd->batchno->str());
		  apbtch->mode->assign("0");
		  apbtch->UpdateUserTime(szUser);
		  apbtch->flush();
		  apjsum->appendStart();
	     apjsum->account->assign(apbtchd->account->str());
	     apjsum->dept->assign(apbtchd->dept->str());
		  apjsum->period->assign(apbtch->period->str());
	     apjsum->date->assign(apbtch->date->str());
		  apjsum->mode->assign("1");
		  apjsum->type->assign("AP");
	     apjsum->batchno->assignDouble(double(Str4ptr(apbtch->batchno->str())));
	     apjsum->reference->assign(apbtch->reference->str());
		  apjsum->descript->assign(apbtch->descript->str());
		  apjsum->debit->assignDouble(double(Str4ptr(apbtchd->debit->str())));
		  apjsum->credit->assignDouble(double(Str4ptr(apbtchd->credit->str())));
		  apjsum->append();
		  apbtchd->skip();
		}
	 }
}




void TAPPostBatch::AppendGLDist(PTapbtch apbtch)
{
  char szSeek[12];
  if(strcmp(apbtch->mode->str(),"1")==0)
	 {
	  if(apbtchd->seek(apbtch->batchno->str())==0)
		{

		 while (double(Str4ptr(apbtchd->batchno->str())) == double(Str4ptr(apbtch->batchno->str())))
		  {
		 SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)apbtchd->batchno->str());

		 strcpy(szSeek,apbtchd->account->str());
		 strcat(szSeek,apbtchd->dept->str());
		  apbtch->mode->assign("0");
		  apbtch->UpdateUserTime(szUser);
		  apbtch->flush();
		  apjsum->appendStart();
		  apjsum->account->assign(apbtchd->account->str());
		  apjsum->dept->assign(apbtchd->dept->str());
		  apjsum->mode->assign("1");
		  apjsum->period->assign(apbtch->period->str());
		  apjsum->date->assign(apbtch->date->str());
				 apjsum->type->assign("AP");
		  apjsum->batchno->assignDouble(double(Str4ptr(apbtch->batchno->str())));
	     apjsum->reference->assign(apbtch->reference->str());
	     apjsum->descript->assign(apbtch->descript->str());
	     apjsum->debit->assignDouble(double(Str4ptr(apbtchd->debit->str())));
	     apjsum->credit->assignDouble(double(Str4ptr(apbtchd->credit->str())));
		  apjsum->append();

				apbtchd->skip(1);
	}
		}
	}

}

void TAPPostBatch::CreateGLSlip(void)
{
  dAPControl = 0;
  apbtch->top();
  while(!apbtch->eof())
  {
	AppendGLDist(apbtch);
	apbtch->skip(1);
  }
 return;
}

void TAPPostBatch::APAccountUpdate(void)
{
/*
  Date4 Today;
  Today.today();
  Today.format("MM/DD/YY");
  apctrl->top();
  while(!apctrl->eof())
  {
	  char szAccount[12];
	  strcpy(szAccount,apctrl->account->str());
	  strcat(szAccount,apctrl->depart->str());
	  if(glaccts->seek(szAccount) == 0 )
	  {
		double dCurrBal  = double(Str4ptr(glaccts->curr_bal->str())) + apctrl->dAmount();
		glaccts->curr_bal->assignDouble(dCurrBal);
		apjsum->appendStart();
		apjsum->account->assign(apctrl->account->str());
		apjsum->dept->assign(apctrl->depart->str());
		char szPeriod[3];
		itoa(GetCurrentPeriodn(),szPeriod,10);
		apjsum->period->assign(szPeriod);
		apjsum->date->assign(Today.str());
		apjsum->mode->assign("1");
		apjsum->batchno->assignDouble(double (Str4ptr(apbtch->batchno->str())));
		char szDescript[21];
		strcpy(szDescript,"AP Control - ");
		strcat(szDescript,Today.str());
		apjsum->reference->assign(GetField(ID_DESCRIPT));
		apjsum->descript->assign(szDescript);
		apjsum->type->assign("APC");
	  if(apctrl->dAmount() > 0){
		apjsum->debit->assignDouble(fabs(apctrl->dAmount()));
		apjsum->credit->assignDouble(0);
	  }
	  else if(apctrl->dAmount() < 0){
		apjsum->debit->assignDouble(0);
		apjsum->credit->assignDouble(fabs(apctrl->dAmount()));
	  }
	  apjsum->append();
	 }
  apctrl->skip();

 }
*/
  return;
}


void TAPPostBatch::Cancel (RTMessage)
{
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TAPPostBatch::SetupWindow()
{
 TMainDialog::SetupWindow();
 HourGlassOn();
 HWND SHWindow;
 RECT rc;
 SS_CELLTYPE 	CellType;
 LOGFONT 	lfFont;
 HFONT   	hFont;

 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);

 SSGetClientRect(SHWindow,&rc);
 short d0Width,d1Width,d3Width,d4Width;

 SSSetColWidth(SHWindow,1,6);
 SSSetColWidth(SHWindow,3,10);
 SSSetColWidth(SHWindow,4,10);

 SSGetColWidthInPixels(SHWindow,0,(LPSHORT)&d0Width);
 SSGetColWidthInPixels(SHWindow,1,(LPSHORT)&d1Width);
 SSGetColWidthInPixels(SHWindow,3,(LPSHORT)&d3Width);
 SSGetColWidthInPixels(SHWindow,4,(LPSHORT)&d4Width);


 short ColWidth = (rc.right - rc.left)
		  - d1Width
		  - d3Width
		  - d4Width;

 SSSetColWidthInPixels(SHWindow,2,ColWidth);


 //lfFont.lfHeight          = -13;
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

SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
SSSetBool(SHWindow,SSB_PROTECT,TRUE);
SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
SSSetBool(SHWindow,SSB_SHOWROWHEADERS, FALSE);

apctrl = new Tapctrl(&cb);
apctrl->zap();

apbtch = new Tapbtch(&cb);
apbtch->SetIndex(GetString(8140));

apbtchd = (Tapbtchd *) new Tapbtchd(&cb);
apbtchd->SetIndex(GetString(8140));

glaccts = (Tglaccts *) new Tglaccts(&cb);
glaccts->SetIndex(GetString(8145));

apdist  = (Tapdist *) new Tapdist(&cb);
apdist->SetIndex(GetString(8145));

apjsum  = (Tapjsum *) new Tapjsum(&cb);
apjsum->SetIndex(GetString(8145));

apjsum->pack();
apdist->pack();

 SetField(1001,"Press OK to Post.");

 SSSetMaxCols(SHWindow, 4);


 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 SSSetData(SHWindow,1,SS_HEADER, GetString(8141));
 SSSetData(SHWindow,2,SS_HEADER, GetString(8142));
 SSSetData(SHWindow,3,SS_HEADER, GetString(8143));
 SSSetData(SHWindow,4,SS_HEADER, GetString(8144));


 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0.00,99999999999.99);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);
 long i=1;
 Relate4set *rQuery = new Relate4set(apbtch);

 rQuery->querySet("MODE = '1'");
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
		SSSetData(SHWindow,1,i,apbtch->batchno->str());
		SSSetData(SHWindow,2,i,apbtch->descript->str());
		SSSetData(SHWindow,3,i,apbtch->invoiceno->str());
		SSSetData(SHWindow,4,i,apbtch->payable->str());
	  i++;
  }
 rQuery->unlock();
 rQuery->free();
 if(i>10)
 SSSetMaxRows(SHWindow,i-1);
 else
 SSSetMaxRows(SHWindow,10);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSReCalc(SHWindow);
 SSSetLock(SHWindow,5,SS_ALLROWS,TRUE);
 SSSetLock(SHWindow,6,SS_ALLROWS,TRUE);
 SSSetBool(SHWindow,SSB_REDRAW,TRUE);
 SSSetBool(SHWindow, SSB_ARROWSEXITEDITMODE, TRUE);
 SSSetEditEnterAction(SHWindow, SS_ENTERACTION_NEXT);
 HourGlassOff();
}

void Tglaccts::UpdateAccount(char *szSeek,int iPeriod, double dDebit, double dCredit)
{
    if(seek(szSeek) == 0)
      {
		 curr_bal->assignDouble(Curr_bal() + dDebit - dCredit);
       switch(iPeriod)
        {
	 case 1: period1->assignDouble(dPeriod1()+ dDebit - dCredit);   break;
	 case 2: period2->assignDouble(dPeriod2()+ dDebit - dCredit);   break;
	 case 3: period3->assignDouble(dPeriod3() + dDebit - dCredit); break;
	 case 4: period4->assignDouble(dPeriod4()+ dDebit - dCredit);   break;
	 case 5: period5->assignDouble(dPeriod5()+ dDebit - dCredit);   break;
	 case 6: period6->assignDouble(dPeriod6()+ dDebit - dCredit);   break;
	 case 7: period7->assignDouble(dPeriod7()+ dDebit - dCredit);   break;
	 case 8: period8->assignDouble(dPeriod8()+ dDebit - dCredit);   break;
	 case 9: period9->assignDouble(dPeriod9()+ dDebit - dCredit);   break;
	 case 10: period10->assignDouble(dPeriod10()+ dDebit - dCredit); break;
	 case 11: period11->assignDouble(dPeriod11()+ dDebit - dCredit); break;
	 case 12: period12->assignDouble(dPeriod12()+ dDebit - dCredit); break;
	 default: return;
		 }
	 UpdateUserTime(szUser);
	 flush();
    }
 return;
}

double Tglaccts::dPeriod(char *szAccount, char *szDept, int iPeriod)
{
  char *szSeek = new char [accounts->len() + dept->len() + 1];
  strcpy(szSeek,szAccount);
  strcat(szSeek,szDept);
  double dPeriod = 0;
  SetIndex(GetString(hInst,8145));
  if(seek(szSeek) == 0)
   {   
       switch(iPeriod)
		  {
	 case 1: dPeriod = dPeriod1();   break;
	 case 2: dPeriod = dPeriod2();   break;
	 case 3: dPeriod = dPeriod3();   break;
	 case 4: dPeriod = dPeriod4();   break;
	 case 5: dPeriod = dPeriod5();   break;
	 case 6: dPeriod = dPeriod6();   break;
	 case 7: dPeriod = dPeriod7();   break;
	 case 8: dPeriod = dPeriod8();   break;
	 case 9: dPeriod = dPeriod9();   break;
	 case 10: dPeriod = dPeriod10(); break;
	 case 11: dPeriod = dPeriod11(); break;
	 case 12: dPeriod = dPeriod12(); break;
	 default: return dPeriod;
       }
   }
  delete szSeek;
  return dPeriod;
}