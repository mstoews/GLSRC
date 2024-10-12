 /*
   General Ledger  - (C) Copyright 1992 by Tegara Corporation
   Delete General Ledger batch.
*/
#include "gll.h"
#include "gl_btcan.h"

extern Code4 cb;

void TGlWindow::GLDeleteBatch (RTMessage){
  GetApplication()->ExecDialog(new TGLCancelBatch(this,"GL_CANCELBATCH"));
}


void TGLCancelBatch::SelectSlip (RTMessage Msg)
{
 char szBuffer[10];
 LPSS_CELLCOORD Cell = (LPSS_CELLCOORD)Msg.LParam;
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SSGetData(SHWindow,1,Cell->Row,szBuffer);
 long lRecord=atol(szBuffer);
 glbtch->SetIndex(GetString(8140));
 glbtchd->SetIndex(GetString(8140));
 if(glbtch->seek(lRecord)==0)
 {
  if( BWCCMessageBox(HWindow,GetString(8148),szApp,MB_ICONQUESTION | MB_YESNO) == IDYES)
   {
    SSDelRow(SHWindow,Cell->Row);
    glbtch->deleteRec();
    if(glbtchd->seek(lRecord) == 0)
      {
		 while(long (Str4ptr(glbtchd->batchno->str())) == lRecord)
       {
	glbtchd->deleteRec();
	glbtchd->skip();
       }
     } 
    glbtch->pack();
    glbtchd->pack();
    glbtchd->unlock();
    glbtch->unlock();
   }
 }
 return;

}

void TGLCancelBatch::Cancel (RTMessage)
{
 delete glbtch;
 ShutDownWindow();
}

void TGLCancelBatch::SetupWindow()
{
 HWND SHWindow;
 SS_CELLTYPE 	CellType;
 LOGFONT 	lfFont;
 HFONT   	hFont;
 //SendDlgItemMsg(ID_PRINT,SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)(LPSTR)"GLPRINT");

 glbtch = new Tglbtch(&cb);
 glbtchd = new Tglbtchd(&cb); 
 HourGlassOn();
 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 
 //lfFont.lfHeight          = 13;
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
 long RecTotal=0;
 glbtch->bottom();
 RecTotal = glbtch->recNo();

 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSSetBool(SHWindow,SSB_HORZSCROLLBAR,FALSE);
 SSSetBool(SHWindow,SSB_SHOWROWHEADERS,FALSE);

 SSSetMaxCols(SHWindow, 4);
 SSSetMaxRows(SHWindow, RecTotal);
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);



 SSSetData(SHWindow,1,SS_HEADER, "Slip");
 SSSetData(SHWindow,2,SS_HEADER, "Description");
 SSSetData(SHWindow,3,SS_HEADER, "Debit Amount");
 SSSetData(SHWindow,5,SS_HEADER, "Credit Amount");

 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 RECT rc;
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

 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 
 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,13,2,0,999999999999.99);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);

 long i=1;
 glbtch->top();
 while(!glbtch->eof())
 {
     if(StrCmp(glbtch->mode->str(),"1")==0)
     {
      SSSetData(SHWindow,1,i,glbtch->batchno->str());
      SSSetData(SHWindow,2,i,glbtch->descript->str());
      SSSetData(SHWindow,3,i,glbtch->debit->str());
      SSSetData(SHWindow,4,i,glbtch->credit->str());
     i++;
     }
     glbtch->skip(1);
 }


 
 SSBuildDependencies(SHWindow);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSReCalc(SHWindow);
 SSSetLock(SHWindow,5,SS_ALLROWS,TRUE);
 SSSetLock(SHWindow,6,SS_ALLROWS,TRUE);
 SSSetBool(SHWindow,SSB_REDRAW,TRUE);
 SSSetBool(SHWindow, SSB_ARROWSEXITEDITMODE, TRUE);
 SSSetEditEnterAction(SHWindow, SS_ENTERACTION_NEXT);

 HourGlassOff();
}

