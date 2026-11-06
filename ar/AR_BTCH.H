#ifndef _AP_BTCHE.H
#define _AP_BTCHE.H
#define ID_MEMO     4376
#include "ar_toolb.h"
#include "gl_acctl.h"
#include "db_drcod.h"
#define CM_ROLLBACK 4375
#define ID_DRCODES  1010


_CLASSDEF(TAPInvoiceEntry)
class TAPInvoiceEntry : public TMainDialog
{
 public:
  char  *szIndTaxAccount,*szIndTaxDept;
  char  *szVATAccount,*szVATDept;
  double lDebit,DebitTotal,CreditTotal,lCredit,lInvoiceAmount,dTerms,dVatPercentage,dRebatePercentage;
  int iBatchno,iEntryno,iPeriodNo;
  long iLineno;
  
  //PTAPBatchList  	SlipList;
  //PTGLAcctList 		AcctList;

  TAccountList         *AcctList;
  TCustomerList        *VendorList;
  TBatchList           *SlipList;
  TARToolBox	       *ToolBox;
  
  PTglaccts  		glacct;
  PTgldist 		gldist;
  PTapdist  		apdist;
  PTapsetup  		apsetup;
  PTapbtch   		apbtch;
  PTapbtchd  		apbtchd;
  PTgldept   		gldept;
  PTprods		prods;
  PTglmedreps		medreps;
  Tgldrcodes            *drcodes;

  RECT child;
  RECT parent;
  RECT rc;
  BOOL bFullSize;

  PTapvendor 		apvendor;
  BOOL bSlipList,bAutomaticCalc,bVendorList;
  Date4 InvoiceDate;
  Date4 DueDate;

  BOOL bDeleted;
  BOOL bRecovered;
  BOOL bTransaction;
  BOOL bModify;


  TAPInvoiceEntry(PTWindowsObject AParent,LPSTR ATitle):TMainDialog(AParent,ATitle)
  {
   bSlipList = FALSE;
   bVendorList = TRUE;
  };
  virtual LPSTR GetClassName(){ return "ARInvoiceEntry"; }

  virtual void HandleDepart    (RTMessage Msg) = [ID_FIRST + ID_DEPART];

  virtual void VList	       (RTMessage Msg) = [ID_FIRST + ID_SEARCHCUST]{Vendorlist();};
  virtual void GoVList	       (RTMessage Msg) = [CM_FIRST + ID_SEARCHCUST]{VList(Msg);};

  virtual void SList           (RTMessage Msg) = [ID_FIRST + ID_SLIPLIST]{Sliplist();};
  virtual void GoSList         (RTMessage Msg) = [CM_FIRST + ID_SLIPLIST]{SList(Msg);};

  virtual void Cancel          (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void GoCancel        (RTMessage Msg) = [CM_FIRST + IDCANCEL]{Cancel(Msg);};

  virtual void Ok              (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void DoOk            (RTMessage Msg) = [CM_FIRST + IDOK]{Ok(Msg);};

  virtual void SearchGL        (RTMessage Msg) = [ID_FIRST + ID_VIEW];
  virtual void GoSearchGL      (RTMessage Msg) = [CM_FIRST + ID_VIEW]{SearchGL(Msg);};

  virtual void CurrentBatch    (RTMessage Msg) = [CM_FIRST + CM_CURRENTBATCH];

  virtual void New             (RTMessage Msg) = [ID_FIRST + ID_NEW];
  virtual void GoNew           (RTMessage Msg) = [CM_FIRST + ID_NEW]{New(Msg);};

  virtual void SetVendor       (RTMessage Msg) = [CM_FIRST + CM_SETVENDOR];

  virtual void Top             (RTMessage Msg) = [ID_FIRST + ID_TOP];
  virtual void GoTop           (RTMessage Msg) = [CM_FIRST + ID_TOP]{Top(Msg);};

  virtual void Bottom          (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
  virtual void GoBottom        (RTMessage Msg) = [CM_FIRST + ID_BOTTOM]{Bottom(Msg);};

  virtual void Next            (RTMessage Msg) = [ID_FIRST + ID_NEXT];
  virtual void GoNext 	       (RTMessage Msg) = [CM_FIRST + ID_NEXT]{Next(Msg);};

  virtual void Previous        (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
  virtual void GoPrevious      (RTMessage Msg) = [CM_FIRST + ID_PREVIOUS]{Previous(Msg);};

  virtual void Delete          (RTMessage Msg) = [ID_FIRST + ID_DELETE];
  virtual void GlDelete        (RTMessage Msg) = [CM_FIRST + ID_DELETE]{Delete(Msg);};

  virtual void Automatic       (RTMessage Msg) = [ID_FIRST + ID_AUTO];
  virtual void SetLineItem     (RTMessage Msg) = [CM_FIRST + CM_LINEITEM];
  virtual void AddVendor       (RTMessage Msg) = [ID_FIRST + ID_APADDVENDORS];
  virtual void Memo            (RTMessage Msg) = [CM_FIRST + ID_MEMO];


  virtual void SelectMedRep    (RTMessage Msg) = [ID_FIRST + ID_MEDREPS];
  virtual void SelectProduct   (RTMessage Msg) = [ID_FIRST + ID_PRODUCTS];
  virtual void SelectDoctor    (RTMessage Msg) = [ID_FIRST + ID_DRCODES];
  virtual void RollBack        (RTMessage Msg) = [CM_FIRST + CM_ROLLBACK];

  virtual void SetupWindow();
  void ChangeSize(BOOL bFull);
  void Sliplist();
  void Accountlist();
  void Vendorlist();
  void LoadMedReps(void);
  void LoadProducts(void);
  void LoadDoctors();
  void InvoiceData();
  void Clear();
  int  CheckGLAccount(char *);
  int  CheckGLDept(char *);
  void ModifyCurrentSlip(int iBatchno);
  void CreateAPBatchEntry(int iBatchno);
  void RecoverPostedInvoice();
  void CreateRebateSlip(PTapbtch apbtch,PTapbtchd apbtchd,PTAPDistList D);
  void CreateVat_RebateSlip(PTapbtch apbtch,PTapbtchd apbtchd,PTAPDistList D);
  void CreateLineItem(PTapbtch apbtch,PTapbtchd apbtchd);
  char *GetControlAcct(char *Account,char *Depart);
  void UpdateBatchDetail(int iBatchno, char *szDescript, char *szAccount,char *szDepart,char *szMedrep, char *szProduct,char *szDoctor,double dEntry);
  void UpdateBatchHeader(int iBatchno, double AMOUNT, double VAT, double REBATE);
  void AddBatchDetail(int iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry);
  void UpdateAR(int iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry);
  void UpdateVAT(int iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry);
  char *GetDesc(int ID);
  char *GetSalesDescription(void);
  
  BOOL EnterError(LPSTR szValid,UINT ID);
  void AddLine(long iEntryno, long iBatchno,
			      char *szDescript,
			      char *szAccount,
			      char *szMedrep,
			      char *szProduct,
                              char *szDoctor,
			      char *szDepart,
			      double dQty,
			      double dPrice,
			      double dEntry);
  PTAPDistList D;
};
#endif