


_CLASSDEF (TAPVendorMod)
class TAPVendorMod : public TAPVendorAdd
{
 public:
 TAPVendorMod(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL):TAPVendorAdd(AParent,AName){};
 virtual void WMInitDialog  (RTMessage Msg)  = [WM_FIRST + WM_INITDIALOG];
};

_CLASSDEF (TAPVendorDel)
class TAPVendorDel : public TAPVendorAdd
{
 public:
 TAPVendorDel(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL):TAPVendorAdd(AParent,AName){};
 virtual void WMInitDialog  (RTMessage Msg)  = [WM_FIRST + WM_INITDIALOG];
 virtual void Ok            (RTMessage Msg)  = [ID_FIRST + IDOK];
};



// Accounts Payable Entry header file.


_CLASSDEF (TAPDistList)
class TAPDistList : public TDialog
{
 public:
  SS_CELLTYPE   CellType;
  LOGFONT       lfFont;
  HFONT         hFont;
  HWND SHWindow;
  PTapbtchd apbtd;
  PTListArray ListArray;
  TAPDistList(PTWindowsObject AParent, LPSTR ATitle,PTapbtchd bdp):TDialog(AParent,ATitle)
  {
  ListArray = new TListArray(1000, 0, 50);
  apbtd = bdp;
  };
  virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  ~TAPDistList(){delete ListArray;};
};


_CLASSDEF(TAPBatchList)
class TAPBatchList : public TDialog
{
 public:
 PTListArray ListArray;
 PTapbtch  apbtch;
 TAPBatchList(PTWindowsObject AParent, LPSTR AName,PTapbtch glb):
  TDialog (AParent, AName) {apbtch=glb; ListArray = new TListArray(1000, 0, 50);};
 ~TAPBatchList();
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void Cancel        (RTMessage Msg) = [ID_FIRST + IDCANCEL] {ShutDownWindow();};
 virtual void SetupWindow();


};

_CLASSDEF(TAPInvoiceEntry)
class TAPInvoiceEntry : public TDialog
{
 private:
  
  
  char  szReference[20],szAccounts[8],szDepart[8],szDescript[41];
  char  szDueDate[10],szInvoiceDate[10];
  double lDebit,DebitTotal,CreditTotal,lCredit,lInvoiceAmount,dTerms,dVatPercentage,dRebatePercentage;
  int iBatchno,iEntryno,iPeriodNo;
  long iLineno;

 public:
  char  *szVendorNo;
  char  *szInvoiceNo;
  char  *szOrderNo;
  char  *szBuffer;
  PTGLAcctList AcctList;
  PTglaccts  glacct;
  PTAPVendorList VendorList;
  PTAPBatchList  SlipList;

  PTapbtch   apbtch;
  PTapbtchd  apbtchd;
  PTgldept   gldept;
  PTapvendor apvendor;
  PT


  TAPInvoiceEntry(PTWindowsObject AParent,LPSTR ATitle):TDialog(AParent,ATitle){};
  virtual LPSTR GetClassName(){ return "APInvoiceEntry"; }
  virtual void WMInitDialog    (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
  virtual void WMRButtonDown   (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
  virtual void HandleDepart    (RTMessage Msg) = [ID_FIRST + ID_DEPART];
  virtual void Cancel          (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Ok              (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void ToolCancel      (RTMessage Msg) = [WM_FIRST + WM_DATACANCEL];
  virtual void ToolOk          (RTMessage Msg) = [ID_FIRST + ID_ADD];
  virtual void ToolForward     (RTMessage Msg) = [WM_FIRST + WM_DATAFORWARD];
  virtual void ToolBack        (RTMessage Msg) = [WM_FIRST + WM_DATABACK];
  virtual void ToolView        (RTMessage Msg) = [ID_FIRST + ID_VIEWBATCH];   //Sliplist(); Slip list
  virtual void ToolNew         (RTMessage Msg) = [ID_FIRST + ID_NEW];         //New()
  virtual void ToolVendor      (RTMessage Msg) = [ID_FIRST + ID_VENDOR];      //Vendorlist();
  virtual void SearchGL        (RTMessage Msg) = [ID_FIRST + ID_VIEW];        //Accountlist();
  virtual void CurrentBatch     (RTMessage Msg) = [WM_FIRST + WM_CURRENTBATCH];
  void SetBitmap(int ID,char *szBitmap);
  void Forward ();
  void Back();
  void Sliplist();
  void Accountlist();
  void Vendorlist();
  void New();
  void InvoiceData();
  int  CheckGLAccount(char *);
  int  CheckGLDept(char *);
  void CreateNewApSlip(PTapbtch apbtch,PTapbtchd apbtchd,PTAPDistList D);
  void ModifyCurrentSlip(PTapbtch apbtch,PTapbtchd apbtchd,PTAPDistList D);
  PTAPDistList D;
};

