#ifndef __AR_CANCB_H
#define __AR_CANCB_H

_CLASSDEF(TARCancelBatch)
class TARCancelBatch : public TMainDialog
{
 public:
 Data4 arbtch;
 Data4 arbtchd;
 TARCancelBatch(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName){};
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL]{cb->closeAll(); ShutDownWindow();};
 virtual void HandleListBox  (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
};
#endif
