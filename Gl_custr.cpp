#include "ap.h"
#include "gl_dlg.h"
#include "gl_pds.hpp"
#include "gl_setat.hpp"
#include "gl_prof.hpp"
#include "utildlg.h"
#include "db_accts.h"



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
  static string strReportName;

  TCustomReport(TWindowsObject* AParent,UINT ID):TMainDialog(AParent,ID){}
  virtual void Ok              (RTMessage Msg) = [ID_FIRST + IDOK];
  void SetReportName(string strReport)
  {
	 strReportName = strReport;
  }
  virtual void SetupWindow();
  void PrintReport();

};

void TCustomReport::SetupWindow()
{
	SetField(IDC_REPORT_TITLE,(LPSTR)strReportName.c_str());
	Tglaccts glaccts(&((TAcctModule *)GetApplication())->cb);
	string strAccount;
	for(glaccts.top();glaccts.eof();glaccts.skip())
	{
		strAccount = glaccts.accounts.str();
		strAccount +="\t";
		strAccount += glaccts.dept.str();
		strAccount +="\t";
		strAccount += glaccts.descript.str();
		SendDlgItemMsg(1105,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)strAccount.c_str());
		SendDlgItemMsg(1106,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)strAccount.c_str());
	}

}

void TCustomReport::Ok(RTMessage)
{
	((TGlWindow *)Parent)->PrintReport((LPSTR)strReportName.c_str(),GetApplcation()->MainWindow->HWindow);
	/*

	PEOpenEngine();

	string strCompany(hInst,4094);
	string strCurrentPd(hInst,4095);


	HANDLE ReportHandle;

	if(strlen((LPSTR)strReportName.c_str()) == 0)
	 {
		MessageBox(HWindow,GetString(6156),((TAcctModule *)GetApplication())->GetAppName(),MB_ICONINFORMATION);
	 }

	short JobNumber = PEOpenPrintJob((LPSTR)strReportName.c_str());
	if(JobNumber != 0)
	{
	 PESetFormula (JobNumber,(LPSTR)strCompany.c_str(),"'Eiwa (Thailand) Co, Ltd.'");

	 char szVendorno[10];
	 wsprintf(szVendorno,"'%s'",StrTrim(GetField(IDC_START)));
	 PESetFormula (JobNumber,"Vendor Number",szVendorno);

	 char szVendorEnd[10];
	 wsprintf(szVendorEnd,"'%s'",StrTrim(GetField(IDC_END)));
	 PESetFormula (JobNumber,"Vendor Ending",szVendorEnd);

	 char szPeriod[10];
	 wsprintf(szPeriod,"%s",StrTrim(GetField(IDC_PERIOD)));
	 PESetFormula (JobNumber,"Period",szPeriod);

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
  */


}

void TGlWindow::GLReportDialog(RTMessage)
{
	TCustomReport* Report = new TCustomReport(this,1025);
	Report->SetReportName("ar_balm.rpt");
	GetApplication()->ExecDialog(Report);
}

