/*
   General Ledger  - (C) Copyright 1993 by Tegara Corporation
   Delete general ledger accounts.
*/
#include <bwcc.h>
#include <tbowl.h>
#include "d4all.hpp"
#include "glclass.h"
#include "gl.h"
//#include "db_dept.h"
#include "db_accts.h"
#include "db_profi.h"
#include "gl_main.h"

char  AccountNumber[7];

BOOL FAR PASCAL BudgetPrintProc(HDC hDC,short dCommand,short nPageNum,long lAppData);

_CLASSDEF(TGLBudgetInq)
class TGLBudgetInq : public TDialog
{
 public:
 Tglaccts *glaccts; 
 Tgldept  *gldept;


 TGLBudgetInq(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TDialog(AParent,AName){};
 virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
 virtual void WMRButtonDown  (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
 virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
};




void TGLBudgetInq::Ok             (RTMessage ){}

void TGLBudgetInq::Cancel (RTMessage)
{
 ShutDownWindow();
}

void TGLBudgetInq::WMInitDialog   (RTMessage){

 HWND SHWindow;
 SS_CELLTYPE 	CellType;

 HFONT   	hFont;
 SendDlgItemMsg(ID_PRINT,SBM_SETPICT,SUPERBTN_PICT_BITMAP,(long)(LPSTR)"GLPRINT");

 glaccts = (Tglaccts *) new Tglaccts(&cb,"glaccts");
  if(!glaccts->isValid())
  {
	ErrorClose(glaccts->GetDbfName());
	return;
  }

 gldept  = (Tgldept *)  new Tgldept (&cb,"gldept");
  if(!gldept->isValid())
  {
	ErrorClose(gldept->GetDbfName());
	return;
  }

 CHourGlass wait;
 SHWindow = GetDlgItem(HWindow,ID_DISTLIST);


 hFont = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
 long RecTotal=0;
 glaccts->bottom();
 RecTotal = glaccts->recNo();

 SSSetBool(SHWindow,SSB_ALLOWUSERSELBLOCK,FALSE);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);

 SSSetMaxCols(SHWindow, 7);
 SSSetMaxRows(SHWindow, RecTotal);
 SSSetFont(SHWindow,SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
 DeleteObject(hFont);


 SSSetData(SHWindow,1,SS_HEADER, "Acct");
 SSSetData(SHWindow,2,SS_HEADER, "Dept");
 SSSetData(SHWindow,3,SS_HEADER, "Description");
 SSSetData(SHWindow,4,SS_HEADER, "Tp");
 SSSetData(SHWindow,5,SS_HEADER, "Current Balance");
 SSSetData(SHWindow,6,SS_HEADER, "Budget Amount");
 SSSetData(SHWindow,7,SS_HEADER, "Difference");

 SSSetBool(SHWindow,SSB_MOVEACTIVEONFOCUS, FALSE);
 SSSetColWidth(SHWindow,1,8);
 SSSetColWidth(SHWindow,2,7);
 SSSetColWidth(SHWindow,3,25);
 SSSetColWidth(SHWindow,4,3);
 SSSetColWidth(SHWindow,5,13);
 SSSetColWidth(SHWindow,6,13);
 SSSetColWidth(SHWindow,7,13);
 SSSetTypeStaticText(SHWindow,&CellType,SS_TEXT_LEFT);
 SSSetCellType(SHWindow,1, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,3, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,2, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,4, SS_ALLROWS, &CellType);

 SSSetTypeFloat(SHWindow,&CellType,FS_SEPARATOR,13,2,0,atof(GetString(6107)));
 SSSetCellType(SHWindow,5, SS_ALLROWS, &CellType);
 SSSetCellType(SHWindow,6, SS_ALLROWS, &CellType);
 SSSetFormula(SHWindow,7,SS_ALLROWS,"E#-F#",TRUE);
 SSSetCellType(SHWindow,7, SS_ALLROWS, &CellType);

 SSSetLock(SHWindow,7,SS_ALLROWS,TRUE);

 long i=1;
 glaccts->top();
 while(!glaccts->eof())
 {
     char szDifference[16];
     long lCurrent,lBudget,lDifference;
     SSSetData(SHWindow,1,i,glaccts->accounts->str());
     SSSetData(SHWindow,2,i,glaccts->dept->str());
     SSSetData(SHWindow,3,i,glaccts->descript->str());
     SSSetData(SHWindow,4,i,glaccts->type->str());
     SSSetData(SHWindow,5,i,glaccts->curr_bal->str());
     SSSetData(SHWindow,6,i,glaccts->budget1->str());
     lCurrent = atol(glaccts->curr_bal->str());
     lBudget  = atol(glaccts->budget1->str());
     lDifference = lCurrent-lBudget;
     ltoa(lDifference,szDifference,10);
     SSSetData(SHWindow,7,i,szDifference);

     i++;
     glaccts->skip(1);
 }

 
 SSBuildDependencies(SHWindow);
 SSSetBool(SHWindow,SSB_PROTECT,TRUE);
 SSReCalc(SHWindow);
 SSSetLock(SHWindow,5,SS_ALLROWS,TRUE);
 SSSetLock(SHWindow,6,SS_ALLROWS,TRUE);
 SSSetBool(SHWindow,SSB_REDRAW,TRUE);
 SSSetBool(SHWindow, SSB_ARROWSEXITEDITMODE, TRUE);
 SSSetEditEnterAction(SHWindow, SS_ENTERACTION_NEXT);


}


void TGLBudgetInq::WMRButtonDown  (RTMessage)
   {
    GetApplication()->ExecDialog(new TGLAcctList(this,"GL_ACCTLIST",glaccts)); 
    if(glaccts->seek(AccountNumber)==0)
     {
      SendDlgItemMsg(ID_ACCOUNTS,WM_SETTEXT,glaccts->accounts->len(),(LONG)(LPSTR)glaccts->accounts->str());
      SendDlgItemMsg(ID_DESCRIPT,WM_SETTEXT,glaccts->descript->len(),(LONG)(LPSTR)glaccts->descript->str());
      SendDlgItemMsg(ID_CURRBAL ,WM_SETTEXT,glaccts->curr_bal->len(),(LONG)(LPSTR)glaccts->curr_bal->str());
      SendDlgItemMsg(ID_DEPART,WM_SETTEXT,glaccts->dept->len(),(LONG)(LPSTR)glaccts->dept->str());
     }
     else
     return;
 }

#pragma argsused
BOOL FAR PASCAL BudgetPrintProc(HDC hDC,short dCommand,short nPageNum,long lAppData)
{
long   TextExtent;
char   Footer[35];
char   Header1[35];
char   Header2[35];
TextExtent = GetTextExtent(hDC, "0", 1);
StrCpy(Header1,"Takeda Chemical Industries Ltd.");
StrCpy(Header2,"Budget Inquiry Listing");
switch (dCommand)
   {
   case SS_PRINTCMD_HEADERLEN:
      return (HIWORD(TextExtent) * 3);

   case SS_PRINTCMD_FOOTERLEN:
      return (HIWORD(TextExtent) * 3);

   case SS_PRINTCMD_PRINTHEADER:
      TextOut(hDC,10, 0,Header1,strlen(Header1));
      TextOut(hDC,10,15,Header2,strlen(Header2));
      break;

   case SS_PRINTCMD_PRINTFOOTER:
      wsprintf(Footer,"%s%d","Page - ",nPageNum);
      TextOut(hDC,10, 0, Footer,strlen(Footer));
      break;

   }

return (0);
}




void TGLBudgetInq::Print (RTMessage)
{
}
