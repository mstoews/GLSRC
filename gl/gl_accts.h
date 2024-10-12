#ifndef __GL_ACCTS_H
#define __GL_ACCTS_H

#define ID_SEARCH 2000
#define ID_FOREIGN 2001
#include "gl_acctl.h"

_CLASSDEF(TGLAcctsAdd)
class TGLAcctsAdd : public TMainDialog
{
 public:

 BOOL bAcctList;
 PTglaccts glaccts;
 PTgldept  gldept;
 TAccountList *GLACCOUNT;
 FARPROC lpfnSetPic;

 TGLAcctsAdd(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TMainDialog (AParent,AName){};
 TGLAcctsAdd(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog (AParent,AName){};
 virtual void SetupWindow();
 virtual void BS       (RTMessage Msg) = [ID_FIRST + ID_BS];
 virtual void IS       (RTMessage Msg) = [ID_FIRST + ID_IS];
 virtual void RE       (RTMessage Msg) = [ID_FIRST + ID_RE];
 virtual void Search   (RTMessage Msg) = [ID_FIRST + ID_GLINQ];
 virtual void Seek     (RTMessage Msg) = [CM_FIRST + ID_GLINQ];
 virtual void GetSeek  (RTMessage Msg) = [ID_FIRST + ID_SEEK];
 virtual void SetData  (RTMessage Msg) = [ID_FIRST + ID_SETDATA];
 virtual void Ok       (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel   (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Update   (RTMessage Msg) = [CM_FIRST + ID_ACCTLIST];
 virtual void Delete   (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 virtual void Modify(void);
 virtual void ShowForeign (RTMessage Msg) = [ID_FIRST + ID_FOREIGN];
 char *Type();
};

_CLASSDEF (TGLAcctDelete)
class TGLAcctsDelete: public TGLAcctsAdd
{
 public:
 TGLAcctsDelete(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TGLAcctsAdd(AParent,AName){};
 TGLAcctsDelete(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TGLAcctsAdd(AParent,AName){};
 virtual void SetupWindow();
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
};

#endif