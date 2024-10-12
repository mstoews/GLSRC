#ifndef __GL_UPD
#define __GL_UPD
_CLASSDEF(TGLARUpdate)
class TGLARUpdate : public TMainDialog
{
 public:
 Data4    *glaccts;
 Tglbtchd *glbtchd;
 Tglbtch  *glbtch;
 Tarjsum  *arjsum;
 Tglsum   *glsum;
 int      period;
 Field4 *descript;

 TGLARUpdate(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TMainDialog(AParent,AName){};
 TGLARUpdate(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName){};

 int  GetNextSlipNumber();
 void CreateSlipDetailEntry(Field4 *account,Field4 *dept,int iSlipNumber,int Entryno,double Debit,double Credit);
 void UpdateGLBatch(int iSlipNumber);
 void UpdateApBatch();
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
};

#endif
