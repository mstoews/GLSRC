#ifndef __PICK_H
#define __PICK_H
_CLASSDEF(TPickList)
class TPickList : public TMainDialog
{
 public:
 TPickList(PTWindowsObject AParent, LPSTR AName):
  TMainDialog (AParent, AName) {};
 virtual void SetListSize(int Rows);
};
#endif
