#include "std_inc.h"

extern HINSTANCE hInst;

#define IDC_START 1105
#define IDC_END   1106
#define IDC_PERIOD 1107
#define IDC_REPORT_TITLE 1108


class TCustomReport : public TMainDialog
{
  public:
  string strStart;
  string strEnd;
  string strReportName;
  short JobNumber;
  string strCaption;

  TCustomReport(TWindowsObject* AParent,UINT ID):TMainDialog(AParent,ID){}
  virtual void Ok              (RTMessage Msg) = [ID_FIRST + IDOK];
  void SetReportName(string strReport)
  {
	 strReportName = strReport;
  }
  virtual void SetupWindow();
  void PrintReport();
  virtual void SetFormulas();

};

class TTrialBalanceByAccount : public TCustomReport
{
  public:
  TTrialBalanceByAccount(TWindowsObject* AParent,UINT ID):TCustomReport(AParent,ID){}
  virtual void SetFormulas();
  virtual void SetupWindow();
};

class TTrialBalance : public TCustomReport
{
  public:

  TTrialBalance(TWindowsObject* AParent,UINT ID):TCustomReport(AParent,ID){}
  virtual void SetupWindow();
  virtual void SetFormulas();
};

void TTrialBalance::SetupWindow()
{
	//TCustomReport::SetupWindow();
	TMainDialog::SetupWindow();
	SetField(IDC_REPORT_TITLE,(LPSTR)strReportName.c_str());
	SetCaption((LPSTR)strCaption.c_str());
	HWND hStart = GetDlgItem(HWindow,IDC_START);
	HWND hEnd = GetDlgItem(HWindow,IDC_END);
	ShowWindow(hStart,SW_HIDE);
	ShowWindow(hEnd,SW_HIDE);
}

void TTrialBalanceByAccount::SetupWindow()
{
	TCustomReport::SetupWindow();
	//TMainDialog::SetupWindow();
	SetField(IDC_REPORT_TITLE,(LPSTR)strReportName.c_str());
	SetCaption((LPSTR)strCaption.c_str());
}

void TGlWindow::GLIncomeSt (RTMessage)
{
 TTrialBalance* Report = new 	TTrialBalance(this,5000);
 Report->strCaption = "Income Statement";
 Report->SetReportName("income.rpt");
 GetApplication()->ExecDialog(Report);

}

void TGlWindow::GLBalance (RTMessage)
{
 //strcpy(FileName,CurDir);
 //strcat(FileName,"\\");
 //strcat(FileName,GetString(hInst,2081));
 //MakeChild(FileName);
 //PrintReport("balance.rpt",HWindow);
 TTrialBalance* Report = new 	TTrialBalance(this,5000);
 Report->SetReportName("balance.rpt");
 Report->strCaption = "Balance Sheet";
 GetApplication()->ExecDialog(Report);

}


void TGlWindow::GLManAccounts    (RTMessage)
 {
 //strcpy(FileName,CurDir);
 //strcat(FileName,"\\");
 //strcat(FileName,GetString(hInst,2082));
 //MakeChild(FileName);
 //PrintReport("gl_man.rpt",HWindow);
 //PrintReport("gl_cgs.rpt",HWindow);
 TTrialBalance* Report = new 	TTrialBalance(this,5000);
 Report->SetReportName("gl_man.rpt");
 Report->strCaption = "Cost of Manufacturing";
 GetApplication()->ExecDialog(Report);
 }


void TGlWindow::GLFinRE    (RTMessage)
 {
 //strcpy(FileName,CurDir);
 //strcat(FileName,"\\");
 //strcat(FileName,GetString(hInst,2083));
 //MakeChild(FileName);
 // PrintReport("gl_rtn.rpt",HWindow);
 TTrialBalance* Report = new 	TTrialBalance(this,5000);
 Report->SetReportName("gl_cgs.rpt");
 Report->strCaption = "Cost of Goods Sold";
 GetApplication()->ExecDialog(Report);

 }


void TTrialBalanceByAccount::SetFormulas()
{
	 char szVendorno[10];
	 wsprintf(szVendorno,"'%s'",StrTrim(GetField(IDC_START)));
	 PESetFormula (JobNumber,"Start",szVendorno);

	 char szVendorEnd[10];
	 if(strlen(StrTrim(GetField(IDC_END)))  ==  0)
		wsprintf(szVendorEnd,"'%s'",StrTrim(GetField(IDC_START)));
	 else
		wsprintf(szVendorEnd,"'%s'",StrTrim(GetField(IDC_END)));

	 PESetFormula (JobNumber,"End",szVendorEnd);

	 char szPeriod[10];
	 wsprintf(szPeriod,"%s",StrTrim(GetField(IDC_PERIOD)));
	 PESetFormula (JobNumber,"Pd",szPeriod);
}

void TTrialBalance::SetFormulas()
{
	 PESetFormula (JobNumber,"Pd",(LPSTR)StrTrim(GetField(IDC_PERIOD)));
}


void TCustomReport::SetFormulas()
{
	 char szVendorno[10];
	 wsprintf(szVendorno,"'%s'",StrTrim(GetField(IDC_START)));
	 PESetFormula (JobNumber,"Vendor Number",szVendorno);

	 char szVendorEnd[10];
	 wsprintf(szVendorEnd,"'%s'",StrTrim(GetField(IDC_END)));
	 PESetFormula (JobNumber,"Vendor Ending",szVendorEnd);

	 char szPeriod[4];
	 wsprintf(szPeriod,"%s",StrTrim(GetField(IDC_PERIOD)));
	 PESetFormula (JobNumber,"Period",szPeriod);

}

void TCustomReport::SetupWindow()
{
	TMainDialog::SetupWindow();
	SetField(IDC_REPORT_TITLE,(LPSTR)strReportName.c_str());
	Tglaccts glaccts;
	if(glaccts.open(((TAcctModule *)GetApplication())->cb) < 0)
	{
	  ErrorClose(glaccts.GetDbfName());
	  return;
	}


	if(glaccts.SetIndexAccts(1) < 0)
	 {
	  ShutDownWindow();
	  return;
	 }

	string strAccount;
	for(glaccts.top();!glaccts.eof();glaccts.skip())
	{
		strAccount = glaccts.accounts.str();
		SendDlgItemMsg(1105,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)strAccount.c_str());
		SendDlgItemMsg(1106,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)strAccount.c_str());
	}
	glaccts.close();
}

void TCustomReport::Ok(RTMessage)
{
	HWND hStart = GetDlgItem(HWindow,IDC_START);
	if(strlen(StrTrim(GetField(IDC_START))) == 0 && IsWindowVisible(hStart))
	{
	 MessageBox(HWindow,GetString(9099),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	 return;
	}
	if(strlen(StrTrim(GetField(IDC_PERIOD))) == 0 )
	{
	 MessageBox(HWindow,GetString(9100),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	 return;
	}

	PEOpenEngine();

	string strCompany(hInst,4094);
	string strCurrentPd(hInst,4095);


	HANDLE ReportHandle;

	if(strlen((LPSTR)strReportName.c_str()) == 0)
	 {
		MessageBox(HWindow,GetString(6156),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	 }

	JobNumber = PEOpenPrintJob((LPSTR)strReportName.c_str());
	if(JobNumber != 0)
	{
	 char szCompanyName[40];
	 wsprintf(szCompanyName,"'%s'",&((TAcctModule *)GetApplication())->szCompanyName);
	 PESetFormula (JobNumber,(LPSTR)strCompany.c_str(),szCompanyName);
	 SetFormulas();

	char   *szReportCaption;
	short  iReportTitleLen;
	PEGetReportTitle(JobNumber,&ReportHandle,&iReportTitleLen);

	if(( szReportCaption = (char *)malloc((short)iReportTitleLen)) == NULL)
	 {
		return;
	 }

	PEGetHandleString(ReportHandle,szReportCaption,iReportTitleLen);
	PEOutputToWindow(JobNumber,szReportCaption,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,GetApplication()->MainWindow->HWindow);
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
  strMsg += strReportName;
  MessageBox(HWindow,(LPSTR)strMsg.c_str(),((TAcctModule *)GetApplication())->GetAppName(),MB_OK);
  free(textBuffer);
  PECloseEngine();
  return;
  }

}

void TGlWindow::GLReportDialog(RTMessage)
{
	TCustomReport* Report = new TCustomReport(this,1025);
	Report->SetReportName("ar_balm.rpt");
	GetApplication()->ExecDialog(Report);
}


void TGlWindow::GLTrialBalanceByAccountAndPeriod(RTMessage)
{
	TTrialBalanceByAccount* Report = new TTrialBalanceByAccount(this,1025);
	Report->SetReportName("gl_accts.rpt");
	GetApplication()->ExecDialog(Report);
}


void TGlWindow::GLAccounts (RTMessage)
{
 //PrintReport(ReportFileName(18009),GetApplication()->MainWindow->HWindow);
	TTrialBalance* Report = new 	TTrialBalance(this,1025);
	Report->SetReportName("gl_chart.rpt");
	Report->strCaption = "Trial Balance by Periods";
	GetApplication()->ExecDialog(Report);
}

void TGlWindow::GLTrialBalance   (RTMessage)
{
  //PrintReport(ReportFileName(18008),GetApplication()->MainWindow->HWindow);
	TTrialBalance* Report = new 	TTrialBalance(this,1025);
	Report->SetReportName("gl_chart.rpt");
	Report->strCaption = "Trial Balance by Periods";
	GetApplication()->ExecDialog(Report);
}
