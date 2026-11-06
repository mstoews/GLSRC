#include "glb.h"
#include "db_vend.h"
#include "db_ardst.h"
#include "db_arset.h"
#include "gl_dlg.h"
#include "db_dist.h"


extern Code4 cb;

_CLASSDEF(TAPVendorList)
class TAPVendorList : public TDialog
{
 public:
 PTListArray ListArray;
 PTapvendor  apvendor;
 Field4 *vendorno;
 Field4 *vdname;
 TAPVendorList(PTWindowsObject AParent, LPSTR AName,PTapvendor vd):
  TDialog (AParent, AName) {apvendor=vd; ListArray = new TListArray(1000, 0, 50);};
 ~TAPVendorList();
 virtual void SetupWindow();
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
};


// Accounts Payable Entry header file.


_CLASSDEF (TAPDistList)
class TAPDistList : public TDialog
{
 public:
  SS_CELLTYPE 	CellType;
  HWND SHWindow;
  PTapbtchd apbtd;
  PTapcashd apcashd;
  PTListArray ListArray;
  TAPDistList(PTWindowsObject AParent, LPSTR ATitle,PTapbtchd bdp):TDialog(AParent,ATitle)
  {
  ListArray = new TListArray(1000, 0, 50);
  apbtd = bdp;
  };
  TAPDistList(PTWindowsObject AParent, LPSTR ATitle,PTapcashd bdp):TDialog(AParent,ATitle)
  {
  ListArray = new TListArray(1000, 0, 50);
  apcashd = bdp;
  };

  virtual void SetupWindow();
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  ~TAPDistList(){delete ListArray;};
};

_CLASSDEF (TAPCashDist)
class TAPCashDist : public TDialog
{
 public:
  SS_CELLTYPE 	CellType;
  HWND SHWindow;
  PTapcashd apcashd;
  PTListArray ListArray;
  TAPCashDist(PTWindowsObject AParent, LPSTR ATitle,PTapcashd bdp):TDialog(AParent,ATitle)
  {
  ListArray = new TListArray(1000, 0, 50);
  apcashd = bdp;
  };

  virtual void SetupWindow();
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  ~TAPCashDist(){delete ListArray;};
};



_CLASSDEF(TAPBatchList)
class TAPBatchList : public TDialog
{
 public:
 PTListArray ListArray;
 PTapbtch  apbtch;
 PTapcash  apcash;
 TAPBatchList(PTWindowsObject AParent, LPSTR AName,PTapbtch glb):
  TDialog (AParent, AName) {apbtch=glb; ListArray = new TListArray(1000, 0, 50);};
 TAPBatchList(PTWindowsObject AParent, LPSTR AName,PTapcash glb):
  TDialog (AParent, AName) {apcash=glb; ListArray = new TListArray(1000, 0, 50);};
 ~TAPBatchList();
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void Cancel        (RTMessage Msg) = [ID_FIRST + IDCANCEL] {ShutDownWindow();};
 virtual void SetupWindow();

};

_CLASSDEF(TAPCashBatch)
class TAPCashBatch : public TDialog
{
 public:
 PTListArray ListArray;
 PTapcash  apcash;
 TAPCashBatch(PTWindowsObject AParent, LPSTR AName,PTapcash glb):
  TDialog (AParent, AName) {apcash=glb; ListArray = new TListArray(1000, 0, 50);};
 ~TAPCashBatch(){delete ListArray;};
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void Cancel        (RTMessage Msg) = [ID_FIRST + IDCANCEL] {ShutDownWindow();};
 virtual void SetupWindow();

};


_CLASSDEF(TAPInvoiceEntry)
class TAPInvoiceEntry : public TMainDialog
{
 public:
  char  szVendorNo[7],szAdj[2];
  char  szInvoiceNo[10],szOrderNo[10],szBuffer[85];
  char  szReference[40],szAccounts[8],szDepart[8],szDescript[41];
  char  szDueDate[10],szInvoiceDate[10];
  char  szCompanyTaxAccount[8],szCompanyTaxDept[5];
  char  szIndTaxAccount[8],szIndTaxDept[5];
  char  szVATAccount[8],szVATDept[5];
  char  szVendorName[41];
  double lDebit,DebitTotal,CreditTotal,lCredit,lInvoiceAmount,dTerms,dVatPercentage,dRebatePercentage;
  int iBatchno,iEntryno,iPeriodNo;
  long iLineno;


  PTGLAcctList AcctList;
  PTglaccts  glacct;
  PTAPVendorList VendorList;
  PTAPBatchList  SlipList;

  PTapbtch   apbtch;
  PTapbtchd  apbtchd;
  PTgldept   gldept;
  PTapvendor apvendor;
  PTapsetup  apsetup;
  BOOL bSlipList;
  Date4 InvoiceDate;
  Date4 DueDate;

  TAPInvoiceEntry(PTWindowsObject AParent,LPSTR ATitle):TMainDialog(AParent,ATitle)
  {
   bSlipList = FALSE;
  };
  virtual LPSTR GetClassName(){ return "APInvoiceEntry"; }

  virtual void HandleDepart    (RTMessage Msg) = [ID_FIRST + ID_DEPART];
  virtual void VList	       (RTMessage Msg) = [ID_FIRST + ID_SEARCHCUST]{Vendorlist();};
  virtual void SList           (RTMessage Msg) = [ID_FIRST + ID_SLIPLIST]{Sliplist();};
  virtual void Cancel          (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Ok              (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void SearchGL        (RTMessage Msg) = [ID_FIRST + ID_VIEW];        //Accountlist();
  virtual void CurrentBatch    (RTMessage Msg) = [CM_FIRST + CM_CURRENTBATCH];
  virtual void New             (RTMessage Msg) = [ID_FIRST + ID_NEW];
  virtual void SetVendor       (RTMessage Msg) = [CM_FIRST + CM_SETVENDOR];
  virtual void SetupWindow();
  void Sliplist();
  void Accountlist();
  void Vendorlist();
  void InvoiceData();
  int  CheckGLAccount(char *);
  int  CheckGLDept(char *);
  void CreateNewApSlip(PTapbtch apbtch,PTapbtchd apbtchd,PTAPDistList D);
  void CreateVatSlip(PTapbtch apbtch,PTapbtchd apbtchd,PTAPDistList D);
  void CreateFiveSlip(PTapbtch apbtch,PTapbtchd apbtchd,PTAPDistList D);
  void CreateThreeSlip(PTapbtch apbtch,PTapbtchd apbtchd,PTAPDistList D);
  void ModifyCurrentSlip(PTapbtch apbtch,PTapbtchd apbtchd,PTAPDistList D);
  void CreateRebateSlip(PTapbtch apbtch,PTapbtchd apbtchd,PTAPDistList D);
  void CreateVat_RebateSlip(PTapbtch apbtch,PTapbtchd apbtchd,PTAPDistList D);
  char *GetControlAcct(char *Account,char *Depart);
  PTAPDistList D;
};

_CLASSDEF(TAPCashEntry)
class TAPCashEntry : public TAPInvoiceEntry
{
 public:
  PTAPCashBatch SlipList;
  PTAPCashDist D;
  PTapcash apcash;
  PTapcashd apcashd;
  void InvoiceData();
  void Sliplist();
  void CreateNewApSlip(PTapcash apcash,PTapcashd apcashd,PTAPCashDist D);
  void CreateVatSlip(PTapcash apcash,PTapcashd apcashd,PTAPCashDist D);
  void CreateRebateSlip(PTapcash apcash,PTapcashd apcashd,PTAPCashDist D);
  void CreateVat_RebateSlip(PTapcash apcash,PTapcashd apcashd,PTAPCashDist D);
  void ModifyCurrentSlip(PTapcash apcash,PTapcashd apcashd,PTAPCashDist D);
  virtual void SList           (RTMessage Msg) = [ID_FIRST + ID_SLIPLIST]{Sliplist();};
  virtual void CurrentBatch    (RTMessage Msg) = [CM_FIRST + CM_CURRENTBATCH];
  virtual void Ok              (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void New             (RTMessage Msg) = [ID_FIRST + ID_NEW];
  virtual void SetupWindow();
  TAPCashEntry(PTWindowsObject AParent,LPSTR ATitle):TAPInvoiceEntry(AParent,ATitle){};
  virtual LPSTR GetClassName(){ return "APCashEntry"; }
};

_CLASSDEF(TAPInvoiceList)
class TAPInvoiceList : public TDialog
{
 public: 
 PTapbtch  apbtch;
 TAPInvoiceList(PTWindowsObject AParent, LPSTR AName,PTapbtch glb):
  TDialog (AParent, AName)
  {
   apbtch=glb;
  }; 
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void Cancel        (RTMessage Msg) = [ID_FIRST + IDCANCEL] {ShutDownWindow();};
 virtual void Redraw        (RTMessage Msg) = [CM_FIRST + CM_SETDATA];
 virtual void SetupWindow();
};

_CLASSDEF(TAPAdjEntry)
class TAPAdjEntry : public TAPInvoiceEntry
{
 public:
  BOOL bRedraw;
  BOOL bDeleted;
  BOOL bRecovered;
  PTAPInvoiceList InvoiceList;
  PTgldist gldist;
  PTglaccts glaccts;
  PTapdist  apdist;
  TAPAdjEntry(PTWindowsObject AParent,LPSTR ATitle):TAPInvoiceEntry(AParent,ATitle){};
  virtual LPSTR GetClassName(){ return "APAdjEntry"; }
  virtual void SetupWindow();
  virtual void SList (RTMessage Msg) = [ID_FIRST + ID_SLIPLIST];
  virtual void CurrentBatch    (RTMessage Msg) = [CM_FIRST + CM_CURRENTBATCH];
  virtual void Ok              (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void CreateAdjustment(RTMessage Msg) = [ID_FIRST + ID_CREATEADJ];
  void RecoverPostedInvoice();
  void SetPeriodAmount(double period,char *szAccountNo,double dBatchNo);
};



