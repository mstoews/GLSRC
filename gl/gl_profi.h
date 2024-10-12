

_CLASSDEF(TGLProfile)
class TGLProfile : public TMainDialog
{
 public:
 char *Start;
 char *End;
 char *Description;
 TGLProfile(PTWindowsObject AParent, LPSTR ATitle):TMainDialog(AParent,ATitle){};
 TGLProfile(PTWindowsObject AParent, int ATitle):TMainDialog(AParent,ATitle){};
 virtual void SetupWindow();
 virtual void Ok              (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel          (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void SetPeriodDates  (RTMessage Msg) = [ID_FIRST + 4000];
 virtual void GetCalc         (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
 virtual void SetAccounts     (RTMessage Msg) = [ID_FIRST + 1125];
 virtual void WithHolding     (RTMessage Msg) = [ID_FIRST + 4026];
 void GetWithHolding();
 void SetWithHolding();
};

_CLASSDEF(TGLSetPeriodDates)
class TGLSetPeriodDates : public TMainDialog
{
 public:
 char *Start;
 char *End;
 char *Description;

 TGLSetPeriodDates(PTWindowsObject AParent, LPSTR ATitle): TMainDialog(AParent,ATitle){};
 TGLSetPeriodDates(PTWindowsObject AParent, int ATitle): TMainDialog(AParent,ATitle){};
 virtual void SetupWindow();
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void ResetDepreciation (RTMessage Msg) = [ID_FIRST + ID_FA];
};

