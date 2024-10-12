
/*
  Accounts Payable  - (C) Copyright 1992 by Tegara Corporation
	AP Cancel Batch.
	ap_cancb.cpp
*/
#include "std_inc.h"

//#include "db_apbtc.h"
//#include "db_cash.h"
#include "ap_cancb.h"

#define ID_DELETE_BATCH 5013






void TManilaGL::APCancelBatch (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPCancelBatch(this,1043));
 }
void TManilaGL::APCancelCash (RTMessage)
 {
  GetApplication()->ExecDialog(new TAPCancelCash(this,1043));
 }


void TAPCancelBatch::SetupWindow()
{
 TMainDialog::SetupWindow();

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
 Code4 *cb = ((TAcctModule *)GetApplication())->GetCodeBase();

 if(apbtchd.open(*cb) < 0)
	{
	 ErrorClose(apbtchd.GetDbfName());
	 return;
	}
 apbtchd.SetIndex(GetString(8140));

 if(apbtch.open(*cb)  < 0 )
  {
	ErrorClose(apbtch.GetDbfName());
	return;
  }
 apbtch.SetIndex(GetString(8140));



long RecTotal=0;

 RecTotal = apbtch.recCount();
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
 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,atof(GetString(6107)));
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);

 Field4 batchno(apbtch,  1);
 Field4 descript(apbtch, 13);
 Field4 invoiceno(apbtch,8);
 Field4 payable(apbtch,  17);


 long i=1;
 Relate4set rQuery(apbtch);
 //"MODE = '1' .AND. PAYABLE > 0"
 rQuery.querySet(GetString(1041));
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
		SSSetData(SHWindow,1,i,batchno.str());
		SSSetData(SHWindow,2,i,descript.str());
		SSSetData(SHWindow,3,i,invoiceno.str());
		SSSetData(SHWindow,4,i,payable.str());
	  i++;
  }
 rQuery.unlock();
 rQuery.free();
 if(i < 10)
  i = 10;
 SSSetMaxRows(SHWindow,i);
}


void TAPCancelCash::SetupWindow()
{
 TMainDialog::SetupWindow();
 SetCaption(GetString(1040));
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

 apcash.open(*cb);
 if(!apcash.isValid())
  {
	ErrorClose(apcash.GetDbfName());
	return;
  }

apcash.SetIndex(GetString(8140));

apcashd.open(*cb);
if(!apcashd.isValid())
  {
	ErrorClose(apcashd.GetDbfName());
	return;
  }

apcashd.SetIndex(GetString(8140));

long RecTotal=0;

 RecTotal = apcash.recCount();
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
 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,atof(GetString(6107)));
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);

 Field4 batchno(apcash,  1);
 Field4 descript(apcash, 14);
 Field4 invoiceno(apcash,8);
 Field4 payable(apcash,  18);


 long i=1;
 Relate4set rQuery(apcash);
 // "MODE = '1' .AND. PAYABLE > 0"
 rQuery.querySet(GetString(1041));
 for(int qc=rQuery.top();qc!=r4eof;qc=rQuery.skip(1L))
  {
		SSSetData(SHWindow,1,i,batchno.str());
		SSSetData(SHWindow,2,i,descript.str());
      SSSetData(SHWindow,3,i,invoiceno.str());
      SSSetData(SHWindow,4,i,payable.str());
     i++;
  }
 rQuery.unlock();
 rQuery.free();
 SetBitmap(IDCANCEL);
 SSSetMaxRows(SHWindow,i);
}

void TAPCancelCash::HandleListBox( RTMessage Msg)
{
 char szBuffer[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);

 Field4 payable(apcash,18);
 Field4 reference(apcash,13);
 Field4 vatamt(apcash,24);
 Field4 mode (apcash,2);

 Field4 batchno(apcashd,1);
 Field4 debit(apcashd,9);
 Field4 credit(apcashd,10);
 Field4 descript(apcashd,"descript");

 if(apcash.seek(lRecord)==0)
 {
  if( MessageBox(HWindow,GetString(ID_DELETE_BATCH),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
	 reference.assign(GetString(1042));
	 payable.assignDouble(0);
	 vatamt.assignDouble(0);
	 mode.assign("2");
	 apcash.UpdateUserTime(szUser);
	 apcash.flush();
	 if(apcashd.seek(lRecord) == 0)
		{
		 while(long (Str4ptr(batchno.str())) == lRecord)
		 {
			debit.assignDouble(0);
			credit.assignDouble(0);
			descript.assign(GetString(1042));
			apcashd.flush();
			apcashd.skip();
		 }
     } 
    SSDelRow(SHWindow,Cell->Row);
   }
 }
 return;
}


void TAPCancelBatch::HandleListBox( RTMessage Msg)
{
 char szBuffer[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);

 Field4 mode (apbtch,2);
 Field4 reference(apbtch,12);
 Field4 payable(apbtch,17);
 Field4 vatamt(apbtch,23);


 Field4 batchno(apbtchd,1);
 Field4 descript(apbtchd,3);
 Field4 debit(apbtchd,9);
 Field4 credit(apbtchd,10);

 string strMsg;
 strMsg = GetString(ID_DELETE_BATCH);
 strMsg += szBuffer;

 if(apbtch.seek(lRecord)==0)
 {
  if( MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
	 reference.assign(GetString(1042));
	 payable.assignDouble(0);
	 vatamt.assignDouble(0);
	 mode.assign("2");
	 apbtch.UpdateUserTime(szUser);
	 apbtch.flush();
	 if(apbtchd.seek(lRecord) == 0)
		{

		 while(long (Str4ptr(batchno.str())) == lRecord)
		 {
			debit.assignDouble(0);
			credit.assignDouble(0);
			descript.assign(GetString(1042));
			apbtchd.flush();
			apbtchd.skip();
       }
     } 
    SSDelRow(SHWindow,Cell->Row);
	 apbtchd.unlock();
	 apbtch.unlock();
   }
 }
 return;
}


