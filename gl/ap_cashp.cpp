#include "ap.h"
#include <math.h>
#include "db_dist.h"
#include "db_apbtc.h"
#include "ap_ctrl.h"
#include "gl_dlg.h"
#include "ap_postb.h"
extern char szApp[20];

void TAPCashPost::SetupWindow()
{
 TMainDialog::SetupWindow();
 SetCaption(GetString(6142));
 HWND SHWindow;
 RECT rc;
 SS_CELLTYPE 	CellType;
 LOGFONT 	lfFont;
 HFONT   	hFont;

 HourGlassOn();

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
SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);


apcash = new Tapcash(&cb);
apcash->SetIndex(GetString(8140));

apcashd = new Tapcashd(&cb);
apcashd->SetIndex(GetString(8140));

glaccts = new Tglaccts(&cb);
glaccts->SetIndex(GetString(8145));

apdist  = new Tapdist(&cb);
apdist->SetIndex(GetString(8145));

apjsum  = new Tapjsum(&cb);
apjsum->SetIndex(GetString(8145));

long RecTotal=0;

 RecTotal = apcash->recCount();
 if(RecTotal < 10 )
 RecTotal = 10;

 SSSetMaxCols(SHWindow, 4);
 SSSetMaxRows(SHWindow, RecTotal);
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);
 SSSetData(SHWindow,1,SS_HEADER, GetString(6043));
 SSSetData(SHWindow,2,SS_HEADER, GetString(6044));
 SSSetData(SHWindow,3,SS_HEADER, GetString(6045));
 SSSetData(SHWindow,4,SS_HEADER, GetString(6046));


 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,9999999999999.99);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);

 long i=1;
 Relate4set *rQuery = new Relate4set(apcash);
 rQuery->querySet("MODE = '1'");
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
		SSSetData(SHWindow,1,i,apcash->batchno->str());
      SSSetData(SHWindow,2,i,apcash->descript->str());
      SSSetData(SHWindow,3,i,apcash->invoiceno->str());
      SSSetData(SHWindow,4,i,apcash->payable->str());
     i++;
  }
 rQuery->unlock();
 rQuery->free();
 HourGlassOff();
}


BOOL TAPCashPost::CheckCashBalance(long lBatchno)
{
  apcashd->SetIndex(GetString(8140));
  double dDebit  = 0 ;
  double dCredit = 0 ;
  if(apcashd->seek(lBatchno) == 0)
   {
    while(apcashd->lBatchno() == lBatchno)
     {
       dDebit  += apcashd->dCredit();
		 dCredit += apcashd->dDebit();
       apcashd->skip();
	  }
	}

  if(dCredit == dDebit)
	return TRUE;
  else
  {
	Str4ten szBatchno;
	szBatchno.assignLong(lBatchno,10);
	szBatchno.trim();
	// The voucher number %s is out of balance and can not be posted.
	wsprintf(szMsg,GetString(1007),StrTrim(szBatchno.str()));
	MessageBox(HWindow,szMsg,szApp,MB_ICONEXCLAMATION);
	return FALSE;
  }
}


void TAPCashPost::Ok             (RTMessage )
{
  if(MessageBox(HWindow,GetString(6048),szApp,MB_ICONQUESTION | MB_YESNO) == IDNO)
	return;
  CreateGLSlip();
  cb.closeAll();
  cb.init();
  ShutDownWindow();
}


void TAPCashPost::HandleListBox( RTMessage Msg)
{
 dAPControl = 0;
 if(MessageBox(HWindow,GetString(6048),szApp,MB_ICONQUESTION | MB_YESNO) == IDNO)
  return;

 char szBuffer[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);
 if(CheckCashBalance(lRecord))
 {
	apcash->SetIndex(GetString(8140));
	if(apcash->seek(lRecord)==0)
	{
	 long lBatchno = apcash->lBatchno();
	 AppendAPSummary(lBatchno);
	}

   SetField(ID_BATCHNO,"");
   SSClearData(SHWindow,SS_ALLCOLS,SS_ALLROWS);
   long i=1;
   apcash->top();
   while(!apcash->eof())
   {
     if(StrCmp(apcash->mode->str(),"1")==0)
     {
		SSSetData(SHWindow,1,i,apcash->batchno->str());
      SSSetData(SHWindow,2,i,apcash->descript->str());
      SSSetData(SHWindow,3,i,apcash->invoiceno->str());
      SSSetData(SHWindow,4,i,apcash->payable->str());
      ListArray->add(* (new TListItem((int)i,(int)apcash->recNo())));
     i++;

     }
     apcash->skip(1);

    }
 }
 return;
}

void TAPCashPost::CreateGLSlip(void)
{
  apcash->top();
  while(!apcash->eof())
  {   
   long lBatchno = apcash->lBatchno();
   if(CheckCashBalance(lBatchno))
   {
    if(apcash->Open())
     AppendAPSummary(lBatchno);
   }
    apcash->skip(1);
  }
 return;
}

BOOL Tapcash::Open()
{
 if(int(Str4ptr(mode->str())) == 1)
 return TRUE;
 else
 return FALSE;
}


void TAPCashPost::AppendAPSummary(long lBatchno)
{
     if(apcashd->seek(lBatchno) == 0)
      {
       while (apcashd->lBatchno() == lBatchno)
        {
		  SendDlgItemMsg(ID_BATCHNO,WM_SETTEXT,NULL,(LPARAM)(LPCSTR)apcashd->batchno->str());
		  apcash->mode->assign("0");
		  apcash->UpdateUserTime(szUser);
	     apcash->flush();
	     apjsum->appendStart();
	     apjsum->account->assign(apcashd->account->str());
	     apjsum->dept->assign(apcashd->dept->str());
	     apjsum->mode->assign("1");
	     apjsum->period->assign(apcash->period->str());
	     apjsum->date->assign(apcash->date->str());
				 apjsum->type->assign("CA");
	     apjsum->batchno->assignDouble(apcashd->lBatchno());
	     apjsum->reference->assign(apcashd->descript->str());
	     apjsum->descript->assign(apcashd->descript->str());
	     apjsum->debit->assignDouble(double(Str4ptr(apcashd->debit->str())));
	     apjsum->credit->assignDouble(double(Str4ptr(apcashd->credit->str())));
	     apjsum->append();
             apcashd->skip();
	}
      }

}

