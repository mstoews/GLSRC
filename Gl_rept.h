#ifndef __GL_REPT_H
#define __GL_REPT_H
#include "db_rept.h"
_CLASSDEF(TGLCustomReport)
class TGLCustomReport : public TMainDialog
{
 public:
 Tglcrept glrept;
 TGLCustomReport(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName)
  {
	SetBitmap(IDOK);
	SetBitmap(ID_DELETE);
	SetBitmap(ID_PRINT);
	SetBitmap(ID_MODIFY);
	SetBitmap(IDCANCEL);
  };
 virtual void SetupWindow();
 //virtual void WMRButtonDown  (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
 virtual void HandleDblclk   (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void HandlePrint    (RTMessage Msg) = [ID_FIRST + ID_PRINT];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Add            (RTMessage Msg) = [ID_FIRST + ID_ADD];
 virtual void Delete         (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 virtual void Modify	        (RTMessage Msg) = [ID_FIRST + ID_MODIFY];
 virtual void PrtReport(char *szFileName);
 virtual void ResetListBox();
};
#endif