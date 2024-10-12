#ifndef __AR_CUSTL_H
#define __AR_CUSTL_H
#include "tlist_w.h"
#include "db_cust.h"
_CLASSDEF(TARCustomerList)
class TARCustomerList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 PTListWindow List;
 Tarcust *arcust;
 TARCustomerList(PTWindowsObject AParent, LPSTR AName,Tarcust *ar):TWindow (AParent, AName)
  {
	arcust = ar;
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
  }
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE]; 
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "AR_CUST";}
};
#endif