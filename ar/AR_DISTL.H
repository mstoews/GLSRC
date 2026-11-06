#ifndef _AR_DIST_H
#define _AR_DIST_H

_CLASSDEF (TARDistList)
class TARDistList : public TDialog
{
 public:
  SS_CELLTYPE 	CellType;
  HWND SHWindow;
  TARDistList(PTWindowsObject AParent, LPSTR ATitle):TDialog(AParent,ATitle){};
  virtual void SetupWindow();
  virtual void SetListSize(int Rows);
};
#endif