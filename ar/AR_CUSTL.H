#ifndef __AR_CUSTL_H
#define __AR_CUSTL_H
#include "tlist_w.h"
#include "db_vend.h"
_CLASSDEF(TCustomerList)
class TCustomerList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 PTListWindow List;
 Tapvendor *arcust;
 TCustomerList(PTWindowsObject AParent, LPSTR AName,Tapvendor *ar):TWindow (AParent, AName)
  {
   arcust = ar;
   List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
  }
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "AR_CUST";}
};
#endif