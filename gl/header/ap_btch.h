#ifndef _AP_BTCH.H
#define _AP_BTCH.H
#include "gl_acctl.h"
#include "ap_vendr.h"
#include "ap_btchl.h"
#include "db_drcod.h"

#define  CM_ROLLBACK 1096
typedef struct EXPENSE {
	 double NEWAP;
	 double OLDAP;
	 double OLDEXPENSE;
	 double VAT;
	 double NEWEXPENSE;
	 double REBATE;
	 double APAMT;
	} AP_EXPENSE;


_CLASSDEF(TAPInvoiceEntry)
class TAPInvoiceEntry : public TMainDialog
{
 public:
  char  *szVendorNo;
  char  *szAdj;
  char  *szInvoiceNo;
  char  *szOrderNo;
  char  *szBuffer;
  char  *szReference,*szAccounts,*szDepart,*szDescript;
  char  *szDueDate,*szInvoiceDate;
  char  szCompanyTaxAccount[8],szCompanyTaxDept[5],szIndTaxAccount[8],szIndTaxDept[5];
  char  szVATAccount[8],szVATDept[5];
  char  *szVendorName;
  BOOL bFullSize;
  RECT rc;
  RECT child;
  RECT parent;

  double lDebit,DebitTotal,CreditTotal,lCredit,lInvoiceAmount,dTerms,dVatPercentage,dRebatePercentage;
  int iBatchno,iEntryno,iPeriodNo;
  long iLineno;

  TAccountList		*AcctList;
  TCustomerList 	*VendorList;
  TBatchList	  	*SlipList;

  PTglaccts  		glacct;
  PTgldist 		   gldist;
  PTapdist  		apdist;
  PTapsetup  		apsetup;
  PTapbtch   		apbtch;
  PTapbtchd  		apbtchd;
  PTgldept   		gldept;
  PTapvendor 		apvendor;
  PTprods		   prods;
  PTglmedreps		medreps;
  Tgldrcodes      *drcodes;
  BOOL bSlipList,bAutomaticCalc,bVendorList;
  Date4 InvoiceDate;
  Date4 DueDate;
  BOOL bDeleted;
  BOOL bRecovered;
  BOOL bTransaction;
  BOOL bModify;

  TAPInvoiceEntry(PTWindowsObject AParent,int ATitle):TMainDialog(AParent,ATitle)
  {
	bSlipList = FALSE;
   bVendorList = TRUE;
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
  virtual void Top             (RTMessage Msg) = [ID_FIRST + ID_TOP];
  virtual void Bottom          (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
  virtual void Next            (RTMessage Msg) = [ID_FIRST + ID_NEXT];
  virtual void Previous        (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
  virtual void Delete          (RTMessage Msg) = [ID_FIRST + ID_DELETE];
  virtual void Automatic       (RTMessage Msg) = [ID_FIRST + ID_AUTO];
  virtual void SetLineItem     (RTMessage Msg) = [CM_FIRST + CM_LINEITEM];
  virtual void Print           (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual void AddVendor       (RTMessage Msg) = [ID_FIRST + ID_APADDVENDORS];
  virtual void ProductList     (RTMessage Msg) = [ID_FIRST + ID_VENDORACCT];
  virtual void Period          (RTMessage Msg) = [ID_FIRST + ID_PERIOD];
  virtual void RollBack        (RTMessage Msg) = [CM_FIRST + CM_ROLLBACK];
  virtual void Company         (RTMessage Msg) = [ID_FIRST + ID_COMPANY];
  virtual void Individual      (RTMessage Msg) = [ID_FIRST + ID_INDIVIDUAL];
  virtual void Clear();

  virtual void SetupWindow();
  void ChangeSize(BOOL bFull);
  void Sliplist();
  void Accountlist();
  void Vendorlist();
  void InvoiceData();

  int  CheckGLAccount(char *);
  int  CheckGLDept(char *);
  void ModifyCurrentSlip();
  void CreateAPBatchEntry(int iBatchno);
  void RecoverPostedInvoice();
  void CreateRebateSlip(PTapbtch apbtch,PTapbtchd apbtchd,PTAPDistList D);
  void CreateVat_RebateSlip(PTapbtch apbtch,PTapbtchd apbtchd,PTAPDistList D);
  void CreateLineItem(PTapbtch apbtch,PTapbtchd apbtchd);
  char *GetControlAcct(char *Account,char *Depart);
  void UpdateBatchDetail (long iBatchno, char *szDescript, char *szAccount,char *szDepart,double dEntry,char *szMedRep,char *szProduct,char *szDoctor);
  void UpdateBatchHeader (long iBatchno, double AMOUNT, double VAT, double REBATE);
  void UpdateBatchExpense(long iBatchno,
								  string szDescript,
								  string szAccount,
								  string szDepart,
								  double dEntry,
								  string szMedRep,
								  string szProduct,
								  string szDoctor);
  char *GetMedRep();
  char *GetProduct();
  char *GetDoctor();
  void LoadDoctors(void);
  void LoadMedReps(void);
  void LoadProducts(void);
  void GetAPExpense(AP_EXPENSE* expense,
						double lInvoiceAmount,
						double dVatPercentage,
						double dRebatPercentage);
  BOOL TAPInvoiceEntry::GetWithHolding();						
  PTAPDistList D;
};
#endif