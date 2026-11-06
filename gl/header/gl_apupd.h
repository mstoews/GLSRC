#ifndef __GL_APUPD_H
#define __GL_APUPD_H
_CLASSDEF(TGLAPUpdate)
class TGLAPUpdate : public TMainDialog
{
 public:
 
 Data4 *glaccts;
 Tprofile *profile;
 Tglbtchd *glbtchd;
 Tglbtch  *glbtch;
 Tapjsum  *apjsum;
 Tarjsum  *arjsum;
 Tglsum   *glsum;
 int      period;
 Field4   *descript;

 TGLAPUpdate(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TMainDialog(AParent,AName){};
 TGLAPUpdate(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName){};

 int  GetNextSlipNumber();
 void CreateSlipDetailEntry(Field4 *account,Field4 *dept,int iSlipNumber,int Entryno,double Debit,double Credit);
 void UpdateGLBatch(int iSlipNumber);
 void UpdateApBatch();
 void UpdateArBatch();
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK]; 
 virtual void APCheck             (RTMessage Msg) = [ID_FIRST + 1001];
 virtual void ARCheck	     (RTMessage Msg) = [ID_FIRST + 1002];
 virtual void ARUpdate();
 virtual void APUpdate();
};

#endif
