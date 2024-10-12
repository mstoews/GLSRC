
/*
  Accounts Payable  - (C) Copyright 1992 by Tegara Corporation
	AP Cancel Batch.
	ap_cancb.cpp
*/
#include "std_inc.h"

/*
#include "tbowl.h"
#include "d4all.hpp"
#include "gl.h"
#include "ar.h"
#include "db_ardst.h"
#include "ar_purge.h"
*/


void TManilaGL::ARPurgeAccts  (RTMessage)
 {
  GetApplication()->ExecDialog(new TARPurgeBatch(this,1062));
 }

void TARPurgeBatch::SetupWindow()
{
 TMainDialog::SetupWindow();
 CHourGlass wait;
 HWND SHWindow;
 RECT rc;
 SS_CELLTYPE 	CellType;
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

 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());

 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);

 ardist  = (Tardist *) new Tardist(cb,"ap_dist");
 if(!ardist->isValid())
  {
	ErrorClose(ardist->GetDbfName());
	return;
  }

ardist->SetIndex("batchno");

long RecTotal=0;

 RecTotal = ardist->recCount();
 if(RecTotal < 10 )
 RecTotal = 10;

 SSSetMaxCols(SHWindow, 4);
 SSSetMaxRows(SHWindow, RecTotal);
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);


 SSSetData(SHWindow,1,SS_HEADER, GetString(5118));
 SSSetData(SHWindow,2,SS_HEADER, GetString(5119));
 SSSetData(SHWindow,3,SS_HEADER, GetString(6045));
 SSSetData(SHWindow,4,SS_HEADER, GetString(1030));

 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,atof(GetString(6107)));
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);
 long i=1;
 Relate4set rQuery(ardist);
 rQuery.querySet("PAYMENTCTL = 'C'");
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
		SSSetData(SHWindow,1,i,ardist->batchno.str());
		SSSetData(SHWindow,2,i,ardist->reference.str());
		SSSetData(SHWindow,3,i,ardist->invoiceno.str());
		SSSetData(SHWindow,4,i,ardist->amountpaid.str());
     i++;
  }
 rQuery.unlock();
 rQuery.free();
 if(i>9)
  SSSetMaxRows(SHWindow,i-1);
 else
  SSSetMaxRows(SHWindow,9);

}


void TARPurgeBatch::SelectItem(RTMessage Msg)
{
 ardist->SetIndex("batchno");
 char* szBuffer = new char[ardist->batchno.len() + 1];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);
  //"Do you want to purge this paid account?"
 if( MessageBox(HWindow,GetString(6028),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
   {
    SSDelRow(SHWindow,Cell->Row);
    if(ardist->seek(lRecord) == 0)
      {
		 while(long (Str4ptr(ardist->batchno.str())) == lRecord)
		 {
	ardist->deleteRec();
	ardist->skip();
       }
     }
    ardist->pack();

   }
 delete [] szBuffer;
 return;
}

void TARPurgeBatch::Ok (RTMessage)
{
 CHourGlass wait;
 Relate4set rQuery(ardist);
 rQuery.querySet("PAYMENTCTL = 'C'");
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
	ardist->deleteRec();
  }
 rQuery.unlock();
 rQuery.free();
 ardist->pack();

 cb->closeAll();
 cb->init();
 ShutDownWindow();
}
