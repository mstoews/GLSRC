#ifndef __AP_CASHL_H
#define __AP_CASHL_H
#include "tlist_w.h"
#include "db_cash.h"
_CLASSDEF(TCashList)
class TCashList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 PTListWindow List;
 Tapcash *apcash;
 TCashList(PTWindowsObject AParent, LPSTR AName,Tapcash *ap):TWindow (AParent, AName)
  {
   apcash = ap;
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
  }
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "AP_CASH";}
};
#endif