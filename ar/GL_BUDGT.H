_CLASSDEF(TGLAcctBudget)
class TGLAcctBudget : public TDialog
{
 public:
 Tglaccts *glaccts; 
 Tgldept  *gldept;


 TGLAcctBudget(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TDialog(AParent,AName){};
 virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
 virtual void WMRButtonDown  (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
 virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
 virtual void Next           (RTMessage Msg) = [ID_FIRST + ID_NEXTONE];
 virtual void Previous       (RTMessage Msg) = [ID_FIRST + ID_PREVIOSONE];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 void GetBudgetRecord(HWND HWindow);
};
