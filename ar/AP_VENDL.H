_CLASSDEF(TAPVendorList)
class TAPVendorList : public TDialog
{
 public:
 PTListArray ListArray;
 PTapvendor  apvendor;
 Field4 *vendorno;
 Field4 *vdname;
 TAPVendorList(PTWindowsObject AParent, LPSTR AName,PTapvendor vd):
  TDialog (AParent, AName) {apvendor=vd; ListArray = new TListArray(1000, 0, 50);};
 ~TAPVendorList();
 virtual void SetupWindow();
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetField(int ID,Field4 *field);
};
