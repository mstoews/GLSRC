
/*
  Accounts Payable  - (C) Copyright 1992 by Tegara Corporation
	AP Cancel Batch.
	ap_cancb.cpp
*/
#include "tbowl.h"
#include "d4all.hpp"

#include "ar.h"
//#include "db_dist.h"

#include "db_ardst.h"
#include "ar_purge.h"
extern char szApp[20];


void TManilaGL::ARPurgeAccts  (RTMessage)
 {
  GetApplication()->ExecDialog(new TARPurgeBatch(this,1062));
 }

void TARPurgeBatch::SetupWindow()
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
 short d1Width,d3Width,d4Width;
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);
 SSSetColWidth(SHWindow,1,6);
 SSSetColWidth(SHWindow,3,10);
 SSSetColWidth(SHWindow,4,10);


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


ardist  = (Tardist *) new Tardist(&cb,"ap_dist");
ardist->SetIndex("batchno");

long RecTotal=0;

 RecTotal = ardist->recCount();
 if(RecTotal < 10 )
 RecTotal = 10;

 SSSetMaxCols(SHWindow, 4);
 SSSetMaxRows(SHWindow, RecTotal);
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);
 
 SSSetData(SHWindow,1,SS_HEADER, "Btch");
 SSSetData(SHWindow,2,SS_HEADER, "Description");
 SSSetData(SHWindow,3,SS_HEADER, "Invoice No.");
 SSSetData(SHWindow,4,SS_HEADER, "Payable Amount");


 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);

  SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,999999999999.99);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);
 long i=1;
 Relate4set *rQuery = new Relate4set(ardist);
 rQuery->querySet("PAYMENTCTL = 'C'");
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
      SSSetData(SHWindow,1,i,ardist->batchno->str());
      SSSetData(SHWindow,2,i,ardist->reference->str());
      SSSetData(SHWindow,3,i,ardist->invoiceno->str());
      SSSetData(SHWindow,4,i,ardist->amountpaid->str());
     i++;
  }
 rQuery->unlock();
 rQuery->free();
 if(i>9)
  SSSetMaxRows(SHWindow,i-1);
 else
  SSSetMaxRows(SHWindow,9);
 HourGlassOff();
}


void TARPurgeBatch::SelectItem(RTMessage Msg)
{
 ardist->SetIndex("batchno");
 char* szBuffer = new char[ardist->batchno->len() + 1];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);
  //"Do you want to purge this paid account?"
 if( MessageBox(HWindow,GetString(6028),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
   {
    SSDelRow(SHWindow,Cell->Row);
    if(ardist->seek(lRecord) == 0)
      {
       while(long (Str4ptr(ardist->batchno->str())) == lRecord)
       {
	ardist->deleteRec();
	ardist->skip();
       }
     }
    ardist->pack();
    ardist->unlock();
   }
 delete [] szBuffer;
 return;
}

void TARPurgeBatch::Ok (RTMessage Msg)
{
 Relate4set *rQuery = new Relate4set(ardist);
 rQuery->querySet("PAYMENTCTL = 'C'");
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
   ardist->deleteRec();   
  }
 rQuery->unlock();
 rQuery->free();
 ardist->pack();
 ardist->unlock();
 cb.closeAll();
 cb.init();
 ShutDownWindow();
}
