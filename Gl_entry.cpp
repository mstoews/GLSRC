#include "std_inc.h"


#define ID_INVOICE 1030
#define ID_COMMIT 1056
#define  ID_VATACCT 1034
#define  ID_VATDEPT 1035
#define  ID_VIN_NUM 1004
#define PROFILE "GL.INI"
#define  NEWBATCH  10
#define  OLDBATCH  11
#define  MODBATCH  12
#define ID_CENTURYDATE 6115
#define ID_PRIORPERIOD 8192


extern HINSTANCE hInst;

extern BOOL bNEW;


_CLASSDEF (TCreateVoucher)
class TCreateVoucher : public TMainDialog
{
 public:
 int iCurrentTab;
 TAPInvoiceEntry* APInvoice;
 TARInvoiceEntry* ARInvoice;
 TGLBatchEntry*   GLEntry;
 TAPCashEntry*    CashVoucher;

 TCreateVoucher(TWindowsObject* AParent,int AName): TMainDialog (AParent,AName){}
 virtual void HasChanged     (RTMessage Msg) = [ID_FIRST + 1001];

 void SetupWindow();
 void ShowGL();
 void ShowAP();
 void ShowAR();
 void ShowCash();
};



void TCreateVoucher::ShowGL()
{
  APInvoice->Show(SW_HIDE);
  ARInvoice->Show(SW_HIDE);
  GLEntry->Show(SW_SHOW);
  CashVoucher->Show(SW_HIDE);
}

void TCreateVoucher::ShowAR()
{
  APInvoice->Show(SW_HIDE);
  ARInvoice->Show(SW_SHOW);
  GLEntry->Show(SW_HIDE);
  CashVoucher->Show(SW_HIDE);
}

void TCreateVoucher::ShowAP()
{
  APInvoice->Show(SW_SHOW);
  ARInvoice->Show(SW_HIDE);
  GLEntry->Show(SW_HIDE);
  CashVoucher->Show(SW_HIDE);
}

void TCreateVoucher::ShowCash()
{
  APInvoice->Show(SW_HIDE);
  ARInvoice->Show(SW_HIDE);
  GLEntry->Show(SW_HIDE);
  CashVoucher->Show(SW_SHOW);
}



void TCreateVoucher::HasChanged(RTMessage)
{
		int iTab = HAGetActive(GetDlgItem(HWindow,1001));
		if(iCurrentTab != iTab)
		{
		 iCurrentTab = iTab;
		 switch(iCurrentTab)
			{
			 case 0 : ShowGL(); break;
			 case 1 : ShowAP(); break;
			 case 2 : ShowAR(); break;
			 case 3 : ShowCash(); break;
			}
		}
}


void TCreateVoucher::SetupWindow()
{
  iCurrentTab = 0;
  APInvoice  = new TAPInvoiceEntry(this,1042);
  APInvoice->EnableAutoCreate();
  GetApplication()->MakeWindow(APInvoice);
  APInvoice->Show(SW_SHOW);
}


