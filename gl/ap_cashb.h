#ifndef __CASHB_H
#define __CASHB_H
#include "gl_setat.hpp"
#include "ap_cashl.h"
#include "gl_acctl.h"
#include "ap_vendr.h"
#include "ap_class.h"

_CLASSDEF(TAPCashBatch)
class TAPCashBatch : public TPickList
{
 public:
 PTListArray ListArray;

 PTapcash  apcash;
 TAPCashBatch(PTWindowsObject AParent, LPSTR AName,PTapcash glb):
  TPickList (AParent, AName) {apcash=glb; ListArray = new TListArray(1000, 0, 50);};
 ~TAPCashBatch(){delete ListArray;};
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void Cancel        (RTMessage Msg) = [ID_FIRST + IDCANCEL] {ShutDownWindow();};
 virtual void SetupWindow();

};


/*
_CLASSDEF (TAPCashDist)
class TAPCashDist : public TDialog
{
 public:
  SS_CELLTYPE 	CellType;
  HWND SHWindow;
  PTapcashd apcashd;
  PTListArray ListArray;
  TAPCashDist(PTWindowsObject AParent, int ATitle,PTapcashd bdp):TDialog(AParent,ATitle)
  {
  ListArray = new TListArray(1000, 0, 50);
  apcashd = bdp;
  };

  virtual void SetupWindow();
  virtual void SetListSize(int Rows);
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  ~TAPCashDist(){delete ListArray;};
};
*/

_CLASSDEF(TAPCashEntry)
class TAPCashEntry : public TAPInvoiceEntry
{
 public:
  TCashList *SlipList;
  BOOL bModify;
  PTapcash apcash;
  PTapcashd apcashd;
  PTapdist  apdist;
  PTgldist  gldist;
  TAPCashList* D;
  GL_SETAT gl_setat;
  void TAPCashEntry::GetCashExpense(AP_EXPENSE* ex,
											 double lInvoiceAmount,
											 double dVatPercentage,
											 double dRebatePercentage);
  void InvoiceData();
  void Sliplist();
  void ModifyCurrentSlip();
  virtual void SList           (RTMessage Msg) = [ID_FIRST + ID_SLIPLIST]{Sliplist();};
  virtual void CurrentBatch    (RTMessage Msg) = [CM_FIRST + CM_CURRENTBATCH];
  virtual void Ok              (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void New             (RTMessage Msg) = [ID_FIRST + ID_NEW];
  virtual void SetLineItem     (RTMessage Msg) = [CM_FIRST + CM_LINEITEM];
  virtual void Top             (RTMessage Msg) = [ID_FIRST + ID_TOP];
  virtual void Bottom          (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
  virtual void Next            (RTMessage Msg) = [ID_FIRST + ID_NEXT];
  virtual void Previous        (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
  virtual void Delete          (RTMessage Msg) = [ID_FIRST + ID_DELETE];
  virtual void RollBack        (RTMessage Msg) = [CM_FIRST + CM_ROLLBACK];
  virtual void Cancel          (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Print           (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual void GetBankAccountDescription (RTMessage Msg) = [ID_FIRST + ID_CASH];
  virtual void RecoverInvoice();
  virtual void SetupWindow();
  void CreateLineItem(PTapcash apcash,PTapcashd apcashd);
  virtual void Clear();
  BOOL Closed(void);
  BOOL PreviousInvoice(void);
  TAPCashEntry(PTWindowsObject AParent,int ATitle):TAPInvoiceEntry(AParent,ATitle){};
  void UpdateBatchDetail(long iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry);
  void AddDetail(long iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry);
  void AddToCurrentSlip(long iBatchno);
  virtual LPSTR GetClassName(){ return "APCashEntry"; }
};
#endif