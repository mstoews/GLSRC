/*
   General Ledger  - (C) Copyright 1993 by Tegara Corporation
   Edit Budget Data.

*/
#include "gll.h"
#include "db_dept.h"
#include "db_accts.h"
#include "gl_budgt.h"



extern Code4 cb;
void TGlWindow::GLAcctBudget  (RTMessage){GetApplication()->ExecDialog(new TGLAcctBudget(this,"GL_ACCTBUDGET"));}
extern char  AccountNumber[7];






void TGLAcctBudget::Next   (RTMessage)
{
 glaccts->skip(1);
 GetBudgetRecord(HWindow);
}
void TGLAcctBudget::Previous (RTMessage)
{
 glaccts->skip(-1);
 GetBudgetRecord(HWindow);
}

void TGLAcctBudget::Ok             (RTMessage )
{
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST); 
 char szBudget[85];    
 for(int i=1;i<13;i++)
 {
  SSGetData(SHWindow,1,i,szBudget);
  switch (i)
  {
    case  1:  glaccts->budget1->assignDouble(atof(szBudget));
    case  2:  glaccts->budget2->assignDouble(atof(szBudget));
    case  3:  glaccts->budget3->assignDouble(atof(szBudget));
    case  4:  glaccts->budget4->assignDouble(atof(szBudget));
    case  5:  glaccts->budget5->assignDouble(atof(szBudget));
    case  6:  glaccts->budget6->assignDouble(atof(szBudget));
    case  7:  glaccts->budget7->assignDouble(atof(szBudget));
    case  8:  glaccts->budget8->assignDouble(atof(szBudget));
    case  9:  glaccts->budget9->assignDouble(atof(szBudget));
    case 10:  glaccts->budget10->assignDouble(atof(szBudget));
    case 11:  glaccts->budget11->assignDouble(atof(szBudget));
    case 12:  glaccts->budget12->assignDouble(atof(szBudget));
  }
 }
 glaccts->UpdateUserTime(szUser);
 glaccts->flush();
}

void TGLAcctBudget::Cancel (RTMessage)
{
 SSSaveToFile(GetDlgItem(HWindow,ID_DISTLIST),"Budgeted.wk1",FALSE);
 cb.closeAll();
  cb.init();
 ShutDownWindow();
}

void TGLAcctBudget::WMInitDialog   (RTMessage)
{ 
 SS_CELLTYPE 	CellType;
 LOGFONT 	lfFont;
 HFONT   	hFont;
 HWND SHWindow = GetDlgItem(HWindow,ID_BATCHSEARCH);
 SendDlgItemMsg(ID_PRINT,SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)(LPSTR)"GLPRINT");
 glaccts = (Tglaccts *) new Tglaccts(&cb,"glaccts");
 HourGlassOn();
 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);

 //lfFont.lfHeight          = -13;
 lfFont.lfHeight          = -(GetFontSize());
 lfFont.lfWidth           = 0;
 lfFont.lfEscapement      = 0;
 lfFont.lfOrientation     = 0;
 lfFont.lfWeight          = FW_NORMAL;
 lfFont.lfItalic          = 0;
 lfFont.lfUnderline       = 0;
 lfFont.lfStrikeOut       = 0;
 lfFont.lfCharSet         = USER_CHAR_SET; //default character set.
  lfFont.lfOutPrecision    = 0;
 lfFont.lfClipPrecision   = 0;
 lfFont.lfQuality         = 0;
 lfFont.lfPitchAndFamily  = FF_SWISS;
 StrCpy (lfFont.lfFaceName, "HELV");
 hFont = CreateFontIndirect((LPLOGFONT) &lfFont);

 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSSetBool(SHWindow,SSB_ALLOWUSERFORMULAS,TRUE);
 SSSetMaxCols(SHWindow, 10);
 SSSetMaxRows(SHWindow, 14);
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);

 SSSetData(SHWindow,1,SS_HEADER, "Budget");
 SSSetData(SHWindow,2,SS_HEADER, "Period");
 SSSetData(SHWindow,3,SS_HEADER, "Calculation");
 SSSetData(SHWindow,4,SS_HEADER, "Current Year");
 SSSetData(SHWindow,5,SS_HEADER, "Previous Year");

 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetColWidth(SHWindow,1,15);
 SSSetColWidth(SHWindow,2,8);
 SSSetColWidth(SHWindow,3,10);
 SSSetColWidth(SHWindow,4,15);
 SSSetColWidth(SHWindow,5,15);
 SSSetTypeEdit(SHWindow,&CellType,SS_TEXT_LEFT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);

 SSSetTypeEdit(SHWindow,&CellType,SS_TEXT_RIGHT,40,SS_CHRSET_CHR,SS_CASE_NOCASE);  // was StaticText
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 


 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,13,2,0,999999999999.99);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,5, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,6, SS_ALLROWS, &CellType);
 glaccts->top();
 GetBudgetRecord(HWindow);
 HourGlassOff();

}


void TGLAcctBudget::GetBudgetRecord (HWND HWindow)
{
 HWND SHWindow = GetDlgItem(HWindow,ID_DISTLIST);
 SendDlgItemMessage(HWindow,ID_ACCOUNTS,WM_SETTEXT,NULL,(LONG)(LPSTR)glaccts->accounts->str());
 SendDlgItemMessage(HWindow,ID_DEPART,WM_SETTEXT,NULL,(LONG)(LPSTR)glaccts->dept->str());
 SendDlgItemMessage(HWindow,ID_DESCRIPT,WM_SETTEXT,NULL,(LONG)(LPSTR)glaccts->descript->str());
 SSSetData(SHWindow,1,1,glaccts->budget1->str());
 SSSetData(SHWindow,1,2,glaccts->budget2->str());
 SSSetData(SHWindow,1,3,glaccts->budget3->str());
 SSSetData(SHWindow,1,4,glaccts->budget4->str());
 SSSetData(SHWindow,1,5,glaccts->budget5->str());
 SSSetData(SHWindow,1,6,glaccts->budget6->str());
 SSSetData(SHWindow,1,7,glaccts->budget7->str());
 SSSetData(SHWindow,1,8,glaccts->budget8->str());
 SSSetData(SHWindow,1,9,glaccts->budget9->str());
 SSSetData(SHWindow,1,10,glaccts->budget10->str());
 SSSetData(SHWindow,1,11,glaccts->budget11->str());
 SSSetData(SHWindow,1,12,glaccts->budget12->str());

 SSSetData(SHWindow,4,1,glaccts->period1->str());
 SSSetData(SHWindow,4,2,glaccts->period2->str());
 SSSetData(SHWindow,4,3,glaccts->period3->str());
 SSSetData(SHWindow,4,4,glaccts->period4->str());
 SSSetData(SHWindow,4,5,glaccts->period5->str());
 SSSetData(SHWindow,4,6,glaccts->period6->str());
 SSSetData(SHWindow,4,7,glaccts->period7->str());
 SSSetData(SHWindow,4,8,glaccts->period8->str());
 SSSetData(SHWindow,4,9,glaccts->period9->str());
 SSSetData(SHWindow,4,10,glaccts->period10->str());
 SSSetData(SHWindow,4,11,glaccts->period11->str());
 SSSetData(SHWindow,4,12,glaccts->period12->str());

 SSSetData(SHWindow,5,1,glaccts->previous1->str());
 SSSetData(SHWindow,5,2,glaccts->previous2->str());
 SSSetData(SHWindow,5,3,glaccts->previous3->str());
 SSSetData(SHWindow,5,4,glaccts->previous4->str());
 SSSetData(SHWindow,5,5,glaccts->previous5->str());
 SSSetData(SHWindow,5,6,glaccts->previous6->str());
 SSSetData(SHWindow,5,7,glaccts->previous7->str());
 SSSetData(SHWindow,5,8,glaccts->previous8->str());
 SSSetData(SHWindow,5,9,glaccts->previous9->str());
 SSSetData(SHWindow,5,10,glaccts->previous10->str());
 SSSetData(SHWindow,5,11,glaccts->previous11->str());
 SSSetData(SHWindow,5,12,glaccts->previous12->str());

}

void TGLAcctBudget::WMRButtonDown  (RTMessage)
   {
    GetApplication()->ExecDialog(new TGLAcctList(this,"GL_LIST",glaccts));    
   }

void TGLAcctBudget::Print (RTMessage)
{
}

