extern Code4 cb;
_CLASSDEF(TAPCancelBatch)
class TAPCancelBatch : public TMainDialog
{
 public:
 Data4 arbtch;
 Data4 arbtchd;
 TAPCancelBatch(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TMainDialog(AParent,AName){};
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL]{cb.closeAll(); ShutDownWindow();};
 virtual void HandleListBox  (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
};

