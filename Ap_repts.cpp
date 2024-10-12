#include "ap.h"
#include "gl_pds.hpp"
#include "gl_setat.hpp"
#include "gl_prof.hpp"
#include "utildlg.h"

#define ID_PERIOD_DESC 1084
#define ID_DATA_ERROR 60
#define ID_DATA_ERROR_MSG 61

#define IDD_STATUSBAR_MENUHELP 2


extern HINSTANCE hInst;
char* GetName();
static char szCompanyName[40];



long GetPeriod(HWND hWnd)
{
 long rc = 0;
 char szPeriod[3];
 int Indx = (int)SendDlgItemMessage(hWnd,ID_PERIOD,CB_GETCURSEL,0,0);
  if(Indx >= 0)
	{
	 SendDlgItemMessage(hWnd,ID_PERIOD,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM)(LPSTR)szPeriod);
	 rc = atol(szPeriod);
	}
 return rc;
}

char *ReportFileName(int type)
{
  static string szFile;
  szFile = "";
  szFile = GetString(hInst,type);
  szFile += ".rpt";
  return (LPSTR)StrTrim((LPSTR)szFile.c_str());
}

void TGlWindow::GLTrialByDept    (RTMessage){PrintReport(ReportFileName(18003),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLPrtPrePost     (RTMessage){PrintReport(ReportFileName(18004),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLPrtSlipSummary (RTMessage){PrintReport(ReportFileName(18005),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLPrtSlip        (RTMessage){PrintReport(ReportFileName(18006),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLPrtSlipDetail  (RTMessage){PrintReport(ReportFileName(18007),GetApplication()->MainWindow->HWindow);}

void TGlWindow::GLPrtDistList    (RTMessage){PrintReport(ReportFileName(18010),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLChartInquiry   (RTMessage){PrintReport(ReportFileName(18011),GetApplication()->MainWindow->HWindow);}
void TGlWindow::GLBudgetInquiry  (RTMessage){PrintReport(ReportFileName(18012),GetApplication()->MainWindow->HWindow);}
void TManilaGL::GLDoctorRpt      (RTMessage){PrintReport(ReportFileName(18013),GetApplication()->MainWindow->HWindow);}
void TManilaGL::GLPrePosting     (RTMessage){PrintReport(ReportFileName(18014),GetApplication()->MainWindow->HWindow);}
void TManilaGL::GLMedRepsList    (RTMessage){PrintReport(ReportFileName(18015),GetApplication()->MainWindow->HWindow);}
void TManilaGL::GLProductList    (RTMessage){PrintReport(ReportFileName(18016),GetApplication()->MainWindow->HWindow);}

void TManilaGL::APPrintAdj       (RTMessage){PrintReport(ReportFileName(18017),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APSummary        (RTMessage){PrintReport(ReportFileName(18018),GetApplication()->MainWindow->HWindow) ; }
void TManilaGL::APOverDue        (RTMessage){PrintReport(ReportFileName(18019),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APAgedCash       (RTMessage){PrintReport(ReportFileName(18020),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APAgedRetain     (RTMessage){PrintReport(ReportFileName(18021),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APBatchStatus    (RTMessage){PrintReport(ReportFileName(18022),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APPostJournal    (RTMessage){PrintReport(ReportFileName(18023),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APRebateSummary  (RTMessage){PrintReport(ReportFileName(18024),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APVatAPReport    (RTMessage){PrintReport(ReportFileName(18025),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APVatGLReport    (RTMessage){PrintReport(ReportFileName(18026),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APChequeCash     (RTMessage){PrintReport(ReportFileName(18027),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APJournalCash    (RTMessage){PrintReport(ReportFileName(18028),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APJournalAP      (RTMessage){PrintReport(ReportFileName(18029),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APBatchCash      (RTMessage){PrintReport(ReportFileName(18030),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APBatchAP        (RTMessage){PrintReport(ReportFileName(18031),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APGLSummary      (RTMessage){PrintReport(ReportFileName(18032),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APGLDetail       (RTMessage){PrintReport(ReportFileName(18033),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APAdjBatchPrt    (RTMessage){PrintReport(ReportFileName(18034),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APPrePayment     (RTMessage){PrintReport(ReportFileName(18035),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APRequestSummary (RTMessage){PrintReport(ReportFileName(18036),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APRequestDetail  (RTMessage){PrintReport(ReportFileName(18037),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APVenLists       (RTMessage){PrintReport(ReportFileName(18038),GetApplication()->MainWindow->HWindow);}
void TManilaGL::APOpenAccts      (RTMessage){PrintReport(ReportFileName(18039),GetApplication()->MainWindow->HWindow);}
void TManilaGL::APPrintBatch     (RTMessage){PrintReport(ReportFileName(18040),GetApplication()->MainWindow->HWindow); }
void TManilaGL::APPrintCash      (RTMessage){PrintReport(ReportFileName(18041),GetApplication()->MainWindow->HWindow);}
void TManilaGL::APChequeSummary  (RTMessage){PrintReport(ReportFileName(18042),GetApplication()->MainWindow->HWindow);}


void TManilaGL::ARVenLists      (RTMessage){PrintReport(ReportFileName(18043),GetApplication()->MainWindow->HWindow);}
void TManilaGL::AROpenAccts     (RTMessage){PrintReport(ReportFileName(18044),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARSummary       (RTMessage){PrintReport(ReportFileName(18045),GetApplication()->MainWindow->HWindow);}
void TManilaGL::AROverDue       (RTMessage){PrintReport(ReportFileName(18046),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARAgedCash      (RTMessage){PrintReport(ReportFileName(18047),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARAgedRetain    (RTMessage){PrintReport(ReportFileName(18048),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARBatchStatus   (RTMessage){PrintReport(ReportFileName(18049),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARVatARReport   (RTMessage){PrintReport(ReportFileName(18050),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARVatGLReport   (RTMessage){PrintReport(ReportFileName(18051),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARPrintBatch    (RTMessage){PrintReport(ReportFileName(18052),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARReceipt       (RTMessage){PrintReport(ReportFileName(18053),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARBatchAR		  (RTMessage){PrintReport(ReportFileName(18054),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARGLSummary	  (RTMessage){PrintReport(ReportFileName(18055),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARGLDetail	     (RTMessage){PrintReport(ReportFileName(18056),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARCustSummary	  (RTMessage){PrintReport(ReportFileName(18057),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARStatements    (RTMessage){PrintReport(ReportFileName(18058),GetApplication()->MainWindow->HWindow);}
void TManilaGL::ARStatementDet  (RTMessage){PrintReport(ReportFileName(18059),GetApplication()->MainWindow->HWindow);}


 void TGlWindow::GLBalanceBudget (RTMessage) { PrintReport("gl_balb.rpt",GetApplication()->MainWindow->HWindow);}
 void TGlWindow::GLBalanceQuarterly (RTMessage) { PrintReport("gl_balq.rpt",GetApplication()->MainWindow->HWindow);}
 void TGlWindow::GLIncomeYTD		(RTMessage) { PrintReport("gl_ytd.rpt",GetApplication()->MainWindow->HWindow);}
 void TGlWindow::GLIncomeQuarterly (RTMessage) { PrintReport("gl_incq.rpt",GetApplication()->MainWindow->HWindow);}
 void TGlWindow::GLIncomeDepartment (RTMessage) { PrintReport("gl_incd.rpt",GetApplication()->MainWindow->HWindow);}
 void TGlWindow::GLIncomeCustomer (RTMessage) { PrintReport("gl_incct.rpt",GetApplication()->MainWindow->HWindow);}
 void TGlWindow::GLIncomeCustomerDetail (RTMessage) { PrintReport("gl_incdt.rpt",GetApplication()->MainWindow->HWindow);}
 void TGlWindow::GLIncomeExpense (RTMessage) { PrintReport("gl_incex.rpt",GetApplication()->MainWindow->HWindow);}
 void TGlWindow::GLIncomeExpenseDetail (RTMessage) { PrintReport("gl_incxd.rpt",GetApplication()->MainWindow->HWindow);}
 void TGlWindow::GLIncomeBudget (RTMessage) { PrintReport("gl_incb.rpt",GetApplication()->MainWindow->HWindow);}



void TGlWindow::PrintReport(char *szReportName,HWND HWindow)
 {
	CHourGlass wait;
	PEOpenEngine();

	string strCompany(hInst,4094);
	string strCurrentPd(hInst,4095);

	string strReportName("Current Report : ");
	strReportName += szReportName;

	SendMessage(HStatusBar, SBRM_SETITEMTEXT, IDD_STATUSBAR_MENUHELP,(LPARAM)(LPSTR)strReportName.c_str());


	HANDLE ReportHandle;

	if(strlen(szReportName) == 0)
	 {
		MessageBox(HWindow,GetString(hInst,6156),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	 }

	JobNumber = PEOpenPrintJob(szReportName);
	if(JobNumber != 0)
	{
	 wsprintf(szCompanyName,"'%s'",&((TAcctModule *)GetApplication())->szCompanyName);
	 PESetFormula (JobNumber,(LPSTR)strCompany.c_str(),szCompanyName);
	 int Period = ((TGlWindow *)Parent)->iGetPeriod();

	 char szPeriod[4];
	 wsprintf(szPeriod,"'%d'",Period);
	 PESetFormula (JobNumber,(LPSTR)strCurrentPd.c_str(),szPeriod);


	 char   *szReportCaption;
	 short  iReportTitleLen;
	 PEGetReportTitle(JobNumber,&ReportHandle,&iReportTitleLen);

	if(( szReportCaption = (char *)malloc((short)iReportTitleLen)) == NULL)
	 {
		return;
	 }
	//PESetDialogParentWindow(JobNumber,HWindow);
	//PEEnableProgressDialog(JobNumber,TRUE);
	PEGetHandleString(ReportHandle,szReportCaption,iReportTitleLen);
	PEOutputToWindow(JobNumber,szReportCaption,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,HWindow);
	//PEShowPrintControls(JobNumber,FALSE);
	PEStartPrintJob(JobNumber,TRUE);
	//PEClosePrintJob(JobNumber);
	free(szReportCaption);
	return;

  }
  else
  {
  HANDLE textHandle;
  char   *textBuffer;
  short   textLength;
  PEGetErrorText(JobNumber,&textHandle,&textLength);
  if((textBuffer = (char *)malloc(textLength)) == NULL)
	return;
  PEGetHandleString(textHandle,textBuffer,textLength);
  string strMsg;
  strMsg = textBuffer;
  strMsg += "\nReport Name : ";
  strMsg += szReportName;
  MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
  free(textBuffer);
  PECloseEngine();

  return;
  }
}

unsigned int TGlWindow::iGetPeriod()
{
 Code4 cb;
 cb.init();

 Data4 gl_pds(cb,"gl_pds");
 Field4 period(gl_pds,2);
 Field4 status(gl_pds,6);
 gl_pds.SetIndex("Period");
 gl_pds.bottom();
 int iPeriod;
 while(!gl_pds.bof())
  {
	if(strcmp(status.str(),"0") == 0)
	 iPeriod = atoi(period.str());
	gl_pds.skip(-1);
  }
 gl_pds.close();
 cb.initUndo();
 return iPeriod;
}


char* GetName()
{
 static char szCompanyName[80];
 GetPrivateProfileString("GL_DIR","COMPANY","",szCompanyName,80,"GL.INI");
 return szCompanyName;
}


char* TGlWindow::szGetPeriod()
{
 Code4 *cb = &((TAcctModule *)GetApplication())->cb;
 cb->autoOpen = FALSE;
 GL_PERIODS gl_pds;
 if(gl_pds.open(*cb) < 0)
 {
	 gl_pds.close();
	 string strMsg(GetString(hInst,4098));
	 exit(0);
 }
 gl_pds.bottom();
 static char szPeriod[11];
 while(!gl_pds.bof())
  {
	if(strcmp(gl_pds.status.str(),"0") == 0)
	 strcpy(szPeriod,gl_pds.pd_desc.str());
	gl_pds.skip(-1);
  }
 gl_pds.close();
 cb->autoOpen = TRUE;
 return szPeriod;
}

void SetPeriods(HWND hWnd)
{
 Code4 cb;
 cb.init();

 DWORD dwIndex;
 GL_PERIODS gl_pds;
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
 cb.initUndo();

}




BOOL ValidateGL(LPSTR lpAccount,LPSTR lpDepart)
{
 Code4 cb;
 cb.init();

 BOOL bValid = FALSE;
 string glacct(hInst,4096);
 string accounts(hInst,4097);
 Data4 glaccts(cb,(LPSTR)glacct.c_str());
 glaccts.SetIndex(accounts);
 Field4 account(glaccts,1);
 Field4 dept(glaccts,2);

 if(glaccts.isValid())
 {
  string szSeek(lpAccount);
  szSeek.append(lpDepart);
  if(glaccts.seek(szSeek))
	bValid = TRUE;
 }
 glaccts.close();
 return bValid;
}


BOOL CheckFile(char *szFileName,HWND hWnd)
{
 Code4 cb;
 cb.init();

 cb.autoOpen = FALSE;
 BOOL bFile = TRUE;

 Data4 db,gl_tag;
 Index4 idx;
 string Msg;
 int rc = db.open(cb,szFileName);
 if(rc != 0)
  {
	wsprintf(szMsg,GetString(hInst,ID_DATA_ERROR),e4text(rc),szFileName);
	Msg = szMsg;
	Msg += string(hInst,ID_DATA_ERROR_MSG);
	MessageBox(hWnd,(LPSTR)Msg.c_str(),"",MB_ICONINFORMATION);
	bFile = FALSE;
  }


 if(bFile)
 {
  rc = idx.open(db,szFileName);
  if(rc != 0)
  {
	wsprintf(szMsg,GetString(hInst,ID_DATA_ERROR),e4text(rc),szFileName);
	MessageBox(hWnd,szMsg,"",MB_ICONINFORMATION);
	bFile = FALSE;
  }
 }
 cb.closeAll();
 return bFile;
}

