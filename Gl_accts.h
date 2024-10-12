#ifndef __GL_ACCTS_H
#define __GL_ACCTS_H
#include "db_dept.h"
#define ID_FOREIGN 2001
#define  ID_SEEK   3005


_CLASSDEF(TGLAcctsAdd)
class TGLAcctsAdd : public TMainDialog
{
 public:

 BOOL bAcctList;
 Tglaccts  glaccts;
 Tgldept  gldept;
 TAccountList *GLACCOUNT;
 TGLAcctsAdd(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TMainDialog (AParent,AName)
 {
	GLACCOUNT = NULL;
 };
 TGLAcctsAdd(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog (AParent,AName)
 {
  GLACCOUNT = NULL;
 };
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
 virtual void Modify   (RTMessage Msg) = [ID_FIRST + ID_MODIFY];
 virtual void ShowForeign (RTMessage Msg) = [ID_FIRST + ID_FOREIGN];
 virtual void New      (RTMessage Msg) = [ID_FIRST + ID_NEW];
 virtual void Details  (RTMessage Msg) = [ID_FIRST + ID_ACCT];
 void UpdateReport(Field4 *account,Field4 *dept);
 void SetCategories();
 ~TGLAcctsAdd()
 {
	if(GLACCOUNT != NULL)
	 delete GLACCOUNT;
 }
 LPSTR Type();
};


#endif