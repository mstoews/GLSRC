

_CLASSDEF(TGLProfile)
class TGLProfile : public TDialog
{
 public:
 PTprofile profile;
 TGLProfile(PTWindowsObject AParent, LPSTR ATitle):TDialog(AParent,ATitle){};
 virtual void WMInitDialog    (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
 virtual void Ok              (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel          (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void SetPeriodDates  (RTMessage Msg) = [ID_FIRST + ID_SETPERIOD];
};

_CLASSDEF(TGLSetPeriodDates)
class TGLSetPeriodDates : public TDialog
{
 public:
 PTprofile profile;
 TGLSetPeriodDates(PTWindowsObject AParent, LPSTR ATitle, PTprofile p): TDialog(AParent,ATitle){profile=p;};
 virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
};

