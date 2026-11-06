#include "gl_dlg.h"
extern Code4 cb;
_CLASSDEF(TAPCancelBatch)
class TAPCancelBatch : public TMainDialog
{
 public:
 Data4  *apbtch;
 Data4  *apbtchd;
 Data4  *apcash;
 Data4  *apcashd;

 TAPCancelBatch(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName)
 {

 };
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL]{cb.closeAll(); ShutDownWindow();};
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