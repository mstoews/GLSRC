#ifndef _AR_DIST_H
#define _AR_DIST_H
_CLASSDEF (TARDistribution)
class TARDistribution : public TDialog
{
 public:
  SS_CELLTYPE 	CellType;
  HWND SHWindow;
  TARDistribution(PTWindowsObject AParent, int ATitle):TDialog(AParent,ATitle){};
  virtual void SetupWindow();
};
#endif