
/*
  Accounts Payable  - (C) Copyright 1992 by Tegara Corporation
   AP Cancel Batch.
   ap_cancb.cpp
*/
#include "std_inc.h"
#include "ar_cancb.h"
#define ID_DELETE_BATCH 5013




void TManilaGL::ARCancelBatch (RTMessage)
 {
  GetApplication()->ExecDialog(new TARCancelBatch(this,1043));
 }



void TARCancelBatch::SetupWindow()
{
 TMainDialog::SetupWindow();
 SetCaption(GetString(4033));
 CHourGlass wait;
 HWND SHWindow;
 RECT rc;
 SS_CELLTYPE 	CellType;
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
 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());

 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSSetBool(SHWindow,SSB_SCROLLBAREXTMODE,TRUE);
 SSSetBool(SHWindow,SSB_NOBEEP,TRUE);
 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);


arbtch.open(cb,"ar_btch");
arbtch.SetIndex("batchno");

arbtchd.open(cb,"ar_btchd");
arbtchd.SetIndex(GetString(8140));


long RecTotal=0;

 RecTotal = arbtch.recCount();
 if(RecTotal < 10 )
 RecTotal = 10;

 SSSetMaxCols(SHWindow, 4);
 SSSetMaxRows(SHWindow, RecTotal);
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 SSSetData(SHWindow,1,SS_HEADER, GetString(8141));
 SSSetData(SHWindow,2,SS_HEADER, GetString(8142));
 SSSetData(SHWindow,3,SS_HEADER, GetString(8143));
 SSSetData(SHWindow,4,SS_HEADER, "Amount");


 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);

 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,atof(GetString(6107)));
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);
 long i=1;

 Field4 batchno(arbtch,1);
 Field4 invoiceno(arbtch,7);
 Field4 reference(arbtch,14);
 Field4 payable(arbtch,21);

 Relate4set rQuery(arbtch);
 rQuery.querySet("MODE = '1'");
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
		SSSetData(SHWindow,1,i,batchno.str());
      SSSetData(SHWindow,2,i,reference.str());
      SSSetData(SHWindow,3,i,invoiceno.str());
      SSSetData(SHWindow,4,i,payable.str());
     i++;
  }

 if(i>8)
  SSSetMaxRows(SHWindow,i--);
 else
  SSSetMaxRows(SHWindow,8);
 rQuery.unlock();
 rQuery.free();

}




void TARCancelBatch::HandleListBox( RTMessage Msg)
{
 char szBuffer[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);

 Field4 payable(arbtch,"payable");
 Field4 reference(arbtch,"reference");
 Field4 mode(arbtch,"mode");

 Field4 batchno(arbtchd,"batchno");
 Field4 debit(arbtchd,"debit");
 Field4 credit(arbtchd,"credit");

 Field4 descript(arbtchd,"descript");

 string szMsg;
 szMsg = GetString(ID_DELETE_BATCH);
 szMsg += szBuffer;

 if(arbtch.seek(lRecord)==0)
 {
  if( MessageBox(HWindow,(LPSTR)szMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
	 reference.assign("Cancelled");
	 mode.assign("2");
	 payable.assignDouble(0);
	 arbtch.UpdateUserTime(szUser);
	 arbtch.flush();
	 if(arbtchd.seek(lRecord) == 0)
		{

		 while(long (Str4ptr(batchno.str())) == lRecord)
		 {
			debit.assignDouble(0);
			credit.assignDouble(0);
			descript.assign("Cancelled");
			arbtchd.flush();
			arbtchd.skip();
		 }
	  }
	 SSDelRow(SHWindow,Cell->Row);
	}
 }
 return;
}


