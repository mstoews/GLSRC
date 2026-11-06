#include "db_ardst.h"
#include "db_cash.h"
_CLASSDEF(TAPPostBatch)
class TAPPostBatch : public TMainDialog
{
 public:
 PTapctrl  apctrl;
 PTapbtch  apbtch;
 PTapbtchd apbtchd;
 PTglaccts glaccts;
 PTapdist  apdist;
 PTgldist  gldist;
 double dAPControl;
 PTListArray ListArray;
 TAPPostBatch(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TMainDialog(AParent,AName)
 {
  ListArray = new TListArray(1500, 0, 50);
 };
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 void CreateGLSlip(void);
 void APAccountUpdate(void);
 ~TAPPostBatch(){delete ListArray;};
};

_CLASSDEF(TAPCashPost)
class TAPCashPost : public TAPPostBatch
{
 public:
 PTapcash apcash;
 PTapcashd apcashd;
 TAPCashPost(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TAPPostBatch(AParent,AName)
 {
  ListArray = new TListArray(1500, 0, 50);
 };
 virtual void SetupWindow();
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 void CreateGLSlip(void);
};

_CLASSDEF(TAPAdjPost)
class TAPAdjPost : public TAPPostBatch
{
 public:
 TAPAdjPost(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TAPPostBatch(AParent,AName)
 {
  ListArray = new TListArray(1500, 0, 50);
 };
};