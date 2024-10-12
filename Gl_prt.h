#ifndef __GL_PRT_H
#define __GL_PRT_H

#define ID_CLOSE_RPT  1005
#define ID_TITLES      3002
#define ID_FORMULAS    3003
#define ID_SELECTION   3004
#define ID_DATABASES   3005


typedef TDoubleListImp <string> CList;

class TRpt : public TWindow
{
 public:
 char szCompanyName[81];
 char szPeriod[4];

 CList FormulaList;
 CList FormulaText;
 short JobNumber;
 HWND    hWnd;
 string  strReportFile;
 TRpt(TWindowsObject *AParent,LPSTR AReportFile):TWindow(AParent,AReportFile)
 {
	strReportFile = AReportFile;
	hWnd = NULL;
	JobNumber = NULL;
 }

 ~TRpt();
 virtual void SetupWindow();
 virtual void WMSize			 (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void Cancel			 (RTMessage Msg) = [CM_FIRST + IDCANCEL]{DestroyWindow(HWindow);};
 virtual void WMMDIActivate (TMessage& Msg);
 virtual void Close         (RTMessage Msg) =   [CM_FIRST + ID_CLOSE_RPT]{};
 virtual void Selection     (RTMessage Msg) =   [CM_FIRST + ID_SELECTION]{};
 virtual void Titles        (RTMessage Msg) =   [CM_FIRST + ID_TITLES]   {};
 virtual void Formula       (RTMessage Msg) =   [CM_FIRST + ID_FORMULAS];
 virtual void Databases     (RTMessage Msg) =   [CM_FIRST + ID_DATABASES]{};
 virtual BOOL CanClose();
 virtual void ErrorMessage();
 protected:
 BOOL PrintReport();
};

#endif
