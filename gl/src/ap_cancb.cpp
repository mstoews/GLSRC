
/*
  Accounts Payable  - (C) Copyright 1992 by Tegara Corporation
   AP Cancel Batch.
   ap_cancb.cpp
*/
#include "ap.h"
#include "db_apbtc.h"
#include "db_cash.h"
#include "ap_cancb.h"
#include "utildlg.h"


#define ID_DELETE_BATCH 5015

extern Code4 cb;
extern char szApp[20];



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
  if(!CheckFile("AP_BTCH",Parent->HWindow) ||
    !CheckFile("AP_BTCHD",Parent->HWindow))
	{
	 cb.init();
    ShutDownWindow();
    return;
   }

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
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);

apbtch =  new Data4(&cb,"ap_btch");
apbtch->SetIndex(GetString(8140));

apbtchd = new Data4(&cb,"ap_btchd");
apbtchd->SetIndex(GetString(8140));

long RecTotal=0;

 RecTotal = apbtch->recCount();
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
 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,99999999999999.99);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);

 Field4 batchno(*(Data4*)apbtch,  1);
 Field4 descript(*(Data4*)apbtch, 13);
 Field4 invoiceno(*(Data4*)apbtch,8);
 Field4 payable(*(Data4*)apbtch,  17);


 long i=1;
 Relate4set *rQuery = new Relate4set(apbtch);
 //"MODE = '1' .AND. PAYABLE > 0"
 rQuery->querySet(GetString(1041));
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
		SSSetData(SHWindow,1,i,batchno.str());
		SSSetData(SHWindow,2,i,descript.str());
		SSSetData(SHWindow,3,i,invoiceno.str());
		SSSetData(SHWindow,4,i,payable.str());
	  i++;
  }
 rQuery->unlock();
 rQuery->free();
 HourGlassOff();
}


void TAPCancelCash::SetupWindow()
{
 TMainDialog::SetupWindow();
 SetCaption(GetString(1040));
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
 lfFont.lfCharSet         = 128; //default character set.
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

apcash =  new Data4(&cb,"ap_cash");
apcash->SetIndex(GetString(8140));

apcashd = new Data4(&cb,"ap_cashd");
apcashd->SetIndex(GetString(8140));

long RecTotal=0;

 RecTotal = apcash->recCount();
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
 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,15,2,0,99999999999999.99);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);

 Field4 batchno(*(Data4*)apcash,  1);
 Field4 descript(*(Data4*)apcash, 14);
 Field4 invoiceno(*(Data4*)apcash,8);
 Field4 payable(*(Data4*)apcash,  18);


 long i=1;
 Relate4set *rQuery = new Relate4set(apcash);
 // "MODE = '1' .AND. PAYABLE > 0"
 rQuery->querySet(GetString(1041));
 for(int qc=rQuery->top();qc!=r4eof;qc=rQuery->skip(1L))
  {
		SSSetData(SHWindow,1,i,batchno.str());
		SSSetData(SHWindow,2,i,descript.str());
      SSSetData(SHWindow,3,i,invoiceno.str());
      SSSetData(SHWindow,4,i,payable.str());
     i++;
  }
 rQuery->unlock();
 rQuery->free();
 SetBitmap(IDCANCEL);
 HourGlassOff();
}

void TAPCancelCash::HandleListBox( RTMessage Msg)
{
 char szBuffer[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);

 Field4 payable(*(Data4*)apcash,18);
 Field4 reference(*(Data4*)apcash,13);
 Field4 vatamt(*(Data4*)apcash,24);
 Field4 mode (*(Data4 *)apcash,2);

 Field4 batchno(*(Data4*)apcashd,1);
 Field4 debit(*(Data4*)apcashd,9);
 Field4 credit(*(Data4*)apcashd,10);
 Field4 descript(*(Data4*)apcashd,"descript");

 if(apcash->seek(lRecord)==0)
 {
  if( MessageBox(HWindow,GetString(ID_DELETE_BATCH),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
	 reference.assign(GetString(1042));
	 payable.assignDouble(0);
	 vatamt.assignDouble(0);
	 mode.assign("2");
	 apcash->UpdateUserTime(szUser);
	 apcash->flush();
	 if(apcashd->seek(lRecord) == 0)
		{
		 while(long (Str4ptr(batchno.str())) == lRecord)
		 {
			debit.assignDouble(0);
			credit.assignDouble(0);
			descript.assign(GetString(1042));
			apcashd->flush();
			apcashd->skip();
		 }
     } 
    SSDelRow(SHWindow,Cell->Row);
    apcashd->unlock();
    apcash->unlock();
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

 Field4 mode (*(Data4 *)apbtch,2);
 Field4 reference(*(Data4 *)apbtch,12);
 Field4 payable(*(Data4 *)apbtch,17);
 Field4 vatamt(*(Data4 *)apbtch,23);
 

 Field4 batchno(*(Data4 *)apbtchd,1);
 Field4 descript(*(Data4 *)apbtchd,3);
 Field4 debit(*(Data4 *)apbtchd,9);
 Field4 credit(*(Data4 *)apbtchd,10);


 if(apbtch->seek(lRecord)==0)
 {
  if( MessageBox(HWindow,GetString(ID_DELETE_BATCH),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
   {
	 reference.assign(GetString(1042));
	 payable.assignDouble(0);
	 vatamt.assignDouble(0);
	 mode.assign("2");
	 apbtch->UpdateUserTime(szUser);
	 apbtch->flush();
	 if(apbtchd->seek(lRecord) == 0)
		{

		 while(long (Str4ptr(batchno.str())) == lRecord)
		 {
	debit.assignDouble(0);
	credit.assignDouble(0);
	descript.assign(GetString(1042));
	apbtchd->flush();
	apbtchd->skip();
       }
     } 
    SSDelRow(SHWindow,Cell->Row);
    apbtchd->unlock();
	 apbtch->unlock();
   }
 }
 return;
}
