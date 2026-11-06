/*
   Accounts Payable  - (C) Copyright 1992 by Tegara Corporation
   Posting 
   ap_post.cpp
*/
#include "tbowl.h"
#include "d4all.hpp"


#include "ar.h"
#include <math.h>
#include "db_dist.h"
#include "db_apbtc.h"
#include "ar_ctrl.h"
#include "gl_dlg.h"
#include "ar_postb.h"


#define   REINDEX "Use the maintainance functions \nto reindex the all databases."

extern Code4 cb;
extern char szApp[20];


void TGlWindow::APPostBatch   (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPPostBatch(this,"AR_POSTBATCH"));
 }


BOOL TAPPostBatch::CheckARBalance(long lBatchno)
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
   return TRUE;
  else
  {
   Str4ten szBatchno;
   szBatchno.assignLong(lBatchno,10);
   char szMsg[100];
   // The voucher number %s is out of balance and can not be posted. 
   wsprintf(szMsg,GetString(1003),StrTrim(szBatchno.str()));
   MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);
   return FALSE;
  }
}


void TAPPostBatch::HandleListBox( RTMessage Msg)
{
 dAPControl = 0;
 /*
 if(*GetField(ID_DESCRIPT) == 0)
  {
   MessageBox(HWindow,GetString(4118),szApp,MB_OK);
   SetFocus(GetDlgItem(HWindow,ID_DESCRIPT));
   return;
  }
 */
 if(MessageBox(HWindow,GetString(8006),szApp,MB_ICONQUESTION | MB_YESNO) == IDNO)
  return;

 char szBuffer[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);
 if(CheckARBalance(lRecord))
 {
   apbtch->SetIndex("batchno");
   if(apbtch->seek(lRecord)==0)
   {
    char szInvoice[15];
    strcpy(szInvoice,apbtch->vendorno->str());
    strcat(szInvoice,apbtch->invoiceno->str());
    AppendAPDist(szInvoice);
    AppendGLDist(apbtch);
   }
   SetField(ID_BATCHNO,"");
   SSClearData(SHWindow,SS_ALLCOLS,SS_ALLROWS);
   long i=1;
   apbtch->top();
   while(!apbtch->eof())
   {
     if(StrCmp(apbtch->mode->str(),"1")==0)
     {
      SSSetData(SHWindow,1,i,apbtch->batchno->str());
      SSSetData(SHWindow,2,i,apbtch->reference->str());
      SSSetData(SHWindow,3,i,apbtch->invoiceno->str());
      SSSetData(SHWindow,4,i,apbtch->payable->str());
      ListArray->add(* (new TListItem((int)i,(int)apbtch->recNo())));
     i++;

     }
     apbtch->skip(1);
   }
   if(dAPControl != 0)
   APAccountUpdate();
  }
  return;

}



void TAPPostBatch::Ok             (RTMessage )
{
  /*
  if(*GetField(ID_DESCRIPT) == 0)
  {
   MessageBox(HWindow,GetString(4118),szApp,MB_OK);
   SetFocus(GetDlgItem(HWindow,ID_DESCRIPT));
   return;
  }
  */
  if(MessageBox(HWindow,GetString(8006),szApp,MB_ICONQUESTION | MB_YESNO) == IDNO)
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
    if(CheckARBalance(lBatchno))
     {
      AppendAPDist(szInvoice);
      AppendAPSummary(lBatchno);
     }
  }
 rQuery->unlock();
 rQuery->free();

 cb.closeAll();
 ShutDownWindow();
}

void TAPPostBatch::AppendAPDist(char *szInvoice)
{
       if(apdist->seek(szInvoice) != 0)
	{
	 SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)apbtchd->batchno->str());
	 apdist->appendStart();
	 apdist->account->assign("");
	 apdist->dept->assign("");
	 apdist->apaccount->assign(apbtch->account->str());
	 apdist->apdepart->assign(apbtch->depart->str());
 	 apdist->period->assignLong(long(Str4ptr(apbtch->period->str())));
	 apdist->date->assign(apbtch->invdate->str());
	 apdist->duedate->assign(apbtch->duedate->str());
	 apdist->batchno->assignDouble(double(Str4ptr(apbtch->batchno->str())));
	 apdist->paymentctl->assign("O");
	 apdist->chequeno->assign("");
	 apdist->datepaid->assign("");
	 apdist->invoiceno->assign(apbtch->invoiceno->str());
         apdist->vendorno->assign(apbtch->vendorno->str());
     	 apdist->reference->assign(apbtch->reference->str());
	 apdist->descript->assign(apbtch->descript->str());
	 apdist->amountpaid->assignDouble(0);
	 apdist->paymentreq->assignDouble(0);
	 apdist->amount->assignDouble(double(Str4ptr(apbtch->payable->str())));
	 apdist->remainder->assignDouble(double(Str4ptr(apbtch->payable->str())));
	 apdist->rebateamt->assignDouble(double(Str4ptr(apbtch->rebateamt->str())));
	 apdist->vatamt->assignDouble(apbtch->dVatamt());
	 apdist->append();
	}
      else
       {
	 SetField(ID_BATCHNO,apbtchd->batchno);
	 apdist->duedate->assign(apbtch->duedate->str());
	 double dAdjustment = apdist->dAdjustment() + apbtch->dPayable(); 
	 apdist->adjustment->assignDouble(dAdjustment);
	 apdist->vatamt->assignDouble(apbtch->dVatamt());
	 apdist->flush();
       }


}

// gldist is really the summary database file to hold the entries that will be transferred to the GL.

void TAPPostBatch::AppendAPSummary(long lBatchno)
{
     if(apbtchd->seek(lBatchno) == 0)
      {
       while (apbtchd->lBatchno() == lBatchno)
        {
 	     SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)apbtchd->batchno->str());	    
	     apbtch->mode->assign("0");
	     apbtch->flush();
	     gldist->appendStart();
	     gldist->account->assign(apbtchd->account->str());
	     gldist->dept->assign(apbtchd->dept->str());
	     gldist->period->assign(apbtch->period->str());
	     gldist->date->assign(apbtch->date->str());
	     gldist->mode->assign("1");
             gldist->type->assign("AR");
	     gldist->batchno->assignDouble(double(Str4ptr(apbtch->batchno->str())));
	     gldist->reference->assign(apbtch->reference->str());
	     gldist->descript->assign(apbtch->descript->str());
	     gldist->debit->assignDouble(double(Str4ptr(apbtchd->debit->str())));
	     gldist->credit->assignDouble(double(Str4ptr(apbtchd->credit->str())));
	     gldist->append();
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
	     //glaccts->UpdateAccount(szSeek,apbtch->iPeriod(),apbtchd->dDebit(),apbtchd->dCredit());
	     // General Ledger is not updated until the end of the month.
	     // Adjustment entries then are not necessary to reverse the GL.

	     apbtch->mode->assign("0");
	     apbtch->flush();
	     gldist->appendStart();
	     gldist->account->assign(apbtchd->account->str());
	     gldist->dept->assign(apbtchd->dept->str());
	     gldist->period->assign(apbtch->period->str());
	     gldist->date->assign(apbtch->date->str());
	     gldist->mode->assign("1");
             gldist->type->assign("AR");
	     gldist->batchno->assignDouble(double(Str4ptr(apbtch->batchno->str())));
	     gldist->reference->assign(apbtch->reference->str());
	     gldist->descript->assign(apbtch->descript->str());
	     gldist->debit->assignDouble(double(Str4ptr(apbtchd->debit->str())));
	     gldist->credit->assignDouble(double(Str4ptr(apbtchd->credit->str())));
	     gldist->append();
	    // dAPControl = dAPControl + double(Str4ptr(apbtchd->debit->str())) - double(Str4ptr(apbtchd->credit->str()));
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
      gldist->appendStart();     
      gldist->account->assign(apctrl->account->str());
      gldist->dept->assign(apctrl->depart->str());
      char szPeriod[3];
      itoa(GetCurrentPeriodn(),szPeriod,10);
      gldist->period->assign(szPeriod);
      gldist->date->assign(Today.str());
      gldist->batchno->assignDouble(double (Str4ptr(apbtch->batchno->str())));
      char szDescript[21];
      strcpy(szDescript,"AP Control - ");
      strcat(szDescript,Today.str());
      gldist->reference->assign(GetField(ID_DESCRIPT));
      gldist->descript->assign(szDescript);
      gldist->type->assign("APC");
     if(apctrl->dAmount() > 0){
      gldist->debit->assignDouble(fabs(apctrl->dAmount()));
      gldist->credit->assignDouble(0);
     }
     else if(apctrl->dAmount() < 0){
      gldist->debit->assignDouble(0);
      gldist->credit->assignDouble(fabs(apctrl->dAmount()));
     }
     gldist->append();
    }
  apctrl->skip();

 }
  return;
}


void TAPPostBatch::Cancel (RTMessage)
{
 cb.closeAll();
 ShutDownWindow();
}

void TAPPostBatch::SetupWindow()
{
 TMainDialog::SetupWindow();
 HWND SHWindow;
 RECT rc;
 SS_CELLTYPE 	CellType;
 LOGFONT 	lfFont;
 HFONT   	hFont;

 HourGlassOn();
 cb.closeAll();
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

SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
SSSetBool(SHWindow,SSB_PROTECT,TRUE);
SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
SSSetBool(SHWindow,SSB_SHOWROWHEADERS, FALSE);


apctrl = new Tapctrl(&cb);
apctrl->zap();

apbtch = new Tapbtch(&cb);
apbtch->SetIndex("batchno");

apbtchd = (Tapbtchd *) new Tapbtchd(&cb);
apbtchd->SetIndex("batchno");

glaccts = (Tglaccts *) new Tglaccts(&cb);
glaccts->SetIndex("account");

apdist  = (Tapdist *) new Tapdist(&cb);
apdist->SetIndex("account");

gldist  = (Tgldist *) new Tgldist(&cb);
gldist->SetIndex("account");


long RecTotal=0;

 RecTotal = apbtch->recCount();
 if(RecTotal < 10 )
 RecTotal = 10;

 SSSetMaxCols(SHWindow, 4);
 SSSetMaxRows(SHWindow, RecTotal);
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);

 SSSetData(SHWindow,1,SS_HEADER, "Vt");
 SSSetData(SHWindow,2,SS_HEADER, "Description");
 SSSetData(SHWindow,3,SS_HEADER, "Invoice No.");
 SSSetData(SHWindow,4,SS_HEADER, "Payable Amount");


 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,14,2,0.00,99999999999.99);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);
 long i=1; 
 Relate4set *rQuery = new Relate4set(apbtch);
 rQuery->querySet("MODE = '1'");
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
      SSSetData(SHWindow,1,i,apbtch->batchno->str());
      SSSetData(SHWindow,2,i,apbtch->reference->str());
      SSSetData(SHWindow,3,i,apbtch->invoiceno->str());
      SSSetData(SHWindow,4,i,apbtch->payable->str());
     i++;
  }
 rQuery->unlock();
 rQuery->free();
 if(i > 9)
  SSSetMaxRows(SHWindow,i--);
 else
  SSSetMaxRows(SHWindow,9);
 SSSetBool(SHWindow,SSB_REDRAW,TRUE);
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
  SetIndex("account");
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
