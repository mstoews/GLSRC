#include "ar.h"
#include <math.h>
#include "db_dist.h"
#include "db_apbtc.h"
#include "ar_ctrl.h"
#include "gl_dlg.h"
#include "ar_postb.h"

void TAPCashPost::SetupWindow()
{
 SetCaption("Post Accounts Payable Cash Journal"); 
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
		  - d0Width
		  - d1Width
		  - d3Width
		  - d4Width;

 SSSetColWidthInPixels(SHWindow,2,ColWidth);


 lfFont.lfHeight          = -10;
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


apcash = (Tapcash *) new Tapcash(&cb,"ap_btch");
apcash->SetIndex("batchno");

apcashd = (Tapcashd *) new Tapcashd(&cb,"ap_btchd");
apcashd->SetIndex("batchno");

glaccts = (Tglaccts *) new Tglaccts(&cb,"glaccts");
glaccts->SetIndex("account");

apdist  = (Tapdist *) new Tapdist(&cb,"ap_dist");
apdist->SetIndex("account");

gldist  = (Tgldist *) new Tgldist(&cb,"gl_dist");
gldist->SetIndex("account");


long RecTotal=0;

 RecTotal = apcash->recCount();
 if(RecTotal < 10 )
 RecTotal = 10;

 SSSetMaxCols(SHWindow, 4);
 SSSetMaxRows(SHWindow, RecTotal);
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);



 SSSetData(SHWindow,1,SS_HEADER, "Btch");
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


void TAPCashPost::Ok             (RTMessage )
{
  if(*GetField(ID_DESCRIPT) == 0)
  {
   MessageBox(HWindow,"Enter a Cash batch description.","AP Post",MB_OK);
   SetFocus(GetDlgItem(HWindow,ID_DESCRIPT));
   return;
  }
  if(MessageBox(HWindow,GetString(8006),"W A R N I N G",MB_ICONQUESTION | MB_YESNO) == IDNO)
  return;
  CreateGLSlip();
  cb.closeAll();
  ShutDownWindow();
}


void TAPCashPost::HandleListBox( RTMessage Msg)
{
 dAPControl = 0;
 if(*GetField(ID_DESCRIPT) == 0)
  {
   MessageBox(HWindow,"Enter a AP batch description.","AP Post",MB_OK);
   SetFocus(GetDlgItem(HWindow,ID_DESCRIPT));
   return;
  }
  if(MessageBox(HWindow,GetString(8006),"W A R N I N G",MB_ICONQUESTION | MB_YESNO) == IDNO)
  return;

 char szBuffer[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);
 apcash->SetIndex("batchno");
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
 return;
}

void TAPCashPost::CreateGLSlip(void)
{
  apcash->top();
  while(!apcash->eof())
  {   
   long lBatchno = apcash->lBatchno();
   if(apcash->Open())
    AppendAPSummary(lBatchno);
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
	     apcash->flush();
	     gldist->appendStart();
	     gldist->account->assign(apcashd->account->str());
	     gldist->dept->assign(apcashd->dept->str());
	     gldist->mode->assign("1");
	     gldist->period->assign(apcash->period->str());
	     gldist->date->assign(apcash->date->str());
             gldist->type->assign("CA");
	     gldist->batchno->assignDouble(apcashd->lBatchno());
	     gldist->reference->assign(apcashd->descript->str());
	     gldist->descript->assign(apcashd->descript->str());
	     gldist->debit->assignDouble(double(Str4ptr(apcashd->debit->str())));
	     gldist->credit->assignDouble(double(Str4ptr(apcashd->credit->str())));
	     gldist->append();
             apcashd->skip();
	}
      }

}

