#ifndef __AR_PAYMT_H
#define __AR_PAYMT_H
#define ID_REDRAW 2001
#define ID_GLOBAL_CLEARING 1005
#define ID_GLOBAL 1004


_CLASSDEF (TARCreatePayment)
class TARCreatePayment : public TMainDialog
{
 public:
  string szSales,szSalesDept,szVAT,szVATDept,szAR,szARDept,szWithAcct,szWithDept;
  string strAccount,strDept;
  TARDistribution *D;
  TARClear* arclear;
  BOOL          bHasInvoices;
  BOOL          bCancel;
  BOOL          bWithHolding;
  SS_CELLTYPE 	CellType;
  HFONT   	hFont;
  HWND 		SHWindow;
  Tardist  ardist;
  Tarcust  arcust;
  Tarbtch	arbtch;
  Tarclear  ar_clr;
  TSpread*  InvoiceList;
  TARCreatePayment(PTWindowsObject AParent, int ATitle):TMainDialog(AParent,ATitle)
  {
	InvoiceList = new TSpread(this,ID_DISTLIST);
	D = NULL;
	bHasInvoices = FALSE;
  };
  ~TARCreatePayment()
	{
	 delete InvoiceList;
	 if(D != NULL)
		 delete D;
	ardist.close();
	arcust.close();
	arbtch.close();
	}
  virtual void SetupWindow();
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  virtual void SetDate        (RTMessage Msg) = [ID_FIRST + ID_DUEDATE];
  virtual void Ok	            (RTMessage Msg) = [ID_FIRST + IDOK]; // Create Journal entry.
  virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual void ViewAll        (RTMessage Msg) = [ID_FIRST + ID_VIEW];
  virtual void Seek           (RTMessage Msg) = [ID_FIRST + ID_SEARCH];
  virtual void SeekMsg        (RTMessage Msg) = [CM_FIRST + ID_SEARCH];
  virtual void Reset          (RTMessage Msg) = [ID_FIRST + ID_SET]
  {
	SetField(ID_INVOICENO,"");
	SetField(ID_VENDORNO,"");
  };
  virtual void Redraw         (RTMessage Msg) = [ID_FIRST + ID_REDRAW];
  virtual void Open           (RTMessage Msg) = [ID_FIRST + ID_OPEN];
  virtual void Paid           (RTMessage Msg) = [ID_FIRST + ID_PAID];
  virtual void All            (RTMessage Msg) = [ID_FIRST + ID_ALL];
  virtual void GlobalClearing (RTMessage Msg) = [ID_FIRST + ID_GLOBAL];  //1004
  virtual void ProcessClearGlobally  (RTMessage Msg) = [ID_FIRST + ID_GLOBAL_CLEARING];  //4102
  virtual void SetOpenMsg     (RTMessage Msg) = [CM_FIRST + ID_OPEN]{Open(Msg);};
  virtual BOOL DistList(char *szQuery);
  void 	 SetRequestNumber(char *Vendorno,int iStartingReq);
  void    PrintPaymentList(void);
  void    SetDistPaid();
  void    SetDistDiscount();
  void    SetWithHolding();
  void    PostClearedInvoices();
  char   *GetDescription(char *szAccount,char *szDept);
  void    ProcessReceivablesGlobally();
  void    SetGlobalPaid();
};
#endif