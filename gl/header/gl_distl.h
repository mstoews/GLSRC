#ifndef __GL_DISTL_H
#define __GL_DISTL_H
class Tglbtchd;
/*
_CLASSDEF (TGLDistList)
class TGLDistList : public TDialog
{
 public:
  SS_CELLTYPE 	CellType;
  LOGFONT 	lfFont;
  HFONT   	hFont;
  HWND SHWindow;
  Tglbtchd *glbtd;
  TGLDistList(PTWindowsObject AParent, int ATitle,Tglbtchd *bdp):TDialog(AParent,ATitle)
  {
  glbtd = bdp;
  };
  TGLDistList(PTWindowsObject AParent, int ATitle):TDialog(AParent,ATitle){};
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual void SetupWindow();
  void SetListSize(int Rows);
};

_CLASSDEF(TGLModifyDistList)
class TGLModifyDistList: public TGLDistList
{
  public:
	TGLModifyDistList(PTWindowsObject AParent, int ATitle,Tglbtchd *bdp):
	TGLDistList(AParent,ATitle,bdp){};
	TGLModifyDistList(PTWindowsObject AParent, int ATitle):
	TGLDistList(AParent,ATitle){};
	virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
	void SetField(UINT ID,Field4 *field);
	void SetField(UINT ID,int iField);
	void SetField(UINT ID,LPCSTR field);
	void SetField(UINT ID,double field);
};


class Tarbtchd;

_CLASSDEF (TARDistList)
class TARDistList : public TGLModifyDistList
{
  public :
  Tarbtchd *arbtchd;
  TARDistList(PTWindowsObject AParent, int ATitle, Tarbtchd *ar):
  TGLModifyDistList(AParent,ATitle)
  {
	arbtchd = ar;
  };
  virtual void SetupWindow();
  virtual void SelectCell (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  virtual void Print      (RTMessage Msg) = [ID_FIRST + ID_PRINT];
};
*/
#endif