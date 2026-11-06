#ifndef __CLOSEM
#define __CLOSEM
_CLASSDEF(TGLCloseMonth)
class TGLCloseMonth : public TDialog
{
 public:
 Tglaccts *glaccts; 
 Tgldept  *gldept;
 Tprofile *profile;
 Tglbtchd *glbtchd;
 Tglbtch  *glbtch;
 Tgldist  *gldist;
 char  *NewPeriod;
 char  *Period;
 short  PeriodNo;
 double NewPeriodn;


 TGLCloseMonth(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TDialog(AParent,AName)
  {
   NewPeriod = NULL;
   Period    = NULL;
   glaccts   = NULL;
   gldept    = NULL;
   profile   = NULL;
   glbtchd   = NULL;
   glbtch    = NULL;
   gldist    = NULL;

  };

 virtual void SetupWindow(){};
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 ~TGLCloseMonth()
 {
  delete NewPeriod[];
  delete Period[];
  };
};

#endif