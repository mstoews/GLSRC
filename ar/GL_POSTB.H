_CLASSDEF(TGLPostBatch)
class TGLPostBatch : public TDialog
{
 public:
 PTglbtch  glbtch;
 PTglbtchd glbtchd;
 PTglaccts glaccts;
 PTgldist  gldist;
 PTListArray ListArray;
 TGLPostBatch(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TDialog(AParent,AName)
 {
  ListArray = new TListArray(1500, 0, 50);
 };
 virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 ~TGLPostBatch(){delete ListArray;};
};
