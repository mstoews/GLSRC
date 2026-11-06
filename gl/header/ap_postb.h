#include "db_jsum.h"
#include "db_apdst.h"
#include "db_cash.h"
#include "db_accts.h"
_CLASSDEF(TAPPostBatch)
class TAPPostBatch : public TMainDialog
{
 public:
 PTapctrl  apctrl;
 PTapbtch  apbtch;
 PTapbtchd apbtchd;
 PTglaccts glaccts;
 PTapdist  apdist;
 PTapjsum  apjsum;
 double dAPControl;
 PTListArray ListArray;
 TAPPostBatch(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName)
 {
  ListArray = new TListArray(1500, 0, 50);
 };
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void HandleListBox  (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 void AppendAPDist(char *szInvoice);
 void AppendGLDist(PTapbtch apbtch);
 void AppendAPSummary(long lBatchno);
 void CreateGLSlip(void);
 void APAccountUpdate(void);
 BOOL CheckAPBalance(long lBatchno);
 ~TAPPostBatch(){delete ListArray;};
};

_CLASSDEF(TAPCashPost)
class TAPCashPost : public TAPPostBatch
{
 public:
 PTapcash apcash;
 PTapcashd apcashd;
 TAPCashPost(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TAPPostBatch(AParent,AName)
 {
  ListArray = new TListArray(1500, 0, 50);
 };
 virtual void SetupWindow();
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void HandleListBox  (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 void CreateGLSlip(void);
 BOOL CheckCashBalance(long lBatchno);
 void AppendAPSummary(long lBatchno);
};

_CLASSDEF(TAPAdjPost)
class TAPAdjPost : public TAPPostBatch
{
 public:
 TAPAdjPost(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TAPPostBatch(AParent,AName)
 {
  ListArray = new TListArray(1500, 0, 50);
 };
};