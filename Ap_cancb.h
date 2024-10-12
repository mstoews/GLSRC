#include "gl_dlg.h"

_CLASSDEF(TAPCancelBatch)
class TAPCancelBatch : public TMainDialog
{
 public:
 Code4 *cb;
 Tapbtch  apbtch;
 Tapbtchd  apbtchd;
 Tapcash  apcash;
 Tapcashd  apcashd;

 TAPCancelBatch(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName)
 {
  cb = &((TAcctModule *)GetApplication())->cb;
 };
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL]{cb->closeAll(); ShutDownWindow();};
 virtual void HandleListBox  (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
};



_CLASSDEF(TAPCancelCash)
class TAPCancelCash : public TAPCancelBatch
{
 public:

 TAPCancelCash(PTWindowsObject AParent, int AName, PTModule AModule = NULL):TAPCancelBatch(AParent,AName)
 {
 };
 virtual void SetupWindow();
 virtual void HandleListBox  (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
};