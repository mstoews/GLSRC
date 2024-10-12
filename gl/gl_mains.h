_CLASSDEF(TGLMainSpecs)
class TGLMainSpecs : public TDialog
{
 public:
// Tglmains *glmains;

 TGLMainSpecs(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TDialog(AParent,AName){};
 virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
};
