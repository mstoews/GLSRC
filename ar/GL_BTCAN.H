_CLASSDEF(TGLCancelBatch)
class TGLCancelBatch : public TDialog
{
 public:
 Tglbtch *glbtch;

 TGLCancelBatch(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TDialog(AParent,AName){};
 virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
 virtual void WMRButtonDown  (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
 virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
};

