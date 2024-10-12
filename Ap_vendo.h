#ifndef __AP_VENDO_H
#define __AP_VENDO_H
#include "ap_vendr.h"
#include "db_accts.h"
#define WM_OPEN_CHILDDLG 3066

_CLASSDEF (TAPVendorAdd)
class TAPVendorAdd : public TMainDialog
{
 public:
 TDialog* dialog;
 TDialog* options;
 BOOL bVendorList;
 TCustomerList *VendorList;
 Tapvendor apvendor;
 Tglaccts  glaccts;
 //char *szVATAccount,*szVATDept,*szAPAccount,*szAPDept;
 string strVATAccount,strVATDept,strAPAccount,strAPDept;
 char szType[2];
 int iVat;
 int iCurrentTab;
 TAPVendorAdd(PTWindowsObject AParent,int AName, PTModule AModule = NULL): TMainDialog (AParent,AName)
 {
	dialog = NULL;
	options = NULL;
 };
 virtual void SetupWindow();
 virtual void Search         (RTMessage Msg) = [ID_FIRST + ID_SEARCHCUST];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Delete         (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Next	        (RTMessage Msg) = [ID_FIRST + ID_NEXT];
 virtual void Top 	        (RTMessage Msg) = [ID_FIRST + ID_TOP];
 virtual void Bottom         (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
 virtual void Previous	     (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
 virtual void SetData	     (RTMessage Msg) = [ID_FIRST + ID_SETDATA];
 virtual void SetDetail      (RTMessage Msg) = [CM_FIRST + CM_SETVENDOR]{SetData(Msg);};
 virtual void New            (RTMessage Msg) = [ID_FIRST + CM_NEW];
 virtual void Clear          (RTMessage Msg) = [CM_FIRST + CM_NEW]{ New(Msg); };
 virtual void WMMove         (RTMessage Msg) = [WM_FIRST + WM_MOVE];
 virtual void HasChanged     (RTMessage Msg) = [ID_FIRST + IDC_TAB];
 virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
 virtual void OpenChildWindow (RTMessage Msg) = [CM_FIRST + WM_OPEN_CHILDDLG];
 virtual void AddLine();
 inline BOOL CheckAPAccounts(){return TRUE;};
 void UpdateData();
 virtual BOOL SetField   (int ID, Field4 *field,HWND HWindow);
 virtual BOOL SetField   (int ID, char *);
 virtual BOOL SetField   (int ID, double);
 virtual BOOL SetField   (int ID, int);
 virtual BOOL SetField(int ID, Field4& field,HWND HWindow);
 virtual BOOL SetField(int ID, string* field);
};

_CLASSDEF (TAPVendorBatchAdd)
class TAPVendorBatchAdd : public TAPVendorAdd
{
 public:
 Tapvendor apvendor;
 Tglaccts  glaccts;
 char szVendor[11];
 TAPVendorBatchAdd(PTWindowsObject AParent,int AName,LPSTR szVendorno):TAPVendorAdd(AParent,AName)
 {
  strcpy(szVendor,szVendorno);
 }
 virtual void SetupWindow();
 virtual void Cancel        (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok            (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void AddLine();
};

#endif