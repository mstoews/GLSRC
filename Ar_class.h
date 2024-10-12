#ifndef __AR_CLASS_H
#define __AR_CLASS_H
_CLASSDEF (TARDistList)
class TARDistList : public TDialog
{
 public:
  SS_CELLTYPE 	CellType;
  HWND SHWindow;
  Tarbtchd *arbtchd;
  TARDistList(PTWindowsObject AParent, int ATitle,Tarbtchd *bdp):TDialog(AParent,ATitle){
	arbtchd = bdp;
  };
  virtual void SetupWindow();
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual void SetListSize(int Rows);
  virtual void SetFields(UINT ID,Field4 *field);
  virtual void SetFields(UINT ID,char  *field);
  virtual void SetFields(UINT ID,long   field);
  virtual void SetFields(UINT ID,double field);
};
#endif
