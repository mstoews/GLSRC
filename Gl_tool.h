#ifndef __GL_TOOL_H
#define __GL_TOOL_H
#define ID_PAYABLES 2210

_CLASSDEF (TToolBar)
class TToolBar : public TDialog
{
  public:
  int iSpace;
  int iWidth;
  int iHeight;
  TToolBar(PTWindowsObject AParent,LPSTR ATitle):TDialog(AParent,ATitle){};
  TToolBar(PTWindowsObject AParent,int ATitle):TDialog(AParent,ATitle){};

  virtual void SetupWindow();
  virtual void WMPaint      (RTMessage Msg) = [WM_FIRST + WM_PAINT];
  virtual void GL           (RTMessage Msg) = [ID_FIRST + ID_GL];
  virtual void GLAdd        (RTMessage Msg) = [ID_FIRST + ID_GLADD];
  virtual void GLBatch      (RTMessage Msg) = [ID_FIRST + ID_GLBATCH];
  virtual void APBatch      (RTMessage Msg) = [ID_FIRST + ID_APBATCH];
  virtual void ARBatch      (RTMessage Msg) = [ID_FIRST + ID_ARBATCHENT];
  virtual void APCash       (RTMessage Msg) = [ID_FIRST + ID_CASH];

  virtual void GLPost       (RTMessage Msg) = [ID_FIRST + ID_GLPOST];
  virtual void GLInquiry    (RTMessage Msg) = [ID_FIRST + ID_GLINQ];
  virtual void GLQuit       (RTMessage Msg) = [ID_FIRST + ID_CANCEL];
  virtual void GLPrint      (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual void GLOpen       (RTMessage Msg) = [ID_FIRST + ID_OPEN];
  virtual void GLSave       (RTMessage Msg) = [ID_FIRST + ID_SAVE];
  virtual void GLNew        (RTMessage Msg) = [ID_FIRST + ID_NEW];
  virtual void Cut          (RTMessage Msg) = [ID_FIRST + ID_CUT];
  virtual void Copy         (RTMessage Msg) = [ID_FIRST + ID_COPY];
  virtual void Paste        (RTMessage Msg) = [ID_FIRST + ID_PASTE];
  virtual void Border       (RTMessage Msg) = [ID_FIRST + ID_BORDER];
  virtual void Number       (RTMessage Msg) = [ID_FIRST + ID_NUMBER];
  virtual void Grids        (RTMessage Msg) = [ID_FIRST + ID_GRIDS];
  virtual void Font         (RTMessage Msg) = [ID_FIRST + ID_FONT];
  virtual void Alignment    (RTMessage Msg) = [ID_FIRST + ID_ALIGNMENT];

  virtual void Print        (RTMessage Msg) =[ID_FIRST + ID_PRINT_REPORT];
  virtual void NextPage     (RTMessage Msg) =[ID_FIRST + ID_NEXT_PAGE];
  virtual void PrevPage     (RTMessage Msg) =[ID_FIRST + ID_PREV_PAGE];
  virtual void LastPage     (RTMessage Msg) =[ID_FIRST + ID_LAST_PAGE];
  virtual void FirstPage    (RTMessage Msg) =[ID_FIRST + ID_FIRST_PAGE];
  virtual void Export       (RTMessage Msg) =[ID_FIRST + ID_EXPORT];

  DWORD SetPic(int ID,double iPosition,BOOL bShow);
  void Mssg( int ID);
};

#endif
