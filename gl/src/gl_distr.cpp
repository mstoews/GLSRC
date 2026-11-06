
#include <bwcc.h>
#include <tbowl.h>
#include "d4all.hpp"

#include "gl.h"
#include "crpe.h"
#include "glclass.h"
#include "db_profi.h"
#include "db_dept.h"

#include "db_accts.h"
#include "db_glopn.h"

#include "gl_dlg.h"
#include "gl_main.h"

#define ID_ITEMSTART 1105
#define ID_ITEMEND   1106
#define ID_TITLE     1107

extern Code4 cb;

_CLASSDEF(TRptDialog)
class TRptDialog : public TDialog
{
 public:
 BOOL bDetail; 
 char szFileName[20];
 char szReportTitle[60];
 PTGLAcctList GLACCTS;
 PTglaccts glaccts;
 PTglopen  glopen;
 TRptDialog(PTWindowsObject AParent, LPSTR AName, LPSTR AReport,LPSTR ATitle):TDialog(AParent,AName)
 {
  strcpy(szReportTitle,ATitle);
  strcpy(szFileName,AReport);
 };

 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL]{ShutDownWindow();cb.closeAll();};
 virtual void Search         (RTMessage Msg) = [ID_FIRST + ID_SEARCH];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 void CreateOpeningBalance();
 double dGetOpenBalance(int lPeriod);
 void PrintReport(char *szFileName,char *szReportTitle,char *szPeriod);
};

double TRptDialog::dGetOpenBalance(int lPeriod)
{
 double dOpenBalance = 0;
 switch (lPeriod)
  {
   case 1:  dOpenBalance = glaccts->Curr_bal() - glaccts->dPeriod1(); break;
   case 2:  dOpenBalance = glaccts->Curr_bal() - glaccts->dPeriod2(); break;
   case 3:  dOpenBalance = glaccts->Curr_bal() - glaccts->dPeriod3(); break;
   case 4:  dOpenBalance = glaccts->Curr_bal() - glaccts->dPeriod4(); break;
   case 5:  dOpenBalance = glaccts->Curr_bal() - glaccts->dPeriod5(); break;
   case 6:  dOpenBalance = glaccts->Curr_bal() - glaccts->dPeriod6(); break;
   case 7:  dOpenBalance = glaccts->Curr_bal() - glaccts->dPeriod7(); break;
   case 8:  dOpenBalance = glaccts->Curr_bal() - glaccts->dPeriod8(); break;
   case 9:  dOpenBalance = glaccts->Curr_bal() - glaccts->dPeriod9(); break;
   case 10: dOpenBalance = glaccts->Curr_bal() - glaccts->dPeriod10(); break;
   case 11: dOpenBalance = glaccts->Curr_bal() - glaccts->dPeriod11(); break;
   case 12: dOpenBalance = glaccts->Curr_bal() - glaccts->dPeriod12(); break;
   default: dOpenBalance = 0;
  }
 return dOpenBalance;
}

void TRptDialog::CreateOpeningBalance()
{
 HourGlassOn();
 glaccts = new Tglaccts(&cb);
 glaccts->SetIndex("accounts");
 glopen = new Tglopen(&cb);
 glopen->zap();
 glopen->pack();
 int lPeriod = GetCurrentPeriodn();
 Relate4set* rQuery = new Relate4set(glaccts);
 rQuery->querySet("ACCOUNT <> ' '");
 for(int qc = rQuery->top();qc!=r4eof;qc=rQuery->skip(1L) )
  {
   glopen->appendStart();
   glopen->accounts->assign(glaccts->accounts->str());
   glopen->dept->assign(glaccts->dept->str());
   glopen->period->assignDouble(lPeriod);
   glopen->openbal->assignDouble(dGetOpenBalance(lPeriod));
   glopen->append();
  }
 HourGlassOff();
 cb.closeAll();

}

void TRptDialog::SetupWindow()
 {
 TDialog::SetupWindow();
 HourGlassOn();
 SetCaption(szReportTitle);
 CreateOpeningBalance();    // Create new opening balance dbase and close all databases.
 glaccts = new Tglaccts(&cb);
 glaccts->SetIndex("accounts");
 glaccts->top();
 SendDlgItemMsg(ID_ITEMSTART,WM_SETTEXT,NULL,(LPARAM)(LPSTR)glaccts->accounts->str());
 glaccts->bottom();
  SendDlgItemMsg(ID_ITEMEND,WM_SETTEXT,NULL,(LPARAM)(LPSTR)glaccts->accounts->str());
 GLACCTS = new TGLAcctList(this,"GL_ACCTLIST",glaccts);
 GLACCTS->EnableAutoCreate(); 
 GetApplication()->MakeWindow(GLACCTS);
 HourGlassOff();
 };


void TRptDialog::Search(RTMessage)
{
 if(IsWindowVisible(GLACCTS->HWindow))
  GLACCTS->Show(SW_HIDE);
  else
  GLACCTS->Show(SW_SHOW);
}


void TRptDialog::Ok(RTMessage)
{
  char szPeriod[3];
  long lCurrentPeriod = GetCurrentPeriodn();
  ltoa(lCurrentPeriod,szPeriod,10);
  PrintReport(szFileName,szReportTitle,szPeriod);
}

#pragma argsused
void TRptDialog::PrintReport (char *szFilename,char *szReportTitle,char *szPeriod)
{
  HourGlassOn();
  EnableWindow(HWindow,FALSE);
  char StartItem[10];
  char EndItem[10];
  char *Start = "'1010000'";
  char *End   = "'1010000'";
  char *Period = "'2'";

  GetDlgItemText(HWindow,ID_ITEMSTART,StartItem,sizeof(StartItem));
  if(*StartItem == 0){
   
   EnableWindow(HWindow,TRUE);
   return;
  }   
  GetDlgItemText(HWindow,ID_ITEMEND,EndItem,sizeof(EndItem));
  if(*EndItem == 0){
   
   EnableWindow(HWindow,TRUE);
   return;
  }   

  strcpy(Start,"'"); strcat(Start,StartItem); strcat(Start,"'");
  strcpy(End,"'");  strcat(End,EndItem);  strcat(End,"'");
  strcpy(Period,"'"); strcat(Period,szPeriod); strcat(Period,"'");

  short JobNumber = PEOpenPrintJob(szFileName);

  if (JobNumber == 0){
   
   EnableWindow(HWindow,TRUE);
   return; 
   }


 
  if(!PESetFormula (JobNumber,"Starting Account",Start))
   {
    
    PEClosePrintJob(JobNumber);
    EnableWindow(HWindow,TRUE);
    return ;
   }

   if(!PESetFormula (JobNumber,"Current Period",Period))
   {
    
    PEClosePrintJob(JobNumber);
    EnableWindow(HWindow,TRUE);
    return ;
   }


  if(!PESetFormula (JobNumber,"Ending Account",End))
   {
    
    PEClosePrintJob(JobNumber);
    EnableWindow(HWindow,TRUE);
    return ;
   }

  PEOutputToWindow(JobNumber,szReportTitle,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL
  ,GetApplication()->MainWindow->HWindow);

  PEStartPrintJob(JobNumber,TRUE);
  PEClosePrintJob(JobNumber);
  HourGlassOff();
  EnableWindow(HWindow,TRUE);
  ShutDownWindow();

}
