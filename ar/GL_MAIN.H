#include <array.h>

// Pre declaration of toolbar class. (Defined in app.cpp)
double GetCurrentPeriodn();
char *GetString(HINSTANCE,int ID);

_CLASSDEF(TWorksheet)
class TWorksheet;
_CLASSDEF(TToolBar)
class TToolBar;

_CLASSDEF(TListItem)
class TListItem : public Object
{
 public:
  int index,rec;
  TListItem(int Aindex,int Arec){index = Aindex, rec = Arec;}
  virtual classType isA() const { return __firstUserClass; }
  virtual Pchar nameOf() const { return "TListItem"; }
  virtual hashValueType hashValue() const { return 0; }
  virtual int isEqual(RCObject AListItem) const
    { return index == ((RTListItem)AListItem).index && rec == ((RTListItem)AListItem).rec; }
  virtual void printOn(Rostream outputStream) const
    { outputStream << "(" << index << "," << rec << ")"; }

};

_CLASSDEF(TListArray)
class TListArray : public Array
{
public:
  TListArray(int upper, int lower = 0, sizeType aDelta = 0)
    : Array(upper, lower, aDelta){};
  virtual classType isA() const { return __firstUserClass + 1; }
  virtual Pchar nameOf() const { return "TListArray"; }
};


_CLASSDEF(TGLClient)
class TGLClient : public TMDIClient
{
 public:
  TGLClient(PTMDIFrame AParent,PTModule AModule = NULL):TMDIClient(AParent){};
  virtual void WMRButtonDown(RTMessage  Msg) = [WM_FIRST + WM_RBUTTONDOWN];
};


_CLASSDEF (TStatBar)
class TStatBar : public TDialog
{
 public:
  TStatBar(PTWindowsObject AParent,LPSTR AName): TDialog(AParent,AName){};
  virtual void WMPaint(TMessage& Msg)       = [WM_FIRST + WM_PAINT];
  virtual void WMInitDialog (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
};


_CLASSDEF (TGlWindow)
class TGlWindow:public TMDIFrame
{
 public:
 char Period[25];
 HWND HToolBar;
 HWND HStatusBar;
 TGlWindow(LPSTR ATitle,LPSTR MenuName);
 PTToolBar ToolBar;
 PTStatBar Status;
 PTprofile profile;       
 PTWorksheet Worksheet;


 // Helper Functions
 void PopUpMenu (RTMessage Msg)      = [WM_FIRST + WM_POPUPMENU];
 void InitClientWindow() { ClientWnd = new TGLClient(this); }
 void CMHelp    (RTMessage Msg)     = [CM_FIRST + ID_HELP];
 virtual void SendStatusMsg (int ID);
 virtual void WMMenuSelect(RTMessage Msg)   = [WM_FIRST + WM_MENUSELECT];
 char  *GetCurrentPeriod ();
 char  *GetCompanyName ();
 // General Functions
 BOOL PrintCR(char *szReportName);
 virtual void ModifyBatch    (RTMessage Msg)   = [CM_FIRST + ID_MODIFYBATCH];
 virtual void NewBatch       (RTMessage Msg)   = [CM_FIRST + ID_NEWBATCH];
 virtual void SearchGL       (RTMessage Msg)   = [CM_FIRST + ID_SEARCHGL];
 virtual void CancelBatch    (RTMessage Msg)   = [CM_FIRST + ID_CANCELBATCH];
 virtual void CMDialogPrint  (RTMessage Msg)   = [CM_FIRST + CM_DIALOGPRINT];
 virtual void CMAbout        (RTMessage Msg)   = [CM_FIRST + ID_ABOUT];



// Accounts Payable
// virtual void APMenu        (RTMessage Msg)   = [CM_FIRST + ID_AP];
 //virtual void APSetup       (RTMessage Msg)   = [CM_FIRST + CM_FILESETUP];

 virtual void APAddVendors  (RTMessage Msg)   = [CM_FIRST + ID_APADDVENDORS];
 virtual void APModVendors  (RTMessage Msg)   = [CM_FIRST + ID_APMODVENDORS];
 virtual void APInqVendors  (RTMessage Msg)   = [CM_FIRST + ID_APINQVENDORS];
 virtual void APDelVendors  (RTMessage Msg)   = [CM_FIRST + ID_APDELVENDORS];
 virtual void APVendorInquiry (RTMessage Msg)   = [CM_FIRST + ID_VENDORACCT];

 virtual void APBatch       (RTMessage Msg)   = [CM_FIRST + ID_APBATCH];

 virtual void APPrintBatch  (RTMessage Msg)   = [CM_FIRST + ID_APPRINTBATCH];
 virtual void APPrintCash   (RTMessage Msg)   = [CM_FIRST + ID_APPRINTCASH];
 virtual void APPrintAdj    (RTMessage Msg)   = [CM_FIRST + ID_APPRINTADJ];

 virtual void APPostBatch   (RTMessage Msg)   = [CM_FIRST + ID_APPOSTBATCH];

 virtual void APCancelBatch (RTMessage Msg)   = [CM_FIRST + ID_BATCHCANCEL];

 virtual void APPostJournal (RTMessage Msg)   = [CM_FIRST + ID_APPOSTJOURNAL];

 virtual void APAdjBatchPrt (RTMessage Msg)   = [CM_FIRST + ID_ADJBATCHPRINT];
 virtual void APAdjPosting  (RTMessage Msg)   = [CM_FIRST + ID_ADJPOSTING];


 virtual void APPrePayment  (RTMessage Msg)   = [CM_FIRST + ID_PREPAYMENT];
 virtual void APVenPayment  (RTMessage Msg)   = [CM_FIRST + ID_VENPAYMENT];
 virtual void APCreateGLSlip(RTMessage Msg)   = [CM_FIRST + ID_GLSLIP];
 //virtual void APPayment     (RTMessage Msg)   = [CM_FIRST + ID_PAYMENT];
 virtual void APModClearReq (RTMessage Msg)   = [CM_FIRST + ID_REQUEST];

 virtual void APVenLists    (RTMessage Msg)   = [CM_FIRST + ID_VENLISTS];
 virtual void APOpenAccts   (RTMessage Msg)   = [CM_FIRST + ID_OPENACCTS];
 virtual void APSummary     (RTMessage Msg)   = [CM_FIRST + CM_SUMMARY];
 virtual void APOverDue     (RTMessage Msg)   = [CM_FIRST + ID_OVERDUE];
 virtual void APAgedCash    (RTMessage Msg)   = [CM_FIRST + ID_AGEDCASH];
 virtual void APAgedRetain  (RTMessage Msg)   = [CM_FIRST + ID_AGEDRETAIN];
 virtual void APBatchStatus (RTMessage Msg)   = [CM_FIRST + ID_BATCHSTATUS];

 virtual void APVatAPReport   (RTMessage Msg) = [CM_FIRST + CM_VATAP_REPORT];
 virtual void APVatGLReport   (RTMessage Msg) = [CM_FIRST + CM_VATGL_REPORT];
 virtual void APChequeCash   (RTMessage Msg) = [CM_FIRST + CM_CHEQUE_CASH];
 virtual void APChequeAP     (RTMessage Msg) = [CM_FIRST + CM_CHEQUE_AP];
 virtual void APJournalAP    (RTMessage Msg) = [CM_FIRST + CM_AP_JSUM];
 virtual void APJournalCash  (RTMessage Msg) = [CM_FIRST + CM_CASH_JSUM];
 virtual void APBatchAP      (RTMessage Msg) = [CM_FIRST + CM_AP_BTCHD];
 virtual void APBatchCash    (RTMessage Msg) = [CM_FIRST + CM_AP_CASHD];
 virtual void APGLSummary    (RTMessage Msg) = [CM_FIRST + CM_GLSUMMARY];
 virtual void APGLDetail      (RTMessage Msg) = [CM_FIRST + CM_GLDETAIL];
 virtual void APRebateSummary (RTMessage Msg) = [CM_FIRST + CM_REBATE_REPORT];

 virtual void APPurgeAccts  (RTMessage Msg)   = [CM_FIRST + ID_PURGEACCTS];
 virtual void APYearEnd     (RTMessage Msg)   = [CM_FIRST + ID_YEAREND];
 virtual void APExternalBat (RTMessage Msg)   = [CM_FIRST + ID_EXTBATCH];
 virtual void APSysReindex  (RTMessage Msg)   = [CM_FIRST + ID_SYSINDEX];
 virtual void APRequestSummary (RTMessage Msg) = [CM_FIRST + ID_REQUESTSUM];
 virtual void APRequestDetail (RTMessage Msg)  = [CM_FIRST + ID_REQUESTDETAIL];
 virtual void Calculator(RTMessage Msg)        = [CM_FIRST + ID_CALCULATOR];
 virtual void GeneralLedger (RTMessage Msg)   = [CM_FIRST + ID_GL];
 virtual void AccountsPayable (RTMessage Msg) = [CM_FIRST + ID_AP];

// GLWindows Windows class registration - this is different than a C++ class.
 virtual LPSTR GetClassName(){ return "ARWindow"; }
 virtual void GetWindowClass(WNDCLASS& wc);

 virtual void WMSize        (RTMessage Msg)   = [WM_FIRST + WM_SIZE];
 HWND GLCreateStatusBar();
 HWND GlCreateToolBar(HWND HWindow);
 void SetupWindow();
};
