
/*
  Accounts Payable  - (C) Copyright 1992 by Tegara Corporation
	AP Cancel Batch.
	ap_cancb.cpp
*/
#include "std_inc.h"

#include "ap.h"
//#include "db_dist.h"

#include "db_apdst.h"
#include "gl_dlg.h"
#include "ap_purge.h"
extern HINSTANCE hInst;



void TManilaGL::APPurgeAccts  (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPPurgeBatch(this,1048));
 }

void TAPPurgeBatch::SetupWindow()
{
 CHourGlass wait;
 cb = &((TAcctModule *)GetApplication())->cb;
 TMainDialog::SetupWindow();
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

apdist  = new Tapdist(cb);
 if(!apdist->isValid())
  {
	ErrorClose(apdist->GetDbfName());
	return;
  }

apdist->SetIndex(GetString(8140));

long RecTotal=0;

 RecTotal = apdist->recCount();
 if(RecTotal < 10 )
 RecTotal = 10;

 SSSetMaxCols(SHWindow, 4);
 SSSetMaxRows(SHWindow, RecTotal);
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
 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0.00,atof(GetString(6107)));
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);
 long i=1;
 Relate4set rQuery(apdist);
 // PAYMENTCTL = 'C'
 rQuery.querySet(GetString(6108));
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
		SSSetData(SHWindow,1,i,apdist->batchno.str());
		SSSetData(SHWindow,2,i,apdist->descript.str());
		SSSetData(SHWindow,3,i,apdist->invoiceno.str());
		SSSetData(SHWindow,4,i,apdist->amountpaid.str());
	  i++;
  }
 rQuery.unlock();
 rQuery.free();
 if(i>9)
  SSSetMaxRows(SHWindow,i-1);
 else
  SSSetMaxRows(SHWindow,9);

}


void TAPPurgeBatch::SelectItem(RTMessage Msg)
{
 apdist->SetIndex(GetString(8140));
 char* szBuffer = new char[apdist->batchno.len() + 1];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);
 //"Do you want to purge this paid account?"
 if( MessageBox(HWindow,GetString(8146),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
   {
	 SSDelRow(SHWindow,Cell->Row);
    if(apdist->seek(lRecord) == 0)
      {
		 while(long (Str4ptr(apdist->batchno.str())) == lRecord)
       {
			apdist->deleteRec();
			apdist->skip();
       }
     }
    apdist->pack();

   }
 delete [] szBuffer;
 return;
}

void TAPPurgeBatch::Ok (RTMessage)
{
 CHourGlass wait;
 Relate4set rQuery(apdist);
 // "PAYMENTCTL = 'P'"
 rQuery.querySet(GetString(6108));
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
	apdist->deleteRec();
  }
 rQuery.unlock();
 rQuery.free();
 apdist->pack();

 cb->closeAll();
 ShutDownWindow();
}

