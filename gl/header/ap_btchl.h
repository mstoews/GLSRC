#ifndef __AR_BTCHL_H
#define __AR_BTCHL_H
#include "tlist_w.h"
#include "db_apbtc.h"
_CLASSDEF(TBatchList)
class TBatchList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 PTListWindow List;
 Tapbtch *apbtch;
 TBatchList(PTWindowsObject AParent, LPSTR AName,Tapbtch *ap):TWindow (AParent, AName)
  {
   apbtch = ap;
   List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
  }
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "AP_BTCH";}
};
#endif