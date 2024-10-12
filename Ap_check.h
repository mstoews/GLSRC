#ifndef __AP_CHECK_H
#define __AP_CHECK_H
_CLASSDEF (TAPCreatePayment)
class TAPCreatePayment : public TMainDialog
{
 public:
  BOOL          bHasInvoices;
  SS_CELLTYPE 	CellType;
  LOGFONT 	lfFont;
  HFONT   	hFont;
  HWND 		SHWindow;
  Tapdist       apdist;
  Tapvendor     apvendor;
  Tapbtch	    apbtch;
  Tapbtchd	    apbtchd;
  Tapjsum       apjsum;
  Tglbtch       glbtch;
  Tglbtchd	    glbtchd;
  Tglaccts      glaccts;
  Tglcash       gl_cash;
  TSpread*      InvoiceList;
  long  lPeriod;
  BOOL bVerified;

  TAPCreatePayment(PTWindowsObject AParent, int ATitle):TMainDialog(AParent,ATitle)
  {
  InvoiceList = new TSpread(this,ID_DISTLIST);
  bHasInvoices = FALSE;
  }
  ~TAPCreatePayment()
  {
	delete InvoiceList;
  }
  BOOL bSetVendors;
  long recno;
  virtual void SetupWindow();
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  virtual void SetDate        (RTMessage Msg) = [ID_FIRST + ID_SETDATE];
  virtual void Ok	            (RTMessage Msg) = [ID_FIRST + IDOK]; // Create the request.
  virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual void ViewAll        (RTMessage Msg) = [ID_FIRST + ID_VIEW];
  virtual void GetBankAccount (RTMessage Msg) = [ID_FIRST + ID_DESCRIPT];
  virtual void GetVendorCode  (RTMessage Msg) = [ID_FIRST + ID_VENDORNO];
  virtual void Verify         (RTMessage Msg) = [ID_FIRST + 1031];
  virtual BOOL DistList(char *szQuery);
  virtual void SetCashAccounts();
  virtual void SetVendorNames();
  void PrintPaymentList(void);
  void SetChequeNo();
  char *UpdateAPVendor(LPSTR szVendorno,double iStartingReq,Date4 dDatepaid);
  void CreateGLEntry(void);
  void AppendHeader(long lBatchno,double dAmount,string* date,string* reference,string* chequeno);
  void AppendDetail(long lBatchno,
						  double dAmount,
						  string* date,
						  string* reference,
						  string* chequeno,
						  string* account,
						  string* dept);

  void TAPCreatePayment::CreateCashTracing( long   lBatchno,
														string *strApaccount,
														string *strApdept,
														string *strCashAccount,
														string *strCashDept,
														long lPeriod);

  char *GetDescription(char* szAccount);
  BOOL CheckVerifiedCheckPrinting();
};
#endif