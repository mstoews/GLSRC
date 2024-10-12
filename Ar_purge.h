
_CLASSDEF(TAPPurgeBatch)
class TARPurgeBatch : public TMainDialog
{
 public:
 PTardist  ardist;
 TARPurgeBatch(PTWindowsObject AParent, int AName, PTModule AModule = NULL)
 : TMainDialog(AParent,AName){};
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL]{cb->closeAll(); ShutDownWindow();};
 virtual void SelectItem     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
};

