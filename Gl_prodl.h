#ifndef __GL_PRODL_H
#define __GL_PRODL_H
#include "tlist_w.h"
_CLASSDEF(TGLProdList)
class TGLProdList : public TWindow
{
 public:
 PTprods glprod;
 PTListWindow List;
 TGLProdList(PTWindowsObject AParent, LPSTR AName,PTprods pd):TWindow (AParent,AName)
 {
  glprod = pd;
  List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
  Attr.Style |= WS_THICKFRAME;
 };
 ~TGLProdList();
 virtual void SetupWindow();
 virtual void HandleList (RTMessage Msg) =  [WM_FIRST + SSM_DBLCLK];
 virtual void WMSize     (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual LPSTR GetClassName(){ return "GL_ITEMM"; }
 void SetDetailWnd();
};
#endif