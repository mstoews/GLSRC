#ifndef __GL_BUDGT_H
#define __GL_BUDGT_H
_CLASSDEF(TGLBudget)
class TGLBudget : public TGLAcctsAdd
{
 public:
 SS_CELLTYPE 	CellType;
 LOGFONT 	lfFont;
 HFONT   	hFont;
 HWND           SHWindow;
 TGLBudget(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TGLAcctsAdd (AParent,AName){};
 TGLBudget(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TGLAcctsAdd (AParent,AName){};
 virtual void SetupWindow();
 virtual void Ok       (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Update   (RTMessage Msg) = [CM_FIRST + ID_ACCTLIST];
};
#endif
