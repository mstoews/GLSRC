#ifndef __AR_POSTB_B
#define __AR_POSTB_B
#include "db_arjsm.h"
#include "db_arbtc.h"
#include "db_ardst.h"
#include "db_cash.h"
#include "ar_ctrl.h"
_CLASSDEF(TARPostBatch)
class TARPostBatch : public TMainDialog
{
 public:
 PTarbtch  arbtch;
 PTarbtchd arbtchd;
 PTglaccts glaccts;
 PTardist  ardist;
 PTarjsum  arjsum;
 PTarctrl  arctrl;
 double dAPControl;
 PTListArray ListArray;
 TARPostBatch(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName)
 {
  ListArray = new TListArray(1500, 0, 50);
 };
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void HandleListBox  (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 void AppendAPDist(char *szInvoice);
 void AppendGLDist(PTarbtch arbtch);
 void AppendAPSummary(long lBatchno);
 void CreateGLSlip(void);
 void APAccountUpdate(void);
 BOOL CheckARBalance(long lBatchno);
 ~TARPostBatch(){delete ListArray;};
};

#endif
