
#include "ap.h"
//#include "gl_acct.hpp"
#include "gl_pds.hpp"
#include "gl_setat.hpp"
#include "gl_prof.hpp"
#include "utildlg.h"

#define ID_PERIOD_DESC 1084
#define ID_DATA_ERROR 60


extern char szApp[20];
extern Code4 cb;
extern HINSTANCE hInst;
/*
char* szGetPeriod();
void PrintReport(char *szReportName,HWND HWindow);
char *GetCompany();
int  iGetPeriod();
char *ReportFileName(char *body,int type = 0);
void SetPeriods(HWND hWnd);
BOOL ValidateGL(LPSTR lpAccount,LPSTR lpDepart);
*/


int GetPeriod(HWND hWnd)
{
 int rc = 0;
 char szPeriod[3];
 DWORD Indx = SendDlgItemMessage(hWnd,ID_PERIOD,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
	{
	 SendDlgItemMessage(hWnd,ID_PERIOD,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szPeriod);
	 rc = atoi(szPeriod);
	}
 return rc;
}

char *ReportFileName(char *body,int type)
{
  static char szFile[12];
  switch(type)
  {
	case 0 : strcpy(szFile,"gl");break;
	case 1 : strcpy(szFile,"ap");break;
	case 2 : strcpy(szFile,"ar");break;
  }
  strcat(szFile,body);
  strcat(szFile,".rpt");
  return szFile;
}

void TGlWindow::GLTrialByDept    (RTMessage){PrintReport(ReportFileName("dept",0),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLPrtPrePost     (RTMessage){PrintReport(ReportFileName("pre",0),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLPrtSlipSummary (RTMessage){PrintReport(ReportFileName("_btchs",0),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLPrtSlip        (RTMessage){PrintReport(ReportFileName("slip",0),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLPrtSlipDetail  (RTMessage){PrintReport(ReportFileName("btchd",0),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLTrialBalance   (RTMessage){PrintReport(ReportFileName("tb",0),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLAccounts       (RTMessage){PrintReport(ReportFileName("accts",0),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLPrtDistList    (RTMessage){PrintReport(ReportFileName("_dist",0),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLChartInquiry   (RTMessage){PrintReport(ReportFileName("_chart",0),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLBudgetInquiry  (RTMessage){PrintReport(ReportFileName("budgt",0),GetApplication()->MainWindow->HWindow);}
void TManilaGL::GLDoctorRpt      (RTMessage){PrintReport(ReportFileName("_drcds",0),GetApplication()->MainWindow->HWindow);}
void TManilaGL::GLPrePosting     (RTMessage){PrintReport(ReportFileName("_prept",0),GetApplication()->MainWindow->HWindow);}
void TManilaGL::GLMedRepsList    (RTMessage){PrintReport(ReportFileName("_medrp",0),GetApplication()->MainWindow->HWindow);}
void TManilaGL::GLProductList    (RTMessage){PrintReport(ReportFileName("_prodl",0),GetApplication()->MainWindow->HWindow);}

void TManilaGL::APPrintAdj       (RTMessage){PrintReport(ReportFileName("adjst",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APSummary        (RTMessage){PrintReport(ReportFileName("_bala",1),GetApplication()->MainWindow->HWindow) ; }
void TManilaGL::APOverDue        (RTMessage){PrintReport(ReportFileName("_overd",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APAgedCash       (RTMessage){PrintReport(ReportFileName("_agedc",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APAgedRetain     (RTMessage){PrintReport(ReportFileName("_agedr",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APBatchStatus    (RTMessage){PrintReport(ReportFileName("_bstat",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APPostJournal    (RTMessage){PrintReport(ReportFileName("_postj",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APRebateSummary  (RTMessage){PrintReport(ReportFileName("_reb",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APVatAPReport    (RTMessage){PrintReport(ReportFileName("_vatap",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APVatGLReport    (RTMessage){PrintReport(ReportFileName("_vatgl",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APChequeCash     (RTMessage){PrintReport(ReportFileName("_cheqc",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APJournalCash    (RTMessage){PrintReport(ReportFileName("_jsuma",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APJournalAP      (RTMessage){PrintReport(ReportFileName("_jsumc",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APBatchCash      (RTMessage){PrintReport(ReportFileName("_cashd",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APBatchAP        (RTMessage){PrintReport(ReportFileName("_btchd",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APGLSummary      (RTMessage){PrintReport(ReportFileName("_jsum",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APGLDetail       (RTMessage){PrintReport(ReportFileName("_jsumd",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APAdjBatchPrt    (RTMessage){PrintReport(ReportFileName("_adjbt",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APPrePayment     (RTMessage){PrintReport(ReportFileName("_prepy",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APRequestSummary (RTMessage){PrintReport(ReportFileName("_cheq",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APRequestDetail  (RTMessage){PrintReport(ReportFileName("_reqsd",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APVenLists       (RTMessage){PrintReport(ReportFileName("_vend",1),GetApplication()->MainWindow->HWindow);}
void TManilaGL::APOpenAccts      (RTMessage){PrintReport(ReportFileName("_detl",1),GetApplication()->MainWindow->HWindow);}
void TManilaGL::APPrintBatch     (RTMessage){PrintReport(ReportFileName("btchsm",1),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APPrintCash      (RTMessage){PrintReport(ReportFileName("_cashb",1),GetApplication()->MainWindow->HWindow);}


void TManilaGL::ARVenLists      (RTMessage){PrintReport(ReportFileName("_vend",2),GetApplication()->MainWindow->HWindow);}
void TManilaGL::AROpenAccts     (RTMessage){PrintReport(ReportFileName("_detl",2),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARSummary       (RTMessage){PrintReport(ReportFileName("_bala",2),GetApplication()->MainWindow->HWindow);}
void TManilaGL::AROverDue       (RTMessage){PrintReport(ReportFileName("_agedc",2),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARAgedCash      (RTMessage){PrintReport(ReportFileName("_agedc",2),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARAgedRetain    (RTMessage){PrintReport(ReportFileName("_agedr",2),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARBatchStatus   (RTMessage){PrintReport(ReportFileName("_bstat",2),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARVatARReport   (RTMessage){PrintReport(ReportFileName("_vatap",2),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARVatGLReport   (RTMessage){PrintReport(ReportFileName("_vatgl",2),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARPrintBatch    (RTMessage){PrintReport(ReportFileName("_batch",2),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARReceipt       (RTMessage){PrintReport(ReportFileName("_recei",2),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARBatchAR	(RTMessage){PrintReport(ReportFileName("_btchd",2),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARGLSummary	(RTMessage){PrintReport(ReportFileName("_jsum",2),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARGLDetail	(RTMessage){PrintReport(ReportFileName("_jsumd",2),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARCustSummary	(RTMessage){PrintReport(ReportFileName("_cust",2),GetApplication()->MainWindow->HWindow);}

void PrintReport(char *szReportName,HWND HWindow)
 {
  PEOpenEngine();
  HANDLE ReportHandle,SelectionFormula;

  short JobNumber = PEOpenPrintJob(szReportName);
  if(JobNumber != 0)
  {

	char   *szReportCaption;
	short  iReportTitleLen;

	char   *szReportSelection;
	short  iSelectionLen;

	PEGetReportTitle(JobNumber,&ReportHandle,&iReportTitleLen);
	szReportCaption = (char *)malloc(iReportTitleLen);

	PEGetSelectionFormula(JobNumber,&SelectionFormula,&iSelectionLen);
	szReportSelection = (char *)malloc(iSelectionLen);

	char *szCompanyName = new char[41];
	char *szPeriod = new char[4];

	wsprintf(szCompanyName,"'%s'",GetCompany());
	wsprintf(szPeriod,"'%d'",iGetPeriod());

	PESetFormula (JobNumber,"Company Name",szCompanyName);
   PESetFormula (JobNumber,"Current Period",szPeriod);

	PEGetHandleString(ReportHandle,szReportCaption,iReportTitleLen);
   PEGetHandleString(SelectionFormula,szReportSelection,iSelectionLen);
   if(strlen(szReportSelection) > 0)
	{
	 //MessageBox(HWindow,szReportSelection,"",MB_OK);
	 //PTDialog TheDialog;
    //TheDialog = new TDialog(HWindow,"Test",DLLModule);

	 PESetSelectionFormula(JobNumber,szReportSelection);
	}

	PEOutputToWindow(JobNumber,szReportCaption,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,HWindow);
	PEStartPrintJob(JobNumber,TRUE);
	PEClosePrintJob(JobNumber);
	delete [] szCompanyName;
	delete [] szPeriod;
	free(szReportCaption);
	return;
  }
  else
  {
  HANDLE textHandle;
  char   *textBuffer;
  short   textLength;
  PEGetErrorText(JobNumber,&textHandle,&textLength);
  textBuffer = (char *)malloc(textLength+20);
  PEGetHandleString(textHandle,textBuffer,textLength);
  strcat(textBuffer,szReportName);
  MessageBox(HWindow,textBuffer,szApp,MB_OK);
  free(textBuffer);
  return;
  }
}

char* GetCompany()
{
 Code4 cb;
 GL_PROF gl_prof;
 gl_prof.open(cb);
 char szCompany[41];
 gl_prof.top();
 if(gl_prof.isValid())
	 strcpy(szCompany,gl_prof.company.str());
 gl_prof.close();
 return szCompany;
}

int iGetPeriod()
{
 Code4 cb;
 GL_PDS gl_pds;
 gl_pds.open(cb);
 gl_pds.SetIndex("Period");
 gl_pds.bottom();
 int iPeriod;
 while(!gl_pds.bof())
  {
	if(strcmp(gl_pds.status.str(),"0") == 0)
	 iPeriod = atoi(gl_pds.period.str());
	gl_pds.skip(-1);
  }
 gl_pds.close();

 return iPeriod;
}

char* szGetPeriod()
{
 Code4 cb;
 GL_PDS gl_pds;
 gl_pds.open(cb);
 gl_pds.SetIndex("Period");
 gl_pds.bottom();
 char szPeriod[11];
 while(!gl_pds.bof())
  {
	if(strcmp(gl_pds.status.str(),"0") == 0)
	 strcpy(szPeriod,gl_pds.pd_desc.str());
	gl_pds.skip(-1);
  }
 gl_pds.close();

 return szPeriod;
}

void SetPeriods(HWND hWnd)
{
 DWORD dwIndex;
 GL_PDS gl_pds;
 gl_pds.open(cb);
 gl_pds.SetIndex("PERIOD");
 gl_pds.bottom();
 while(!gl_pds.bof())
  {
	if(strcmp(gl_pds.status.str(),"0") == 0)
	  dwIndex = SendDlgItemMessage(hWnd,ID_PERIOD,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)gl_pds.period.str());
	gl_pds.skip(-1);
  }

 char szPeriod[3];
 SendDlgItemMessage(hWnd,ID_PERIOD,CB_SETCURSEL,(WPARAM)dwIndex,NULL);
 SendDlgItemMessage(hWnd,ID_PERIOD,WM_GETTEXT,sizeof(szPeriod),(LPARAM)(LPSTR)szPeriod);
 if(gl_pds.seek(szPeriod) == 0)
	SendDlgItemMessage(hWnd,ID_PERIOD_DESC,WM_SETTEXT,NULL, (LPARAM)(LPSTR)gl_pds.pd_desc.str());
 gl_pds.close();
}




BOOL ValidateGL(LPSTR lpAccount,LPSTR lpDepart)
{
 Code4 cb;
 BOOL bValid = FALSE;
 Data4 glaccts;
 glaccts.open(cb,"GLACCTS");
 glaccts.SetIndex("ACCOUNTS");
 Field4 account(glaccts,1);
 Field4 dept(glaccts,2);

 if(glaccts.isValid())
 {
  string szSeek(lpAccount);
  szSeek += lpDepart;
  if(glaccts.seek(szSeek))
	bValid = TRUE;
 }
 glaccts.close();
 return bValid;
}


BOOL CheckFile(char *szFileName,HWND hWnd)
{
 Code4 cb;
 cb.autoOpen = FALSE;
 BOOL bFile = TRUE;

 Data4 db,gl_tag;
 Index4 idx;

 int rc = db.open(cb,szFileName);
 if(rc != 0)
  {
	wsprintf(szMsg,GetString(hInst,ID_DATA_ERROR),e4text(rc),szFileName);
	MessageBox(hWnd,szMsg,szApp,MB_ICONINFORMATION);
	bFile = FALSE;
  }


 if(bFile)
 {
  rc = idx.open(db,szFileName);
  if(rc != 0)
  {
	wsprintf(szMsg,GetString(hInst,ID_DATA_ERROR),e4text(rc),szFileName);
	MessageBox(hWnd,szMsg,szApp,MB_ICONINFORMATION);
	bFile = FALSE;
  }
 }

 cb.closeAll();
 cb.init();
 return bFile;
}

