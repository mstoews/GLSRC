#include "gl_pick.h"
_CLASSDEF(TAPVendorList)
class TAPVendorList : public TPickList
{
 public:
 PTListArray ListArray;
 PTarcust  arcust;
 Field4 *vendorno;
 Field4 *vdname;
 TAPVendorList(PTWindowsObject AParent, LPSTR AName,PTarcust vd):
  TPickList (AParent, AName) {arcust=vd; ListArray = new TListArray(1000, 0, 50);};
 ~TAPVendorList();
 virtual void SetupWindow();
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetParentField(int ID,Field4 *field);
 virtual void SetList();
};
