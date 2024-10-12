#ifndef _AP_ADJST_H
#define _AP_ADJST_H

#define CM_INVOICE 6001

_CLASSDEF(TAPInvoiceList)
class TAPInvoiceList : public TPickList
{
 public:
 PTapbtch  apbtch;
 TAPInvoiceList(PTWindowsObject AParent, LPSTR AName,PTapbtch glb):
  TPickList (AParent, AName)
  {
   apbtch=glb;
  };
 virtual BOOL CanClose(){return FALSE;};
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void Cancel        (RTMessage Msg) = [ID_FIRST + IDCANCEL] {ShutDownWindow();};
 virtual void Redraw        (RTMessage Msg) = [CM_FIRST + CM_SETDATA];
 virtual void SetupWindow();
};

_CLASSDEF(TAPInvoiceBatch)
class TAPInvoiceBatch :public TPickList
{
 public:
 PTapdist apdist;
 PTapbtch  apbtch;
 TAPInvoiceBatch(PTWindowsObject AParent, LPSTR AName,PTapdist d,PTapbtch apb):
  TPickList(AParent,AName)
  {
   apdist = d;
   apbtch = apb;
  }
 virtual BOOL CanClose(){return FALSE;};
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void Redraw        (RTMessage Msg) = [CM_FIRST + CM_SETDATA];
 virtual void SetupWindow();
};

_CLASSDEF(TAPAdjEntry)
class TAPAdjEntry : public TAPInvoiceEntry
{
 public:
  BOOL bRedraw,bRedrawInvoice;
  PTAPInvoiceList InvoiceList;
  PTAPInvoiceBatch InvoiceBatch;
  TAPAdjEntry(PTWindowsObject AParent,LPSTR ATitle);
  virtual LPSTR GetClassName(){ return "APAdjEntry"; }
  virtual void SetupWindow();
  virtual void SList (RTMessage Msg) = [ID_FIRST + ID_SLIPLIST];
  virtual void CurBatch          (RTMessage Msg) = [CM_FIRST + CM_BATCH];
  virtual void Invoice           (RTMessage Msg) = [CM_FIRST + CM_INVOICE];
  virtual void Ok                (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void CreateAdjustment  (RTMessage Msg) = [ID_FIRST + ID_CREATEADJ];
  virtual void RebateCalc        (RTMessage Msg) = [ID_FIRST + ID_ADJUSTMENT];
  virtual void ClosedInvoiceList (RTMessage Msg) = [ID_FIRST + ID_CLOSEDINVOICE_BATCH];
  virtual BOOL CanClose(){return FALSE;};
  void SetPeriodAmount(double period,char *szAccountNo,double dBatchNo);  
  int  UpdateDetailWindow(long lBatchno);
  void SetDetailLine(HWND List,int i);
};

#endif