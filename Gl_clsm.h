#ifndef __CLOSEM
#define __CLOSEM

_CLASSDEF(Tglbtch)
class Tglbtch : public Data4
{
  public:
  Field4 *mode;
  Tglbtch(Code4 *cb,LPSTR name=0):Data4(cb,"gl_btch")
  {
   mode = (Field4 *) new Field4(*(Data4 *)this,"mode");
  }  
  BOOL Open();
};


_CLASSDEF(TGLCloseMonth)
class TGLCloseMonth : public TDialog
{
 public:
 Data4* glaccts; 
 Data4* gldept;
 Data4* glbtchd;
 Tglbtch* glbtch;
 Tgldist* gldist;
 Tgldistb* gldistb;
 char  *NewPeriod;
 char  *Period;
 short  PeriodNo;
 double NewPeriodn;
 Field4* batchnod;


 TGLCloseMonth(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TDialog(AParent,AName) {};
 TGLCloseMonth(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TDialog(AParent,AName) {};

 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 char *GetPeriod(char *szPeriod);
 int  GetPeriodn(char *szPeriod);
 void CreateDistBackup(void);
 void CopyINI();
};

#endif
