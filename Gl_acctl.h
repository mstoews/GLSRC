#ifndef __GL_ACCTL_H
#define __GL_ACCTL_H
#include "tlist_w.h"
#include "db_accts.h"
_CLASSDEF(TAccountList)
class TAccountList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 Code4 *cb;
 long lRows;
 BOOL bInValid;
 TListWindow* List;
 string strKey;
 int iAcct;
 int iDept;
 int iDescript;
 
 Tglaccts glaccts;
 TAccountList(PTWindowsObject AParent, LPSTR AName):TWindow (AParent, AName)
  {
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
	cb = &((TAcctModule *)GetApplication())->cb;
   SetAccountID();
  }
 ~TAccountList()
 {
	delete List;
	glaccts.close();
 }
 void SetAccountID(int iA = 2013,int iD = 2014,int iDs = 3037)
 {
	iAcct = iA;
	iDept = iD;
	iDescript = iDs;
 }

 void SetInfo(Tglaccts* gl) {}
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "GLACCTS";}
};
#endif