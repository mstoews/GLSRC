#include "db_rept.h"
_CLASSDEF(TGLCustomReport)
class TGLCustomReport : public TDialog
{
 public:
 PTListBox ReportList;
 PTStatic  ReportName;
 PTglcrept glrept;
 TGLCustomReport(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TDialog(AParent,AName)
 {
  ReportList = new TListBox(this,ID_ACCTLIST);
  ReportName = new TStatic(this,ID_PRINT,40);
 };
 virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
 virtual void WMRButtonDown  (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
 virtual void HandlePrint    (RTMessage Msg) = [ID_FIRST + IDM_PRINT];
 virtual void HandleListBox  (RTMessage Msg) = [ID_FIRST + ID_ACCTLIST];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Add            (RTMessage Msg) = [ID_FIRST + ID_ACCTADD];
 virtual void Delete         (RTMessage Msg) = [ID_FIRST + ID_ACCTDEL];
 virtual void Modify	     (RTMessage Msg) = [ID_FIRST + ID_ACCTMOD];
 virtual void ResetListBox();
};
