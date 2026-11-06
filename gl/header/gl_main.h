#ifndef __GL_MAIN_H
#define __GL_MAIN_H
#include "d4all.hpp"

extern char szUser[11];

BOOL CheckFile(Code4 *cb,char *szFileName,PTWindowsObject AWindow);
BOOL PrintCR(HWND HWindow, char *szReportName);
int GetCurrentPeriodn();
char *GetString(HINSTANCE,int ID);

_CLASSDEF (TToolBar)
class TToolBar;

class TVToolBar;

_CLASSDEF (TStatBar)
class TStatBar;

_CLASSDEF (TGlWindow)
class TGlWindow:public TMDIFrame
{
 public:
 char Period[25];
 HWND HToolBar;
 TVToolBar *VToolBar;
 HWND HStatusBar;
 TGlWindow(LPSTR ATitle,LPSTR MenuName);
 PTToolBar ToolBar;
 PTStatBar Status;
 //PTWorksheet Worksheet;

 char  *GetCurrentPeriod ();
 char  *GetCompanyName();
 void CMDialogPrint (RTMessage Msg) = [CM_FIRST + CM_DIALOGPRINT];
 void PopUpMenu (RTMessage Msg)      = [WM_FIRST + WM_POPUPMENU];
 void InitClientWindow() { ClientWnd = new TGLClient(this); }
 void CMHelp    (RTMessage Msg)     = [CM_FIRST + ID_HELP];
 virtual void CMAbout   (RTMessage Msg)     = [CM_FIRST + ID_ABOUT];
 virtual void SendStatusMsg (int ID);
 virtual void WMMenuSelect(RTMessage Msg)   = [WM_FIRST + WM_MENUSELECT];

 // Global Menu items
 // General Ledger 


 // General Ledger virtual functions
 virtual void GLAcctAdd      (RTMessage Msg)   = [CM_FIRST + ID_ACCTADD];
 virtual void GLAcctDel      (RTMessage Msg)   = [CM_FIRST + ID_ACCTDEL];
 virtual void GLDeptCodes    (RTMessage Msg)   = [CM_FIRST + ID_DEPTCODES];
 virtual void GLAcctBudget   (RTMessage Msg)   = [CM_FIRST + ID_ACCTBUDGET];
 virtual void GLChartInquiry (RTMessage Msg)   = [CM_FIRST + ID_GLCHARTINQ];
 virtual void GLTransInquiry (RTMessage Msg)   = [CM_FIRST + ID_GLTRANSINQ];
 virtual void GLBudgetInquiry(RTMessage Msg)   = [CM_FIRST + ID_GLBUDGETINQ];
 virtual void GLBatchEnt     (RTMessage Msg)   = [CM_FIRST + ID_BATCHENT];
 virtual void GLPostBatch    (RTMessage Msg)   = [CM_FIRST + ID_GLPOSTBATCH];
 virtual void GLRepBatch     (RTMessage Msg)   = [CM_FIRST + ID_GLREPBATCH];
 virtual void GLPrtDistList  (RTMessage Msg)   = [CM_FIRST + ID_DISTLIST];
 virtual void GLIncomeSt     (RTMessage Msg)   = [CM_FIRST + ID_FININCOME];
 virtual void GLBalance      (RTMessage Msg)   = [CM_FIRST + ID_FINBS];
 virtual void GLManAccounts  (RTMessage Msg)   = [CM_FIRST + ID_MANUFACTURE];
 virtual void GLFinRE	     (RTMessage Msg)   = [CM_FIRST + ID_FINRE];
 virtual void GLFinFA        (RTMessage Msg)   = [CM_FIRST + ID_FINFA];
 virtual void GLFinDep       (RTMessage Msg)   = [CM_FIRST + ID_FINDEP];
 virtual void GLPrtPrePost   (RTMessage Msg)   = [CM_FIRST + ID_PREPOST];

 virtual void GLDeleteBatch  (RTMessage Msg)   = [CM_FIRST + ID_BATCHDELETE];
 virtual void GLCloseMonth   (RTMessage Msg)   = [CM_FIRST + ID_CLOSEM];
 virtual void GLPrtSlipSummary (RTMessage Msg) = [CM_FIRST + ID_SLIPSUM];
 virtual void GLPrtSlip      (RTMessage Msg)   = [CM_FIRST + ID_SLIP];
 virtual void GLPrtSlipDetail(RTMessage Msg)   = [CM_FIRST + CM_SLIPDETAIL];
 virtual void GLTrialBalance (RTMessage Msg)   = [CM_FIRST + ID_TB];
 virtual void GLTrialByDept  (RTMessage Msg)   = [CM_FIRST + ID_TBBYDEPT];
 virtual void GLAccounts     (RTMessage Msg)   = [CM_FIRST + ID_ACCOUNT];
 virtual void GLModifyBatch  (RTMessage Msg)   = [CM_FIRST + ID_MODBATCH];
 virtual void GLCustomReport (RTMessage Msg)   = [CM_FIRST + ID_PRINT];
 virtual void GLReindex      (RTMessage Msg)   = [CM_FIRST + ID_REINDEX];
 virtual void GLBackup       (RTMessage Msg)   = [CM_FIRST + ID_BACKUP];
 virtual void GLRestore      (RTMessage Msg)   = [CM_FIRST + ID_RESTORE];
 virtual void GLOpenSheet    (RTMessage Msg)   = [CM_FIRST + ID_OPEN];
 virtual void GLUpdateAP     (RTMessage Msg)   = [CM_FIRST + 1028];
 virtual void GLUpdateAR     (RTMessage Msg)   = [CM_FIRST + 1029];
 virtual void GLYearEnd      (RTMessage Msg)   = [CM_FIRST + ID_YEAREND];
 virtual void GLCompanySet   (RTMessage Msg)   = [CM_FIRST + ID_COMPANYPROFILE];
 virtual void AccountsPayable(RTMessage Msg)   = [CM_FIRST + CM_ACCOUNT_PAYABLE];
 virtual void AccountsReceive(RTMessage Msg)   = [CM_FIRST + CM_ACCOUNT_RECEIVABLE];
 virtual void GLPrior        (RTMessage Msg)   = [CM_FIRST + CM_PRIOR];
 virtual LPSTR GetClassName(){ return "GLWindow"; }
 virtual void GetWindowClass(WNDCLASS& wc);
 virtual void WMSize        (RTMessage Msg)   = [WM_FIRST + WM_SIZE];
 virtual void New           (RTMessage Msg)   = [CM_FIRST + 111];
 BOOL PrintCR(char *szReportName);
 void MakeChild(LPSTR TITLE);
 HWND GLCreateStatusBar();
 HWND GlCreateToolBar(HWND HWindow);
 void SetupWindow();
};
#endif