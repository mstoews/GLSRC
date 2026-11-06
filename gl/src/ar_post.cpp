/*
   Accounts Payable  - (C) Copyright 1992 by Tegara Corporation
   Posting 
   ap_post.cpp
*/
#include "tbowl.h"
#include "d4all.hpp"


#include "ar.h"
#include <math.h>
#include "ar_ctrl.h"
#include "gl_dlg.h"
#include "ar_postb.h"

extern Code4 cb;
extern char szApp[20];


void TManilaGL::ARPostBatch   (RTMessage)
 {
  GetApplication()->ExecDialog(new TARPostBatch(this,1061));
 }


BOOL TARPostBatch::CheckARBalance(long lBatchno)
{
  arbtchd->SetIndex("batchno");
  double dDebit  = 0 ;
  double dCredit = 0 ;
  if(arbtchd->seek(lBatchno) == 0)
	{
	 while(arbtchd->lBatchno() == lBatchno)
	  {
		 dDebit  += arbtchd->dCredit();
		 dCredit += arbtchd->dDebit();
		 arbtchd->skip();
	  }
	}

  if(dCredit == dDebit)
	return TRUE;
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


void TARPostBatch::HandleListBox( RTMessage Msg)
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

 char szBuffer[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);
 wsprintf(szMsg,GetString(8129),szBuffer);
 if(MessageBox(HWindow,szMsg,szApp,MB_ICONQUESTION | MB_YESNO) == IDNO)
  return;

 if(CheckARBalance(lRecord))
 {
	arbtch->SetIndex(GetString(8140));
	if(arbtch->seek(lRecord)==0)
	{
	 char szInvoice[15];
	 strcpy(szInvoice,arbtch->vendorno->str());
	 strcat(szInvoice,arbtch->invoiceno->str());
	 AppendAPDist(szInvoice);
	 AppendGLDist(arbtch);
	}
	SetField(ID_BATCHNO,"");
	SSClearData(SHWindow,SS_ALLCOLS,SS_ALLROWS);
	long i=1;
	arbtch->top();
	while(!arbtch->eof())
	{
	  if(StrCmp(arbtch->mode->str(),"1")==0)
	  {
		SSSetData(SHWindow,1,i,arbtch->batchno->str());
		SSSetData(SHWindow,2,i,arbtch->reference->str());
		SSSetData(SHWindow,3,i,arbtch->invoiceno->str());
		SSSetData(SHWindow,4,i,arbtch->payable->str());
		i++;
	  }
	  arbtch->skip(1);
	}
	if(dAPControl != 0)
	APAccountUpdate();
  }
  return;

}



void TARPostBatch::Ok             (RTMessage )
{
  /*
  if(*GetField(ID_DESCRIPT) == 0)
  {
	MessageBox(HWindow,GetString(4118),szApp,MB_OK);
	SetFocus(GetDlgItem(HWindow,ID_DESCRIPT));
	return;
  }
  */
 if(MessageBox(HWindow,GetString(6048),szApp,MB_ICONQUESTION | MB_YESNO) == IDNO)
  return;
 ardist->lockAll();
 arbtch->lockAll();
 arbtchd->lockAll();
 ardist->SetIndex("invoiceno");
 Relate4set *rQuery = new Relate4set(arbtch);
 rQuery->querySet("MODE = '1'");
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
	 char szInvoice[15];
	 long lBatchno = arbtch->lBatchno();
	 strcpy(szInvoice,arbtch->vendorno->str());
	 strcat(szInvoice,arbtch->invoiceno->str());
	 if(CheckARBalance(lBatchno))
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

void TARPostBatch::AppendAPDist(char *szInvoice)
{
       if(ardist->seek(szInvoice) != 0)
	{
	 SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)arbtchd->batchno->str());
	 ardist->appendStart();
	 ardist->account->assign("");
	 ardist->dept->assign("");
	 ardist->apaccount->assign(arbtch->account->str());
	 ardist->apdepart->assign(arbtch->depart->str());
 	 ardist->period->assignLong(long(Str4ptr(arbtch->period->str())));
	 ardist->date->assign(arbtch->invdate->str());
	 ardist->duedate->assign(arbtch->duedate->str());
	 ardist->batchno->assignDouble(double(Str4ptr(arbtch->batchno->str())));
	 ardist->paymentctl->assign("O");
	 ardist->chequeno->assign("");
	 ardist->datepaid->assign("");
	 ardist->invoiceno->assign(arbtch->invoiceno->str());
         ardist->vendorno->assign(arbtch->vendorno->str());
		 ardist->reference->assign(arbtch->reference->str());
	 ardist->descript->assign(arbtch->descript->str());
	 ardist->amountpaid->assignDouble(0);
	 ardist->paymentreq->assignDouble(0);
	 ardist->amount->assignDouble(double(Str4ptr(arbtch->payable->str())));
	 ardist->remainder->assignDouble(double(Str4ptr(arbtch->payable->str())));
	 //ardist->rebateamt->assignDouble(double(Str4ptr(arbtch->rebateamt->str())));
	 ardist->vatamt->assignDouble(arbtch->dVatamt());
	 ardist->UpdateUserTime(szUser);
	 ardist->append();
	}
		else
		 {
	 SetField(ID_BATCHNO,arbtchd->batchno);
	 ardist->duedate->assign(arbtch->duedate->str());
	 double dAdjustment = ardist->dAdjustment() + arbtch->dPayable();
	 ardist->adjustment->assignDouble(dAdjustment);
	 ardist->vatamt->assignDouble(arbtch->dVatamt());
	 ardist->UpdateUserTime(szUser);
	 ardist->flush();
       }


}

// arjsum is really the summary database file to hold the entries that will be transferred to the GL.

void TARPostBatch::AppendAPSummary(long lBatchno)
{
     if(arbtchd->seek(lBatchno) == 0)
      {
       while (arbtchd->lBatchno() == lBatchno)
        {
		  SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)arbtchd->batchno->str());
		  arbtch->mode->assign("0");
        arbtch->UpdateUserTime(szUser);
		  arbtch->flush();
		  arjsum->appendStart();
		  arjsum->account->assign(arbtchd->account->str());
		  arjsum->dept->assign(arbtchd->dept->str());
	     arjsum->period->assign(arbtch->period->str());
	     arjsum->date->assign(arbtch->date->str());
		  arjsum->mode->assign("1");
				 arjsum->type->assign("AR");
		  arjsum->batchno->assignDouble(double(Str4ptr(arbtch->batchno->str())));
	     arjsum->reference->assign(arbtch->reference->str());
		  arjsum->descript->assign(arbtch->descript->str());
	     arjsum->debit->assignDouble(double(Str4ptr(arbtchd->debit->str())));
	     arjsum->credit->assignDouble(double(Str4ptr(arbtchd->credit->str())));
	     arjsum->append();
             arbtchd->skip();
	}
      }

}




void TARPostBatch::AppendGLDist(PTarbtch arbtch)
{
  char szSeek[12];
  if(strcmp(arbtch->mode->str(),"1")==0)
    {
	  if(arbtchd->seek(arbtch->batchno->str())==0)
      {

		 while (double(Str4ptr(arbtchd->batchno->str())) == double(Str4ptr(arbtch->batchno->str())))
        {
		 SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)arbtchd->batchno->str());
	    
	    strcpy(szSeek,arbtchd->account->str());
	    strcat(szSeek,arbtchd->dept->str());
	     //glaccts->UpdateAccount(szSeek,apbtch->iPeriod(),apbtchd->dDebit(),apbtchd->dCredit());
	     // General Ledger is not updated until the end of the month.
	     // Adjustment entries then are not necessary to reverse the GL.

		  arbtch->mode->assign("0");
		  arbtch->UpdateUserTime(szUser);
		  arbtch->flush();
		  arjsum->appendStart();
		  arjsum->account->assign(arbtchd->account->str());
	     arjsum->dept->assign(arbtchd->dept->str());
	     arjsum->period->assign(arbtch->period->str());
		  arjsum->date->assign(arbtch->date->str());
		  arjsum->mode->assign("1");
             arjsum->type->assign("AR");
		  arjsum->batchno->assignDouble(double(Str4ptr(arbtch->batchno->str())));
	     arjsum->reference->assign(arbtch->reference->str());
	     arjsum->descript->assign(arbtch->descript->str());
	     arjsum->debit->assignDouble(double(Str4ptr(arbtchd->debit->str())));
	     arjsum->credit->assignDouble(double(Str4ptr(arbtchd->credit->str())));
	     arjsum->append();
	    // dAPControl = dAPControl + double(Str4ptr(apbtchd->debit->str())) - double(Str4ptr(apbtchd->credit->str()));
            arbtchd->skip(1);
	}
      }
   }

}

void TARPostBatch::CreateGLSlip(void)
{
  dAPControl = 0;
  arbtch->top();
  while(!arbtch->eof())
  {
   AppendGLDist(arbtch);
   arbtch->skip(1);
  }
 return;
}

void TARPostBatch::APAccountUpdate(void)
{
  Date4 Today;
  Today.today();
  Today.format(GetString(4024));
  arctrl->top();
  while(!arctrl->eof())
  {
     char szAccount[12];
     strcpy(szAccount,arctrl->account->str());
	  strcat(szAccount,arctrl->depart->str());
	  if(glaccts->seek(szAccount) == 0 )
     {
      double dCurrBal  = double(Str4ptr(glaccts->curr_bal->str())) + arctrl->dAmount();
      glaccts->curr_bal->assignDouble(dCurrBal);
		arjsum->appendStart();
      arjsum->account->assign(arctrl->account->str());
      arjsum->dept->assign(arctrl->depart->str());
      char szPeriod[3];
      itoa(GetCurrentPeriodn(),szPeriod,10);
      arjsum->period->assign(szPeriod);
      arjsum->date->assign(Today.str());
		arjsum->batchno->assignDouble(double (Str4ptr(arbtch->batchno->str())));
      char szDescript[21];
      strcpy(szDescript,"AP Control - ");
      strcat(szDescript,Today.str());
      arjsum->reference->assign(GetField(ID_DESCRIPT));
      arjsum->descript->assign(szDescript);
      arjsum->type->assign("APC");
     if(arctrl->dAmount() > 0){
      arjsum->debit->assignDouble(fabs(arctrl->dAmount()));
      arjsum->credit->assignDouble(0);
     }
     else if(arctrl->dAmount() < 0){
      arjsum->debit->assignDouble(0);
      arjsum->credit->assignDouble(fabs(arctrl->dAmount()));
     }
	  arjsum->append();
    }
  arctrl->skip();

 }
  return;
}


void TARPostBatch::Cancel (RTMessage)
{
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}

void TARPostBatch::SetupWindow()
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


arctrl = new Tarctrl(&cb);
arctrl->zap();

arbtch = new Tarbtch(&cb);
arbtch->SetIndex(GetString(8140));

arbtchd = (Tarbtchd *) new Tarbtchd(&cb);
arbtchd->SetIndex(GetString(8140));

glaccts = (Tglaccts *) new Tglaccts(&cb);
glaccts->SetIndex(GetString(8145));

ardist  = (Tardist *) new Tardist(&cb);
ardist->SetIndex(GetString(8145));

arjsum  = (Tarjsum *) new Tarjsum(&cb);
arjsum->SetIndex(GetString(8145));


long RecTotal=0;

 RecTotal = arbtch->recCount();
 if(RecTotal < 10 )
 RecTotal = 10;

 SSSetMaxCols(SHWindow, 4);
 SSSetMaxRows(SHWindow, RecTotal);
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);
 
 SSSetData(SHWindow,1,SS_HEADER, GetString(4015)); // Vt.
 SSSetData(SHWindow,2,SS_HEADER, GetString(4019)); // Description
 SSSetData(SHWindow,3,SS_HEADER, GetString(4012)); // Invoice No.
 SSSetData(SHWindow,4,SS_HEADER, GetString(4020)); // Payable Amount


 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);

 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,999999999999.99);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);
 long i=1; 
 Relate4set *rQuery = new Relate4set(arbtch);
 rQuery->querySet("MODE = '1'");
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
		SSSetData(SHWindow,1,i,arbtch->batchno->str());
      SSSetData(SHWindow,2,i,arbtch->reference->str());
      SSSetData(SHWindow,3,i,arbtch->invoiceno->str());
		SSSetData(SHWindow,4,i,arbtch->payable->str());
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

