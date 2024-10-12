#ifndef _AR_BTCHE.H
#define _AR_BTCHE.H
/*
#include "gl_dlg.h"
#include "db_itemm.h"
#include "db_medrp.h"
#include "db_cash.h"
#include "db_drcod.h"
#include "db_dist.h"
#include "db_ardst.h"
#include "db_cust.h"
#include "db_arbtc.h"

//#include "ar_class.h"   // AR distribution list dialog class.
#include "gl_distl.h"
#include "gl_acctl.h"
#include "ar_custl.h"
#include "ar_btchl.h"
#include "ar_toolb.h"
#include "gl_acctl.h"
#include "ap_class.h"
*/

#define ID_DRCODE  1010



typedef struct LINEEXPENSE
	{
	 long iBatchno;
	 int  iEntryno;
	 string szAccount;
	 string szDept;
	 string szDescript;
	 string szProduct;
	 string szMedrep;
	 string szDoctor;
	 double dPrice;
	 double dQty;
	 double dEntry;
	} LPLINEITEM;

class TARBatchList;

_CLASSDEF(TARInvoiceEntry)
class TARInvoiceEntry : public TMainDialog
{
 public:
  string mode;
  char  *szIndTaxAccount,*szIndTaxDept;
  char  *szVATAccount,*szVATDept;
  double lDebit,DebitTotal,CreditTotal,lCredit,lInvoiceAmount,dTerms,dVatPercentage,dRebatePercentage;
  long iBatchno,iEntryno,iPeriodNo;
  long iLineno;

  TAccountList        *AcctList;
  TARCustomerList     *VendorList;
  TARBatchList        *SlipList;
  //TARToolBox	       *ToolBox;
  TARDistList 			 *D;

  Tglaccts  		glacct;
  Tgldist 		   gldist;
  Tardist  		   ardist;
  Tarbtch   		arbtch;
  Tarbtchd  		arbtchd;
  Tprods		      prods;
  //Tglmedreps		medreps;
  Tgldept         gldept;
  Tgldrcodes      drcodes;
  Tarcust 		   arcust;

  RECT child;
  RECT parent;
  RECT rc;
  BOOL bFullSize;
  BOOL bRedraw;


  BOOL bSlipList,bAutomaticCalc,bVendorList;
  Date4 InvoiceDate;
  Date4 DueDate;

  BOOL bDeleted;
  BOOL bRecovered;
  BOOL bTransaction;
  BOOL bModify;


  TARInvoiceEntry(PTWindowsObject AParent,int ATitle):TMainDialog(AParent,ATitle)
  {
	bSlipList = FALSE;
	bVendorList = TRUE;

  };
  virtual LPSTR GetClassName(){ return "ARInvoiceEntry"; }

  virtual void HandleDepart    (RTMessage Msg) = [ID_FIRST + ID_DEPART];
  virtual void Vendors         (RTMessage Msg) = [ID_FIRST + ID_SEARCH]{Vendorlist();};
  virtual void VList	          (RTMessage Msg) = [ID_FIRST + ID_SEARCHCUST]{Vendorlist();};
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
  virtual void Memo            (RTMessage Msg) = [ID_FIRST + 1100];
  //virtual void GoMemo          (RTMessage Msg) = [ID_FIRST + ID_MEMO];
  virtual void Period         (RTMessage Msg) = [ID_FIRST + ID_PERIOD];
  virtual void Print 	      (RTMessage Msg) = [ID_FIRST + ID_PRINT];

  //virtual void SelectMedRep    (RTMessage Msg) = [ID_FIRST + ID_MEDREP];
  virtual void SelectProduct   (RTMessage Msg) = [ID_FIRST + ID_PRODUCTS];
  virtual void SelectDoctor    (RTMessage Msg) = [ID_FIRST + ID_DRCODES];
  virtual void RollBack        (RTMessage Msg) = [CM_FIRST + CM_ROLLBACK];
  virtual void ChangeDept      (RTMessage Msg) = [ID_FIRST + ID_DEPTCODES];

  virtual void SetupWindow();
  void UpdateBatchSupp(long iBatchno,string* szMedRep,string* szProduct,string* szDoctor);
  void ChangeSize(BOOL bFull);
  void Sliplist();
  void Accountlist();
  void Vendorlist();
  //void LoadMedReps(void);
  void LoadDepartments(void);
  void LoadProducts(void);
  void LoadDoctors();
  void InvoiceData();
  void Clear();
  long  GetPeriod();
  int  CheckGLAccount(char *);
  int  CheckGLDept(char *);
  void ModifyCurrentSlip(long iBatchno);
  void CreateAPBatchEntry(long iBatchno);
  void RecoverPostedInvoice();
  void CreateRebateSlip(PTarbtch arbtch,PTarbtchd arbtchd,TARDistList *D);
  void CreateVat_RebateSlip(PTarbtch arbtch,PTarbtchd arbtchd,PTARDistList D);
  void CreateLineItem(PTarbtch arbtch,PTarbtchd arbtchd);
  char *GetControlAcct(char *Account,char *Depart);

  char *GetDept();

  void UpdateBatchHeader(long iBatchno, double AMOUNT, double VAT, double REBATE);
  //void AddBatchDetail(int iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry);
  void UpdateAR(long iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry);
  void UpdateVAT(long iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry);
  char *GetDesc(int ID);
  char *GetSalesDescription(void);
  BOOL EnterError(LPSTR szValid,UINT ID);
  void AddLine(LPLINEITEM* lineItem);
  void AddBatchDetail(LPLINEITEM* lineItem);
  void UpdateBatchDetail(LPLINEITEM* lineItem);
  //void AddBatchDetail(int iBatchno, char *szDescript, char *szProduct,char *szMedrep,char *szDoctor,char *szAccount,char *szDepart,double dEntry);
  //void UpdateBatchDetail(int iBatchno, char *szDescript, char *szAccount,char *szDepart,char *szMedrep, char *szProduct,char *szDoctor,double dEntry);

};

_CLASSDEF(TARPriorPeriod)
class TARPriorPeriod : public TARInvoiceEntry
{
 public:
  TARPriorPeriod(PTWindowsObject AParent,int ATitle):TARInvoiceEntry(AParent,ATitle)
  {
	bSlipList = FALSE;
	bVendorList = TRUE;
  };
 virtual void SetupWindow();
};
#endif