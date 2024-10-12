#ifndef __GL_DRCOD_H
#define __GL_DRCOD_H

#include "gl_drcdl.h"


_CLASSDEF(TGLDrCodes)
class TGLDrCodes : public TMainDialog
{
 public: 
 PTgldrcodes gldrcodes;  // Medical Representative database.
 PTGLDrCodeList DRCODE;
 BOOL bDrCodes;

 TGLDrCodes(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TMainDialog (AParent,AName){};
 TGLDrCodes(PTWindowsObject AParent, int ResourceId, PTModule AModule = NULL): TMainDialog (AParent,ResourceId){};
 virtual void SetupWindow();
 virtual void SetData  (RTMessage Msg) = [ID_FIRST + ID_SETDATA];
 virtual void Ok       (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel   (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Update   (RTMessage Msg) = [CM_FIRST + ID_ACCTLIST];
 virtual void Next     (RTMessage Msg) = [ID_FIRST + ID_NEXT];
 virtual void Previous (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
 virtual void Top      (RTMessage Msg) = [ID_FIRST + ID_TOP];
 virtual void Bottom   (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
 virtual void RepsList (RTMessage Msg) = [ID_FIRST + ID_SLIPLIST];
 virtual void New        (RTMessage Msg) = [ID_FIRST + ID_NEW];
 virtual void Current    (RTMessage Msg) = [CM_FIRST + CM_UPDATE]
    {SetData(Msg);}
 virtual void Delete   (RTMessage Msg) = [ID_FIRST + ID_DELETE];
 BOOL TestDatabase();
 BOOL dCheckField(int ID);
 BOOL szCheckField(int ID);
};
#endif