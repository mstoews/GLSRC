#ifndef __AP_VERIF_H
#define __AP_VERIF_H

class TVerifyCheques: public TMainDialog
{
 public:
 Tapdist  apdist;
 TVerifyCheques(PTWindowsObject AParent,int ATitle):TMainDialog(AParent,ATitle){}
 virtual void SetupWindow();
 virtual void Verify         (RTMessage Msg) = [ID_FIRST + 1031];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 void UpdateCheque(double dAmount,int dChequeNo);
 void AddCheque(double dChequeNo);
 void SetChequeNumbers();
 void UpdateAPVendor(double iStartingReq);
};
#endif
