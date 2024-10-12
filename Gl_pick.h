#ifndef __PICK_H
#define __PICK_H
#include "gl_dlg.h"
_CLASSDEF(TPickList)
class TPickList : public TMainDialog
{
 public:
 TPickList(PTWindowsObject AParent, LPSTR AName):
  TMainDialog (AParent, AName) {};
 TPickList(PTWindowsObject AParent, int AName):
  TMainDialog (AParent, AName) {};

 virtual void SetListSize(int Rows);
};
#endif
