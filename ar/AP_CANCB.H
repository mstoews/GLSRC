extern Code4 cb;
_CLASSDEF(TAPCancelBatch)
class TAPCancelBatch : public TDialog
{
 public:
 PTapbtch  apbtch;
 PTapbtchd apbtchd;
 PTglaccts glaccts;
 PTapdist  apdist;
 PTListArray ListArray;
 TAPCancelBatch(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TDialog(AParent,AName)
 {
  ListArray = new TListArray(1500, 0, 50);
 };
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL]{cb.closeAll(); ShutDownWindow();};
 virtual void HandleListBox  (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 ~TAPCancelBatch(){delete ListArray;};
};

_CLASSDEF(TAPCancelAdj)
class TAPCancelAdj : public TAPCancelBatch
{
 public:

 TAPCancelAdj(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TAPCancelBatch(AParent,AName)
 {
  ListArray = new TListArray(1500, 0, 50);
  SetCaption("Cancel Accounts Payable Adjustment Journal");
 };
 virtual void SetupWindow();
};


_CLASSDEF(TAPCancelCash)
class TAPCancelCash : public TAPCancelBatch
{
 public:

 TAPCancelCash(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL):TAPCancelBatch(AParent,AName)
 {
  ListArray = new TListArray(1500, 0, 50);
 };
 virtual void SetupWindow();
};