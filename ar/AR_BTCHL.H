#ifndef __AR_BTCHL_H
#define __AR_BTCHL_H
#include "tlist_w.h"
#include "db_arbtc.h"
_CLASSDEF(TBatchList)
class TBatchList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 PTListWindow List;
 Tapbtch *ar_btch;
 TBatchList(PTWindowsObject AParent, LPSTR AName,Tapbtch *ar):TWindow (AParent, AName)
  {
   ar_btch = ar;
   List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
  }
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "AR_BTCH";}
};
#endif