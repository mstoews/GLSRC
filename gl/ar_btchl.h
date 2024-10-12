#ifndef __AR_BTCHL_H
#define __AR_BTCHL_H
#include "tlist_w.h"
#include "db_arbtc.h"
_CLASSDEF(TARBatchList)
class TARBatchList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 PTListWindow List;
 Tarbtch *ar_btch;
 TARBatchList(PTWindowsObject AParent, LPSTR AName,Tarbtch *ar):TWindow (AParent, AName)
  {
   ar_btch = ar;
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
  }
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "AR_BTCH";}
};
#endif