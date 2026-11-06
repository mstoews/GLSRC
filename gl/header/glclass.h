#ifndef __GLCLASS_H
#define __GLCLASS_H
#include "gl.h"
#include "gl_lista.h"

_CLASSDEF (TWorksheet)
class TWorksheet;


_CLASSDEF(TGLClient)
class TGLClient : public TMDIClient
{
 public:
  TGLClient(PTMDIFrame AParent,PTModule AModule = NULL):TMDIClient(AParent){};
  virtual void WMRButtonDown(RTMessage  Msg) = [WM_FIRST + WM_RBUTTONDOWN];
};

_CLASSDEF (TStatBar)
class TStatBar : public TDialog
{
 public:
  TStatBar(PTWindowsObject AParent,LPSTR AName): TDialog(AParent,AName){};
  virtual void WMPaint(TMessage& Msg)       = [WM_FIRST + WM_PAINT];
  virtual void WMInitDialog (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
};

BOOL FAR PASCAL DistListPrintProc(HDC hDC,short dCommand,short nPageNum,long lAppData);
#endif
