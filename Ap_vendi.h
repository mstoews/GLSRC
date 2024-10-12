#ifndef __AP_VENDI_H
#define __AP_VENDI_H
#include "db_accts.h"
_CLASSDEF (TAPVendorInquiry)
class TAPVendorInquiry : public TMainDialog
{
 public:
 PTAPVendorList VendorList;
 PTapvendor apvendor;
 PTglaccts  glaccts;
 PTSpread List;
 int iVat; 
 TAPVendorInquiry(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TMainDialog (AParent,AName)
 {
  List = new TSpread(this,ID_DISTLIST);
 };
 virtual void SetupWindow();
 virtual void Search         (RTMessage Msg) = [ID_FIRST + ID_SEARCHCUST];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Next	     (RTMessage Msg) = [ID_FIRST + ID_NEXT];
 virtual void Previous	     (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
 virtual void SetData	     (RTMessage Msg) = [ID_FIRST + ID_SETDATA];
 virtual void SetDetail      (RTMessage Msg) = [CM_FIRST + CM_SETDATA]{SetData(Msg);};
};

#endif

