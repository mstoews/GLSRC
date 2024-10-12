#ifndef __AR_CLR_H
#define __AR_CLR_H

class TARClear: public TMainDialog
{
 public:
 BOOL Hold;
 BOOL Force;
 BOOL Partial;
 BOOL Discount;
 BOOL bDelete;
 BOOL bWithHolding;
 Tardist* ardist;
 Tarcust* arcust;
 string*  szSales;
 string*  szSalesDept;
 string*	 szVAT;
 string*	 szVATDept;
 string*	 szAR;
 string*	 szARDept;
 string*  szWithAcct;
 string*	 szWithDept;

 char szInvoice[21];
 TARClear(PTWindowsObject AParent, int AName,char *szInvoiceno): TMainDialog(AParent,AName)
 {
	strcpy(szInvoice,szInvoiceno);
 };
 ~TARClear()
  {
	//delete ardist;
	//delete ar_cust;
  }
 virtual void SetupWindow();
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void GetBankAccount (RTMessage Msg) = [ID_FIRST + ID_DESCRIPT];
 virtual void SetPartial     (RTMessage Msg) = [ID_FIRST + ID_PARTIAL];
 virtual void SetDiscount    (RTMessage Msg) = [ID_FIRST + ID_DISCOUNT];
 virtual void SetWithholding (RTMessage Msg) = [ID_FIRST + ID_WITHHOLDING];
 virtual void Reset          (RTMessage Msg) = [ID_FIRST + ID_RESET];
 void    SetCashAccounts();
 BOOL CheckAccount(char *szAccount,char *szDepart);
 virtual void SetData(Data4 *dbf,int ID,int szField);
};

_CLASSDEF (TGlobalClearing)
class TGlobalClearing : public TARClear
{
  public:
  TGlobalClearing(PTWindowsObject AParent, int AName,char* szInvoiceno = ""): TARClear(AParent,AName,szInvoiceno){};
  virtual void SetupWindow();
  virtual void Ok (RTMessage Msg) = [ID_FIRST + IDOK];
};
#endif
