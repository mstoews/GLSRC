#ifndef __GL_MAIN_H
#define __GL_MAIN_H
#include "d4all.hpp"
#include "gl_user.h"
#include "crpe.h"
extern char szUser[11];

BOOL CheckFile(Code4 *cb,char *szFileName,PTWindowsObject AWindow);
BOOL PrintCR(HWND HWindow, char *szReportName);
int GetCurrentPeriodn();
char *GetString(HINSTANCE,int ID);

class CHourGlass
 {
  TWindow* m_pCapWnd;
  HCURSOR  m_hSaveCursor;
  public:
  CHourGlass(TWindow *pWnd=NULL);
  ~CHourGlass();
 };

class TAcctModule :public TApplication
{
 private:
 UINT iPeriod;
 POINT pt;
 int iFontSize;
 int iSytle;
 char szApp[20];
 BOOL crpeOpen;
 LOGFONT 	lfFont;

 public:
  Code4 cb;
  char szCompanyName[40];
  TAcctModule(LPSTR AName, HINSTANCE hInstance, HINSTANCE hPrevInstance,
	 LPSTR lpCmdLine, int nCmdShow)
	 : TApplication(AName, hInstance, hPrevInstance, lpCmdLine, nCmdShow){}
  virtual void InitMainWindow();
  UINT GetPeriod(){return iPeriod;}
  void SetPoint(int x,int y) {pt.x = x; pt.y = y;}
  POINT GetPoint(){ return pt;}
  void SetFontStyle();
  LPLOGFONT GetFontStyle() {return &lfFont;}
  char *GetAppName(){return szApp;}
  void SetAppName(char* szName){strcpy(szApp,szName);}
  HWND GetMainWin(){ return MainWindow->HWindow; }
  Code4* GetCodeBase()
	{
	  return &cb;
	}
  ~TAcctModule();
};


_CLASSDEF (TToolBar)
class TToolBar;

_CLASSDEF (TStatBar)
class TStatBar;

_CLASSDEF (TWnd)
class TWnd;

_CLASSDEF (TPrintJob)
class TPrintJob : public TWindow
{
  private:
  HWND hReportWindow;
  string strReportFile;
  short printJob;
  public:

  TPrintJob(PTWindowsObject AWindow,LPSTR szReportFile) : TWindow(AWindow,szReportFile)
	{
	  printJob = NULL;
	  strReportFile = szReportFile;
	}
  virtual void WMSize (RTMessage Msg) = [WM_FIRST + WM_SIZE];
  virtual void SetupWindow();
  string* GetReportName(){ return &strReportFile; }

 ~TPrintJob()
  {
	 //if(printJob != NULL)
	 //	PEClosePrintJob(printJob);
  }

 void PrintReport();


};

_CLASSDEF (TGlWindow)
class TGlWindow:public TMDIFrame
{
 public:
 char Period[25];
 TWnd* WndXls;
 TPrintJob* printJob;
 HWND HToolBar;
 HWND HStatusBar;
 TGlWindow(LPSTR ATitle,LPSTR MenuName);

 short JobNumber;

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


 // General Ledger virtual functions
 virtual void GLAcctAdd      (RTMessage Msg)   = [CM_FIRST + ID_ACCTADD];
 virtual void GLAcctDel      (RTMessage Msg)   = [CM_FIRST + ID_ACCTDEL];
 virtual void GLDeptCodes    (RTMessage Msg)   = [CM_FIRST + ID_DEPTS];
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
 virtual void GLCreateIndex  (RTMessage Msg)   = [CM_FIRST + ID_CREATEINDEX];
 virtual void GLMonthEndClose(RTMessage Msg)   = [CM_FIRST + ID_MONTHENDCLOSE];

 virtual void GLBalanceBudget (RTMessage Msg)  = [CM_FIRST + 1025];
 virtual void GLBalanceQuarterly (RTMessage Msg) = [CM_FIRST + 1102];
 virtual void GLIncomeYTD		(RTMessage Msg) = [CM_FIRST + 1100];
 virtual void GLIncomeQuarterly (RTMessage Msg) = [CM_FIRST + 1101];
 virtual void GLIncomeDepartment (RTMessage Msg) = [CM_FIRST + 1103];
 virtual void GLIncomeCustomer (RTMessage Msg) = [CM_FIRST + 1104];
 virtual void GLIncomeCustomerDetail (RTMessage Msg) = [CM_FIRST + 1105];
 virtual void GLIncomeExpense (RTMessage Msg) = [CM_FIRST + 1106];
 virtual void GLIncomeExpenseDetail (RTMessage Msg) = [CM_FIRST + 1107];
 virtual void GLIncomeBudget   (RTMessage Msg) = [CM_FIRST + 1108];




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
 virtual void GLReportDialog (RTMessage Msg)   = [CM_FIRST + CM_REPORTDIALOG];
 virtual void GLTrialBalanceByAccountAndPeriod (RTMessage Msg)   = [CM_FIRST + CM_TRIALBALANCE_BY_ACCTS];


 //virtual void Print        (RTMessage Msg) =[CM_FIRST + ID_PRINT_REPORT];
 //virtual void NextPage     (RTMessage Msg) =[CM_FIRST + ID_NEXT_PAGE];
 //virtual void PrevPage     (RTMessage Msg) =[CM_FIRST + ID_PREV_PAGE];
 //virtual void LastPage     (RTMessage Msg) =[CM_FIRST + ID_LAST_PAGE];
 //virtual void FirstPage    (RTMessage Msg) =[CM_FIRST + ID_FIRST_PAGE];
 //virtual void Export       (RTMessage Msg) =[CM_FIRST + ID_EXPORT];


 void PrintReport(char *szReportName,HWND HWindow);
 virtual LPSTR GetClassName(){ return "GLWindow"; }
 virtual void GetWindowClass(WNDCLASS& wc);
 virtual void WMSize        (RTMessage Msg)   = [WM_FIRST + WM_SIZE];
 virtual void New           (RTMessage Msg)   = [CM_FIRST + 111];
 char *szGetPeriod();
 unsigned int iGetPeriod();
 BOOL PrintCR(char *szReportName);
 void MakeChild(LPSTR TITLE);
 HWND GLCreateStatusBar();
 HWND GlCreateToolBar(HWND HWindow);
 void SetupWindow();
};
#endif