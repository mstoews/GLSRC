#ifndef __AP_BTCHL_H
#define __AP_BTCHL_H
#include "tlist_w.h"
#include "db_apbtc.h"
_CLASSDEF(TBatchList)
class TBatchList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 // This is used to set the max number of rows in the windows.
 int iRows;
 PTListWindow List;
 Code4 *cb;
 TBatchList(PTWindowsObject AParent, LPSTR AName):TWindow (AParent, AName)
  {
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
	cb = &((TAcctModule *)GetApplication())->cb;
  }
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual void SetQuery();
 virtual LPSTR GetClassName(){ return "AP_BTCH";}
};
#endif