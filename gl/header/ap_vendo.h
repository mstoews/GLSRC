#ifndef __AP_VENDO_H
#define __AP_VENDO_H
#include "ap_vendr.h"
#include "db_accts.h"

_CLASSDEF (TAPVendorAdd)
class TAPVendorAdd : public TMainDialog
{
 public:
 BOOL bVendorList;
 TCustomerList *VendorList;
 PTapvendor apvendor;
 PTglaccts  glaccts;
 char *szVATAccount,*szVATDept,*szAPAccount,*szAPDept;
 char szType[2];
 int iVat; 
 TAPVendorAdd(PTWindowsObject AParent,int AName, PTModule AModule = NULL): TMainDialog (AParent,AName)
 {
 };
 virtual void SetupWindow();
 virtual void Search         (RTMessage Msg) = [ID_FIRST + ID_SEARCHCUST];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Delete         (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Next	     (RTMessage Msg) = [ID_FIRST + ID_NEXT];
 virtual void Top 	     (RTMessage Msg) = [ID_FIRST + ID_TOP];
 virtual void Bottom         (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
 virtual void Previous	     (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
 virtual void SetData	     (RTMessage Msg) = [ID_FIRST + ID_SETDATA];
 virtual void SetDetail      (RTMessage Msg) = [CM_FIRST + CM_SETVENDOR]{SetData(Msg);};
 virtual void New            (RTMessage Msg) = [ID_FIRST + CM_NEW];
 virtual void Clear          (RTMessage Msg) = [CM_FIRST + CM_NEW]{ New(Msg); };
 void AddLine();
};

_CLASSDEF (TAPVendorBatchAdd)
class TAPVendorBatchAdd : public TAPVendorAdd
{
 public:
 TAPVendorBatchAdd(PTWindowsObject AParent,int AName,PTapvendor ap,PTglaccts gl):TAPVendorAdd(AParent,AName)
 {
  apvendor = ap;
  glaccts  = gl;
 }
 virtual void SetupWindow();
 virtual void Cancel        (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok            (RTMessage Msg) = [ID_FIRST + IDOK];
};

#endif